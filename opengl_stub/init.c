/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */
	
#include <string.h>
#include "cr_error.h"
#include "api_templates.h"
#include "stub.h"

SPUDispatchTable glim;
Stub stub;


#ifdef WINDOWS
/* Let me cast function pointers to data pointers, I know what I'm doing. */
#pragma warning( disable: 4054 )
#endif


static void NativeOpenGLInit( void )
{
	SPUNamedFunctionTable gl_funcs[1000];
	int numFuncs;

	numFuncs = crLoadOpenGL( &stub.wsInterface, gl_funcs );

	stub.haveNativeOpenGL = (numFuncs > 0);

	/* XXX call this after context binding */
	numFuncs += crLoadOpenGLExtensions( &stub.wsInterface, gl_funcs + numFuncs );

	CRASSERT(numFuncs < 1000);

	crSPUInitDispatch( &stub.nativeDispatch, gl_funcs );
	crSPUInitDispatchNops( &stub.nativeDispatch );
}


void stubFakerInit( SPU *spu )
{
	crSPUInitDispatchTable( &stub.spuDispatch );
	crSPUCopyDispatchTable( &stub.spuDispatch, &(spu->dispatch_table) );

	memcpy(&glim, &stub.spuDispatch, sizeof(SPUDispatchTable));

	NativeOpenGLInit();
}



/*
 * This function should be called from MakeCurrent().  It'll detect if
 * we're in a multi-thread situation, and do the right thing for dispatch.
 */
#ifdef CHROMIUM_THREADSAFE
void stubCheckMultithread( void )
{
	static unsigned long knownID;
	static GLboolean firstCall = GL_TRUE;

	if (stub.threadSafe)
		return;  /* nothing new, nothing to do */

	if (firstCall) {
		knownID = crThreadID();
		firstCall = GL_FALSE;
	}
	else if (knownID != crThreadID()) {
		/* going thread-safe now! */
		stub.threadSafe = GL_TRUE;
		memcpy(&glim, &stubThreadsafeDispatch, sizeof(SPUDispatchTable));
	}
}
#endif


/*
 * Install the given dispatch table as the table used for all gl* calls.
 */
void stubSetDispatch( const SPUDispatchTable *table )
{
	CRASSERT(table);

#ifdef CHROMIUM_THREADSAFE
	/* always set the per-thread dispatch pointer */
	crSetTSD(&stub.dispatchTSD, (void *) table);
	if (stub.threadSafe) {
		/* Do nothing - the thread-safe dispatch functions will call GetTSD()
		 * to get a pointer to the dispatch table, and jump through it.
		 */
	}
	else 
#endif
	{
		/* Single thread mode - just install the caller's dispatch table */
		memcpy(&glim, table, sizeof(SPUDispatchTable));
	}
}


