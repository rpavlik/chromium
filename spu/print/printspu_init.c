/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "cr_error.h"
#include "printspu.h"
#include <stdio.h>

extern SPUNamedFunctionTable _cr_print_table[];

static SPUFunctions print_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	_cr_print_table /* THE ACTUAL FUNCTIONS */
};

PrintSpu print_spu;

static SPUFunctions *
printSPUInit( int id, SPU *child, SPU *self,
							unsigned int context_id,
							unsigned int num_contexts )
{
	(void) context_id;
	(void) num_contexts;
	(void) child;

	print_spu.id = id;
	printspuGatherConfiguration( child );

	crSPUInitDispatchTable( &(print_spu.passthrough) );
	crSPUCopyDispatchTable( &(print_spu.passthrough), &(self->superSPU->dispatch_table) );
	return &print_functions;
}

static void
printSPUSelfDispatch(SPUDispatchTable *parent)
{
	(void)parent;
}

static int
printSPUCleanup(void)
{
	if (print_spu.fp != stderr || print_spu.fp != stdout)
		fclose(print_spu.fp);
	return 1;
}

extern SPUOptions printSPUOptions[];

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options, int *flags )
{
	*name = "print";
	*super = "passthrough";
	*init = printSPUInit;
	*self = printSPUSelfDispatch;
	*cleanup = printSPUCleanup;
	*options = printSPUOptions;
	*flags = (SPU_NO_PACKER|SPU_NOT_TERMINAL|SPU_MAX_SERVERS_ZERO);
	
	return 1;
}
