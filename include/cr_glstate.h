#ifndef CR_GLSTATE_H
#define CR_GLSTATE_H

#include "state/cr_buffer.h"
#include "state/cr_client.h"
#include "state/cr_current.h"
#include "state/cr_evaluators.h"
#include "state/cr_fog.h"
#include "state/cr_lighting.h"
#include "state/cr_line.h"
#include "state/cr_lists.h"
#include "state/cr_pixel.h"
#include "state/cr_polygon.h"
#include "state/cr_stencil.h"
#include "state/cr_texture.h"
#include "state/cr_transform.h"
#include "state/cr_viewport.h"

#include "state/cr_statefuncs.h"
#include "state/cr_stateerror.h"

typedef struct {
	CRBufferBits    buffer;
	CRClientBits    client;
	CRCurrentBits   current;
	CREvaluatorBits eval;
	CRFogBits       fog;
	CRLightingBits  lighting;
	CRLineBits      line;
	CRListsBits     lists;
	CRPixelBits     pixel;
	CRPolygonBits   polygon;
	CRStencilBits   stencil;
	CRTextureBits   texture;
	CRTransformBits transform;
	CRViewportBits  viewport;
} CRStateBits;

typedef struct {
	int id;
	GLbitvalue bitid;
	GLbitvalue neg_bitid;

	CRBufferState    buffer;
	CRClientState    client;
	CRCurrentState   current;
	CREvaluatorState eval;
	CRFogState       fog;
	CRLightingState  lighting;
	CRLineState      line;
	CRListsState     lists;
	CRPixelState     pixel;
	CRPolygonState   polygon;
	CRStencilState   stencil;
	CRTextureState   texture;
	CRTransformState transform;
	CRViewportState  viewport;
} CRContext;

extern CRContext *__currentContext;
extern CRStateBits *__currentBits;
#define GetCurrentContext() __currentContext
#define GetCurrentBits() __currentBits

void crStateInit(void);
CRContext *crStateCreateContext();
void crStateMakeCurrent(CRContext *ctx);

#endif /* CR_GLSTATE_H */
