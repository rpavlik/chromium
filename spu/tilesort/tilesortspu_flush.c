#include "tilesortspu.h"
#include "cr_pack.h"
#include "cr_net.h"
#include "cr_mem.h"
#include "cr_protocol.h"
#include "cr_error.h"
#include "cr_packfunctions.h"
#include "cr_applications.h"

#include <memory.h>
#include <math.h>
#include <stdlib.h>

static TileSortSPUServer *state_server = NULL;

#ifdef WINDOWS
extern __declspec( dllimport ) CRPackGlobals cr_packer_globals;
#endif

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

void tilesortspuDebugOpcodes( CRPackBuffer *pack )
{
	unsigned char *tmp;
	for (tmp = pack->opcode_start; tmp > pack->opcode_current; tmp--)
	{
		crDebug( "  %d (0x%p, 0x%p)", *tmp, tmp, pack->opcode_current );
	}
	crDebug( "\n" );
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

	//crWarning( "In __appendBuffer: %d bytes left, packing %d bytes", cr_packer_globals.buffer.data_end - cr_packer_globals.buffer.data_current, num_data );

	if ( cr_packer_globals.buffer.data_current + num_data > 
			 cr_packer_globals.buffer.data_end )
	{
		// No room to append -- send now

		crWarning( "OUT OF ROOM!") ;
		__sendServerBuffer( state_server );
		crPackSetBuffer( &(state_server->pack) );
	}

	crPackAppendBuffer( src );
	//crWarning( "Back from crPackAppendBuffer: 0x%x", cr_packer_globals.buffer.data_current );
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
		crPackSetBuffer( &(state_server->pack) );
	}

	crPackAppendBoundedBuffer( src, bounds );
}

void tilesortspuShipBuffers( void )
{
	int i;
	for (i = 0 ; i < tilesort_spu.num_servers; i++)
	{
		TileSortSPUServer *server = tilesort_spu.servers + i;
		__sendServerBuffer( server );
	}
}

void tilesortspuHuge( CROpcode opcode, void *buf )
{
#if 1
	unsigned int          len;
	unsigned char        *src;
	CRMessageOpcodes *msg;

	/* packet length is indicated by the variable length field, and
	   includes an additional word for the opcode (with alignment) and
	   a header */
	len = ((unsigned int *) buf)[-1] + 4 + sizeof(CRMessageOpcodes);

	/* write the opcode in just before the length */
	((unsigned char *) buf)[-5] = (unsigned char) opcode;

	/* fix up the pointer to the packet to include the length & opcode
       & header */
	src = (unsigned char *) buf - 8 - sizeof(CRMessageOpcodes);

	msg = (CRMessageOpcodes *) src;

	msg->type       = CR_MESSAGE_OPCODES;
	msg->senderId   = state_server->net.conn->sender_id;
	msg->numOpcodes = 1;

	/* the pipeserver's buffer might have data in it, and that should
       go across the wire before this big packet */
	__sendServerBuffer( state_server );
	crNetSend( state_server->net.conn, NULL, src, len );
#else
	crError( "Trying to send a huge packet, which is unimplemented!" );
	(void) opcode;
	(void) buf;
#endif
}


