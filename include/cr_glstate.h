#ifndef CR_GLSTATE_H
#define CR_GLSTATE_H

#include "state/cr_buffer.h"
#include "state/cr_current.h"
#include "state/cr_evaluators.h"
#include "state/cr_fog.h"
#include "state/cr_lighting.h"
#include "state/cr_line.h"
#include "state/cr_pixel.h"
#include "state/cr_polygon.h"
#include "state/cr_stencil.h"
#include "state/cr_texture.h"
#include "state/cr_transform.h"

#include "state/cr_statefuncs.h"
#include "state/cr_stateerror.h"

typedef struct {
	CRBufferBits    buffer;
	CRCurrentBits   current;
	CREvaluatorBits eval;
	CRFogBits       fog;
	CRLightingBits  lighting;
	CRLineBits      line;
	CRPixelBits     pixel;
	CRPolygonBits   polygon;
	CRStencilBits   stencil;
	CRTextureBits   texture;
	CRTransformBits transform;
} CRStateBits;

typedef struct {
	int id;
	GLbitvalue bitid;
	GLbitvalue neg_bitid;

	CRBufferState    buffer;
	CRCurrentState   current;
	CREvaluatorState eval;
	CRFogState       fog;
	CRLightingState  lighting;
	CRLineState      line;
	CRPixelState     pixel;
	CRPolygonState   polygon;
	CRStencilState   stencil;
	CRTextureState   texture;
	CRTransformState transform;
} CRContext;

extern CRContext *__currentContext;
extern CRStateBits *__currentBits;
#define GetCurrentContext() __currentContext
#define GetCurrentBits() __currentBits

void crStateInit(void);
CRContext *crStateCreateContext();
void crStateMakeCurrent(CRContext *ctx);

#endif /* CR_GLSTATE_H */
