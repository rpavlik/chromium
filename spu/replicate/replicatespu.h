/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_REPLICATESPU_H
#define CR_REPLICATESPU_H

#ifdef WINDOWS
#define REPLICATESPU_APIENTRY __stdcall
#else
#define REPLICATESPU_APIENTRY
#endif

#include "cr_glstate.h"
#include "cr_netserver.h"
#include "cr_pack.h"
#include "cr_spu.h"
#include "cr_threads.h"
#include "cr_dlm.h"
#include "state/cr_client.h"

#define CR_MAX_REPLICANTS 10

typedef struct thread_info_t ThreadInfo;
typedef struct context_info_t ContextInfo;
typedef struct window_info_t WindowInfo;

struct window_info_t {
	GLint id;
	GLint visBits;

	GLint width, height;

	GLint nativeWindow;
	
	GLboolean viewable;
};

#define CHROMIUM_START_PORT 7000

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
	unsigned int broadcast;
};

struct context_info_t {
	CRContext *State; 
	GLint serverCtx;         /* context ID returned by server */
	GLint visBits;
	GLint currentWindow;
	GLint rserverCtx[CR_MAX_REPLICANTS];
	CRDLM *displayListManager;
	CRDLMContextState *dlmState;
};

typedef struct {
	int id;
	int swap;
	int ReadPixels;

	int has_child;
	int VncEventsBase;
	SPUDispatchTable self;
	SPUDispatchTable diff_dispatch;
	Display *glx_display;
	char dpyName[1000];

	CRNetServer rserver[CR_MAX_REPLICANTS];

	CRHashTable *windowTable;

	int vncAvailable;

	char *name;
	int buffer_size;

	int numThreads;
	ThreadInfo thread[MAX_THREADS];

	int numContexts;
	ContextInfo context[CR_MAX_CONTEXTS];
} ReplicateSPU;

extern ReplicateSPU replicate_spu;

#ifdef CHROMIUM_THREADSAFE_notyet
extern CRmutex _ReplicateMutex;
extern CRtsd _ReplicateTSD;
#define GET_THREAD(T)  ThreadInfo *T = crGetTSD(&_ReplicateTSD)
#else
#define GET_THREAD(T)  ThreadInfo *T = &(replicate_spu.thread[0])
#endif

#define GET_CONTEXT(C)                      \
  GET_THREAD(thread);                       \
  ContextInfo *C = thread->currentContext

extern void replicatespuCreateFunctions( void );
extern void replicatespuGatherConfiguration( const SPU *child_spu );
extern void replicatespuConnectToServer( CRNetServer *server );
extern void replicatespuFlush( void *arg );
extern void replicatespuHuge( CROpcode opcode, void *buf );
extern void replicatespuReplicateCreateContext( int ipaddress );
extern void replicatespuRePositionWindow(WindowInfo *winInfo);
extern void replicatespuCreateDiffAPI( void );

extern ThreadInfo *replicatespuNewThread( unsigned long id );

extern GLint REPLICATESPU_APIENTRY replicatespu_CreateContext( const char *dpyName, GLint visual );
extern void REPLICATESPU_APIENTRY replicatespu_MakeCurrent( GLint window, GLint nativeWindow, GLint ctx );


#endif /* CR_REPLICATESPU_H */
