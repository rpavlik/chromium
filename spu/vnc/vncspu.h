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
#include "cr_timer.h"
#include "rfblib.h"
#include "async_io.h"

#if defined(HAVE_XCLIPLIST_EXT) || defined(HAVE_VNC_EXT)
#include <X11/Xlib.h>
#endif

#ifdef NETLOGGER
#include "nl_log.h"
#endif


#if defined(HAVE_XCLIPLIST_EXT)
#include <X11/extensions/Xcliplist.h>
#endif
#include "region.h"

#include "buffer.h"


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

	/* Current and previous frame (in terms of window-SwapBuffers) dirty regions.
	 */
	RegionRec currDirtyRegion;
	RegionRec prevDirtyRegion;

	/* Current and previous (in terms of ScreenBuffer swapping) dirty regions.
	 */
	RegionRec accumDirtyRegion;
	RegionRec prevAccumDirtyRegion;

	GLint width, height;
	GLint xPos, yPos;
	RegionRec clipRegion;
	GLint clippingHash;

	GLboolean isClear;  /* indicates window has only been cleared, no drawing */
	GLuint newSize;
	double prevSwapTime;
	GLint swapCounter;
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
	int use_bounding_boxes;
	int frame_drop;
	int double_buffer;
#ifdef NETLOGGER
	char *netlogger_url;
	char *hostname;
#endif
	int half_rez;

  /** Empty buffers available to the main/app thread */
	ScreenBufferQueue emptyQueue;
	/** Buffers with pixel data waiting to be sent to client */
	ScreenBufferQueue filledQueue;
  /** buffer currently being encoded/sent by VNC server thread */
	ScreenBuffer *serverBuffer;
  /** buffer currently being filled by the main/app thread (glReadPixels) */
	ScreenBuffer *readpixBuffer;																 

	int pixel_size;               /**< 24 or 32 */
	CRHashTable *windowTable;
	WindowInfo *currentWindow;

	/** Lock and condition var for thread stuff */
	CRmutex lock;
	CRcondition cond;

	/** For computing FPS */
	CRTimer *timer;

	/** Debug */
	GLboolean inSwapBuffers;
	CRmutex logLock;

#if defined(HAVE_XCLIPLIST_EXT) || defined(HAVE_VNC_EXT)
	Display *dpy;
	int haveXClipListExt;
#endif
	int haveVncExt;
} VncSPU;


extern int opt_write_coalescing;
extern int opt_force_tight_jpeg;

/** Vnc state descriptor */
extern VncSPU vnc_spu;

/** Named SPU functions */
extern SPUNamedFunctionTable _cr_vnc_table[];

/** Option table for SPU */
extern SPUOptions vncSPUOptions[];

extern void *vnc_main(void *);

extern void vncspuGatherConfiguration( void );

extern void vncspuStartServerThread(void);

extern void vncspuInitialize(void);

extern void vncspuSendVncStartUpMsg(int port);

extern GLboolean vncspuGetDirtyRects(RegionPtr region);
extern GLboolean vncspuWaitDirtyRects(RegionPtr region, const BoxRec *roi,
                                      int serial_no);

extern void vncspuUnlockFrameBuffer(void);

extern void vncspuGetScreenRects(RegionPtr reg);

extern void PrintRegion(const char *s, const RegionPtr r);


extern void
vncspuHalfImage(int origWidth, int origHeight, int origStride,
                const GLubyte *origImage,
                int newStride, GLubyte *newImage,
                GLenum format);


extern void vncspuLog(int threadNum, const char *s, int i);

#endif /* VNC_SPU_H */
