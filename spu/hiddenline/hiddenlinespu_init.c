/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "hiddenlinespu.h"
#include "cr_packfunctions.h"
#include <stdio.h>

extern SPUNamedFunctionTable _cr_hiddenline_table[];
HiddenlineSPU hiddenline_spu;

#ifdef CHROMIUM_THREADSAFE
CRtsd _HiddenlineTSD;
#endif

static SPUFunctions hiddenline_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	_cr_hiddenline_table /* THE ACTUAL FUNCTIONS */
};

static SPUFunctions *hiddenlineSPUInit( int id, SPU *child, SPU *self,
		unsigned int context_id,
		unsigned int num_contexts )
{

	(void) context_id;
	(void) num_contexts;

#ifdef CHROMIUM_THREADSAFE
	crDebug("Hiddenline SPU: thread-safe");
#endif

	hiddenline_spu.id = id;
	hiddenline_spu.has_child = 0;
	if (child)
	{
		crSPUInitDispatchTable( &(hiddenline_spu.child) );
		crSPUCopyDispatchTable( &(hiddenline_spu.child), &(child->dispatch_table) );
		hiddenline_spu.has_child = 1;
	}
	crSPUInitDispatchTable( &(hiddenline_spu.super) );
	crSPUCopyDispatchTable( &(hiddenline_spu.super), &(self->superSPU->dispatch_table) );
	hiddenlinespuGatherConfiguration( child );

	/* We need to track state so that the packer can deal with pixel data */
	crStateInit();

	hiddenlinespuCreateFunctions();

	hiddenline_spu.contextTable = crAllocHashtable();
#ifndef CHROMIUM_THREADSAFE
	hiddenline_spu.currentContext = NULL;
#endif
	crInitMutex(&(hiddenline_spu.mutex));

	return &hiddenline_functions;
}

static void hiddenlineSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(hiddenline_spu.self) );
	crSPUCopyDispatchTable( &(hiddenline_spu.self), self );
}

static int hiddenlineSPUCleanup(void)
{
	crFreeHashtable(hiddenline_spu.contextTable);
	return 1;
}

extern SPUOptions hiddenlineSPUOptions[];


int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options, int *flags )
{
	*name = "hiddenline";
	*super = "hlpassthrough";
	*init = hiddenlineSPUInit;
	*self = hiddenlineSPUSelfDispatch;
	*cleanup = hiddenlineSPUCleanup;
	*options = hiddenlineSPUOptions;
	*flags = (SPU_NO_PACKER|SPU_NOT_TERMINAL|SPU_MAX_SERVERS_ZERO);
	
	return 1;
}
