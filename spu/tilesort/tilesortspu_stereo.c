#include "tilesortspu.h"
#include "tilesortspu_proto.h"
#include "cr_pack.h"
#include "cr_packfunctions.h"
#include "cr_unpack.h"
#include <math.h>


/**
** Make m an identity matrix
*  \param m matrix set to identity
*/
static void tilesortMakeIdentityf(GLfloat m[16])
{
	m[0 + 4 * 0] = 1;
	m[0 + 4 * 1] = 0;
	m[0 + 4 * 2] = 0;
	m[0 + 4 * 3] = 0;
	m[1 + 4 * 0] = 0;
	m[1 + 4 * 1] = 1;
	m[1 + 4 * 2] = 0;
	m[1 + 4 * 3] = 0;
	m[2 + 4 * 0] = 0;
	m[2 + 4 * 1] = 0;
	m[2 + 4 * 2] = 1;
	m[2 + 4 * 3] = 0;
	m[3 + 4 * 0] = 0;
	m[3 + 4 * 1] = 0;
	m[3 + 4 * 2] = 0;
	m[3 + 4 * 3] = 1;
}


/**
 * When in passive stereo mode, calls to glDrawBuffer() control
 * whether we render to the right or left (or both) crservers.
 * \param buffer buffer mode
 */
static void
setup_passive(GLenum buffer)
{
	GET_THREAD(thread);
	ContextInfo *context = thread->currentContext;
	int destFlags = 0;

	CRASSERT(tilesort_spu.stereoMode == PASSIVE);

	/* Update stereoDestFlags which controls which servers get which
	 * command buffers during bucketing.
	 */
	switch (buffer) {
	case GL_LEFT:
	case GL_FRONT_LEFT:
	case GL_BACK_LEFT:
		destFlags = EYE_LEFT;
		break;
	case GL_RIGHT:
		destFlags = EYE_RIGHT;
		break;
	case GL_FRONT_RIGHT:
		destFlags = EYE_RIGHT;
		break;
	case GL_BACK_RIGHT:
		destFlags = EYE_RIGHT;
		break;
	case GL_FRONT:
	case GL_BACK:
	case GL_FRONT_AND_BACK:
	case GL_NONE:
		/* Note, not zero.  We send rendering commands to the left and right
		 * servers.  It's just that the front and/or back _color_ buffers might
		 * not get updated.
		 */
		destFlags = EYE_LEFT | EYE_RIGHT;
		break;
	default:
		/* GL_AUXn */
		destFlags = EYE_LEFT | EYE_RIGHT;
		break;

	}

	context->stereoDestFlags = destFlags;
}


/**
 * Send either the left or right stereo view/projection matrices to
 * _all_ the servers.
 * NOTE: This would not be used for a passive stereo config since in
 * that situation, the servers are designated as either being left or right.
 *
 * \param eye - bitmask of EYE_LEFT and EYE_RIGHT
 */
