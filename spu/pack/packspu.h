/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_PACKSPU_H
#define CR_PACKSPU_H

#ifdef WINDOWS
#define PACKSPU_APIENTRY __stdcall
#else
#define PACKSPU_APIENTRY
#endif

#include "cr_glstate.h"
#include "cr_netserver.h"
#include "cr_pack.h"
#include "cr_spu.h"

#include "state/cr_limits.h"

void packspuCreateFunctions( void );
void packspuGatherConfiguration( const SPU *child_spu );
void packspuConnectToServer( void );
void packspuFlush( void *arg );
void packspuHuge( CROpcode opcode, void *buf );

extern GLint PACKSPU_APIENTRY packspu_CreateContext( void * display, GLint visual );
extern void PACKSPU_APIENTRY packspu_MakeCurrent( void *dpy, GLint draw, GLint ctx );

typedef struct {
	int id;

	CRNetServer server;
	CRPackBuffer buffer;

	int swap;
	int ReadPixels;

	CRLimitsState limits; /* OpenGL limits of receiving unpacker */
	CRContext *currentCtx;  /* used to store client-side GL state */
} PackSPU;

extern PackSPU pack_spu;

#endif /* CR_PACKSPU_H */
