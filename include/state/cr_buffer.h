#ifndef CR_STATE_BUFFER_H
#define CR_STATE_BUFFER_H

#include "cr_glwrapper.h"
#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLbitvalue	dirty;
	GLbitvalue  enable;
	GLbitvalue	alphaFunc;
	GLbitvalue	depthFunc;
	GLbitvalue	blendFunc;
	GLbitvalue	blendColor;
	GLbitvalue	logicOp;
	GLbitvalue	drawBuffer;
	GLbitvalue	readBuffer;
	GLbitvalue	indexMask;
	GLbitvalue	colorWriteMask;
	GLbitvalue	clearColor;
	GLbitvalue	clearIndex;
	GLbitvalue	clearDepth;
	GLbitvalue	clearAccum;
	GLbitvalue  depthMask;
} CRBufferBits;

typedef struct {
	GLboolean	depthTest;
	GLboolean	blend;
	GLboolean	alphaTest;
	GLboolean	logicOp;
	GLboolean	dither;
	GLboolean depthMask;

	GLenum		alphaTestFunc;
	GLfloat		alphaTestRef;
	GLenum		depthFunc;
	GLenum		blendSrc;
	GLenum		blendDst;
	GLcolorf	blendColor;	
	GLenum		logicOpMode;
	GLenum		drawBuffer;
	GLenum		readBuffer;
	GLint     indexWriteMask;
	GLcolorb	colorWriteMask;
	GLcolorf	colorClearValue;
	GLfloat 	indexClearValue;
	GLdefault	depthClearValue;
	GLcolorf	accumClearValue;

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
	GLint	depthBits;
	GLint	stencilBits;
	GLint	accumredBits;
	GLint	accumgreenBits;
	GLint	accumblueBits;
	GLint	accumalphaBits;

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
