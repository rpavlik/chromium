#ifndef CR_STATE_LINE_H
#define SR_STATE_LINE_H

#include "cr_glwrapper.h"
#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLbitvalue enable;
	GLbitvalue size;
	GLbitvalue width;
	GLbitvalue stipple;
	GLbitvalue dirty;
} CRLineBits;

typedef struct {
	GLboolean	lineSmooth;
	GLboolean	lineStipple;
	GLboolean	pointSmooth;

	GLfloat		pointSize;
	GLfloat		width;
	GLushort	pattern;
	GLint		repeat;
	//GLfloat	aliasedpointsizerange_min;
	//GLfloat aliasedpointsizerange_max;
	//GLfloat	aliasedpointsizegranularity;
	//GLfloat smoothpointsizerange_min;
	//GLfloat smoothpointsizerange_max;
	//GLfloat smoothpointgranularity;
	//GLfloat aliasedlinewidth_min;
	//GLfloat aliasedlinewidth_max;
	//GLfloat aliasedlinegranularity;
	//GLfloat smoothlinewidth_min;
	//GLfloat smoothlinewidth_max;
	//GLfloat smoothlinegranularity;
} CRLineState;

void crStateLineInitBits (CRLineBits *l);
void crStateLineInit (CRLineState *l);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_LINE_H */
