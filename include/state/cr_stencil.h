/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_STENCIL_H
#define CR_STATE_STENCIL_H

#include "cr_glstate.h"
#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLbitvalue dirty[CR_MAX_BITARRAY];
	GLbitvalue enable[CR_MAX_BITARRAY];
	GLbitvalue func[CR_MAX_BITARRAY];
	GLbitvalue op[CR_MAX_BITARRAY];
	GLbitvalue clearValue[CR_MAX_BITARRAY];
	GLbitvalue writeMask[CR_MAX_BITARRAY];
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

void crStateStencilDiff(CRStencilBits *bb, GLbitvalue *bitID, 
		CRStencilState *from, CRStencilState *to);
void crStateStencilSwitch(CRStencilBits *bb, GLbitvalue *bitID, 
		CRStencilState *from, CRStencilState *to);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_STENCIL_H */
