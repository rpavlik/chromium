/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef TILESORT_SPU_H
#define TILESORT_SPU_H

#ifdef WINDOWS
#define TILESORTSPU_APIENTRY __stdcall
#else
#define TILESORTSPU_APIENTRY
#endif

#include "cr_glstate.h"
#include "cr_hash.h"
#include "cr_net.h"
#include "cr_netserver.h"
#include "cr_pack.h"
#include "cr_spu.h"
#include "cr_threads.h"
#include "cr_dlm.h"

#include "state/cr_limits.h"
#include "state/cr_statetypes.h"

#define END_FLUFF 4 /* space for phantom GLEND opcode for splitting */

/*
 * Bucketing / tilesort modes
 */
typedef enum {
	BROADCAST,       /* send all geometry to all servers */
	TEST_ALL_TILES,  /* test bounding box against all tiles */
	UNIFORM_GRID,    /* all columns are equal width, all rows are equal height */
	NON_UNIFORM_GRID,/* columns and rows are of varying width, height */
	RANDOM,          /* randomly bucket geometry */
	WARPED_GRID      /* warped tiles (Karl Rasche) */
} TileSortBucketMode;

typedef struct server_window_info_t ServerWindowInfo;
typedef struct server_context_info_t ServerContextInfo;
typedef struct thread_info_t ThreadInfo;
typedef struct context_info_t ContextInfo;
typedef struct window_info_t WindowInfo;
typedef struct backend_window_info_t BackendWindowInfo;
typedef struct warp_display_info_t WarpDisplayInfo;

struct server_context_info_t {
	CRContext *State;
	GLint serverContext;
	GLint vertexCount;  /* for debug/validation purposes */
};

struct context_info_t {
	GLint id;
	CRContext *State;
	GLint serverContext;   /* returned by server's CreateContext() */
	WindowInfo *currentWindow;
	CRDLMContextState *dlmContext; /* display list manager state */
#ifdef WINDOWS
	HDC client_hdc;
#else
	Display *dpy;
#endif

	ServerContextInfo *server;  /* array [num_servers] of ServerContextInfo */

	/* Misc per-context tilesort state */
	GLenum providedBBOX;  /* GL_OBJECT/SCREEN/DEFAULT_BBOX_CR */
	GLboolean inDrawPixels;
	int readPixelsCount;   /* for gathering pieces of glReadPixels image */
	GLboolean everCurrent; /* has this context ever been bound? */
	GLboolean validRasterOrigin;
};

/* For DMX */
struct backend_window_info_t {
#ifndef WINDOWS
	GLXDrawable xwin;     /* backend server's X window */
	GLXDrawable xsubwin;  /* child of xwin, clipped to screen bounds */
	Display *dpy;
#endif
	CRrecti visrect; /* visible rect, in front-end screen coords */
};

struct server_window_info_t {
	int window;           /* window number on server */
	int num_extents;
	CRrecti extents[CR_MAX_EXTENTS];
	/* warped grid */
	int display_ndx[CR_MAX_EXTENTS];
	GLfloat world_extents[CR_MAX_EXTENTS][8]; /* x1, y1, x2, y2, x3, y3, ... */
};

struct window_info_t {
	GLint id;                    /* Chromium window number */
	GLint lastX, lastY;          /* last known position for the window */
	GLint lastWidth, lastHeight; /* last known size for the window */

	GLint visBits;               /* CR_RGB_BIT | CR_DOUBLE_BIT, etc */

	BackendWindowInfo *backendWindows;       /* array [num_servers] */

	/* tilesort stuff */
	GLint muralWidth, muralHeight;  /* may not match window size */
	TileSortBucketMode bucketMode;
	float widthScale, heightScale;  /* mural size / window size */
	float viewportCenterX, viewportCenterY;
	float halfViewportWidth, halfViewportHeight;
	void *bucketInfo;          /* private to tilesortspu_bucket.c */

	ServerWindowInfo *server;  /* array [num_servers] of ServerWindowInfo */

	GLboolean validRasterOrigin;
	GLboolean newBackendWindows;

#ifdef WINDOWS
	HWND client_hwnd;
#else
	Display *dpy;
	GLXDrawable xwin;
#endif
};

struct warp_display_info_t {
	GLint id, width, height;
	GLint num_tiles;
	GLfloat correct[9];		
	GLfloat correct_inv[9];
};