static void
send_matrices(int eye)
{
	GET_THREAD(thread);
	WindowInfo *winInfo = thread->currentContext->currentWindow;
	int server;

	CRASSERT(tilesort_spu.stereoMode != PASSIVE);

	/*printf("%s %d\n", __FUNCTION__, eye);*/
	CRASSERT((eye & ~(EYE_LEFT | EYE_RIGHT)) == 0);

	/* release geom buffer */
	crPackReleaseBuffer( thread->packer );

	for (server = 0; server < tilesort_spu.num_servers; server++) {
		GLfloat params[17];

		/* send new frustum to <server> */
		crPackSetBuffer( thread->packer, &(thread->buffer[server]) );

		params[0] = (GLfloat) server;

		if (eye == EYE_LEFT || eye == EYE_RIGHT) {
			/* Either updating left or right view, not both */

			if (winInfo->matrixSource == MATRIX_SOURCE_SERVERS) {
				/* just tell servers whether to use left or right matrices */
				const int i = (eye == EYE_LEFT) ? 0 : 1;
				if (tilesort_spu.swap)
					crPackChromiumParameteriCRSWAP(GL_SERVER_CURRENT_EYE_CR, i);
				else
					crPackChromiumParameteriCR(GL_SERVER_CURRENT_EYE_CR, i);
			}
			else {
				 /* send new view matrix to the server */
				 const int i = (eye == EYE_LEFT) ? 0 : 1;
				 if (tilesort_spu.forceQuadBuffering) {
					 /* the server doesn't know anything about left vs. right */
					 params[1] = 0;
				 }
				 else {
					 /* app is already issuing correct left/right transformations */
					 params[1] = (float) i; /* 0=left eye, 1=right eye */
				 }

				 crMatrixGetFloats(params + 2, &tilesort_spu.stereoViewMatrices[i]);
				 if (tilesort_spu.swap) {
					 crPackChromiumParametervCRSWAP(GL_SERVER_VIEW_MATRIX_CR, GL_FLOAT,
																					18, params);
				 }
				 else {
					 crPackChromiumParametervCR(GL_SERVER_VIEW_MATRIX_CR, GL_FLOAT,
																			18, params);
				 }

				 /* send new projection matrix to the server */
				 crMatrixGetFloats(params + 2, &tilesort_spu.stereoProjMatrices[i]);
				 if (tilesort_spu.swap) {
					 crPackChromiumParametervCRSWAP(GL_SERVER_PROJECTION_MATRIX_CR, GL_FLOAT,
																					18, params);
				 }
				 else {
					 crPackChromiumParametervCR(GL_SERVER_PROJECTION_MATRIX_CR, GL_FLOAT,
																			18, params);
				 }
			}
		}
		else {
			/* both views - same projection for both views (ala 2D overlays gfx) */
			CRASSERT(eye == (EYE_LEFT | EYE_RIGHT));

			if (winInfo->matrixSource == MATRIX_SOURCE_SERVERS) {
				/** XXX \todo should tell server to use identity matrix */
				/** XXX \todo this whole business is kind of broken anyway - that is,
				 * glDrawBuffer(GL_LEFT_AND_RIGHT) is really hard to deal with
				 */
#if 0
				const int i = -1;
				if (tilesort_spu.swap)
					crPackChromiumParameteriCRSWAP(9993, i);
				else
					crPackChromiumParameteriCR(9993, i);
#endif
			}
			else {
				params[1] = 0; /* just left eye */
				/*printf("sending identity transform\n");*/
				/* send identity view matrix to the server */
				tilesortMakeIdentityf(params + 2);
				if (tilesort_spu.swap) {
					crPackChromiumParametervCRSWAP(GL_SERVER_VIEW_MATRIX_CR, GL_FLOAT,
																				 18, params);
				}
				else {
					crPackChromiumParametervCR(GL_SERVER_VIEW_MATRIX_CR, GL_FLOAT,
																		 18, params);
				}

				/* send user's projection matrix to the server */
				crMatrixGetFloats(params + 2,
									thread->currentContext->State->transform.projectionStack.top);
				if (tilesort_spu.swap) {
					crPackChromiumParametervCRSWAP(GL_SERVER_PROJECTION_MATRIX_CR, GL_FLOAT,
																				 18, params);
				}
				else {
					crPackChromiumParametervCR(GL_SERVER_PROJECTION_MATRIX_CR, GL_FLOAT,
																		 18, params);
				}
			}
		}

		/* release server buffer */
		crPackReleaseBuffer( thread->packer );
	}
	/* Restore default geom buffer */
	crPackSetBuffer( thread->packer, &(thread->geometry_buffer) );
}


/**
 * When running in CRYSTAL-Eyes stereo mode, do whatever's needed in response
 * to a call to glDrawBuffer().
 * This would typically only be called when force_quad_buffering is true.
 * An OpenGL app that's already quad-buffered aware (i.e. active stereo)
 * wouldn't have to set 'stereo_mode'="CrystalEyes".
 * \param buffer buffer mode
 */
static void
setup_crystal(GLenum buffer)
{
	/* send left/right view/projection matrices to servers */
	if (buffer == GL_LEFT ||
			buffer == GL_FRONT_LEFT ||
			buffer == GL_BACK_LEFT)
		send_matrices(EYE_LEFT);
	else if (buffer == GL_RIGHT ||
					 buffer == GL_FRONT_RIGHT ||
					 buffer == GL_BACK_RIGHT)
		send_matrices(EYE_RIGHT);
	else
		send_matrices(EYE_LEFT | EYE_RIGHT);
}




/**
 * When running in ANAGLYPH-Eyes stereo mode, do whatever's needed in response
 * to a call to glDrawBuffer().
 * This would typically only be called when force_quad_buffering is true.
 * An OpenGL app that's already quad-buffered aware (i.e. active stereo)
 * wouldn't have to set 'stereo_mode'="Anaglyph".
 * \param buffer buffer mode
 */
static void
setup_anaglyph(GLenum buffer)
{
	GET_THREAD(thread);
	ContextInfo *context = thread->currentContext;

	/* send left/right view/projection matrices to servers */
	if (buffer == GL_LEFT ||
			buffer == GL_FRONT_LEFT ||
			buffer == GL_BACK_LEFT) {
		/* Drawing left view - setup left eye filter color */
		crStateColorMask(tilesort_spu.anaglyphMask[0][0],
										 tilesort_spu.anaglyphMask[0][1],
										 tilesort_spu.anaglyphMask[0][2],
										 tilesort_spu.anaglyphMask[0][3]);
		context->stereoDestFlags = EYE_LEFT;
	}
	else if (buffer == GL_RIGHT ||
					 buffer == GL_FRONT_RIGHT ||
					 buffer == GL_BACK_RIGHT) {
		/* Drawing right view - setup right eye filter color */
		crStateColorMask(tilesort_spu.anaglyphMask[1][0],
										 tilesort_spu.anaglyphMask[1][1],
										 tilesort_spu.anaglyphMask[1][2],
										 tilesort_spu.anaglyphMask[1][3]);
		context->stereoDestFlags = EYE_RIGHT;
	}
	else {
		/* Drawing to both buffers.
		 * Use bitwise-OR of left and right eye masks to enable writing
		 * to both filtered colors.
		 */
		crStateColorMask((GLboolean)(tilesort_spu.anaglyphMask[0][0] |
										 tilesort_spu.anaglyphMask[1][0]),
										 (GLboolean)(tilesort_spu.anaglyphMask[0][1] |
										 tilesort_spu.anaglyphMask[1][1]),
										 (GLboolean)(tilesort_spu.anaglyphMask[0][2] |
										 tilesort_spu.anaglyphMask[1][2]),
										 (GLboolean)(tilesort_spu.anaglyphMask[0][3] |
										 tilesort_spu.anaglyphMask[1][3]));
		context->stereoDestFlags = (EYE_LEFT | EYE_RIGHT);
	}

	send_matrices(context->stereoDestFlags);
}



