/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "tilesortspu.h"
#include "cr_pack.h"
#include "cr_net.h"
#include "cr_mem.h"
#include "cr_protocol.h"
#include "cr_error.h"
#include "cr_packfunctions.h"

#include <math.h>
#include <stdlib.h>
#include <float.h>

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

	if (tilesort_spu.swap)
	{
		hdr->header.type= (CRMessageType) SWAP32(CR_MESSAGE_OPCODES);
		hdr->numOpcodes  = SWAP32(num_opcodes);
	}
	else
	{
		hdr->header.type = CR_MESSAGE_OPCODES;
		hdr->numOpcodes  = num_opcodes;
	}

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

void tilesortspuSendServerBuffer( int server_index /*TileSortSPUServer *server*/ )
{
	GET_THREAD(thread);
	CRPackBuffer *pack = &(thread->pack[server_index]);
	CRNetServer *net = &(thread->net[server_index]);
	unsigned int len;
	CRMessageOpcodes *hdr;

	if ( pack->opcode_current == pack->opcode_start )
		return;

	hdr = __applySendBufferHeader( pack, &len );

	if ( pack->holds_BeginEnd && pack->canBarf )
		crNetBarf( net->conn, &pack->pack, hdr, len );
	else
		crNetSend( net->conn, &pack->pack, hdr, len );

	crPackInitBuffer( pack, crNetAlloc( net->conn ), net->conn->buffer_size, net->conn->mtu );
	pack->canBarf = net->conn->Barf ? GL_TRUE : GL_FALSE;
}

static void __appendBuffer( CRPackBuffer *src )
{
	GET_THREAD(thread);

	/*crWarning( "In __appendBuffer: %d bytes left, packing %d bytes", thread->packer->buffer.data_end - thread->packer->buffer.data_current, num_data ); */

	if ( !crPackCanHoldBuffer(src)
		|| thread->packer->buffer.holds_BeginEnd ^ src->holds_BeginEnd)
	{
		/* No room to append -- send now */

		/*crWarning( "OUT OF ROOM!") ; */
		tilesortspuSendServerBuffer( thread->state_server_index );
		crPackSetBuffer( thread->packer, &(thread->pack[thread->state_server_index]) );
		CRASSERT(crPackCanHoldBuffer( src ));
	}

	crPackAppendBuffer( src );
	/*crWarning( "Back from crPackAppendBuffer: 0x%x", thread->packer->buffer.data_current ); */
}

void __appendBoundedBuffer( CRPackBuffer *src, GLrecti *bounds )
{
	GET_THREAD(thread);
	int length = ((src->data_current - src->opcode_current - 1) + 3) & ~3;

	if (length == 0)
	{
		/* nothing to send. */

		return;
	}

	/* 24 is the size of the bounds info packet */
	if ( !crPackCanHoldOpcode( 1, length + 24 )
		|| thread->packer->buffer.holds_BeginEnd ^ src->holds_BeginEnd)
	{
		/* No room to append -- send now */
		tilesortspuSendServerBuffer( thread->state_server_index );
		crPackSetBuffer( thread->packer, &(thread->pack[thread->state_server_index]) );
		CRASSERT(crPackCanHoldOpcode( 1, length + 24 ) || src->holds_BeginEnd );
	}

	crPackAppendBoundedBuffer( src, bounds );
}

void tilesortspuShipBuffers( void )
{
	int i;
	for (i = 0 ; i < tilesort_spu.num_servers; i++)
	{
		tilesortspuSendServerBuffer( i );
	}
}

