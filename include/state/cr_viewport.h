/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_VIEWPORT_H
#define CR_STATE_VIEWPORT_H

#include "cr_glwrapper.h"
#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLbitvalue dirty;
	GLbitvalue v_dims;
	GLbitvalue s_dims;
	GLbitvalue enable;
	GLbitvalue depth;
} CRViewportBits;

typedef struct {
	/* Viewport state */
	GLint viewportX;
	GLint viewportY;
	GLint viewportW;
	GLint viewportH;
	GLclampd nearClip;
	GLclampd farClip;
	GLboolean viewportValid;

	GLint maxViewportDimsWidth, maxViewportDimsHeight;

	/* Scissor state */
	GLboolean	scissorTest;
	GLint		scissorX;
	GLint		scissorY;
	GLsizei		scissorW;
	GLsizei		scissorH;
	GLboolean   scissorValid;
} CRViewportState;

void crStateViewportInit(CRViewportState *);

void crStateViewportApply( CRViewportState *v, GLvectorf *p );
void crStateViewportMakeCurrent(CRViewportState *v, CRViewportBits *vb);

void crStateViewportDiff(CRViewportBits *bb, GLbitvalue bitID, 
		CRViewportState *from, CRViewportState *to);
void crStateViewportSwitch(CRViewportBits *bb, GLbitvalue bitID, 
		CRViewportState *from, CRViewportState *to);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_VIEWPORT_H */
