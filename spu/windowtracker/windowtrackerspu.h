/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef WINDOWTRACKER_SPU_H
#define WINDOWTRACKER_SPU_H

#ifdef WINDOWS
#define WINDOWTRACKERSPU_APIENTRY __stdcall
#else
#define WINDOWTRACKERSPU_APIENTRY
#endif

#include "cr_spu.h"
#include "cr_server.h"


/**
 * Windowtracker SPU descriptor
 */
typedef struct {
	int id; /**< Spu id */
	int has_child; 	/**< Spu has a child  Not used */
	SPUDispatchTable self, child, super;	/**< SPUDispatchTable self for this SPU, child spu and super spu */
	CRServer *server;	/**< crserver descriptor */

	/* config options */
	char *display;
	char *window_title;

#if defined(GLX)
	Display *dpy;
	Window win;
#endif

} WindowtrackerSPU;

/** Windowtracker state descriptor */
extern WindowtrackerSPU windowtracker_spu;

/** Named SPU functions */
extern SPUNamedFunctionTable _cr_windowtracker_table[];

/** Option table for SPU */
extern SPUOptions windowtrackerSPUOptions[];

extern void windowtrackerspuGatherConfiguration( void );


#endif /* WINDOWTRACKER_SPU_H */
