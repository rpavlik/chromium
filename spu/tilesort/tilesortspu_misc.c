/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_packfunctions.h"
#include "cr_error.h"
#include "tilesortspu.h"
#include "tilesortspu_proto.h"

#include <float.h>

static const GLvectorf maxVector = {FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX};
static const GLvectorf minVector = {-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX};


static void PropogateCursorPosition(const GLint pos[2])
{
	GET_THREAD(thread);
	WindowInfo *winInfo = thread->currentContext->currentWindow;
	int i;

	/* release geometry buffer */
	crPackReleaseBuffer( thread->packer );

	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		const ServerWindowInfo *servWinInfo = winInfo->server + i;
		GLint tilePos[2];

		/* transform the client window cursor position to the tile position */
		tilePos[0] = (GLint) (pos[0] * winInfo->widthScale) - servWinInfo->extents[0].x1;
		tilePos[1] = (GLint) (pos[1] * winInfo->heightScale) - servWinInfo->extents[0].y1;

		crPackSetBuffer( thread->packer, &(thread->buffer[i]) );

		if (tilesort_spu.swap)
			crPackChromiumParametervCRSWAP(GL_CURSOR_POSITION_CR, GL_INT, 2, tilePos);
		else
			crPackChromiumParametervCR(GL_CURSOR_POSITION_CR, GL_INT, 2, tilePos);

		/* release server buffer */
		crPackReleaseBuffer( thread->packer );
	}

	/* The default buffer */
	crPackSetBuffer( thread->packer, &(thread->geometry_buffer) );
}


static void resetVertexCounters(void)
{
	GET_THREAD(thread);
	int i;
	for (i = 0; i < tilesort_spu.num_servers; i++)
		thread->currentContext->server[i].vertexCount = 0;
}


void TILESORTSPU_APIENTRY tilesortspu_ChromiumParameteriCR(GLenum target, GLint value)
{
	GET_THREAD(thread);
	int i;

	(void) value;

	switch (target) {
	case GL_RESET_VERTEX_COUNTERS_CR:  /* GL_CR_tilesort_info */
		resetVertexCounters();
		break;
	default:
		/* release geometry buffer */
		crPackReleaseBuffer( thread->packer );

		for (i = 0; i < tilesort_spu.num_servers; i++)
		{
			crPackSetBuffer( thread->packer, &(thread->buffer[i]) );

			if (tilesort_spu.swap)
				crPackChromiumParameteriCRSWAP( target, value );
			else
				crPackChromiumParameteriCR( target, value );

			/* release server buffer */
			crPackReleaseBuffer( thread->packer );
		}

		/* The default buffer */
		crPackSetBuffer( thread->packer, &(thread->geometry_buffer) );
		break;
	}
}


void TILESORTSPU_APIENTRY tilesortspu_ChromiumParameterfCR(GLenum target, GLfloat value)
{
	GET_THREAD(thread);
	int i;

	(void) value;

	switch (target) {
	case GL_RESET_VERTEX_COUNTERS_CR:  /* GL_CR_tilesort_info */
		resetVertexCounters();
		break;
	default:
		/* release geom buffer */
		crPackReleaseBuffer( thread->packer );

		for (i = 0; i < tilesort_spu.num_servers; i++)
		{
			crPackSetBuffer( thread->packer, &(thread->buffer[i]) );

			if (tilesort_spu.swap)
				crPackChromiumParameterfCRSWAP( target, value );
			else
				crPackChromiumParameterfCR( target, value );

			/* release server buffer */
			crPackReleaseBuffer( thread->packer );
		}

		/* The default buffer */
		crPackSetBuffer( thread->packer, &(thread->geometry_buffer) );
		break;
	}
}


