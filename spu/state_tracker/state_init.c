/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_glstate.h"
#include "cr_mem.h"
#include "cr_error.h"
#include <stdio.h>
#include <memory.h>

// To increase this, we need a more general bitvector representation.
#define CR_MAX_CONTEXTS 32

CRContext *__currentContext = NULL;
CRStateBits *__currentBits = NULL;

CRContext *__hwcontext = NULL;

void crStateInit(void)
{
	__currentBits = (CRStateBits *) crAlloc( sizeof( *__currentBits) );
	memset( __currentBits, 0, sizeof( *__currentBits ) );

  crStateClientInitBits( &(__currentBits->client) );
  crStateLightingInitBits( &(__currentBits->lighting) );
  crStateTransformInitBits( &(__currentBits->transform) );
}

GLbitvalue g_availableContexts = 0xFFFFFFFF;

static CRContext *crStateCreateContextId(int i)
{
	CRContext *ctx = (CRContext *) crAlloc( sizeof( *ctx ) );
	ctx->id = i;
	ctx->flush_func = NULL;
	ctx->bitid = (1<<i);
	ctx->neg_bitid = ~(ctx->bitid);
	ctx->update = GLBITS_ONES;

	crDebug( "Creating a context: %d (0x%x)", ctx->id, (int)ctx->bitid );
	crStateBufferInit( &(ctx->buffer) );
	crStateClientInit (&(ctx->client) );
	crStateCurrentInit( &(ctx->current) );
	crStateEvaluatorInit( &(ctx->eval) );
	crStateFogInit( &(ctx->fog) );
	crStateLightingInit( &(ctx->lighting) );
	crStateLineInit( &(ctx->line) );
	crStateListsInit (&(ctx->lists) );
	crStatePixelInit( &(ctx->pixel) );
	crStatePolygonInit (&(ctx->polygon) );
	crStateStencilInit( &(ctx->stencil) );
	crStateTextureInit( &(ctx->texture) );
	crStateTransformInit( &(ctx->transform) );
	crStateViewportInit (&(ctx->viewport) );
	
	// This has to come last.
	crStateAttribInit( &(ctx->attrib) );
	return ctx;
}

CRContext *crStateCreateContext(void)
{
	int i;
	int id = 1;
	
	for (i = 0 ; i < CR_MAX_CONTEXTS ; i++)
	{
		if (id & g_availableContexts)
		{
			g_availableContexts ^= id; // it's no longer available
			return crStateCreateContextId( i );
		}
		id <<= 1;
	}
	crError( "Out of available contexts in crStateCreateContexts (max %d)", CR_MAX_CONTEXTS );
	// NOTREACHED
	return NULL;
}

void crStateMakeCurrent( CRContext *ctx )
{
	if (__currentContext == ctx) return;
	__currentContext = ctx;

#if 0
	if (ctx && !ctx->viewport.viewportValid && !ctx->viewport.scissorValid)
	{
		crStateViewportMakeCurrent( &(ctx->viewport), &(__currentBits->viewport) );
	}
#endif

	if (__hwcontext && ctx)
	{
		crStateSwitchContext( __hwcontext, ctx );	
	}

	if (ctx)
	{
		__hwcontext = ctx;
	}
}
