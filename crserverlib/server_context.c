/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */
	
#include "cr_spu.h"
#include "chromium.h"
#include "cr_error.h"
#include "cr_net.h"
#include "server_dispatch.h"
#include "server.h"


GLint SERVER_DISPATCH_APIENTRY crServerDispatchCreateContext( const char *dpyName, GLint visualBits )
{
	GLint i, retVal = 0, ctxPos = -1;

	/* Since the Cr server serialized all incoming clients/contexts into
	 * one outgoing GL stream, we only need to create one context for the
	 * head SPU.  We'll only have to make it current once too, below.
	 */
	if (cr_server.firstCallCreateContext) {
		cr_server.SpuContextVisBits = visualBits;
		cr_server.SpuContext = cr_server.head_spu->dispatch_table.
			CreateContext(dpyName, cr_server.SpuContextVisBits);
		if (cr_server.SpuContext < 0) {
			crWarning("crServerDispatchCreateContext() failed.");
			return -1;
		}
		cr_server.firstCallCreateContext = GL_FALSE;
	}
	else {
		/* second or third or ... context */
		if ((visualBits & cr_server.SpuContextVisBits) != visualBits) {
			/* the new context needs new visual attributes */
			cr_server.SpuContextVisBits |= visualBits;
			crDebug("crServerDispatchCreateContext requires new visual (0x%x).",
							cr_server.SpuContextVisBits);
			/* destroy old rendering context */
			cr_server.head_spu->dispatch_table.DestroyContext(cr_server.SpuContext);
			/* create new rendering context with suitable visual */
			cr_server.SpuContext = cr_server.head_spu->dispatch_table.
				CreateContext(dpyName, cr_server.SpuContextVisBits);
			if (cr_server.SpuContext < 0) {
				crWarning("crServerDispatchCreateContext() failed.");
				return -1;
			}
		}
	}

	/* find an empty position in the context[] array */
	for (i = 0; i < CR_MAX_CONTEXTS; i++) {
		if (cr_server.context[i] == NULL) {
			ctxPos = i;
			break;
		}
	}

	if (ctxPos >= 0) {
		/* Now create a new state-tracker context and initialize the
		 * dispatch function pointers.
		 */
		CRContext *newCtx = crStateCreateContext( &cr_server.limits, visualBits );
		if (newCtx) {
			cr_server.context[ctxPos] = newCtx;
			crStateSetCurrentPointers( newCtx, &(cr_server.current) );
			retVal = MAGIC_OFFSET + ctxPos;
		}
	}

	crServerReturnValue( &retVal, sizeof(retVal) );
	return retVal;
}


void SERVER_DISPATCH_APIENTRY crServerDispatchDestroyContext( GLint ctx )
{
	const int ctxPos = ctx - MAGIC_OFFSET;
	CRContext *crCtx;

	CRASSERT(ctxPos >= 0);

	crCtx = cr_server.context[ctxPos];
	if (crCtx) {
		crStateDestroyContext( crCtx );
		cr_server.context[ctxPos] = NULL;
	}

	/* If we delete our current context, default back to the null context */
	if (cr_server.curClient->currentCtx == crCtx)
		cr_server.curClient->currentCtx = cr_server.DummyContext;
}


void SERVER_DISPATCH_APIENTRY crServerDispatchMakeCurrent( GLint window, GLint nativeWindow, GLint context )
{
	CRMuralInfo *mural;
	int ctxPos;
	CRContext *ctx;

	if (context >= 0 && window >= 0) {
		ctxPos = context - MAGIC_OFFSET;
		CRASSERT(ctxPos >= 0);
		CRASSERT(ctxPos < CR_MAX_CONTEXTS);

		mural = (CRMuralInfo *) crHashtableSearch(cr_server.muralTable, window);
		if (!mural && window == MAGIC_OFFSET &&
				!cr_server.clients[0].conn->actual_network) {
			/* We're reading from a file and not a real network connection so
			 * we have to fudge the window id here.
			 */
			window = 0;
			mural = (CRMuralInfo *) crHashtableSearch(cr_server.muralTable, 0);
		}
		CRASSERT(mural);

		/* Update the state tracker's current context */
		ctx = cr_server.context[ctxPos];
	}
	else {
		ctx = cr_server.DummyContext;
		window = -1;
		mural = NULL;
	}

	cr_server.curClient->currentCtx = ctx;
	cr_server.curClient->currentMural = mural;
	cr_server.curClient->currentWindow = window;

	/* This is a hack to force updating the 'current' attribs */
	crStateUpdateColorBits();

	if (ctx)
		crStateSetCurrentPointers( ctx, &(cr_server.current) );

	/* check if being made current for first time, update viewport */
	if (ctx) {
		/* initialize the viewport */
		if (ctx->viewport.viewportW == 0) {
			ctx->viewport.viewportW = mural->width;
			ctx->viewport.viewportH = mural->height;
			ctx->viewport.scissorW = mural->width;
			ctx->viewport.scissorH = mural->height;
		}
	}

	if (cr_server.firstCallMakeCurrent ||
			cr_server.currentWindow != window ||
			cr_server.currentNativeWindow != nativeWindow) {
		/* Since the cr server serialized all incoming contexts/clients into
		 * one output stream of GL commands, we only need to call the head
		 * SPU's MakeCurrent() function once.
		 * BUT, if we're rendering to multiple windows, we do have to issue
		 * MakeCurrent() calls sometimes.  The same GL context will always be
		 * used though.
		 */
		cr_server.head_spu->dispatch_table.MakeCurrent( window, nativeWindow, cr_server.SpuContext );
		cr_server.firstCallMakeCurrent = GL_FALSE;
		cr_server.currentWindow = window;
		cr_server.currentNativeWindow = nativeWindow;

		/* Set initial raster/window position for this context.
		 * The position has to be translated according to the tile origin.
		 */
		if (mural->numExtents > 0)
		{
			GLint x = -mural->extents[0].imagewindow.x1;
			GLint y = -mural->extents[0].imagewindow.y1;
			cr_server.head_spu->dispatch_table.WindowPos2iARB(x, y);
			/* This MakeCurrent is a bit redundant (we do it again below)
			 * but it's only done the first time we activate a context.
			 */
			crStateMakeCurrent(ctx);
			crStateWindowPos2iARB(x, y);
		}
	}

	/* This used to be earlier, after crStateUpdateColorBits() call */
	crStateMakeCurrent( ctx );

	/* This is pessimistic - we really don't have to invalidate the viewport
	 * info every time we MakeCurrent, but play it safe for now.
	 */
	mural->viewportValidated = GL_FALSE;
}

