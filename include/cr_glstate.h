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

#include "state/cr_buffer.h"
#include "state/cr_bufferobject.h"
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
#include "state/cr_multisample.h"
#include "state/cr_occlude.h"
#include "state/cr_pixel.h"
#include "state/cr_point.h"
#include "state/cr_polygon.h"
#include "state/cr_program.h"
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

/**
 * Bit vectors describing GL state
 */
typedef struct {
	CRAttribBits      attrib;
	CRBufferBits      buffer;
#ifdef CR_ARB_vertex_buffer_object
	CRBufferObjectBits bufferobject;
#endif
	CRClientBits      client;
	CRCurrentBits     current;
	CREvaluatorBits   eval;
	CRFeedbackBits    feedback;
	CRFogBits         fog;
	CRHintBits        hint;
	CRLightingBits    lighting;
	CRLineBits        line;
	CRListsBits       lists;
	CRMultisampleBits multisample;
#if CR_ARB_occlusion_query
	CROcclusionBits   occlusion;
#endif
	CRPixelBits       pixel;
	CRPointBits	  point;
	CRPolygonBits     polygon;
	CRProgramBits     program;
	CRRegCombinerBits regcombiner;
	CRSelectionBits	  selection;
	CRStencilBits     stencil;
	CRTextureBits     texture;
	CRTransformBits   transform;
	CRViewportBits    viewport;
} CRStateBits;

typedef void (*CRStateFlushFunc)( void *arg );


typedef struct _CRSharedState {
	CRHashTable *textureTable;  /* all texture objects */
	CRHashTable *dlistTable;    /* all display lists */
	GLint refCount;
} CRSharedState;


/**
 * Chromium version of the state variables in OpenGL
 */
struct CRContext {
	int id;
	CRbitvalue bitid[CR_MAX_BITARRAY];
	CRbitvalue neg_bitid[CR_MAX_BITARRAY];

	CRSharedState *shared;

	GLenum     renderMode;

	GLenum     error;

	CRStateFlushFunc flush_func;
	void            *flush_arg;

	CRAttribState      attrib;
	CRBufferState      buffer;
#ifdef CR_ARB_vertex_buffer_object
	CRBufferObjectState bufferobject;
#endif
	CRClientState      client;
	CRCurrentState     current;
	CREvaluatorState   eval;
	CRExtensionState   extensions;
	CRFeedbackState    feedback;
	CRFogState         fog;
	CRHintState        hint;
	CRLightingState    lighting;
	CRLimitsState      limits;
	CRLineState        line;
	CRListsState       lists;
	CRMultisampleState multisample;
#if CR_ARB_occlusion_query
	CROcclusionState   occlusion;
#endif
	CRPixelState       pixel;
	CRPointState       point;
	CRPolygonState     polygon;
	CRProgramState     program;
	CRRegCombinerState regcombiner;
	CRSelectionState   selection;
	CRStencilState     stencil;
	CRTextureState     texture;
	CRTransformState   transform;
	CRViewportState    viewport;

	/** For buffering vertices for selection/feedback */
	/*@{*/
	GLuint    vCount;
	CRVertex  vBuffer[4];
	GLboolean lineReset;
	GLboolean lineLoop;
	/*@}*/
};


void crStateInit(void);
CRContext *crStateCreateContext(const CRLimitsState *limits, GLint visBits, CRContext *share);
void crStateMakeCurrent(CRContext *ctx);
void crStateSetCurrent(CRContext *ctx);
CRContext *crStateGetCurrent(void);
void crStateDestroyContext(void *ctx);

void crStateFlushFunc( CRStateFlushFunc ff );
void crStateFlushArg( void *arg );
void crStateDiffAPI( SPUDispatchTable *api );
void crStateUpdateColorBits( void );

void crStateSetCurrentPointers( CRContext *ctx, CRCurrentStatePointers *current );

void crStateSetExtensionString( CRContext *ctx, const GLubyte *extensions );

void crStateDiffContext( CRContext *from, CRContext *to );
void crStateSwitchContext( CRContext *from, CRContext *to );


   /* XXX move these! */

void STATE_APIENTRY
crStateChromiumParameteriCR( GLenum target, GLint value );

void STATE_APIENTRY
crStateChromiumParameterfCR( GLenum target, GLfloat value );

void STATE_APIENTRY
crStateChromiumParametervCR( GLenum target, GLenum type, GLsizei count, const GLvoid *values );

void STATE_APIENTRY
crStateGetChromiumParametervCR( GLenum target, GLuint index, GLenum type,
                                GLsizei count, GLvoid *values );

void STATE_APIENTRY
crStateReadPixels( GLint x, GLint y, GLsizei width, GLsizei height,
                   GLenum format, GLenum type, GLvoid *pixels );

#ifdef __cplusplus
}
#endif

#endif /* CR_GLSTATE_H */
