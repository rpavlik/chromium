/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef INJECTOR_SPU_H
#define INJECTOR_SPU_H

#ifdef WINDOWS
#define INJECTORSPU_APIENTRY __stdcall
#else
#define INJECTORSPU_APIENTRY
#endif

#include "cr_spu.h"
#include "cr_server.h"
#include "GL/gl.h"

#define INJECTORSPU_OOB_PORT 8194

typedef struct CRMessageOobFrameInfo {
	CRMessageHeader header;
	int             frameNum ;
} CRMessageOobFrameInfo ;

typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super, oob_dispatch;
	CRServer *server;
	CRConnection* oob_conn ;
	char* oob_url ;
	int reading_frame ;
	CRMessageOobFrameInfo info_msg ;
} InjectorSPU;

extern InjectorSPU injector_spu;

extern void injectorspuGatherConfiguration( InjectorSPU* );
extern void INJECTORSPU_APIENTRY injectorspuOOBSwapBuffers( GLint window, GLint i );
extern void INJECTORSPU_APIENTRY injectorspuChromiumParameteri( GLenum param, GLint i );

#endif /* INJECTOR_SPU_H */