void TILESORTSPU_APIENTRY tilesortspu_ChromiumParametervCR(GLenum target, GLenum type, GLsizei count, const GLvoid *values)
{
	GET_THREAD(thread);
	WindowInfo *winInfo = thread->currentContext->currentWindow;
	int i;

	switch (target) {
	case GL_CURSOR_POSITION_CR:  /* GL_CR_cursor_position */
		PropogateCursorPosition((GLint *) values);
		break;
	case GL_SCREEN_BBOX_CR:  /* GL_CR_bounding_box */
		CRASSERT(type == GL_FLOAT);
		CRASSERT(count == 8);
		/* must flush any pending rendering */
		tilesortspuFlush( thread );
		if (values) {
			const GLfloat *bbox = (const GLfloat *) values;
			thread->packer->bounds_min.x = bbox[0];
			thread->packer->bounds_min.y = bbox[1];
			thread->packer->bounds_min.z = bbox[2];
			thread->packer->bounds_min.w = bbox[3];
			thread->packer->bounds_max.x = bbox[4];
			thread->packer->bounds_max.y = bbox[5];
			thread->packer->bounds_max.z = bbox[6];
			thread->packer->bounds_max.w = bbox[7];
			/*crWarning( "I should really switch to the non-bbox API now, but API switching doesn't work" ); */
			thread->packer->updateBBOX = 0;
			thread->currentContext->providedBBOX = target;
		}
		else {
			/* disable bbox */
			thread->packer->bounds_min = maxVector;
			thread->packer->bounds_max = minVector;
			thread->packer->updateBBOX = 1;
			thread->currentContext->providedBBOX = GL_DEFAULT_BBOX_CR;
		}
		break;
	case GL_OBJECT_BBOX_CR:  /* GL_CR_bounding_box */
		CRASSERT(type == GL_FLOAT);
		CRASSERT(count == 6);
		/* must flush any pending rendering */
		tilesortspuFlush( thread );
		if (values) {
			const GLfloat *bbox = (const GLfloat *) values;
			thread->packer->bounds_min.x = bbox[0];
			thread->packer->bounds_min.y = bbox[1];
			thread->packer->bounds_min.z = bbox[2];
			thread->packer->bounds_min.w = 1.0;
			thread->packer->bounds_max.x = bbox[3];
			thread->packer->bounds_max.y = bbox[4];
			thread->packer->bounds_max.z = bbox[5];
			thread->packer->bounds_max.w = 1.0;
			/*crWarning( "I should really switch to the non-bbox API now, but API switching doesn't work" ); */
			thread->packer->updateBBOX = 0;
			thread->currentContext->providedBBOX = target;
		}
		else {
			/* disable bbox */
			thread->packer->bounds_min = maxVector;
			thread->packer->bounds_max = minVector;
			thread->packer->updateBBOX = 1;
			thread->currentContext->providedBBOX = GL_DEFAULT_BBOX_CR;
		}
		break;
	case GL_DEFAULT_BBOX_CR:  /* GL_CR_bounding_box */
		/* must flush any pending rendering */
		tilesortspuFlush( thread );
		CRASSERT(count == 0);
		thread->packer->bounds_min = maxVector;
		thread->packer->bounds_max = minVector;
		/*crWarning( "I should really switch to the bbox API now, but API switching doesn't work" ); */
		thread->packer->updateBBOX = 1;
		thread->currentContext->providedBBOX = GL_DEFAULT_BBOX_CR;
		break;

	case GL_TILE_INFO_CR:  /* GL_CR_tile_info */
		if (tilesort_spu.useDMX) {
			crWarning("It's illegal to set tiling with GL_TILE_INFO_CR with DMX");
		}
		else {
			const int *ivalues = (const int *) values;
			const int server = ivalues[0];
			const int muralWidth = ivalues[1];
			const int muralHeight = ivalues[2];
			const int numTiles = ivalues[3];
			int i;
			if (server < tilesort_spu.num_servers) {
				winInfo->muralWidth = muralWidth;
				winInfo->muralHeight = muralHeight;
				winInfo->server[server].num_extents = numTiles;
				for (i = 0; i < numTiles; i++)
				{
					const int x = ivalues[4 + i * 4 + 0];
					const int y = ivalues[4 + i * 4 + 1];
					const int w = ivalues[4 + i * 4 + 2];
					const int h = ivalues[4 + i * 4 + 3];
					winInfo->server[server].extents[i].x1 = x;
					winInfo->server[server].extents[i].y1 = y;
					winInfo->server[server].extents[i].x2 = x + w;
					winInfo->server[server].extents[i].y2 = y + h;
				}

				tilesortspuBucketingInit(winInfo);
				tilesortspuSendTileInfoToServers(winInfo);
			}
			else {
				crWarning("glChromiumParameteri(GL_TILE_INFO_CR) specified illegal server index");
			}
#if 0
			/* this leads to crashes - investigate */
			/*tilesortspuComputeMaxViewport(winInfo);*/
#endif
		}
		break;

	case GL_SERVER_FRUSTUM_CR:
		/* Set per-server projection matrix / frustum (for CAVE-like stuff) */
		CRASSERT(count == 7);
		CRASSERT(type == GL_FLOAT);
		{
			const GLfloat *v = (const GLfloat *) values;
			const GLfloat left   = v[1];
			const GLfloat right  = v[2];
			const GLfloat bottom = v[3];
			const GLfloat top    = v[4];
			const GLfloat hither = v[5];
			const GLfloat yon    = v[6];
			GLfloat m[18];

			m[0] = v[0];		/* server number */
			m[1] = 0; /* eye */

			/* build projection matrix */
			m[2] = (2.0f * hither) / (right - left);
			m[3] = 0.0;
			m[4] = 0.0;
			m[5] = 0.0;

			m[6] = 0.0;
			m[7] = (2.0f * hither) / (top - bottom);
			m[8] = 0.0;
			m[9] = 0.0;

			m[10] = (right + left) / (right - left);
			m[11] = (top + bottom) / (top - bottom);
			m[12] = (-hither - yon) / (yon - hither);
			m[13] = -1.0;

			m[14] = 0.0;
			m[15] = 0.0;
			m[16] = (2.0f * yon * hither) / (hither - yon);
			m[17] = 0.0;

			/* Recurse, using the code in the next switch case */
			tilesortspu_ChromiumParametervCR(GL_SERVER_PROJECTION_MATRIX_CR,
																			 type, 18, m);
		}
		break;

	case GL_SERVER_PROJECTION_MATRIX_CR:
		/* Set per-server projection matrix / frustum (for CAVE-like stuff) */
		CRASSERT(count == 18);
		CRASSERT(type == GL_FLOAT);
		{
			const GLfloat *v = (const GLfloat *) values;
			const int server = (int) v[0];
			const int eye = v[1] == 0.0 ? 0 : 1;

			if (server < tilesort_spu.num_servers) {
				ServerWindowInfo *servWinInfo = winInfo->server + server;

				/* save new matrix */
				crMatrixInitFromFloats(&servWinInfo->projectionMatrix[eye], v + 2);
				winInfo->matrixSource = MATRIX_SOURCE_SERVERS;

				/* release geom buffer */
				crPackReleaseBuffer( thread->packer );
				/* send new frustum to <server> */
				crPackSetBuffer( thread->packer, &(thread->buffer[server]) );

				/* XXX Perhaps it should be the application's responsibility to
				 * set the matrix mode, load the identity matrix, etc?
				 */
				if (tilesort_spu.swap) {
					crPackPushAttribSWAP(GL_TRANSFORM_BIT);
					crPackMatrixModeSWAP(GL_PROJECTION);
					crPackLoadMatrixfSWAP(v + 2);
					crPackPopAttribSWAP();
				}
				else {
					crPackPushAttrib(GL_TRANSFORM_BIT);
					crPackMatrixMode(GL_PROJECTION);
					crPackLoadMatrixf(v + 2);
					crPackPopAttrib();
				}
				/* release server buffer */
				crPackReleaseBuffer( thread->packer );
				/* Restore default geom buffer */
				crPackSetBuffer( thread->packer, &(thread->geometry_buffer) );
			}
			else {
				crWarning("Bad server index [%d] passed to "
									"glChromiumParametervCR(GL_SERVER_FRUSTUM_CR\n", server);
			}
		}
		break;

	case GL_SERVER_VIEW_MATRIX_CR:
		/* Set per-server view matrix (for CAVE-like stuff) */
		CRASSERT(count == 18);
		CRASSERT(type == GL_FLOAT);
		{
			const GLfloat *v = (const GLfloat *) values;
			const int server = (int) v[0];
			const int eye = v[1] == 0.0 ? 0 : 1;
			if (server >= 0 && server < tilesort_spu.num_servers){
				ServerWindowInfo *servWinInfo = winInfo->server + server;

				/* save new matrix */
				crMatrixInitFromFloats(&servWinInfo->viewMatrix[eye], v + 2);
				winInfo->matrixSource = MATRIX_SOURCE_SERVERS;

				/* release geom buffer */
				crPackReleaseBuffer( thread->packer );
				/* send new frustum to <server> */
				crPackSetBuffer( thread->packer, &(thread->buffer[server]) );

				/* send new view matrix to the server */
				if (tilesort_spu.swap) {
					crPackChromiumParametervCRSWAP(target, type, count, values);
				}
				else {
					crPackChromiumParametervCR(target, type, count, values);
				}

				/* release server buffer */
				crPackReleaseBuffer( thread->packer );
				/* Restore default geom buffer */
				crPackSetBuffer( thread->packer, &(thread->geometry_buffer) );
			}
			else {
				crWarning("Bad server index [%d] passed to "
									"glChromiumParametervCR(GL_SERVER_VIEW_MATRIX_CR\n", server);
			}
		}
		break;

	default:
		/* Propogate the call to the servers */

		/* release geom buffer */
		crPackReleaseBuffer( thread->packer );

		for (i = 0; i < tilesort_spu.num_servers; i++)
		{
			crPackSetBuffer( thread->packer, &(thread->buffer[i]) );

			if (tilesort_spu.swap)
				crPackChromiumParametervCRSWAP( target, type, count, values );
			else
				crPackChromiumParametervCR( target, type, count, values );

			/* release server buffer */
			crPackReleaseBuffer( thread->packer );
		}

		/* Restore default geom buffer */
		crPackSetBuffer( thread->packer, &(thread->geometry_buffer) );
		break;
	}
}


