/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "hiddenlinespu.h"
#include "cr_packfunctions.h"
#include <stdio.h>

extern SPUNamedFunctionTable hiddenline_table[];
HiddenlineSPU hiddenline_spu;

SPUFunctions hiddenline_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	hiddenline_table /* THE ACTUAL FUNCTIONS */
};

SPUFunctions *hiddenlineSPUInit( int id, SPU *child, SPU *self,
		unsigned int context_id,
		unsigned int num_contexts )
{

	(void) context_id;
	(void) num_contexts;

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

	hiddenline_spu.frame_head = hiddenline_spu.frame_tail = NULL;

	hiddenline_spu.packer = crPackNewContext( 0 ); /* Certainly don't want to swap bytes */
        crPackSetContext( hiddenline_spu.packer );
	hiddenlineProvidePackBuffer();
	crPackFlushFunc( hiddenline_spu.packer, hiddenlineFlush );
	crPackSendHugeFunc( hiddenline_spu.packer, hiddenlineHuge );

	/* We need to track state so that the packer can deal with pixel data */
	crStateInit();
	hiddenline_spu.ctx = crStateCreateContext( &hiddenline_spu.limits );	
	crStateMakeCurrent( hiddenline_spu.ctx );

	hiddenlinespuCreateFunctions();

	return &hiddenline_functions;
}

void hiddenlineSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(hiddenline_spu.self) );
	crSPUCopyDispatchTable( &(hiddenline_spu.self), self );
}

int hiddenlineSPUCleanup(void)
{
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