typedef struct {
	GLboolean    isLoop;
	GLint        numRestore;
	GLint        wind;
	CRVertex     vtx[3];
	unsigned char *beginOp, *beginData;
} TileSortSPUPinchState;

struct thread_info_t {
	CRPackBuffer geometry_buffer;
	CRPackContext *packer;
	ContextInfo *currentContext;
	TileSortSPUPinchState pinchState;
	int state_server_index;           /* only used during __doFlush() */

	/* Array of network connections to the servers */
	CRNetServer *net;    /* array net[num_servers] */

	/* Array of buffers for packing state changes for servers.
	 * Usually just used during state differencing.
	 */
	CRPackBuffer *buffer;  /* array buffer[num_servers] */
};

typedef struct {
	/* SPU stuff */
	int id;
	SPUDispatchTable self;

	/* for display lists */
	SPUDispatchTable packerDispatch; /* for display list uploads */
	SPUDispatchTable stateDispatch;  /* for display list state reconciliation */

	/* Threads */
	int numThreads;
	ThreadInfo thread[MAX_THREADS];

	/* Contexts */
	CRHashTable *contextTable;

	/* Windows */
	CRHashTable *windowTable; /* map SPU window IDs to WindowInfo */

	/* config options */
	int splitBeginEnd;
	int drawBBOX;
	float bboxLineWidth;
	float bboxScale;
	int syncOnFinish, syncOnSwap;
	int scaleToMuralSize;
	int localTileSpec;
	int emit_GATHER_POST_SWAPBUFFERS;
	int useDMX;
	int retileOnResize;
	int autoDListBBoxes;
	int lazySendDLists;
	TileSortBucketMode defaultBucketMode;
	unsigned int fakeWindowWidth, fakeWindowHeight;

	int swap;  /* byte swapping */
	unsigned int MTU;
	unsigned int buffer_size;
	unsigned int geom_buffer_size;
	unsigned int geom_buffer_mtu;
	int num_servers;

	/* WGL/GLX interface for DMX */
	crOpenGLInterface ws;

	/* warped display   XXX this might be per-window state */
	WarpDisplayInfo *displays;

	CRLimitsState limits;  /* OpenGL limits computed from children */

	CRDLM *dlm;  /* Display list manager */
} TileSortSPU;

typedef struct {
	CRbitvalue hits[CR_MAX_BITARRAY];
	GLvectorf  screenMin;
	GLvectorf  screenMax;
	GLvectorf  objectMin;
	GLvectorf  objectMax;
	CRrecti    pixelBounds;
} TileSortBucketInfo;


extern TileSortSPU tilesort_spu;


#ifdef CHROMIUM_THREADSAFE
extern CRmutex _TileSortMutex;
extern CRtsd _ThreadTSD;
#define GET_THREAD(T) ThreadInfo *T = crGetTSD(&_ThreadTSD)
#else
#define GET_THREAD(T) ThreadInfo *T = &(tilesort_spu.thread[0])
#endif

#define GET_CONTEXT(C)			\
	GET_THREAD(thread);		\
	CRContext *C = thread->currentContext->State


/* tilesortspu.c */
void tilesortspuCreateFunctions( void );

/* tilesortspu_config.c */
void tilesortspuGatherConfiguration( const SPU *child_spu );

/* tilesortspu_net.c */
void tilesortspuConnectToServers( void );

/* tilesortspu_flush.c */
void tilesortspuSendServerBuffer( int server_index );
void tilesortspuSendServerBufferThread( int server_index, ThreadInfo *thread );
void tilesortspuHuge( CROpcode opcode, void *buf );
void tilesortspuFlush( ThreadInfo *thread );
void tilesortspuFlush_callback( void *arg );
void tilesortspuBroadcastGeom( int send_state_anyway );
void tilesortspuShipBuffers( void );
void tilesortspuDebugOpcodes( CRPackBuffer *buffer );

/* tilesortspu_diffapi.c */
void tilesortspuCreateDiffAPI( void );

/* tilesortspu_pinch.c */
void tilesortspuPinch( void );
void tilesortspuPinchRestoreTriangle( void );

/* tilesortspu_beginend.c */
void TILESORTSPU_APIENTRY tilesortspu_Begin(GLenum prim);
void TILESORTSPU_APIENTRY tilesortspu_End(void);

/* tilesortspu_eval.c */
void tilesortspuInitEvaluators(void);

