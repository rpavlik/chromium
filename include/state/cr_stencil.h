#ifndef CR_STATE_STENCIL_H
#define CR_STATE_STENCIL_H

#include "cr_glstate.h"
#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLbitvalue dirty;
	GLbitvalue enable;
	GLbitvalue func;
	GLbitvalue op;
	GLbitvalue clearValue;
	GLbitvalue writeMask;
} CRStencilBits;

typedef struct {
	GLboolean	stencilTest;
	GLenum		func;
	GLint		mask;
	GLint		ref;
	GLenum		fail;
	GLenum		passDepthFail;
	GLenum		passDepthPass;
	GLint		clearValue;
	GLint		writeMask;
} CRStencilState;

void crStateStencilInitBits (CRStencilBits *s);
void crStateStencilInit(CRStencilState *s);

void crStateStencilDiff(CRStencilBits *bb, GLbitvalue bitID, 
		CRStencilState *from, CRStencilState *to);
void crStateStencilSwitch(CRStencilBits *bb, GLbitvalue bitID, 
		CRStencilState *from, CRStencilState *to);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_STENCIL_H */
