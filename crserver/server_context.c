/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */
	
#include <stdio.h>
#include "cr_spu.h"
#include "cr_glwrapper.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_net.h"
#include "server_dispatch.h"
#include "server.h"


/* This makes context numbers more readable during debugging */
#define MAGIC_OFFSET 5000


GLint SERVER_DISPATCH_APIENTRY crServerDispatchCreateContext( const char *dpyName, GLint visualBits )
{
	static GLboolean firstCall = GL_TRUE;
	GLint i, retVal = 0, ctxPos = -1;
	CRContext *newCtx;

#if 1
	/* XXX temporary */
	extern CRContext *__currentContext;
	crDebug("crserver: &__currentContext = %p\n", (void *) &__currentContext);
#endif

	/* Since the Cr server serialized all incoming clients/contexts into
	 * one outgoing GL stream, we only need to create one context for the
	 * head SPU.  We'll only have to make it current once too, below.
	 */
	if (firstCall) {
		cr_server.SpuContext
			= cr_server.head_spu->dispatch_table.CreateContext( dpyName, visualBits );
		if (cr_server.SpuContext < 0) {
			crWarning("headSpu.CreateContext failed in crServerDispatchCreateContext()");
			return -1;
		}
		firstCall = GL_FALSE;
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
		newCtx = crStateCreateContext( &cr_server.limits );
		if (newCtx) {
			cr_server.context[ctxPos] = newCtx;
			crStateSetCurrentPointers( newCtx, &(cr_server.current) );
			retVal = MAGIC_OFFSET + ctxPos;

			/* Fix up viewport & scissor */
			cr_server.context[ctxPos]->viewport.viewportW = cr_server.muralWidth;
			cr_server.context[ctxPos]->viewport.viewportH = cr_server.muralHeight;
			cr_server.context[ctxPos]->viewport.scissorW = cr_server.muralWidth;
			cr_server.context[ctxPos]->viewport.scissorH = cr_server.muralHeight;

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
	static GLboolean firstCall = GL_TRUE;
	int ctxPos;
	CRContext *ctx;

	if (context >= 0 && window >= 0) {
		ctxPos = context - MAGIC_OFFSET;
		CRASSERT(ctxPos >= 0);
		CRASSERT(ctxPos < CR_MAX_CONTEXTS);

		/* Update the state tracker's current context */
		ctx = cr_server.context[ctxPos];

		crStateSetCurrentPointers( ctx, &(cr_server.current) );
	}
	else {
		ctx = NULL;
		window = -1;
	}

	cr_server.curClient->currentCtx = ctx;
	cr_server.curClient->currentWindow = window;

	/* This is a hack to force updating the 'current' attribs */
	crStateUpdateColorBits();

	/* XXX do we really want to do this here, or wait until we get back to
	 * the SerializeStreams loop?  It seems to work OK here.
	 */
	crStateMakeCurrent( ctx );

	/* check if being made current for first time, update viewport */
	if (ctx &&
		ctx->viewport.outputDims.x1 == 0 &&
		ctx->viewport.outputDims.x2 == 0 &&
		ctx->viewport.outputDims.y1 == 0 &&
		ctx->viewport.outputDims.y2 == 0) {
		ctx->viewport.outputDims.x1 = cr_server.underlyingDisplay[0];
		ctx->viewport.outputDims.x2 = cr_server.underlyingDisplay[2];
		ctx->viewport.outputDims.y1 = cr_server.underlyingDisplay[1];
		ctx->viewport.outputDims.y2 = cr_server.underlyingDisplay[3];
	}

	if (firstCall) {
		/* Since the cr server serialized all incoming contexts/clients into
		 * one output stream of GL commands, we only need to call the head
		 * SPU's MakeCurrent() function once.
		 */
		cr_server.head_spu->dispatch_table.MakeCurrent( window, nativeWindow, cr_server.SpuContext );
		firstCall = GL_FALSE;
	}
}

