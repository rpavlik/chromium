/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef READBACK_SPU_H
#define READBACK_SPU_H

#ifdef WINDOWS
#define READBACKSPU_APIENTRY __stdcall
#else
#define READBACKSPU_APIENTRY
#endif

#include "cr_spu.h"
#include "cr_server.h"
#include "cr_threads.h"

#define MAX_WINDOWS 32
#define MAX_CONTEXTS 32

typedef struct {
	unsigned long id;
	int currentContext;
	int currentWindow;
#ifndef WINDOWS
	Display *dpy;
#endif
} ThreadInfo;

typedef struct {
	GLboolean inUse;
	GLint renderWindow;
	GLint childWindow;
	GLint width, height;
	GLint childWidth, childHeight;
	GLubyte *colorBuffer;
	GLvoid *depthBuffer;
	GLint cppColor, cppDepth;
	GLenum depthType;  /* GL_UNSIGNED_SHORT or GL_FLOAT */
	GLenum rgbaFormat; /* GL_RGBA or GL_BGRA */
	GLenum rgbFormat;  /* GL_RGB or GL_BGR */
} WindowInfo;

typedef struct {
	GLboolean inUse;
	GLint renderContext;
	GLint childContext;
	CRContext *tracker;  /* for tracking matrix state */
} ContextInfo;

typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super;
	CRServer *server;

	/* config options */
	int extract_depth;
	int extract_alpha;
	int local_visualization;
	int visualize_depth;
	int resizable;

	char *gather_url;
	int gather_mtu;
	CRConnection *gather_conn;

	WindowInfo windows[MAX_WINDOWS];
	ContextInfo contexts[MAX_CONTEXTS];

#ifndef CHROMIUM_THREADSAFE
	ThreadInfo singleThread;
#endif

	GLint renderWindow;
	GLint renderContext;
	GLint childWindow;
	GLint childContext;

	GLint barrierCount;

	float halfViewportWidth, halfViewportHeight, viewportCenterX, viewportCenterY;
	int cleared_this_frame;
	struct { float xmin, ymin, zmin, xmax, ymax, zmax; } *bbox;

} ReadbackSPU;

#define READBACK_BARRIER        1
#define CREATE_CONTEXT_BARRIER  2
#define MAKE_CURRENT_BARRIER    3
#define DESTROY_CONTEXT_BARRIER 4


extern ReadbackSPU readback_spu;

#ifdef CHROMIUM_THREADSAFE
extern CRtsd _ReadbackTSD;
#define GET_THREAD(T)  ThreadInfo *T = crGetTSD(&_ReadbackTSD)
#else
#define GET_THREAD(T)  ThreadInfo *T = &(readback_spu.singleThread)
#endif


extern void readbackspuGatherConfiguration( ReadbackSPU *spu );

#endif /* READBACK_SPU_H */
