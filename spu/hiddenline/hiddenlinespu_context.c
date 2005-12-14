/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */
	
#include "hiddenlinespu.h"
#include "hiddenlinespu_proto.h"
#include "cr_packfunctions.h"
#include "cr_mem.h"

void HIDDENLINESPU_APIENTRY hiddenlinespu_MakeCurrent(GLint crWindow, GLint nativeWindow, GLint ctx)
{
	ContextInfo *context;

	/* look up the context */
	context = crHashtableSearch(hiddenline_spu.contextTable, ctx);

	/* Set current context pointer */
#ifdef CHROMIUM_THREADSAFE
	crSetTSD(&_HiddenlineTSD, context);
#else
	hiddenline_spu.currentContext = context;
#endif

	if (context)
	{
		hiddenlineFlush(NULL);

		/* setup initial buffer for packing commands */
		hiddenlineProvidePackBuffer();
		crPackSetContext( context->packer );
		crStateMakeCurrent( context->ctx );
		hiddenline_spu.super.MakeCurrent(crWindow, nativeWindow, context->super_context);
	}
	else
	{
		crPackSetContext( NULL );
		crStateMakeCurrent( NULL );
		hiddenline_spu.super.MakeCurrent(crWindow, nativeWindow, 0); /* -1? */
	}
}


GLint HIDDENLINESPU_APIENTRY hiddenlinespu_CreateContext( const char *dpyName, GLint visBits, GLint shareCtx )
{
	static int freeContext = 0;  /* unique context id generator */
	ContextInfo *context;

	if (shareCtx > 0) {
		crWarning("Hiddenline SPU: context sharing not implemented.");
		shareCtx = 0;
	}

	/* allocate new ContextInfo */
	context = (ContextInfo *) crCalloc(sizeof(ContextInfo));
	if (!context)
		return 0;

	/* init ContextInfo */
	context->packer = crPackNewContext( 0 ); /* no byte swapping */
	/* context->pack_buffer initialized in hiddenlineProvidePackBuffer() */
	context->ctx = crStateCreateContext( NULL, visBits );
	context->super_context = hiddenline_spu.super.CreateContext(dpyName, visBits, shareCtx);
	context->clear_color.r = 0.0f;
	context->clear_color.g = 0.0f;
	context->clear_color.b = 0.0f;
	context->clear_color.a = 0.0f;
	context->frame_head = NULL;
	context->frame_tail = NULL;

	/* put context into hash table */
	freeContext++;
	crHashtableAdd(hiddenline_spu.contextTable, freeContext, (void *) context);

	context->bufpool = crBufferPoolInit(16);
	crPackFlushFunc(context->packer, hiddenlineFlush);
	crPackSendHugeFunc(context->packer, hiddenlineHuge);

	return freeContext;
}

void HIDDENLINESPU_APIENTRY hiddenlinespu_DestroyContext( GLint ctx )
{
	ContextInfo *context;
	context = crHashtableSearch(hiddenline_spu.contextTable, ctx);
	CRASSERT(context);
	hiddenline_spu.super.DestroyContext(context->super_context);
	crHashtableDelete(hiddenline_spu.contextTable, ctx, crFree);
	crBufferPoolFree(context->bufpool);
}
