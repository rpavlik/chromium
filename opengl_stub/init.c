/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */
	
#include <string.h>
#include "cr_error.h"
#include "api_templates.h"
#include "stub.h"

#ifdef WINDOWS
/* Let me cast function pointers to data pointers, I know what I'm doing. */
#pragma warning( disable: 4054 )
#endif

SPUDispatchTable glim;          /* a copy of either glstub or glnative */
SPUDispatchTable glstub;        /* pointers to first SPU's functions */
SPUDispatchTable glnative;      /* pointers to native GLX/WGL functions */
crOpenGLInterface glinterface;  /* GLX/WGL interface functions */
GLboolean haveNativeOpenGL;


static void NativeOpenGLInit( void )
{
	SPUNamedFunctionTable gl_funcs[1000];
	int numFuncs;

	numFuncs = crLoadOpenGL( &glinterface, gl_funcs );

	haveNativeOpenGL = (numFuncs > 0);

	/* XXX call this after context binding */
	numFuncs += crLoadOpenGLExtensions( &glinterface, gl_funcs + numFuncs );

	CRASSERT(numFuncs < 1000);

	crSPUInitDispatch( &glnative, gl_funcs );
	crSPUInitDispatchNops( &glnative );

}


void FakerInit( SPU *spu )
{
	crSPUInitDispatchTable( &glstub );
	crSPUCopyDispatchTable( &glstub, &(spu->dispatch_table) );

	memcpy(&glim, &glstub, sizeof(SPUDispatchTable));

	NativeOpenGLInit();
}



TSDhandle __DispatchTSD;
static GLboolean ThreadSafe = GL_FALSE;


/*
 * This function should be called from MakeCurrent().  It'll detect if
 * we're in a multi-thread situation, and do the right thing for dispatch.
 */
void crCheckMultithread( void )
{
	static unsigned long knownID;
	static GLboolean firstCall = GL_TRUE;

	if (ThreadSafe)
		return;  /* nothing new, nothing to do */

	if (firstCall) {
		knownID = crThreadID();
		firstCall = GL_FALSE;
	}
	else if (knownID != crThreadID()) {
		/* going thread-safe now! */
		ThreadSafe = GL_TRUE;
		memcpy(&glim, &__ThreadsafeDispatch, sizeof(SPUDispatchTable));
	}
}


/*
 * Install the given dispatch table as the table used for all gl* calls.
 */
void crSetDispatch( const SPUDispatchTable *table )
{
	CRASSERT(table);

	/* always set the per-thread dispatch pointer */
	crSetTSD(&__DispatchTSD, (void *) table);
	if (ThreadSafe) {
		/* Do nothing - the thread-safe dispatch functions will call GetTSD()
		 * to get a pointer to the dispatch table, and jump through it.
		 */
	}
	else {
		/* Single thread mode - just install the caller's dispatch table */
		memcpy(&glim, table, sizeof(SPUDispatchTable));
	}
}