void tilesortspuHuge( CROpcode opcode, void *buf )
{
	GET_THREAD(thread);
	unsigned int          len;
	unsigned char        *src;
	CRMessageOpcodes *msg;

	if (tilesort_spu.inDrawPixels)
	{
		tilesortspuFlush( thread );
		return;
	}

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

	if (tilesort_spu.swap)
	{
		msg->header.type = (CRMessageType) SWAP32(CR_MESSAGE_OPCODES);
		msg->numOpcodes  = SWAP32(1);
	}
	else
	{
		msg->header.type = CR_MESSAGE_OPCODES;
		msg->numOpcodes  = 1;
	}

	/* the pipeserver's buffer might have data in it, and that should
       go across the wire before this big packet */
	tilesortspuSendServerBuffer( thread->state_server_index );
	crNetSend( thread->net[thread->state_server_index].conn, NULL, src, len );
	crPackFree( buf );
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
#if 0
		if (CHECKDIRTY(bucket_info->hits, a)) 
#endif
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

	if (tilesort_spu.swap)
	{
		crPackPushAttribSWAP(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LINE_BIT);
		crPackDisableSWAP(GL_TEXTURE_2D);
		crPackDisableSWAP(GL_TEXTURE_1D);
		crPackDisableSWAP(GL_LIGHTING);
		crPackDisableSWAP(GL_BLEND);
		crPackDisableSWAP(GL_ALPHA_TEST);
                crPackDisableSWAP(GL_DEPTH_TEST);
		crPackDisableSWAP(GL_FOG);
		crPackDisableSWAP(GL_STENCIL_TEST);
		crPackDisableSWAP(GL_SCISSOR_TEST);
		crPackDisableSWAP(GL_LOGIC_OP);

		crPackLineWidthSWAP(tilesort_spu.bboxLineWidth);
		crPackColor3fvSWAP(outcolor);
		crPackBeginSWAP(GL_LINE_LOOP);
		crPackVertex3fSWAP(xmin, ymin, zmin);
		crPackVertex3fSWAP(xmin, ymin, zmax);
		crPackVertex3fSWAP(xmin, ymax, zmax);
		crPackVertex3fSWAP(xmin, ymax, zmin);
		crPackEndSWAP();
		crPackBeginSWAP(GL_LINE_LOOP);
		crPackVertex3fSWAP(xmax, ymin, zmin);
		crPackVertex3fSWAP(xmax, ymin, zmax);
		crPackVertex3fSWAP(xmax, ymax, zmax);
		crPackVertex3fSWAP(xmax, ymax, zmin);
		crPackEndSWAP();
		crPackBeginSWAP(GL_LINE_LOOP);
		crPackVertex3fSWAP(xmin, ymin, zmin);
		crPackVertex3fSWAP(xmax, ymin, zmin);
		crPackVertex3fSWAP(xmax, ymax, zmin);
		crPackVertex3fSWAP(xmin, ymax, zmin);
		crPackEndSWAP();
		crPackBeginSWAP(GL_LINE_LOOP);
		crPackVertex3fSWAP(xmin, ymin, zmax);
		crPackVertex3fSWAP(xmax, ymin, zmax);
		crPackVertex3fSWAP(xmax, ymax, zmax);
		crPackVertex3fSWAP(xmin, ymax, zmax);
		crPackEndSWAP();

		crPackPopAttribSWAP();
	}
	else
	{
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
}

static void __doFlush( CRContext *ctx, int broadcast, int send_state_anyway )
{
	GET_THREAD(thread);
	CRMessageOpcodes *big_packet_hdr = NULL;
	unsigned int big_packet_len = 0;
	TileSortBucketInfo bucket_info;
	int i;

	/*crDebug( "in __doFlush (broadcast = %d)", broadcast ); */

	if (thread->state_server != NULL)
	{
		/* This means that the context differencer had so much state 
		 * to dump that it overflowed the server's network buffer 
		 * while doing its difference.  In this case, we just want 
		 * to get the buffer out the door so the differencer can 
		 * keep doing what it's doing. */

		/*crDebug( "Overflowed while doing a context difference!" ); */
		CRASSERT( broadcast == 0 );
		
		/* First, extract the packing state into the server */
		crPackGetBuffer( thread->packer, &(thread->pack[thread->state_server_index]) );

		/* Now, get the buffer out of here. */
		
		tilesortspuSendServerBuffer( thread->state_server_index );
		
		/* Finally, restore the packing state so we can continue 
		 * This isn't the same as calling crPackResetPointers, 
		 * because the state server now has a new pack buffer 
		 * from the buffer pool. */

		crPackSetBuffer( thread->packer, &(thread->pack[thread->state_server_index]) );

		return;
	}

	/*crDebug( "Looks like it was not a state overflow" ); */

	/* First, test to see if this is a big packet. */
	
	if (thread->packer->buffer.size > tilesort_spu.buffer_size )
	{
		crDebug( "It was a big packet!" );
		big_packet_hdr = __applySendBufferHeader( &(thread->packer->buffer), &big_packet_len );
	}

	/* Here's the big part -- call the bucketer! */

	if (!broadcast)
	{
		/* We've called the flush routine when no vertices
		 * have been sent. Therefore we have no bounding box.
		 * In this case, we have to broadcast, so set the
		 * broadcast flag and continue.
		 */
		if (thread->packer->bounds_min.x == FLT_MAX &&
		    thread->packer->bounds_max.x == -FLT_MAX) {
			/*crDebug("No vertices or bounding box set during this flush - Broadcasting geometry\n");*/
			broadcast = 1;
		} else {
			/*crDebug( "About to bucket the geometry" ); */
			tilesortspuBucketGeometry(&bucket_info);
			if (tilesort_spu.providedBBOX != GL_OBJECT_BBOX_CR)
				crPackResetBBOX( thread->packer );
		}
	}
	else
	{
		/*crDebug( "Broadcasting the geometry!" ); */
	}

	/* Now, we want to let the state tracker extract the "current" state 
	 * from the collection of pointers that we have in the geometry 
	 * buffer. */

	crStateCurrentRecover( );

	/* At this point, we invoke the "pincher", which will check if 
	 * the current buffer needs to be stopped mid-triangle (or whatever), 
	 * and tuck away the appropriate state so that the triangle can be 
	 * re-started once this flush is done. */

	tilesortspuPinch();

	/* If GL_COLOR_MATERIAL is enabled, we need to copy the color values 
	 * into the state element for materials so that Gets will work and so 
	 * that the material will get sent down. 
	 * 
	 * We know we can do this now because this function will get called if 
	 * glDisable turns off COLOR_MATERIAL, so that would be the very very 
	 * latest time we could actually get away with that. */

	crStateColorMaterialRecover( );

	/* Okay.  Now, we need to un-hide the bonus space for the extra glEnd packet 
	 * and try to close off the begin/end if it exists.  This is a pretty 
	 * serious hack, but we *did* reserve space in the pack buffer just 
	 * for this purpose. */
	
	if ( thread->currentContext->State->current.inBeginEnd )
	{
		/*crDebug( "Closing this Begin/end!!!" ); */
		thread->packer->buffer.mtu += END_FLUFF + 4; /* 4 for opcode */
		if (tilesort_spu.swap)
		{
			crPackEndSWAP();
		}
		else
		{
			crPackEnd();
		}
	}

	/* Now, we grab the packing state into the globals, since we're 
	 * going to start packing into other places like crazy but 
	 * we need to keep these pointers around, both for 
	 * doing context differencing, and also to restore them 
	 * at the end of this function. */

	crPackGetBuffer( thread->packer, &(thread->geometry_pack) );

	/* Now, see if we need to do things to each server */

	for ( i = 0 ; i < tilesort_spu.num_servers; i++ )
	{
		int node32 = i >> 5;
		int node = i & 0x1f;

		thread->state_server = tilesort_spu.servers + i;
		thread->state_server_index = i;

		/* Check to see if this server needs geometry from us. */
		if (!broadcast && !(bucket_info.hits[node32] & (1 << node)))
		{
			/*crDebug( "NOT sending to server %d", i ); */
			continue;
		}
		/*crDebug( "Sending to server %d", i ); */

		/* Okay, it does.  */

		thread->state_server->vertexCount += thread->packer->current.vtx_count;

		/* We're going to do lazy state evaluation now */

		crPackSetBuffer( thread->packer, &(thread->pack[thread->state_server_index]) );
		if (!broadcast || send_state_anyway)
		{
			/*crDebug( "pack buffer before differencing" ); 
			 *tilesortspuDebugOpcodes( &(thread->packer->buffer) ); */
			CRContext *serverContext = thread->state_server->context[thread->currentContextIndex];
			crStateDiffContext( serverContext, ctx );
			if (tilesort_spu.drawBBOX && !broadcast)
			{
				__drawBBOX( &bucket_info );
			}
			crPackGetBuffer( thread->packer, &(thread->pack[thread->state_server_index]) );
			/*crDebug( "pack buffer after differencing" ); 
			 *tilesortspuDebugOpcodes( &(thread->state_server->pack) ); */
		}

		/* The state server's buffer now contains the commands 
		 * needed to bring it up to date on the state.  All 
		 * that's left to do is to append the geometry. */

		if ( big_packet_hdr != NULL )
		{
			/* The packet was big, so send the state commands 
			 * by themselves, followed by the big packet. */

			crDebug( "Doing the big packet send thing" );
			tilesortspuSendServerBuffer( thread->state_server_index );
			crNetSend( thread->net[thread->state_server_index].conn, NULL, big_packet_hdr, big_packet_len );
		}
		else
		{
			/* Now, we have to decide whether or not to append the geometry buffer 
			 * as a BOUNDS_INFO packet or just a bag-o-commands.  			 
			 * 
			 * As it turns out, we *always* send geometry as a BOUNDS_INFO 
			 * packet.  All the code lying around to deal with non-bounds-info 
			 * geometry was just a red herring. 
			 * 
			 * Now we see why I tucked away the geometry buffer earlier. */

			if ( !broadcast )
			{
				/*crDebug( "Appending a bounded buffer" ); */
				__appendBoundedBuffer( &(thread->geometry_pack),
									   &bucket_info.pixelBounds );
			}
			else
			{
				/*crDebug( "Appending a NON-bounded buffer" ); 
				 *tilesortspuDebugOpcodes( &(thread->geometry_pack) ); 
				 *tilesortspuDebugOpcodes( &(thread->packer->buffer) ); */
				__appendBuffer( &(thread->geometry_pack) );
				/*tilesortspuDebugOpcodes( &(thread->packer->buffer) ); */
			}
			crPackGetBuffer( thread->packer, &(thread->pack[thread->state_server_index]) );
		}
	}

	/* We're done with the servers.  Wipe the thread->state_server 
	 * variable so we know that.  */

	thread->state_server = NULL;
	thread->state_server_index = -1;

	for ( i = 0 ; i < tilesort_spu.num_servers; i++ )
	{
		if (thread->pack[i].mtu > thread->net[i].conn->mtu)
			thread->pack[i].mtu = thread->net[i].conn->mtu;
		if (tilesort_spu.MTU > thread->net[i].conn->mtu)
			tilesort_spu.MTU = thread->net[i].conn->mtu;
	}
	if ( big_packet_hdr != NULL )
	{
		/* Throw away the big packet and make a new, smaller one */

		crDebug( "Throwing away the big packet" );
		crFree( thread->geometry_pack.pack );
		crPackInitBuffer( &(thread->geometry_pack), crAlloc( thread->geom_pack_size ),
											thread->geom_pack_size, tilesort_spu.MTU - (24+END_FLUFF+8) );

	/* 24 is the size of the bounds info packet */
	/* and 8 since End and BoundInfo opcodes may indeed take this space */
	}
	else
	{
		/*crDebug( "Reverting to the old geometry buffer" ); */
		thread->geometry_pack.mtu = tilesort_spu.MTU - (24+END_FLUFF+8);
		crPackSetBuffer( thread->packer, &(thread->geometry_pack ) );
		crPackResetPointers( thread->packer );
	}

	/*crDebug( "Resetting the current vertex count and friends" ); */

	thread->packer->current.vtx_count = 0;
	thread->packer->current.vtx_count_begin = 0;

	/*crDebug( "Setting all the Current pointers to NULL" ); */
	crPackNullCurrentPointers();

	tilesortspuPinchRestoreTriangle();
}

void tilesortspuBroadcastGeom( int send_state_anyway )
{
	GET_THREAD(thread);
	__doFlush( thread->currentContext->State, 1, send_state_anyway );
}


void tilesortspuFlush_callback( void *arg )
{
	tilesortspuFlush( (ThreadInfo *) arg );
}


void tilesortspuFlush( ThreadInfo *thread )
{
	CRContext *ctx;

	CRPackBuffer old_geometry_pack;

	CRASSERT(thread);
	CRASSERT(thread->currentContext);
	CRASSERT(thread->currentContext->State);

	ctx = thread->currentContext->State;

	/*crDebug( "In tilesortspuFlush" ); 
	 *crDebug( "BBOX MIN: (%f %f %f)", thread->packer->bounds_min.x, thread->packer->bounds_min.y, thread->packer->bounds_min.z ); 
	 *crDebug( "BBOX MAX: (%f %f %f)", thread->packer->bounds_max.x, thread->packer->bounds_max.y, thread->packer->bounds_max.z ); */

	/* If we're not in a begin/end, or if we're splitting them, go ahead and 
	 * do the flushing. */
	
	if ( tilesort_spu.splitBeginEnd || !(ctx->current.inBeginEnd) )
	{
		__doFlush( ctx, 0, 0 );
		return;
	}


	/* Otherwise, we need to grow the buffer to make room for more stuff. 
	 * Since we don't want to do this forever, we mark the context as 
	 * "Flush on end", which will force a flush the next time glEnd() 
	 * is called. 
	 * 
	 * Note that this can only happen to the geometry buffer, so we 
	 * assert that. */
	
	crDebug( "-=-=-=- Growing the buffer -=-=-=-" );

	CRASSERT( ctx == thread->currentContext->State );

	ctx->current.flushOnEnd = 1;

	/* Grab the pack buffer info from the packing library */

	crPackGetBuffer( thread->packer, &(old_geometry_pack) );

	crPackInitBuffer( &(thread->geometry_pack),
					  crAlloc( thread->geometry_pack.size << 1 ), 
					  thread->geometry_pack.size << 1, (thread->geometry_pack.size << 1) - (24+END_FLUFF+8) );
	thread->geometry_pack.geometry_only = GL_TRUE;

	crPackSetBuffer( thread->packer, &(thread->geometry_pack) );
	crPackAppendBuffer(&(old_geometry_pack));

	/* The location of the geometry buffer has changed, so we need to 
	 * tell the state tracker to update its "current" pointers. 
	 * The "offset" computed here does *not* require that the two 
	 * buffers be contiguous -- in fact they almost certainly won't be. */

	crPackOffsetCurrentPointers( old_geometry_pack.data_current - 
															 thread->packer->buffer.data_current );

	crFree( old_geometry_pack.pack );
}
