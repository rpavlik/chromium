#ifndef CR_STATE_POLYGON_H
#define CR_STATE_POLYGON_H

#include "cr_glwrapper.h"
#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLbitvalue enable;
	GLbitvalue offset;
	GLbitvalue mode;
	GLbitvalue stipple;
	GLbitvalue dirty;
} CRPolygonBits;

typedef struct {
	GLboolean	polygonSmooth;
	GLboolean polygonOffset;
	GLboolean	polygonStipple;
	GLboolean cullFace;
	GLfloat		offsetFactor;
	GLfloat		offsetUnits;
	GLenum		cullFaceMode;
	GLenum		frontFace;
	GLenum		frontMode;
	GLenum		backMode;
	GLint		  stipple[32];
} CRPolygonState;

void crStatePolygonInitBits(CRPolygonBits *p);
void crStatePolygonInit(CRPolygonState *p);

#ifdef __cplusplus
}
#endif

#endif