static void __drawBBOX(const TileSortBucketInfo * bucket_info)
{
	
#define DRAW_BBOX_MAX_SERVERS 128
	static int init=0;
	static GLfloat c[DRAW_BBOX_MAX_SERVERS][3];
	unsigned int i;
	GLbitvalue a;
	GLfloat outcolor[3] = {0.0f, 0.0f, 0.0f};
	GLfloat tot;
	GLfloat xmin = bucket_info->objectMin.x;
	GLfloat xmax = bucket_info->objectMax.x;
	GLfloat ymin = bucket_info->objectMin.y;
	GLfloat ymax = bucket_info->objectMax.y;
	GLfloat zmin = bucket_info->objectMin.z;
	GLfloat zmax = bucket_info->objectMax.z;

	if (!init) 
	{
		for (i=0; i<DRAW_BBOX_MAX_SERVERS; i++) 
		{
			c[i][0] = (GLfloat) rand();
			c[i][1] = (GLfloat) rand();
			c[i][2] = (GLfloat) rand();
			tot = (GLfloat) sqrt (c[i][0]*c[i][0] + c[i][1]*c[i][1] + c[i][2]*c[i][2]);
			c[i][0] /= tot;
			c[i][1] /= tot;
			c[i][2] /= tot;
		}
		init = 1;
	}		

	tot = 0.0f;
	for (i=0, a=1; i<DRAW_BBOX_MAX_SERVERS; i++, a <<= 1)
	{
		if (bucket_info->hits & a) 
		{
			outcolor[0] += c[i][0];
			outcolor[1] += c[i][1];
			outcolor[2] += c[i][2];
			tot+=1.0f;
		}
	}
	outcolor[0] /= tot;
	outcolor[1] /= tot;
	outcolor[2] /= tot;

	crPackPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LINE_BIT);
	crPackDisable(GL_TEXTURE_2D);
	crPackDisable(GL_TEXTURE_1D);
	crPackDisable(GL_LIGHTING);
	crPackDisable(GL_BLEND);
	crPackDisable(GL_ALPHA_TEST);
	crPackDisable(GL_DEPTH_TEST);
	crPackDisable(GL_FOG);
	crPackDisable(GL_STENCIL_TEST);
	crPackDisable(GL_SCISSOR_TEST);
	crPackDisable(GL_LOGIC_OP);

	crPackLineWidth(tilesort_spu.bboxLineWidth);
	crPackColor3fv(outcolor);
	crPackBegin(GL_LINE_LOOP);
	crPackVertex3f(xmin, ymin, zmin);
	crPackVertex3f(xmin, ymin, zmax);
	crPackVertex3f(xmin, ymax, zmax);
	crPackVertex3f(xmin, ymax, zmin);
	crPackEnd();
	crPackBegin(GL_LINE_LOOP);
	crPackVertex3f(xmax, ymin, zmin);
	crPackVertex3f(xmax, ymin, zmax);
	crPackVertex3f(xmax, ymax, zmax);
	crPackVertex3f(xmax, ymax, zmin);
	crPackEnd();
	crPackBegin(GL_LINE_LOOP);
	crPackVertex3f(xmin, ymin, zmin);
	crPackVertex3f(xmax, ymin, zmin);
	crPackVertex3f(xmax, ymax, zmin);
	crPackVertex3f(xmin, ymax, zmin);
	crPackEnd();
	crPackBegin(GL_LINE_LOOP);
	crPackVertex3f(xmin, ymin, zmax);
	crPackVertex3f(xmax, ymin, zmax);
	crPackVertex3f(xmax, ymax, zmax);
	crPackVertex3f(xmin, ymax, zmax);
	crPackEnd();

	crPackPopAttrib();
}