/* tilesortspu_tiles.c */
void tilesortspuGetTilingFromServers(CRConnection *conn, WindowInfo *winInfo);
void tilesortspuGetNewTiling(WindowInfo *winInfo);
void tilesortspuComputeMaxViewport( WindowInfo *winInfo );
void tilesortspuSendTileInfoToServers( WindowInfo *winInfo );

/* tilesortspu_bucket.c */
void tilesortspuSetBucketingBounds( WindowInfo *winInfo, int x, int y, unsigned int w, unsigned int h );
void tilesortspuBucketingInit( WindowInfo *winInfo );
void tilesortspuBucketGeometry(WindowInfo *winInfo, TileSortBucketInfo *info);

/* tilesortspu_window.c */
WindowInfo *tilesortspuCreateWindowInfo(GLint window, GLint visBits);
WindowInfo *tilesortspuGetWindowInfo(GLint window, GLint nativeWindow);
void tilesortspuFreeWindowInfo(WindowInfo *winInfo);
void tilesortspuUpdateWindowInfo(WindowInfo *winInfo);
void tilesortspuGetBackendWindowInfo(WindowInfo *winInfo);

/* tilesortspu_context.c */
void tilesortspuInitThreadPacking( ThreadInfo *thread );

/* tilesortspu_misc.c */
void TILESORTSPU_APIENTRY tilesortspu_ChromiumParametervCR(GLenum target, GLenum type, GLsizei count, const GLvoid *values);

/* tilesortspu_get.c */
const GLubyte *tilesortspuGetExtensionsString(void);

/* tilesortspu_list_gen.c */
void tilesortspuLoadSortTable(SPUDispatchTable *t);
void tilesortspuLoadStateTable(SPUDispatchTable *t);
void tilesortspuLoadPackTable(SPUDispatchTable *t);

/* tilesortspu_lists.c */
void tilesortspuStateCallList(GLuint list);
void tilesortspuStateCallLists(GLsizei n, GLenum type, const GLvoid *lists);

/* tilesortspu_pixels.c */
void TILESORTSPU_APIENTRY
tilesortspu_PackBitmap(GLsizei width, GLsizei height,
											 GLfloat xorig, GLfloat yorig,
											 GLfloat xmove, GLfloat ymove,
											 const GLubyte * bitmap);
void TILESORTSPU_APIENTRY
tilesortspu_PackDrawPixels(GLsizei width, GLsizei height, GLenum format,
													 GLenum type, const GLvoid *pixels);
void TILESORTSPU_APIENTRY
tilesortspu_PackTexImage1D(GLenum target, GLint level,
													 GLint internalformat, GLsizei width, GLint border,
													 GLenum format, GLenum type, const GLvoid * pixels);
void TILESORTSPU_APIENTRY
tilesortspu_PackTexImage2D(GLenum target, GLint level,
													 GLint internalformat, GLsizei width, GLsizei height,
													 GLint border,
													 GLenum format, GLenum type, const GLvoid * pixels);
void TILESORTSPU_APIENTRY
tilesortspu_PackTexImage3D(GLenum target, GLint level,
													 GLint internalformat, GLsizei width, GLsizei height,
													 GLsizei depth, GLint border,
													 GLenum format, GLenum type, const GLvoid * pixels);
void TILESORTSPU_APIENTRY
tilesortspu_PackTexImage3DEXT(GLenum target, GLint level,
															GLenum internalformat, GLsizei width,
															GLsizei height, GLsizei depth, GLint border,
															GLenum format, GLenum type,
															const GLvoid * pixels);
void TILESORTSPU_APIENTRY
tilesortspu_PackTexSubImage1D(GLenum target, GLint level, GLint xoffset,
															GLsizei width, GLenum format, GLenum type,
															const GLvoid *pixels);
void TILESORTSPU_APIENTRY
tilesortspu_PackTexSubImage2D(GLenum target, GLint level, GLint xoffset,
															GLint yoffset, GLsizei width, GLsizei height,
															GLenum format, GLenum type,
															const GLvoid *pixels);
void TILESORTSPU_APIENTRY
tilesortspu_PackTexSubImage3D(GLenum target, GLint level, GLint xoffset,
															GLint yoffset, GLint zoffset, GLsizei width,
															GLsizei height, GLsizei depth, GLenum format,
															GLenum type, const GLvoid *pixels);


#endif /* TILESORT_SPU_H */
