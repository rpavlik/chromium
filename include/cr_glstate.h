/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_GLSTATE_H
#define CR_GLSTATE_H

#include "state/cr_extensions.h"

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

#include "state/cr_attrib.h"

#include "state/cr_statefuncs.h"
#include "state/cr_stateerror.h"

#include "spu_dispatch_table.h"

#define CR_MAX_EXTENTS 256

typedef struct {
	CRAttribBits    attrib;
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

typedef void (*CRStateFlushFunc)( void *arg );

typedef struct CRContext {
	int id;
	GLbitvalue bitid;
	GLbitvalue neg_bitid;
	GLbitvalue update;

	CRStateFlushFunc flush_func;
	void            *flush_arg;

	CRAttribState    attrib;
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
#define GLUPDATE_ATTRIB     0x20000 

extern CRContext *__currentContext;
extern CRStateBits *__currentBits;
extern SPUDispatchTable diff_api;

#define GetCurrentContext() __currentContext
#define GetCurrentBits() __currentBits

void crStateInit(void);
CRContext *crStateCreateContext(void);
void crStateMakeCurrent(CRContext *ctx);

void crStateFlushFunc( CRStateFlushFunc ff );
void crStateFlushArg( void *arg );
void crStateDiffAPI( SPUDispatchTable *api );

void crStateSetCurrentPointers( CRContext *ctx, CRCurrentStatePointers *current );

void crStateDiffContext( CRContext *from, CRContext *to );
void crStateSwitchContext( CRContext *from, CRContext *to );

#endif /* CR_GLSTATE_H */
