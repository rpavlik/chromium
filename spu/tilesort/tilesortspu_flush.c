#include "tilesortspu.h"
#include "cr_pack.h"
#include "cr_net.h"
#include "cr_mem.h"
#include "cr_protocol.h"
#include "cr_error.h"
#include "cr_packfunctions.h"
#include <memory.h>

static TileSortSPUServer *state_server = NULL;

static CRMessageOpcodes *__applySendBufferHeader( CRPackBuffer *pack, unsigned int *len )
{
	int num_opcodes;
	CRMessageOpcodes *hdr;

	CRASSERT( pack->opcode_current < pack->opcode_start );
	CRASSERT( pack->opcode_current >= pack->opcode_end );
	CRASSERT( pack->data_current > pack->data_start );
	CRASSERT( pack->data_current <= pack->data_end );

	num_opcodes = pack->opcode_start - pack->opcode_current;
	hdr = (CRMessageOpcodes *) 
		( pack->data_start - ( ( num_opcodes + 3 ) & ~0x3 ) - sizeof(*hdr) );

	CRASSERT( (void *) hdr >= pack->pack );

	hdr->type       = CR_MESSAGE_OPCODES;
	hdr->senderId   = (unsigned int) ~0;  /* to be initialized by caller */
	hdr->numOpcodes = num_opcodes;

	*len = pack->data_current - (unsigned char *) hdr;

	return hdr;
}

static void __sendServerBuffer( TileSortSPUServer *server )
{
	CRPackBuffer *pack = &server->pack;
	unsigned int len;
	CRMessageOpcodes *hdr;

	if ( pack->opcode_current == pack->opcode_start )
		return;

	hdr = __applySendBufferHeader( pack, &len );
	hdr->senderId = server->net.conn->sender_id;

	crNetSend( server->net.conn, &pack->pack, hdr, len );
	crPackInitBuffer( pack, crNetAlloc( server->net.conn ), pack->size, END_FLUFF );
}

static void __appendBuffer( CRPackBuffer *src )
{
	int num_data = src->data_current - src->data_start;

	if ( cr_packer_globals.buffer.data_current + num_data > 
			 cr_packer_globals.buffer.data_end )
	{
		// No room to append -- send now

		__sendServerBuffer( state_server );
	}

	crPackAppendBuffer( src );
}

void __appendBoundedBuffer( CRPackBuffer *src, GLrecti *bounds )
{
	// 24 is the size of the bounds info packet
	int num_data = src->data_current - src->opcode_current - 1 + 24;

	if ( cr_packer_globals.buffer.data_current + num_data > 
			 cr_packer_globals.buffer.data_end )
	{
		// No room to append -- send now
		__sendServerBuffer( state_server );
	}

	crPackAppendBoundedBuffer( src, bounds );
}

