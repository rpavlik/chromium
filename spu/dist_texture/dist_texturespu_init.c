/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "dist_texturespu.h"
#include <stdio.h>

Dist_textureSPU dist_texture_spu;

static SPUFunctions dist_texture_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	_cr_dist_texture_table /* THE ACTUAL FUNCTIONS */
};

static SPUFunctions *dist_textureSPUInit( int id, SPU *child, SPU *self,
		unsigned int context_id,
		unsigned int num_contexts )
{

	(void) context_id;
	(void) num_contexts;

	dist_texture_spu.id = id;
	dist_texture_spu.has_child = 0;
	if (child)
	{
		crSPUInitDispatchTable( &(dist_texture_spu.child) );
		crSPUCopyDispatchTable( &(dist_texture_spu.child), &(child->dispatch_table) );
		dist_texture_spu.has_child = 1;
	}
	crSPUInitDispatchTable( &(dist_texture_spu.super) );
	crSPUCopyDispatchTable( &(dist_texture_spu.super), &(self->superSPU->dispatch_table) );
	dist_texturespuGatherConfiguration();

	return &dist_texture_functions;
}

static void dist_textureSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(dist_texture_spu.self) );
	crSPUCopyDispatchTable( &(dist_texture_spu.self), self );
}

static int dist_textureSPUCleanup(void)
{
	return 1;
}

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options, int *flags )
{
	*name = "dist_texture";
	*super = "passthrough";
	*init = dist_textureSPUInit;
	*self = dist_textureSPUSelfDispatch;
	*cleanup = dist_textureSPUCleanup;
	*options = dist_textureSPUOptions;
	*flags = (SPU_NO_PACKER|SPU_NOT_TERMINAL|SPU_MAX_SERVERS_ZERO);
	
	return 1;
}
