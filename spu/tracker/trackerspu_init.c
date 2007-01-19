/* 
 * Copyright (c) 2006  Michael Duerig  
 * Bern University of Applied Sciences
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 */

/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <math.h>
#include <stdio.h>

#include "cr_spu.h"

#include "trackerspu.h"
#include "trackerspu_udp.h"
#include "cr_mem.h"
#include "cr_matrix.h"


/** Tracker SPU descriptor */ 
TrackerSPU tracker_spu;

/** SPU functions */
static SPUFunctions tracker_functions = {
	NULL, /**< CHILD COPY */
	NULL, /**< DATA */
	_cr_tracker_table /**< THE ACTUAL FUNCTIONS - pointer to NamedFunction table */
};

/**
 * Tracker spu init function
 * \param id
 * \param child
 * \param self
 * \param context_id
 * \param num_contexts
 */
static SPUFunctions *
trackerSPUInit( int id, SPU *child, SPU *self,
								 unsigned int context_id,
								 unsigned int num_contexts )
{
	(void) self;
	(void) context_id;
	(void) num_contexts;

	tracker_spu.id = id;
	tracker_spu.has_child = 0;
	tracker_spu.server = NULL;
	if (child)
	{
		crSPUInitDispatchTable( &(tracker_spu.child) );
		crSPUCopyDispatchTable( &(tracker_spu.child), &(child->dispatch_table) );
		tracker_spu.has_child = 1;
	}
	crSPUInitDispatchTable( &(tracker_spu.super) );
	crSPUCopyDispatchTable( &(tracker_spu.super), &(self->superSPU->dispatch_table) );
	trackerspuGatherConfiguration();

  trackerspuStartUDPServer();

  crDebug("Tracker SPU successfully initialized");
	return &tracker_functions;
}

static void
trackerSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(tracker_spu.self) );
	crSPUCopyDispatchTable( &(tracker_spu.self), self );

	tracker_spu.server = (CRServer *)(self->server);
}

static int
trackerSPUCleanup(void)
{
  trackerspuStopUDPServer();
  crFree(tracker_spu.listenIP);
  crFree(tracker_spu.screens);
	return 1;
}

int
SPULoad( char **name, char **super, SPUInitFuncPtr *init,
				 SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
				 SPUOptionsPtr *options, int *flags )
{
	*name = "tracker";
	*super = "passthrough";
	*init = trackerSPUInit;
	*self = trackerSPUSelfDispatch;
	*cleanup = trackerSPUCleanup;
	*options = trackerSPUOptions;
	*flags = (SPU_NO_PACKER|SPU_NOT_TERMINAL|SPU_MAX_SERVERS_ZERO);
	
	return 1;
}
