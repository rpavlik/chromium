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

/*
 * We use a linked list of these structures to keep track of available
 * display list IDs.
 */
typedef struct CRListsFreeElem {
	GLuint min;
	GLuint max;
	struct CRListsFreeElem *next;
	struct CRListsFreeElem *prev;
} CRListsFreeElem;

typedef struct {
	GLbitvalue dirty[CR_MAX_BITARRAY];
} CRListsBits;

typedef struct {
	GLboolean newEnd;
	CRListsFreeElem *freeList;
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


void crStateListsInit(CRListsState *l);

void crStateListsDiff(CRListsBits *bb, GLbitvalue *bitID, 
		CRListsState *from, CRListsState *to);
void crStateListsSwitch(CRListsBits *bb, GLbitvalue *bitID, 
		CRListsState *from, CRListsState *to);

#ifdef __cplusplus
}
#endif

#endif
