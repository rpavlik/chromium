#ifndef CR_STATE_PIXEL_H
#define CR_STATE_PIXEL_H

#include "cr_glwrapper.h"
#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLint		   rowLength;
	GLint		   skipRows;
	GLint		   skipPixels;
	GLint		   skipAlignment;
	GLint		   alignment;
	GLint		   imageHeight;
	GLint		   skipImages;
	GLboolean	 swapBytes;
	GLboolean	 LSBFirst;
} CRPackState;

typedef struct {
	CRPackState pack;
	CRPackState unpack;
	GLint		   indexShift;
	GLint		   indexOffset;
	GLcolorf	 scale;
	GLfloat		 depthScale;
	GLcolorf	 bias;
	GLfloat		 depthBias;
	GLfloat		 xZoom;
	GLfloat		 yZoom;
} CRPixelState;

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_PIXEL_H */
