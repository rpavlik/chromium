/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef TEMPLATE_SPU_H
#define TEMPLATE_SPU_H

#ifdef WINDOWS
#define TEMPLATESPU_APIENTRY __stdcall
#else
#define TEMPLATESPU_APIENTRY
#endif

#include "cr_spu.h"
#include "cr_server.h"

/**
 * Template SPU descriptor
 */
typedef struct {
	int id; /**< Spu id */
	int has_child; 	/**< Spu has a child  Not used */
	SPUDispatchTable self, child, super;	/**< SPUDispatchTable self for this SPU, child spu and super spu */
	CRServer *server;	/**< crserver descriptor */
} TemplateSPU;

/** Template state descriptor */
extern TemplateSPU template_spu;

/** Named SPU functions */
extern SPUNamedFunctionTable _cr_template_table[];

/** Option table for SPU */
extern SPUOptions templateSPUOptions[];

extern void templatespuGatherConfiguration( void );


#endif /* TEMPLATE_SPU_H */
