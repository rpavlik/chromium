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

void packspuCreateFunctions( void );
void packspuGatherConfiguration( void );
void packspuConnectToServer( void );
void packspuFlush( void *arg );
void packspuHuge( CROpcode opcode, void *buf );

typedef struct {
	int id;

	CRNetServer server;
	CRPackBuffer buffer;

	int swap;

	CRContext *ctx;
} PackSPU;

extern PackSPU pack_spu;

#endif /* CR_PACKSPU_H */
