/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_LISTS_H
#define CR_STATE_LISTS_H

#include "cr_hash.h"
#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
	CRbitvalue dirty[CR_MAX_BITARRAY];
} CRListsBits;

typedef struct {
	GLboolean newEnd;
	GLuint base;
	GLuint currentIndex;  /* list being built */
	GLenum mode;
	CRHashTable *hash;  /* map display list IDs to CRListEffect structs */
} CRListsState;

/*
 * This structure records the state changes that take place while
 * executing a display list.  At this time we're only tracking changes
 * to the raster pos in order to solve problems with glRasterPos. glBitmap
 * and display lists with the tilesort SPU.
 * The hash table maps display list IDs to these structures.
 */
typedef struct {
	GLfloat rasterPosDx, rasterPosDy; /* from glBitmap */
} CRListEffect;


void crStateListsInit(CRContext *ctx);
void crStateListsDestroy(CRContext *ctx);

void crStateListsDiff(CRListsBits *bb, CRbitvalue *bitID,
                      CRContext *fromCtx, CRContext *toCtx);
void crStateListsSwitch(CRListsBits *bb, CRbitvalue *bitID, 
                        CRContext *fromCtx, CRContext *toCtx);

#ifdef __cplusplus
}
#endif

#endif
