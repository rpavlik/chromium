/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "cr_environment.h"
#include "cr_string.h"
#include "cr_error.h"
#include "perfspu.h"
#include <fcntl.h>
#include <unistd.h>

extern SPUNamedFunctionTable perf_table[];
perfSPU perf_spu;

SPUFunctions perf_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	perf_table /* THE ACTUAL FUNCTIONS */
};

SPUFunctions *perfSPUInit( int id, SPU *child, SPU *super,
		unsigned int context_id,
		unsigned int num_contexts )
{

	(void) super;
	(void) context_id;
	(void) num_contexts;

	perf_spu.id = id;
	perf_spu.has_child = 0;
	if (child)
	{
		crSPUInitDispatchTable( &(perf_spu.child) );
		crSPUCopyDispatchTable( &(perf_spu.child), &(child->dispatch_table) );
		perf_spu.has_child = 1;
	}
	crSPUInitDispatchTable( &(perf_spu.super) );
	crSPUCopyDispatchTable( &(perf_spu.super), &(super->dispatch_table) );
	perfspuGatherConfiguration();

	if (crStrlen(crGetenv("CR_PERF_LOG_FILE")) > 0)
	    perf_spu.log_file = open( crGetenv("CR_PERF_LOG_FILE"), O_RDWR );

	if (!perf_spu.log_file)
	    crWarning("Couldn't open log file %s, not logging to this file.\n",crGetenv("CR_PERF_LOG_FILE"));

	return &perf_functions;
}

void perfSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(perf_spu.self) );
	crSPUCopyDispatchTable( &(perf_spu.self), self );
}

int perfSPUCleanup(void)
{
	close( perf_spu.log_file );
	return 1;
}

extern SPUOptions perfSPUOptions[];

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options, int *flags )
{
	*name = "perf";
	*super = "passthrough";
	*init = perfSPUInit;
	*self = perfSPUSelfDispatch;
	*cleanup = perfSPUCleanup;
	*options = perfSPUOptions;
	*flags = (SPU_NO_PACKER|SPU_NOT_TERMINAL|SPU_MAX_SERVERS_ZERO);
	
	return 1;
}
