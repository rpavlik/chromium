/* Copyright (c) 2004, Tungsten Graphics, Inc.
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_string.h"
#include "cr_mem.h"
#include "vncspu.h"
#ifdef NETLOGGER
#include <unistd.h>
#endif


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
	vnc_spu.screen_buffer[0] = NULL;
	vnc_spu.screen_buffer[1] = NULL;
	vnc_spu.windowTable = crAllocHashtable();

#ifdef NETLOGGER
	if (vnc_spu.netlogger_url) {
		char *c;

		crDebug("VNC SPU: NetLogger URL: %s", vnc_spu.netlogger_url);
#if 0
		if (vnc_spu.netlogger_url) {
			/* XXX add unlink() wrapper to Cr util package */
			unlink(vnc_spu.netlogger_url);
		}
#endif
		NL_logger_module("vncspu", /* module name */
										 vnc_spu.netlogger_url,
										 NL_LVL_DEBUG, /* logging level */
										 NL_TYPE_APP, /* target type */
										 "" /* terminator */
										 );
		NL_info("vncspu", "spu.program.begin", "");

		vnc_spu.hostname = crAlloc(101);
		crGetHostname(vnc_spu.hostname, 100);
		/* truncate at first dot */
		if ((c = crStrchr(vnc_spu.hostname, '.')))
			*c = 0;
	}
	else {
		crDebug("VNC SPU: NetLogger disabled");
	}
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

static void
free_screenbuffer(ScreenBuffer *b)
{
	if (b->buffer)
		crFree(b->buffer);
	crFree(b);
}

static int
vncSPUCleanup(void)
{
#ifdef NETLOGGER
	if (vnc_spu.netlogger_url) {
		NL_info("vncspu", "spu.program.end", "");
		NL_logger_del();
	}
#endif
	if (vnc_spu.screen_buffer[0])
		free_screenbuffer(vnc_spu.screen_buffer[0]);
	if (vnc_spu.screen_buffer[1])
		free_screenbuffer(vnc_spu.screen_buffer[1]);
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
