/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_GLSTATE_H
#define CR_GLSTATE_H

/* Forward declaration since some of the state/cr_*.h files need the CRContext type */
struct CRContext;
typedef struct CRContext CRContext;

#include "cr_version.h"
#include "state/cr_extensions.h"

#include "state/cr_buffer.h"
#include "state/cr_client.h"
#include "state/cr_current.h"
#include "state/cr_evaluators.h"
#include "state/cr_feedback.h"
#include "state/cr_fog.h"
#include "state/cr_hint.h"
#include "state/cr_lighting.h"
#include "state/cr_limits.h"
#include "state/cr_line.h"
#include "state/cr_lists.h"
#include "state/cr_pixel.h"
#include "state/cr_point.h"
#include "state/cr_polygon.h"
#include "state/cr_regcombiner.h"
#include "state/cr_stencil.h"
#include "state/cr_texture.h"
#include "state/cr_transform.h"
#include "state/cr_viewport.h"

#include "state/cr_attrib.h"

#include "state/cr_statefuncs.h"
#include "state/cr_stateerror.h"

#include "spu_dispatch_table.h"


#define CR_MAX_EXTENTS 256

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	CRAttribBits    attrib;
	CRBufferBits    buffer;
	CRClientBits    client;
	CRCurrentBits   current;
	CREvaluatorBits eval;
	CRFeedbackBits	feedback;
	CRFogBits       fog;
	CRHintBits      hint;
	CRLightingBits  lighting;
	CRLineBits      line;
	CRListsBits     lists;
	CRPixelBits     pixel;
	CRPointBits	point;
	CRPolygonBits   polygon;
	CRRegCombinerBits regcombiner;
	CRSelectionBits	selection;
	CRStencilBits   stencil;
	CRTextureBits   texture;
	CRTransformBits transform;
	CRViewportBits  viewport;
} CRStateBits;

typedef void (*CRStateFlushFunc)( void *arg );

struct CRContext {
	int id;
	GLbitvalue bitid[CR_MAX_BITARRAY];
	GLbitvalue neg_bitid[CR_MAX_BITARRAY];

	GLenum     renderMode;

	GLenum     error;

	CRStateFlushFunc flush_func;
	void            *flush_arg;

	CRAttribState    attrib;
	CRBufferState    buffer;
	CRClientState    client;
	CRCurrentState   current;
	CREvaluatorState eval;
	CRExtensionState extensions;
	CRFeedbackState  feedback;
	CRFogState       fog;
	CRHintState      hint;
	CRLightingState  lighting;
	CRLimitsState    limits;
	CRLineState      line;
	CRListsState     lists;
	CRPixelState     pixel;
	CRPointState	 point;
	CRPolygonState   polygon;
	CRRegCombinerState regcombiner;
	CRSelectionState selection;
	CRStencilState   stencil;
	CRTextureState   texture;
	CRTransformState transform;
	CRViewportState  viewport;
};


#define GLUPDATE_TRANS		0x00001
#define GLUPDATE_PIXEL		0x00002
#define GLUPDATE_CURRENT	0x00004
#define GLUPDATE_VIEWPORT	0x00008
#define GLUPDATE_FOG		0x00010
#define GLUPDATE_TEXTURE	0x00020
#define GLUPDATE_LISTS		0x00040
#define GLUPDATE_CLIENT		0x00080
#define GLUPDATE_BUFFER		0x00100
#define GLUPDATE_HINT		0x00200
#define GLUPDATE_LIGHTING	0x00400
#define GLUPDATE_LINE		0x00800
#define GLUPDATE_POLYGON	0x01000
#define GLUPDATE_STENCIL	0x02000
#define GLUPDATE_EVAL		0x04000
#define GLUPDATE_IMAGING	0x08000
#define GLUPDATE_SELECTION	0x10000
#define GLUPDATE_ATTRIB		0x20000
#define GLUPDATE_REGCOMBINER	0x40000

/*
 * XXX might move these elsewhere someday so that the render SPU is no
 * longer dependent on the state tracker.
 */
extern const char *__stateExtensionString;
extern const char *__stateAppOnlyExtensions;
extern const char *__stateChromiumExtensions;


void crStateInit(void);
CRContext *crStateCreateContext(const CRLimitsState *limits);
void crStateMakeCurrent(CRContext *ctx);
void crStateSetCurrent(CRContext *ctx);
CRContext *crStateGetCurrent(void);
void crStateDestroyContext(CRContext *ctx);

void crStateFlushFunc( CRStateFlushFunc ff );
void crStateFlushArg( void *arg );
void crStateDiffAPI( SPUDispatchTable *api );
void crStateUpdateColorBits( void );

void crStateSetCurrentPointers( CRContext *ctx, CRCurrentStatePointers *current );

void crStateDiffContext( CRContext *from, CRContext *to );
void crStateSwitchContext( CRContext *from, CRContext *to );

#ifdef __cplusplus
}
#endif

#endif /* CR_GLSTATE_H */
