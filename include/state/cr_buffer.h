/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_BUFFER_H
#define CR_STATE_BUFFER_H

#include "cr_glwrapper.h"
#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLbitvalue	dirty;
	GLbitvalue	enable;
	GLbitvalue	alphaFunc;
	GLbitvalue	depthFunc;
	GLbitvalue	blendFunc;
	GLbitvalue	logicOp;
	GLbitvalue	drawBuffer;
	GLbitvalue	readBuffer;
	GLbitvalue	indexMask;
	GLbitvalue	colorWriteMask;
	GLbitvalue	clearColor;
	GLbitvalue	clearIndex;
	GLbitvalue	clearDepth;
	GLbitvalue	clearAccum;
	GLbitvalue	depthMask;
#ifdef CR_EXT_blend_color
	GLbitvalue	blendColor;
#endif
#if defined(CR_EXT_blend_minmax) || defined(CR_EXT_blend_subtract)
	GLbitvalue	blendEquation;
#endif
} CRBufferBits;

typedef struct {
	GLboolean	depthTest;
	GLboolean	blend;
	GLboolean	alphaTest;
	GLboolean	logicOp;
	GLboolean	dither;
	GLboolean	depthMask;

	GLenum		alphaTestFunc;
	GLfloat		alphaTestRef;
	GLenum		depthFunc;
	GLenum		blendSrc;
	GLenum		blendDst;
	GLenum		logicOpMode;
	GLenum		drawBuffer;
	GLenum		readBuffer;
	GLint		indexWriteMask;
	GLcolorb	colorWriteMask;
	GLcolorf	colorClearValue;
	GLfloat 	indexClearValue;
	GLdefault	depthClearValue;
	GLcolorf	accumClearValue;
#ifdef CR_EXT_blend_color
	GLcolorf	blendColor;
#endif
#if defined(CR_EXT_blend_minmax) || defined(CR_EXT_blend_subtract)
	GLenum		blendEquation;
#endif

	/* static config state */
	GLint	auxBuffers;
	GLboolean	rgbaMode;
	GLboolean	indexMode;
	GLboolean	doubleBuffer;
	GLboolean	stereo;
	GLint	subPixelBits;			
	GLint	redBits;
	GLint	greenBits;
	GLint	blueBits;
	GLint	alphaBits;
	GLint	indexBits;
	GLint	depthBits;
	GLint	stencilBits;
	GLint	accumRedBits;
	GLint	accumGreenBits;
	GLint	accumBlueBits;
	GLint	accumAlphaBits;
} CRBufferState;

void crStateBufferInitBits(CRBufferBits *bb);
void crStateBufferInit(CRBufferState *b);

void crStateBufferDiff(CRBufferBits *bb, GLbitvalue bitID, 
		CRBufferState *from, CRBufferState *to);
void crStateBufferSwitch(CRBufferBits *bb, GLbitvalue bitID, 
		CRBufferState *from, CRBufferState *to);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_BUFFER_H */
