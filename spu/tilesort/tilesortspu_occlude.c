/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_packfunctions.h"
#include "cr_error.h"
#include "cr_net.h"
#include "tilesortspu.h"
#include "tilesortspu_proto.h"


void TILESORTSPU_APIENTRY
tilesortspu_GetQueryObjectivARB(GLuint id, GLenum pname, GLint *params)
{
	GET_THREAD(thread);
	ThreadInfo *thread0 = &(tilesort_spu.thread[0]);
	GLint values[1024], i, total;

	CRASSERT(tilesort_spu.num_servers <= 1024);

	/* flush anything pending */
	tilesortspuFlush( thread );

	/* release geometry buffer */
	crPackReleaseBuffer( thread0->packer );

	/*
	 * loop over servers, issuing the glGet.
	 * We send it via the zero-th thread's server connections.
	 */
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		int writeback = 1;

		crPackSetBuffer( thread0->packer, &(thread0->buffer[i]) );

		if (tilesort_spu.swap)
			crPackGetQueryObjectivARBSWAP( id, pname, values + i, &writeback );
		else
			crPackGetQueryObjectivARB( id, pname, values + i, &writeback );

		/* release server buffer */
		crPackReleaseBuffer( thread0->packer );

		/* Flush buffer (send to server) */
		tilesortspuSendServerBuffer( i );

		/* Get return value */
		while (writeback) {
			crNetRecv();
		}
	}

	/* Restore the default pack buffer */
	crPackSetBuffer( thread0->packer, &(thread0->geometry_buffer) );

	/* total the results */
	total = 0;
	for (i = 0; i < tilesort_spu.num_servers; i++)
		total += values[i];

	if (pname == GL_QUERY_RESULT_ARB) {
		/* return the sum */
		*params = total;
	}
	else if (pname == GL_QUERY_RESULT_AVAILABLE_ARB) {
		/* if all servers are ready, total will equal the number of servers */
		if (total == tilesort_spu.num_servers)
			*params = 1;
		else
			*params = 0;
	}
}


void TILESORTSPU_APIENTRY
tilesortspu_GetQueryObjectuivARB(GLuint id, GLenum pname, GLuint *params)
{
	GLint result;

	tilesortspu_GetQueryObjectivARB(id, pname, &result);
	*params = (GLuint) result;
}


void TILESORTSPU_APIENTRY
tilesortspu_BeginQueryARB(GLenum target, GLuint id)
{
	 /* Need to broadcast this */
	GET_THREAD(thread);
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
		crPackBeginQueryARBSWAP(target, id);
	else
		crPackBeginQueryARB(target, id);
	tilesortspuBroadcastGeom(1);
}


void TILESORTSPU_APIENTRY
tilesortspu_EndQueryARB(GLenum target)
{
	/* Need to broadcast this */
	GET_THREAD(thread);
	tilesortspuFlush( thread );
	if (tilesort_spu.swap)
		crPackEndQueryARBSWAP(target);
	else
		crPackEndQueryARB(target);
	tilesortspuBroadcastGeom(1);
}
