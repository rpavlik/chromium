/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef EXPANDO_SPU_H
#define EXPANDO_SPU_H

#ifdef WINDOWS
#define EXPANDOSPU_APIENTRY __stdcall
#else
#define EXPANDOSPU_APIENTRY
#endif

#include "cr_spu.h"
#include "cr_server.h"

typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super;
	CRServer *server;

	/* Expando-specific variables */
	CRHashTable *contextTable;
} ExpandoSPU;

extern ExpandoSPU expando_spu;

extern SPUNamedFunctionTable _cr_expando_table[];

extern SPUOptions expandoSPUOptions[];

extern void expandospuGatherConfiguration( void );

extern void expando_free_dlm_context(void *data);


#endif /* EXPANDO_SPU_H */
