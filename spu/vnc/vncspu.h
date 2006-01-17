/* Copyright (c) 2004, Tungsten Graphics, Inc.
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef VNC_SPU_H
#define VNC_SPU_H

#ifdef WINDOWS
#define VNCSPU_APIENTRY __stdcall
#else
#define VNCSPU_APIENTRY
#endif

#include "cr_hash.h"
#include "cr_spu.h"
#include "cr_server.h"
#include "cr_threads.h"
#include "rfblib.h"
#include "async_io.h"

#if defined(HAVE_XCLIPLIST_EXT) || defined(HAVE_VNC_EXT)
#include <X11/Xlib.h>
#endif

#ifdef NETLOGGER
#include "nl_log.h"
#endif


/**
 * We have a range of ports that we try to use.
 * Need this to support multiple, simultaneous SPUs running on one host.
 */
#define FIRST_SERVER_PORT  5905
#define NUM_SERVER_PORTS     10


/**
 * Per window info
 */
typedef struct {
	GLint id;             /* my id */
	GLint nativeWindow;
	GLuint frameCounter;
} WindowInfo;


/**
 * Vnc SPU descriptor
 */
typedef struct {
	int id; /**< Spu id */
	int has_child; 	/**< Spu has a child  Not used */
	SPUDispatchTable self, child, super;	/**< SPUDispatchTable self for this SPU, child spu and super spu */
	CRServer *server;	/**< crserver descriptor */

	/* config options */
	int server_port;
	int screen_width, screen_height;
	int max_update_rate;
	char display_string[1000];

	GLubyte *screen_buffer;
	int pixel_size;               /* 24 or 32 */
	CRHashTable *windowTable;
	WindowInfo *currentWindow;

	CRmutex lock;
	CRcondition cond;
#if defined(HAVE_XCLIPLIST_EXT) || defined(HAVE_VNC_EXT)
	Display *dpy;
	int haveXClipListExt;
#endif
	int haveVncExt;
} VncSPU;

/** Vnc state descriptor */
extern VncSPU vnc_spu;

/** Named SPU functions */
extern SPUNamedFunctionTable _cr_vnc_table[];

/** Option table for SPU */
extern SPUOptions vncSPUOptions[];

extern void vncspuGatherConfiguration( void );

extern void vncspuStartServerThread(void);

extern void vncspuInitialize(void);

extern void vncspuSendVncStartUpMsg(int port);

#endif /* VNC_SPU_H */
