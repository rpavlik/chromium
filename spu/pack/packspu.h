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
#include "cr_threads.h"
#include "state/cr_client.h"

typedef struct thread_info_t ThreadInfo;
typedef struct context_info_t ContextInfo;

struct thread_info_t {
	unsigned long id;
	CRNetServer server;
	CRPackBuffer buffer;
	CRPackBuffer normBuffer;
	CRPackBuffer BeginEndBuffer;
	GLenum BeginEndMode;
	int BeginEndState;
	ContextInfo *currentContext;
	CRPackContext *packer;
	int writeback;
};

struct buffer_object_t {
	GLvoid *mappedBuffer;
	GLuint mappedSize;
	GLenum access;
	GLenum usage;
};

struct context_info_t {
	CRContext *clientState;  /* used to store client-side GL state */
	GLint serverCtx;         /* context ID returned by server */
	char glVersion[100];     /* GL_VERSION string */
};

typedef struct {
	int id;
	int swap;
	int emit_GATHER_POST_SWAPBUFFERS;
	int ReadPixels;

	char *name;
	int buffer_size;

	int numThreads;
	ThreadInfo thread[MAX_THREADS];

	int numContexts;
	ContextInfo context[CR_MAX_CONTEXTS];
} PackSPU;

extern PackSPU pack_spu;

#ifdef CHROMIUM_THREADSAFE
extern CRmutex _PackMutex;
extern CRtsd _PackTSD;
#define GET_THREAD(T)  ThreadInfo *T = crGetTSD(&_PackTSD)
#else
#define GET_THREAD(T)  ThreadInfo *T = &(pack_spu.thread[0])
#endif

#define GET_CONTEXT(C)                      \
  GET_THREAD(thread);                       \
  ContextInfo *C = thread->currentContext

extern void packspuCreateFunctions( void );
extern void packspuGatherConfiguration( const SPU *child_spu );
extern void packspuConnectToServer( CRNetServer *server );
extern void packspuFlush( void *arg );
extern void packspuHuge( CROpcode opcode, void *buf );

extern ThreadInfo *packspuNewThread( unsigned long id );

extern GLint PACKSPU_APIENTRY packspu_CreateContext( const char *dpyName, GLint visual );
extern void PACKSPU_APIENTRY packspu_MakeCurrent( GLint window, GLint nativeWindow, GLint ctx );


#endif /* CR_PACKSPU_H */
