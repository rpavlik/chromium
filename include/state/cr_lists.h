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
	GLint	maxListNesting;
	GLboolean newEnd;
	CRListsFreeElem *freeList;
	GLuint base;
} CRListsState;

void crStateListsInit(CRListsState *l);

#ifdef __cplusplus
}
#endif

#endif