/**
 * Left half of window gets left eye's view, right half of window gets
 * right eye's view.
 * \param buffer buffer mode
 */
static void
setup_side_by_side(GLenum buffer)
{
	GET_THREAD(thread);
	ContextInfo *context = thread->currentContext;
	const WindowInfo *winInfo = context->currentWindow;
	const int w = winInfo->muralWidth / 2;
	const int h = winInfo->muralHeight;

	/* Determine if we're drawing the left or right view */
	if (buffer == GL_LEFT ||
			buffer == GL_FRONT_LEFT ||
			buffer == GL_BACK_LEFT) {
		context->stereoDestFlags = EYE_LEFT;
	}
	else if (buffer == GL_RIGHT ||
					 buffer == GL_FRONT_RIGHT ||
					 buffer == GL_BACK_RIGHT) {
		context->stereoDestFlags = EYE_RIGHT;
	}
	else {
		context->stereoDestFlags = EYE_LEFT | EYE_RIGHT;
	}
	/* Send the view/projection matrices for this view to the servers */
	send_matrices(context->stereoDestFlags);

	/* set up scissor/viewport to render to left or right half of mural */
	crStateEnable(GL_SCISSOR_TEST);
	if (context->stereoDestFlags & EYE_LEFT) {
		crStateViewport(0, 0, w, h);
		crStateScissor(0, 0, w, h);
	}
	else {
		crStateViewport(w, 0, w, h);
		crStateScissor(w, 0, w, h);
	}
}



/**
 * Setup stereo buffers 
 * \param buffer buffer to be initialized
 */
void
tilesortspuSetupStereo(GLenum buffer)
{
	switch (tilesort_spu.stereoMode) {
	case NONE:
		 /* nothing */
		break;
	case PASSIVE:
		setup_passive(buffer);
		break;
	case CRYSTAL:
		setup_crystal(buffer);
		break;
	case SIDE_BY_SIDE:
		setup_side_by_side(buffer);
		break;
	case ANAGLYPH:
		setup_anaglyph(buffer);
		break;
	default:
		crError("bad stereoMode in tilesortspuSetupStereo");
	}
}



/**
 * Called by applications, as well as by SwapBuffers when
 * 'force_quad_buffering' is set.
 * \param buffer which buffer to draw
 */
void TILESORTSPU_APIENTRY
tilesortspu_DrawBuffer(GLenum buffer)
{
#if 0
	printf("%s 0x%x", __FUNCTION__, buffer);
	if (buffer == GL_LEFT)
		printf(" LEFT\n");
	else if (buffer == GL_RIGHT)
		printf(" RIGHT\n");
	else
		printf("\n");
#endif

	/* adjust the actual DrawBuffer target */
	switch (tilesort_spu.stereoMode) {
	case NONE:
	case CRYSTAL:
		crStateDrawBuffer(buffer);
		break;
	case PASSIVE:
	case SIDE_BY_SIDE:
	case ANAGLYPH:
		/* Note: we don't have right color buffers in these modes! */
		if (buffer == GL_FRONT_LEFT ||
				buffer == GL_FRONT_RIGHT ||
				buffer == GL_FRONT)
			crStateDrawBuffer(GL_FRONT);
		else if (buffer == GL_BACK_LEFT ||
						 buffer == GL_BACK_RIGHT ||
						 buffer == GL_BACK)
			crStateDrawBuffer(GL_BACK);
		else
			crStateDrawBuffer(buffer);
		break;
	default:
		crError("bad stereoMode in tilesortspu_DrawBuffer");
	}

	tilesortspuSetupStereo(buffer);
}



/**
 * This is called the first time a rendering context is make current
 * in case any special setup is need for stereo.
 * \param ctx pointer to Cr context
 */
void
tilesortspuStereoContextInit(ContextInfo *ctx)
{
	if (tilesort_spu.stereoMode == ANAGLYPH) {
		/* first, clear color buffer to default color w/out masking */
		crStateColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		crPackClear( GL_COLOR_BUFFER_BIT );
		tilesortspuBroadcastGeom(GL_TRUE);
		/* now, setup color masking */
		setup_anaglyph(GL_LEFT);
	}
	else if (tilesort_spu.stereoMode == SIDE_BY_SIDE) {
		setup_side_by_side(GL_LEFT);
	}
}