static void __doFlush( CRContext *ctx )
{
	CRMessageOpcodes *big_packet_hdr = NULL;
	unsigned int big_packet_len = 0;
	GLrecti bounds;
	const TileSortBucketInfo * bucket_info;
	int i;

	crDebug( "in __doFlush." );

	if (state_server != NULL)
	{
		// This means that the context differencer had so much state
		// to dump that it overflowed the server's network buffer
		// while doing its difference.  In this case, we just want
		// to get the buffer out the door so the differencer can 
		// keep doing what it's doing.

		crDebug( "Overflowed while doing a context difference!" );
		
		// First, extract the packing state into the server
		crPackGetBuffer( &(state_server->pack) );

		// Now, get the buffer out of here.
		
		__sendServerBuffer( state_server );
		
		// Finally, restore the packing state so we can continue
		// This isn't the same as calling crPackResetPointers,
		// because the state server now has a new pack buffer
		// from the buffer pool.

		crPackSetBuffer( &(state_server->pack) );

		return;
	}

	crDebug( "Looks like it was not a state overflow" );

	// First, test to see if this is a big packet.
	
	if (cr_packer_globals.buffer.size > crNetMTU())
	{
		crDebug( "It was a big packet" );
		big_packet_hdr = __applySendBufferHeader( &(cr_packer_globals.buffer), &big_packet_len );
	}

	// Here's the big part -- call the bucketer!
	// This will also call the "pincher".

	crDebug( "About to bucket the geometry" );
	bucket_info = tilesortspuBucketGeometry();
	bounds = bucket_info->pixelBounds;

	// Okay.  Now, we need to un-hide the bonus space for the extra glEnd packet
	// and try to close off the begin/end if it exists.  This is a pretty
	// serious hack, but we *did* reserve space in the pack buffer just
	// for this purpose.
	
	if ( tilesort_spu.ctx->current.inBeginEnd )
	{
		crDebug( "Closing this Begin/end!!!" );
		cr_packer_globals.buffer.data_end += END_FLUFF;
		crPackEnd();
	}

	// Now, we grab the packing state into the globals, since we're
	// going to start packing into other places like crazy but
	// we need to keep these pointers around, both for
	// doing context differencing, and also to restore them
	// at the end of this function.

	crPackGetBuffer( &(tilesort_spu.geometry_pack) );

	// Now, see if we need to do things to each server

	for ( i = 0 ; i < tilesort_spu.num_servers; i++ )
	{
		// Check to see if this server needs geometry from us.
		if (!(bucket_info->hits & (1 << i)))
		{
			crDebug( "NOT sending to server %d", i );
			continue;
		}
		crDebug( "Sending to server %d", i );

		// Okay, it does.  
		
		state_server = tilesort_spu.servers + i;

		// We're going to do lazy state evaluation now

		crPackSetBuffer( &(state_server->pack) );
		crStateDiffContext( state_server->ctx, ctx );
		crPackGetBuffer( &(state_server->pack) );

		// The state server's buffer now contains the commands 
		// needed to bring it up to date on the state.  All
		// that's left to do is to append the geometry.

		if ( big_packet_hdr != NULL )
		{
			// The packet was big, so send the state commands
			// by themselves, followed by the big packet.

			crDebug( "Doing the big packet send thing" );
			__sendServerBuffer( state_server );
			big_packet_hdr->senderId = state_server->net.conn->sender_id;
			crNetSend( state_server->net.conn, NULL, big_packet_hdr, big_packet_len );
		}
		else
		{
			// Now, we have to decide whether or not to append the geometry buffer
			// as a BOUNDS_INFO packet or just a bag-o-commands.  This is
			// controlled by the apply_viewtransform flag, which basically tells
			// us whether or not any servers are managing multiple tiles.  Should
			// this be done on a per-server basis?
			
			// Now we see why I tucked away the geometry buffer earlier.

			if ( tilesort_spu.apply_viewtransform )
			{
				crDebug( "Appending a bounded buffer" );
				__appendBoundedBuffer( &(tilesort_spu.geometry_pack), &bounds );
			}
			else
			{
				crDebug( "Appending a NON-bounded buffer" );
				__appendBuffer( &(tilesort_spu.geometry_pack) );
			}
		}
	}

	// We're done with the servers.  Wipe the global state_server
	// variable so we know that.  

	state_server = NULL;

	if ( big_packet_hdr != NULL )
	{
		// Throw away the big packet and make a new, smaller one

		crDebug( "Throwing away the big packet" );
		crFree( tilesort_spu.geometry_pack.pack );
	  crPackInitBuffer( &(tilesort_spu.geometry_pack), crAlloc( tilesort_spu.geom_pack_size ), 
			              tilesort_spu.geom_pack_size, END_FLUFF );
	}
	else
	{
		crDebug( "Reverting to the old geometry buffer" );
		crPackSetBuffer( &(tilesort_spu.geometry_pack ) );
		crPackResetPointers( END_FLUFF );
	}

	crError( "Here is where I would do pinching and whatever else goes in __glEndFlush()" );
}

void tilesortspuFlush( void *arg )
{
	CRContext *ctx = (CRContext *) arg;
	CRPackBuffer old_geometry_pack;

	crDebug( "In tilesortspuFlush" );
	crDebug( "BBOX MIN: (%f %f %f)", cr_packer_globals.bounds_min.x, cr_packer_globals.bounds_min.y, cr_packer_globals.bounds_min.z );
	crDebug( "BBOX MAX: (%f %f %f)", cr_packer_globals.bounds_max.x, cr_packer_globals.bounds_max.y, cr_packer_globals.bounds_max.z );

	// If we're not in a begin/end, or if we're splitting them, go ahead and
	// do the flushing.
	
	if ( tilesort_spu.splitBeginEnd || !(ctx->current.inBeginEnd) )
	{
		__doFlush( ctx );
		return;
	}


	// Otherwise, we need to grow the buffer to make room for more stuff.
	// Since we don't want to do this forever, we mark the context as
	// "Flush on end", which will force a flush the next time glEnd() 
	// is called.
	//
	// Note that this can only happen to the geometry buffer, so we 
	// assert that.
	
	crDebug( "-=-=-=- Growing the buffer -=-=-=-" );

	CRASSERT( ctx == tilesort_spu.ctx );

	ctx->current.flushOnEnd = 1;

	// Grab the pack buffer info from the packing library

	crPackGetBuffer( &(old_geometry_pack) );

	crPackInitBuffer( &(tilesort_spu.geometry_pack), crAlloc( tilesort_spu.geometry_pack.size << 1 ), 
			              tilesort_spu.geometry_pack.size << 1, END_FLUFF );
	crPackSetBuffer( &(tilesort_spu.geometry_pack) );
	crPackAppendBuffer(&(old_geometry_pack));
	crFree( old_geometry_pack.pack );
}
