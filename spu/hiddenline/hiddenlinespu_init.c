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

SPUFunctions *hiddenlineSPUInit( int id, SPU *child, SPU *super,
		unsigned int context_id,
		unsigned int num_contexts )
{

	(void) super;
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
	crSPUCopyDispatchTable( &(hiddenline_spu.super), &(super->dispatch_table) );
	hiddenlinespuGatherConfiguration( child );

	hiddenline_spu.frame_head = hiddenline_spu.frame_tail = NULL;

	crPackInit( 0 ); /* Certainly don't want to swap bytes */
	hiddenlineProvidePackBuffer();
	crPackFlushFunc( hiddenlineFlush );
	crPackSendHugeFunc( hiddenlineHuge );

	/* We need to track state so that the packer can deal with pixel data */
	crStateInit();
	hiddenline_spu.ctx = crStateCreateContext( &hiddenline_spu.limits );	
	crStateMakeCurrent( hiddenline_spu.ctx );

	hiddenlinespuCreateFunctions();

	/* We prevent the application from disabling this, so we should
	 * be good to go here. */

	hiddenline_spu.super.Enable( GL_DEPTH_TEST );
	hiddenline_spu.super.LineWidth( 5 );

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

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup )
{
	*name = "hiddenline";
	*super = "hlpassthrough";
	*init = hiddenlineSPUInit;
	*self = hiddenlineSPUSelfDispatch;
	*cleanup = hiddenlineSPUCleanup;
	
	return 1;
}
