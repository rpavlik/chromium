/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */
	
#include <stdio.h>
#include "cr_spu.h"
#include "chromium.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_net.h"
#include "server_dispatch.h"
#include "server.h"


/* This makes context numbers more readable during debugging */
#define MAGIC_OFFSET 5000


GLint SERVER_DISPATCH_APIENTRY crServerDispatchCreateContext( const char *dpyName, GLint visualBits )
{
	GLint i, retVal = 0, ctxPos = -1;
	CRContext *newCtx;

	/* Since the Cr server serialized all incoming clients/contexts into
	 * one outgoing GL stream, we only need to create one context for the
	 * head SPU.  We'll only have to make it current once too, below.
	 */
	if (cr_server.firstCallCreateContext) {
		cr_server.SpuContext
			= cr_server.head_spu->dispatch_table.CreateContext( dpyName, visualBits );
		if (cr_server.SpuContext < 0) {
			crWarning("headSpu.CreateContext failed in crServerDispatchCreateContext()");
			return -1;
		}
		cr_server.firstCallCreateContext = GL_FALSE;
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
		newCtx = crStateCreateContext( &cr_server.limits, visualBits );
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
		CRASSERT(mural);

		/* Update the state tracker's current context */
		ctx = cr_server.context[ctxPos];
	}
	else {
		ctx = NULL;
		window = -1;
		mural = NULL;
	}

	cr_server.curClient->currentCtx = ctx;
	cr_server.curClient->currentMural = mural;
	cr_server.curClient->currentWindow = window;

	/* This is a hack to force updating the 'current' attribs */
	crStateUpdateColorBits();

	/* XXX do we really want to do this here, or wait until we get back to
	 * the SerializeStreams loop?  It seems to work OK here.
	 */
	crStateMakeCurrent( ctx );

	if (ctx)
		crStateSetCurrentPointers( ctx, &(cr_server.current) );

	/* check if being made current for first time, update viewport */
	if (ctx) {
		if (ctx->viewport.outputDims.x1 == 0 &&
				ctx->viewport.outputDims.x2 == 0 &&
				ctx->viewport.outputDims.y1 == 0 &&
				ctx->viewport.outputDims.y2 == 0) {
			ctx->viewport.outputDims.x1 = mural->underlyingDisplay[0];
			ctx->viewport.outputDims.x2 = mural->underlyingDisplay[2];
			ctx->viewport.outputDims.y1 = mural->underlyingDisplay[1];
			ctx->viewport.outputDims.y2 = mural->underlyingDisplay[3];
			/* this requires a GL context */
			crServerBeginTiling(mural);
		}
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
	}
}

