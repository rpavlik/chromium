/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_OCCLUSION_H
#define CR_OCCLUSION_H

#include "cr_hash.h"
#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	CRbitvalue	dirty[CR_MAX_BITARRAY];
} CROcclusionBits;


/*
 * Occlusion query object.
 */
typedef struct {
	GLenum target;
	GLuint name;
	GLuint refCount;
	GLuint passedCounter;
	GLboolean active;
	CRbitvalue dirty[CR_MAX_BITARRAY];  /* dirty data or state */
	GLintptrARB dirtyStart, dirtyLength; /* dirty region */
} CROcclusionObject;


typedef struct {
	CRHashTable *objects;
	GLuint currentQueryObject;
} CROcclusionState;


void crStateOcclusionInit(CRContext *ctx);

void crStateOcclusionDestroy(CRContext *ctx);

void crStateOcclusionDiff(CROcclusionBits *bb, CRbitvalue *bitID,
													CRContext *fromCtx, CRContext *toCtx);
void crStateOcclusionSwitch(CROcclusionBits *bb, CRbitvalue *bitID, 
														CRContext *fromCtx, CRContext *toCtx);


#ifdef __cplusplus
}
#endif

#endif /* CR_OCCLUSION_H */
