/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "state.h"
#include "cr_mem.h"
#include "cr_error.h"
#include "cr_spu.h"
#include <stdio.h>

#ifdef CHROMIUM_THREADSAFE
CRtsd __contextTSD;
#else
CRContext *__currentContext = NULL;
#endif

CRStateBits *__currentBits = NULL;
GLboolean g_availableContexts[CR_MAX_CONTEXTS];

static CRContext *defaultContext = NULL;



/*
 * Helper for crStateCreateContext, below.
 */
static CRContext *crStateCreateContextId(int i, const CRLimitsState *limits)
{
	CRContext *ctx = (CRContext *) crAlloc( sizeof( *ctx ) );
	int j;
	int node32 = i >> 5;
	int node = i & 0x1f;

	ctx->id = i;
	ctx->flush_func = NULL;
	for (j=0;j<CR_MAX_BITARRAY;j++){
		if (j == node32) {
			ctx->bitid[j] = (1 << node);
		} else {
			ctx->bitid[j] = 0;
		}
		ctx->neg_bitid[j] = ~(ctx->bitid[j]);
	}

	crDebug( "Creating a context (id=%d addr=%p)", ctx->id, ctx);

	/* This has to come first. */
	if (limits) {
		/* use provided OpenGL limits */
		crSPUCopyGLLimits( &(ctx->limits), limits);
	}
	else {
		/* use Chromium's OpenGL defaults */
		crSPUInitGLLimits( &(ctx->limits) );
	}

	crStateBufferInit( &(ctx->buffer) );
	crStateClientInit (&(ctx->limits), &(ctx->client) );
	crStateCurrentInit( &(ctx->limits), &(ctx->current) );
	crStateEvaluatorInit( &(ctx->eval) );
	crStateExtensionsInit( ctx );
	crStateFogInit( &(ctx->fog) );
	crStateHintInit( &(ctx->hint) );
	crStateLightingInit( &(ctx->lighting) );
	crStateLineInit( &(ctx->line) );
	crStateListsInit (&(ctx->lists) );
	crStatePixelInit( &(ctx->pixel) );
	crStatePolygonInit (&(ctx->polygon) );
	crStatePointInit (&(ctx->point) );
	crStateRegCombinerInit (&(ctx->regcombiner) );
	crStateStencilInit( &(ctx->stencil) );
	crStateTextureInit( (&ctx->limits), &(ctx->texture) );
	crStateTransformInit( &(ctx->limits), &(ctx->transform) );
	crStateViewportInit (&(ctx->viewport) );
	
	/* This has to come last. */
	crStateAttribInit( &(ctx->attrib) );

	return ctx;
}


/*
 * Allocate the state (dirty) bits data structures.
 * This should be called before we create any contexts.
 * We'll also create the default/NULL context at this time and make
 * it the current context by default.  This means that if someone
 * tries to set GL state before calling MakeCurrent() they'll be
 * modifying the default state object, and not segfaulting on a NULL
 * pointer somewhere.
 */
void crStateInit(void)
{
	int j;

	if (!defaultContext) {
		/* Allocate the default/NULL context */
		defaultContext = crStateCreateContextId(0, NULL);
		CRASSERT(g_availableContexts[0] == 0);
		g_availableContexts[0] = 1; /* in use forever */
	}

#ifdef CHROMIUM_THREADSAFE
	crSetTSD(&__contextTSD, defaultContext);
#else
	if (!__currentContext) {
		__currentContext = defaultContext;
	}
#endif

	__currentBits = (CRStateBits *) crCalloc( sizeof(CRStateBits) );
	crStateClientInitBits( &(__currentBits->client) );
	crStateLightingInitBits( &(__currentBits->lighting) );
	crStateTransformInitBits( &(__currentBits->transform) );

	for (j=0;j<CR_MAX_CONTEXTS;j++)
		g_availableContexts[j] = 0;
}



