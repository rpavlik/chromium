/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_LISTS_H
#define CR_STATE_LISTS_H

#include "cr_glwrapper.h"
#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CRListsFreeElem {
	GLuint min;
	GLuint max;
	struct CRListsFreeElem *next;
	struct CRListsFreeElem *prev;
} CRListsFreeElem;

typedef struct {
	GLbitvalue dirty;
} CRListsBits;

typedef struct {
	GLboolean newEnd;
	CRListsFreeElem *freeList;
	GLuint base;
} CRListsState;

void crStateListsInit(CRListsState *l);

void crStateListsDiff(CRListsBits *bb, GLbitvalue bitID, 
		CRListsState *from, CRListsState *to);
void crStateListsSwitch(CRListsBits *bb, GLbitvalue bitID, 
		CRListsState *from, CRListsState *to);

#ifdef __cplusplus
}
#endif

#endif
