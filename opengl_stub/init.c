/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */
	
#include <string.h>
#include "cr_error.h"
#include "cr_spu.h"
#include "api_templates.h"

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
