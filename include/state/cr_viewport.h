/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_VIEWPORT_H
#define CR_STATE_VIEWPORT_H

#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLbitvalue dirty[CR_MAX_BITARRAY];
	GLbitvalue v_dims[CR_MAX_BITARRAY];
	GLbitvalue s_dims[CR_MAX_BITARRAY];
	GLbitvalue enable[CR_MAX_BITARRAY];
	GLbitvalue depth[CR_MAX_BITARRAY];
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

	/* Scissor state */
	GLboolean	scissorTest;
	GLint		scissorX;
	GLint		scissorY;
	GLsizei		scissorW;
	GLsizei		scissorH;
	GLboolean   scissorValid;

	GLfloat widthScale;
	GLfloat heightScale;
	GLrecti outputDims;

	GLint x_offset;
	GLint y_offset;

} CRViewportState;

void crStateViewportInit(CRViewportState *);

void crStateViewportApply( CRViewportState *v, GLvectorf *p );
void crStateViewportMakeCurrent(CRViewportState *v, CRViewportBits *vb);

void crStateViewportDiff(CRViewportBits *bb, GLbitvalue *bitID, 
		CRViewportState *from, CRViewportState *to);
void crStateViewportSwitch(CRViewportBits *bb, GLbitvalue *bitID, 
		CRViewportState *from, CRViewportState *to);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_VIEWPORT_H */
