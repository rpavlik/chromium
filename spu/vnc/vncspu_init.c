/* Copyright (c) 2004, Tungsten Graphics, Inc.
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "vncspu.h"

/** Vnc SPU descriptor */ 
VncSPU vnc_spu;

/** SPU functions */
static SPUFunctions vnc_functions = {
	NULL, /**< CHILD COPY */
	NULL, /**< DATA */
	_cr_vnc_table /**< THE ACTUAL FUNCTIONS - pointer to NamedFunction table */
};

/**
 * Vnc spu init function
 * \param id
 * \param child
 * \param self
 * \param context_id
 * \param num_contexts
 */
static SPUFunctions *
vncSPUInit( int id, SPU *child, SPU *self,
								 unsigned int context_id,
								 unsigned int num_contexts )
{

	(void) self;
	(void) context_id;
	(void) num_contexts;

	vnc_spu.id = id;
	vnc_spu.has_child = 0;
	vnc_spu.server = NULL;
	if (child)
	{
		crSPUInitDispatchTable( &(vnc_spu.child) );
		crSPUCopyDispatchTable( &(vnc_spu.child), &(child->dispatch_table) );
		vnc_spu.has_child = 1;
	}
	crSPUInitDispatchTable( &(vnc_spu.super) );
	crSPUCopyDispatchTable( &(vnc_spu.super), &(self->superSPU->dispatch_table) );
	vncspuGatherConfiguration();
	vnc_spu.screen_buffer = NULL;
	vnc_spu.windowTable = crAllocHashtable();

#ifdef NETLOGGER
	NL_logger_module("vncspu", /* module name */
									 NULL, /* destination URL (use NL_DEST env var) */
									 NL_LVL_DEBUG, /* logging level */
									 NL_TYPE_APP, /* target type */
									 "" /* terminator */
									 );
	NL_info("vncspu", "program.begin", "");
#endif

	vncspuInitialize();

	vncspuStartServerThread();

	return &vnc_functions;
}

static void
vncSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(vnc_spu.self) );
	crSPUCopyDispatchTable( &(vnc_spu.self), self );

	vnc_spu.server = (CRServer *)(self->server);
}

static int
vncSPUCleanup(void)
{
#ifdef NETLOGGER
	NL_info("vncspu", "program.start", "", 1.0);
	NL_logger_del();
#endif
	return 1;
}

int
SPULoad( char **name, char **super, SPUInitFuncPtr *init,
				 SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
				 SPUOptionsPtr *options, int *flags )
{
	*name = "vnc";
	*super = "render";
	*init = vncSPUInit;
	*self = vncSPUSelfDispatch;
	*cleanup = vncSPUCleanup;
	*options = vncSPUOptions;
	*flags = (SPU_NO_PACKER|SPU_TERMINAL_MASK|SPU_MAX_SERVERS_ZERO);
	
	return 1;
}
