/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_GRABBERSPU_H
#define CR_GRABBERSPU_H

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define GRABBER_APIENTRY __stdcall
#elif defined(DARWIN)
#include <AGL/AGL.h>
#define GRABBER_APIENTRY
#else
#include <GL/glx.h>
#define GRABBER_APIENTRY
#endif
#include "cr_threads.h"
#include "cr_spu.h"
#include "cr_hash.h"
#include "cr_server.h"

/**
 * state info
 */
typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super;
	CRServer *server;

	/** config options */
	/*@{*/
	char *currentWindowAttributeName;
	/*@}*/

	CRConnection *mothershipConnection;
} GrabberSPU;

extern GrabberSPU grabber_spu;
extern SPUNamedFunctionTable _cr_grabber_table[];
extern SPUOptions grabberSPUOptions[];

extern void grabberGatherConfiguration( GrabberSPU *spu );

#endif /* CR_GRABBERSPU_H */
