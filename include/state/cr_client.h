#ifndef CR_STATE_CLIENT_H
#define CR_STATE_CLIENT_H

#include "cr_glwrapper.h"
#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLbitvalue	dirty;
	GLbitvalue	enableClientState;
	GLbitvalue	clientPointer;
	GLbitvalue  element;
	GLbitvalue  *v;
	GLbitvalue  *n;
	GLbitvalue  *c;
	GLbitvalue  *i;
	GLbitvalue  *t;
	GLbitvalue  *e;
	int valloc;
	int nalloc;
	int calloc;
	int ialloc;
	int talloc;
	int ealloc;
} CRClientBits;

typedef struct {
	unsigned char *p;
	GLint size;
	GLint type;
	GLint stride;
	GLboolean enabled;

	int bytesPerIndex;
} CRClientPointer;

typedef struct {
	CRClientPointer v;
	CRClientPointer n;
	CRClientPointer c;
	CRClientPointer i;
	CRClientPointer t;
	CRClientPointer e;

	int *list;
	int list_alloc;
	int list_size;

	GLint	maxElementsIndices;
	GLint	maxElementsVertices;
} CRClientState;

void crStateClientInitBits(CRClientBits *c);
void crStateClientInit (CRClientState *c);

void crStateClientDiff(CRClientBits *bb, GLbitvalue bitID, 
		CRClientState *from, CRClientState *to);
void crStateClientSwitch(CRClientBits *bb, GLbitvalue bitID, 
		CRClientState *from, CRClientState *to);
#ifdef __cplusplus
}
#endif

#endif /* CR_CLIENT_H */