void TILESORTSPU_APIENTRY tilesortspu_GetChromiumParametervCR(GLenum target, GLuint index, GLenum type, GLsizei count, GLvoid *values)
{
	GET_THREAD(thread);
	int writeback = tilesort_spu.num_servers ? 1 : 0;
	int i;

	switch (target) {
	case GL_WINDOW_SIZE_CR:
		if (type != GL_INT || count != 2) {
			crError("glGetChromiumParametervCR(GL_WINDOW_SIZE_CR, type!=GL_INT or count !=2");
		}
		else if (!thread->currentContext) {
			((GLint *) values)[0] = tilesort_spu.fakeWindowWidth;
			((GLint *) values)[1] = tilesort_spu.fakeWindowHeight;
			break;
		}
		/* FALL-THROUGH -return mural size */
	case GL_MURAL_SIZE_CR:		 /* GL_CR_tilesort_info */
		if (type == GL_INT && count == 2)
		{
			WindowInfo *winInfo = thread->currentContext->currentWindow;
			((GLint *) values)[0] = winInfo->lastWidth;
			((GLint *) values)[1] = winInfo->lastHeight;
		}
		break;
	case GL_NUM_SERVERS_CR:		 /* GL_CR_tilesort_info */
		if (type == GL_INT && count == 1)
			*((GLint *) values) = tilesort_spu.num_servers;
		else
			*((GLint *) values) = 0;
		break;
  case GL_NUM_TILES_CR:
		if (type == GL_INT && count == 1 && index < (GLuint) tilesort_spu.num_servers) {
			WindowInfo *winInfo = thread->currentContext->currentWindow;
			*((GLint *) values) = winInfo->server[index].num_extents;
		}
		break;
	case GL_TILE_BOUNDS_CR:
		if (type == GL_INT && count == 4)
		{
			GLint *ivalues = (GLint *) values;
			WindowInfo *winInfo = thread->currentContext->currentWindow;
			int server = index >> 16;
			int tile = index & 0xffff;
			if (server < tilesort_spu.num_servers &&
					tile < winInfo->server[server].num_extents) {
				ivalues[0] = (GLint) winInfo->server[server].extents[tile].x1;
				ivalues[1] = (GLint) winInfo->server[server].extents[tile].y1;
				ivalues[2] = (GLint) winInfo->server[server].extents[tile].x2;
				ivalues[3] = (GLint) winInfo->server[server].extents[tile].y2;
			}
		}
		break;
	case GL_VERTEX_COUNTS_CR:		 /* GL_CR_tilesort_info */
		if (type == GL_INT && count >= 1)
		{
			int n = tilesort_spu.num_servers;
			if (count < n)
				n = count;
			for (i = 0; i < n; i++)
			{
				((GLint *) values)[i] = thread->currentContext->server[i].vertexCount;
			}
		}
		break;
	default:
		/* release geom buffer */
		crPackReleaseBuffer( thread->packer );

		for (i = 0; i < tilesort_spu.num_servers; i++)
		{
			crPackSetBuffer( thread->packer, &(thread->buffer[i]) );

			if (tilesort_spu.swap)
				crPackGetChromiumParametervCRSWAP( target, index, type, count, values, &writeback );
			else
				crPackGetChromiumParametervCR( target, index, type, count, values, &writeback );

			/* release server buffer */
			crPackReleaseBuffer( thread->packer );

			tilesortspuSendServerBuffer( i );

			while (writeback)
				crNetRecv();
		}

		/* pack into geom buffer again */
		crPackSetBuffer( thread->packer, &(thread->geometry_buffer) );
		break;
	}
}
