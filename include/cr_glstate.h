#ifndef CR_GLSTATE_H
#define CR_GLSTATE_H

#include "state/cr_pixel.h"
#include "state/cr_current.h"
#include "state/cr_statefuncs.h"
#include "state/cr_stateerror.h"

typedef struct {
	CRCurrentState current;
	CRPixelState pixel;
} CRContext;

extern CRContext *__currentcontext;
#define GetCurrentContext() __currentcontext

void crStateInit(void);
CRContext *crStateCreateContext();
void crStateMakeCurrent(CRContext *ctx);

#endif /* CR_GLSTATE_H */