static void __doFlush( CRContext *ctx, int broadcast )
{
	CRMessageOpcodes *big_packet_hdr = NULL;
	unsigned int big_packet_len = 0;
	GLrecti bounds;
	const TileSortBucketInfo * bucket_info = NULL;
	int i;

	//crDebug( "in __doFlush (broadcast = %d)", broadcast );

	if (state_server != NULL)
	{
		// This means that the context differencer had so much state
		// to dump that it overflowed the server's network buffer
		// while doing its difference.  In this case, we just want
		// to get the buffer out the door so the differencer can 
		// keep doing what it's doing.

		crDebug( "Overflowed while doing a context difference!" );
		CRASSERT( broadcast == 0 );
		
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

	//crDebug( "Looks like it was not a state overflow" );

	// First, test to see if this is a big packet.
	
	if (cr_packer_globals.buffer.size > tilesort_spu.servers[0].net.conn->mtu )
	{
		crDebug( "It was a big packet!" );
		big_packet_hdr = __applySendBufferHeader( &(cr_packer_globals.buffer), &big_packet_len );
	}

	// Here's the big part -- call the bucketer!

	if (!broadcast)
	{
		//crDebug( "About to bucket the geometry" );
		bucket_info = tilesortspuBucketGeometry();
		bounds = bucket_info->pixelBounds;
		if (tilesort_spu.providedBBOX != CR_OBJECT_BBOX_HINT)
		{
			crPackResetBBOX();
		}
	}
	else
	{
		//crDebug( "Broadcasting the geometry!" );
	}

	// Now, we want to let the state tracker extract the "current" state
	// from the collection of pointers that we have in the geometry 
	// buffer.

	crStateCurrentRecover( &(cr_packer_globals.current) );

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
		if (!broadcast && !(bucket_info->hits & (1 << i)))
		{
			//crDebug( "NOT sending to server %d", i );
			continue;
		}
		//crDebug( "Sending to server %d", i );

		// Okay, it does.  
		
		state_server = tilesort_spu.servers + i;

		// We're going to do lazy state evaluation now

		crPackSetBuffer( &(state_server->pack) );
		if (!broadcast)
		{
			//crDebug( "pack buffer before differencing" );
			//tilesortspuDebugOpcodes( &(cr_packer_globals.buffer) );
			crStateDiffContext( state_server->ctx, ctx );
			if (tilesort_spu.drawBBOX)
			{
				__drawBBOX( bucket_info );
			}
			crPackGetBuffer( &(state_server->pack) );
			//crDebug( "pack buffer after differencing" );
			//tilesortspuDebugOpcodes( &(state_server->pack) );
		}

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
			// controlled by the sendBounds flag, which basically tells
			// us whether or not any servers are managing multiple tiles.  Should
			// this be done on a per-server basis?
			
			// Now we see why I tucked away the geometry buffer earlier.

			if ( tilesort_spu.sendBounds && !broadcast )
			{
				crDebug( "Appending a bounded buffer" );
				__appendBoundedBuffer( &(tilesort_spu.geometry_pack), &bounds );
			}
			else
			{
				//crDebug( "Appending a NON-bounded buffer" );
				//tilesortspuDebugOpcodes( &(tilesort_spu.geometry_pack) );
				//tilesortspuDebugOpcodes( &(cr_packer_globals.buffer) );
				__appendBuffer( &(tilesort_spu.geometry_pack) );
				//tilesortspuDebugOpcodes( &(cr_packer_globals.buffer) );
			}
			crPackGetBuffer( &(state_server->pack) );
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
		//crDebug( "Reverting to the old geometry buffer" );
		crPackSetBuffer( &(tilesort_spu.geometry_pack ) );
		crPackResetPointers( END_FLUFF );
	}

	//crDebug( "Resetting the current vertex count and friends" );

	cr_packer_globals.current.vtx_count = 0;
	cr_packer_globals.current.vtx_count_begin = 0;

	//crDebug( "Setting all the Current pointers to NULL" );
	crPackNullCurrentPointers();

	// crError( "Here is where I would do pinching and whatever else goes in __glEndFlush()" );
}

void tilesortspuBroadcastGeom( void )
{
	__doFlush( tilesort_spu.ctx, 1 );
}

void tilesortspuFlush( void *arg )
{
	CRContext *ctx = (CRContext *) arg;
	CRPackBuffer old_geometry_pack;

	//crDebug( "In tilesortspuFlush" );
	//crDebug( "BBOX MIN: (%f %f %f)", cr_packer_globals.bounds_min.x, cr_packer_globals.bounds_min.y, cr_packer_globals.bounds_min.z );
	//crDebug( "BBOX MAX: (%f %f %f)", cr_packer_globals.bounds_max.x, cr_packer_globals.bounds_max.y, cr_packer_globals.bounds_max.z );

	// If we're not in a begin/end, or if we're splitting them, go ahead and
	// do the flushing.
	
	if ( tilesort_spu.splitBeginEnd || !(ctx->current.inBeginEnd) )
	{
		__doFlush( ctx, 0 );
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

	// The location of the geometry buffer has changed, so we need to
	// tell the state tracker to update its "current" pointers.
	// The "offset" computed here does *not* require that the two
	// buffers be contiguous -- in fact they almost certainly won't be.

	crPackOffsetCurrentPointers( old_geometry_pack.data_current - 
			 										     cr_packer_globals.buffer.data_current );

	crFree( old_geometry_pack.pack );
}