/*
 * Notes on context switching and the "default context".
 *
 * See the paper "Tracking Graphics State for Networked Rendering"
 * by Ian Buck, Greg Humphries and Pat Hanrahan for background
 * information about how the state tracker and context switching
 * works.
 *
 * When we make a new context current, we call crStateSwitchContext()
 * in order to transform the 'from' context into the 'to' context
 * (i.e. the old context to the new context).  The transformation
 * is accomplished by calling GL functions through the 'diff_api'
 * so that the downstream GL machine (represented by the __currentContext
 * structure) is updated to reflect the new context state.  Finally, 
 * we point __currentContext to the new context.
 *
 * A subtle problem we have to deal with is context destruction.
 * This issue arose while testing with Glean.  We found that when
 * the currently bound context was getting destroyed that state
 * tracking was incorrect when a subsequent new context was activated.
 * In DestroyContext, the __hwcontext was being set to NULL and effectively
 * going away.  Later in MakeCurrent we had no idea what the state of the
 * downstream GL machine was (since __hwcontext was gone).  This meant
 * we had nothing to 'diff' against and the downstream GL machine was
 * in an unknown state.
 *
 * The solution to this problem is the "default/NULL" context.  The
 * default context is created the first time CreateContext is called
 * and is never freed.  Whenever we get a crStateMakeCurrent(NULL) call
 * or destroy the currently bound context in crStateDestroyContext()
 * we call crStateSwitchContext() to switch to the default context and
 * then set the __currentContext pointer to point to the default context.
 * This ensures that the dirty bits are updated and the diff_api functions
 * are called to keep the downstream GL machine in a known state.
 * Finally, the __hwcontext variable is no longer needed now.
 *
 * Yeah, this is kind of a mind-bender, but it really solves the problem
 * pretty cleanly.
 *
 * -Brian
 */


CRContext *crStateCreateContext(const CRLimitsState *limits)
{
	int i;

	if (!defaultContext) {
		/* Allocate the default/NULL context.  We really shouldn't
		 * have to do this here, but we're playing it safe.
		 */
		defaultContext = crStateCreateContextId(0, limits);
		CRASSERT(g_availableContexts[0] == 0);
		g_availableContexts[0] = 1; /* in use forever */
	}

	for (i = 0 ; i < CR_MAX_CONTEXTS ; i++)
	{
		if (!g_availableContexts[i])
		{
			g_availableContexts[i] = 1; /* it's no longer available */
			return crStateCreateContextId( i, limits );
		}
	}
	crError( "Out of available contexts in crStateCreateContexts (max %d)",
					 CR_MAX_CONTEXTS );
	/* never get here */
	return NULL;
}


void crStateDestroyContext( CRContext *ctx )
{
	CRContext *current = GetCurrentContext();

	if (current == ctx) {
		/* destroying the current context - have to be careful here */
		CRASSERT(defaultContext);
		crStateSwitchContext(current, defaultContext);
#ifdef CHROMIUM_THREADSAFE
		crSetTSD(&__contextTSD, defaultContext);
#else
		__currentContext = defaultContext;
#endif
	}
	g_availableContexts[ctx->id] = 0;
	crFree( ctx );
}


void crStateMakeCurrent( CRContext *ctx )
{
	CRContext *current = GetCurrentContext();

	if (ctx == NULL)
		ctx = defaultContext;

	if (current == ctx)
		return; /* no-op */

	CRASSERT(ctx);

	if (current)
		crStateSwitchContext( current, ctx );

#ifdef CHROMIUM_THREADSAFE
	crSetTSD(&__contextTSD, ctx);
#else
	__currentContext = ctx;
#endif
}


/*
 * As above, but don't call crStateSwitchContext().
 */
void crStateSetCurrent( CRContext *ctx )
{
	CRContext *current = GetCurrentContext();

	if (ctx == NULL)
		ctx = defaultContext;

	if (current == ctx)
		return; /* no-op */

	CRASSERT(ctx);

#ifdef CHROMIUM_THREADSAFE
	crSetTSD(&__contextTSD, ctx);
#else
	__currentContext = ctx;
#endif
}


void crStateUpdateColorBits(void)
{
	/* This is a hack to force updating the 'current' attribs */
	CRStateBits *sb = GetCurrentBits();
	FILLDIRTY(sb->current.dirty);
	FILLDIRTY(sb->current.color);
}
