/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef APIHISTOGRAM_SPU_H
#define APIHISTOGRAM_SPU_H

#ifdef WINDOWS
#define APIHISTOGRAMSPU_APIENTRY __stdcall
#else
#define APIHISTOGRAMSPU_APIENTRY
#endif

#include "cr_spu.h"
#include "cr_server.h"

/**
 * Apihistogram SPU descriptor
 */
typedef struct {
	int id; /**< Spu id */
	int has_child; 	/**< Spu has a child  Not used */
	SPUDispatchTable self, child, super;	/**< SPUDispatchTable self for this SPU, child spu and super spu */
	CRServer *server;	/**< crserver descriptor */

	FILE *fp;
} ApihistogramSPU;

/** Apihistogram state descriptor */
extern ApihistogramSPU apihistogram_spu;

/** Named SPU functions */
extern SPUNamedFunctionTable _cr_apihistogram_table[];

/** Option table for SPU */
extern SPUOptions apihistogramSPUOptions[];

extern void apihistogramspuGatherConfiguration( void );

extern void apihistogramspuInitCounts(void);

extern void apihistogramspuPrintReport(FILE *fp);


#endif /* APIHISTOGRAM_SPU_H */
