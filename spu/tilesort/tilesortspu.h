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
#include "cr_mem.h"

#include "state/cr_limits.h"
#include "state/cr_statetypes.h"

#define END_FLUFF 4 /* space for phantom GLEND opcode for splitting */

/** bitflags for stereo purposes */
#define EYE_LEFT   0x1
#define EYE_RIGHT  0x2

/**
 * Stereo modes
 */
typedef enum {
	NONE,
	PASSIVE,
	CRYSTAL,
	SIDE_BY_SIDE,
	ANAGLYPH
} StereoMode;

/**
 *  Glasses filter types
 */
typedef enum {
	RED_BLUE,
	RED_GREEN,
	RED_CYAN,
	BLUE_RED,
	GREEN_RED,
	CYAN_RED
} GlassesType;

/**
 * Which view/projection matrices do we use?
 */
typedef enum {
	MATRIX_SOURCE_APP = 0,   /**< application's matrices */
	MATRIX_SOURCE_SERVERS,   /**< matrices from servers */
	MATRIX_SOURCE_CONFIG     /**< matrices from tilesort config options */
} MatrixSource;


/**
 * Bucketing / tilesort modes
 */
typedef enum {
	BROADCAST,       /**< send all geometry to all servers */
	TEST_ALL_TILES,  /**< test bounding box against all tiles */
	UNIFORM_GRID,    /**< all columns are equal width, all rows are equal height */
	NON_UNIFORM_GRID,/**< columns and rows are of varying width, height */
	RANDOM,          /**< randomly bucket geometry */
	WARPED_GRID,     /**< warped tiles (Karl Rasche) */
	FRUSTUM          /**< test 3D bounding box against the frustum */
} TileSortBucketMode;

typedef struct server_window_info_t ServerWindowInfo;
typedef struct server_context_info_t ServerContextInfo;
typedef struct thread_info_t ThreadInfo;
typedef struct context_info_t ContextInfo;
typedef struct window_info_t WindowInfo;
typedef struct backend_window_info_t BackendWindowInfo;
typedef struct warp_display_info_t WarpDisplayInfo;

/**
 * Server context info
 */
struct server_context_info_t {
	CRContext *State;
	GLint serverContext;
	GLint vertexCount;  /**< for debug/validation purposes */
};

/**
 * Tilesorter context info
 */
struct context_info_t {
	GLint id;
	CRContext *State;
	GLint serverContext;   /**< returned by server's CreateContext() */
	WindowInfo *currentWindow;
	CRDLMContextState *dlmContext; /* display list manager state */
	GLenum displayListMode;
	GLuint displayListIdentifier;
#ifdef WINDOWS
	HDC client_hdc;
#elif defined(Darwin)
	AGLContext context;
#else
	Display *dpy;
#endif

	ServerContextInfo *server;  /**< array [num_servers] of ServerContextInfo */

	/**
	 * \name Misc per-context tilesort state
	 */
	/*@{*/
	GLenum providedBBOX;  /**< GL_OBJECT/SCREEN/DEFAULT_BBOX_CR */
	GLboolean inDrawPixels;
	GLboolean inZPix;
	int readPixelsCount;   /**< for gathering pieces of glReadPixels image */
	GLboolean everCurrent; /**< has this context ever been bound? */
	GLboolean validRasterOrigin;
	char glVersion[50];
	int stereoDestFlags;   /**< mask of EYE_LEFT/EYE_RIGHT set by glDrawBuffer */
	GLboolean perspectiveProjection; /**< is current proj matrix perspective?*/
	/*@}*/
};

/**
 * For DMX
 */
struct backend_window_info_t {
#ifdef GLX
	GLXDrawable xwin;     /**< backend server's X window */
	GLXDrawable xsubwin;  /**< child of xwin, clipped to screen bounds */
	Display *dpy;
#endif
	CRrecti visrect; /**< visible rect, in front-end screen coords */
};

struct server_window_info_t {
	int window;           /**< window number on server */
	int num_extents;
	CRrecti extents[CR_MAX_EXTENTS];
	int eyeFlags;   /**< bitmask of EYE_LEFT, EYE_RIGHT, for passive stereo */
	/**
	 * \name per-server viewing and projection matrices (for non-planar tilesort) 
	 */
	/*@{*/
	CRmatrix viewMatrix[2]; /**< 0=left, 1=right */
	CRmatrix projectionMatrix[2]; /**< 0=left, 1=right */
 	/*@}*/
	/**
	 *  \name warped grid
	 */
	/*@{*/
	int display_ndx[CR_MAX_EXTENTS];
	GLfloat world_extents[CR_MAX_EXTENTS][8]; /**< x1, y1, x2, y2, x3, y3, ... */
 	/*@}*/
};

/**
 * Window info
 */
struct window_info_t {
	GLint id;                    /**< Chromium window number */
	GLint lastX, lastY;          /**< last known position for the window */
	GLint lastWidth, lastHeight; /**< last known size for the window */

	GLint visBits;               /**< CR_RGB_BIT | CR_DOUBLE_BIT, etc */

	BackendWindowInfo *backendWindows;       /**< array [num_servers] */

	/**
	 *  \name tilesort stuff
	 */
	/*@{*/
	GLint muralWidth, muralHeight;  /**< may not match window size */
	TileSortBucketMode bucketMode;
	float widthScale, heightScale;  /**< mural size / window size */
	float viewportCenterX, viewportCenterY;
	float halfViewportWidth, halfViewportHeight;
	void *bucketInfo;          /**< private to tilesortspu_bucket.c */
	/*@}*/

	/**
	 * \name stereo stuff
	 */
	/*@{*/
	GLboolean passiveStereo;  /**< true if working in passive stereo mode */
	GLboolean forceQuadBuffering;
	GLboolean parity;  /**< for forcing quad-buffer stereo behaviour */
	MatrixSource matrixSource; /**< for stereo, per-server transformations */
	/*@}*/

	ServerWindowInfo *server;  /**< array [num_servers] of ServerWindowInfo */

	GLboolean validRasterOrigin;
	GLboolean newBackendWindows;

#ifdef WINDOWS
	HWND client_hwnd;
#elif defined(Darwin)
	WindowRef window;
#else
	Display *dpy;
	GLXDrawable xwin;
#endif
	GLboolean isDMXWindow;   /**< true if this window is on a DMX display */
};

/**
 * Warped tile info
 */
struct warp_display_info_t {
	GLint id, width, height;
	GLint num_tiles;
	GLfloat correct[9];		
	GLfloat correct_inv[9];
};

/**
 * Pincher state
 */
typedef struct {
	GLboolean    isLoop;
	GLint        numRestore;
	GLint        wind;
	CRVertex     vtx[3];
	unsigned char *beginOp, *beginData;
} TileSortSPUPinchState;

/**
 * Per thread info
 */
struct thread_info_t {
	CRPackBuffer geometry_buffer;
	CRPackContext *packer;
	ContextInfo *currentContext;
	TileSortSPUPinchState pinchState;
	int state_server_index;           /**< only used during __doFlush() */

	/**
	 *  Array of network connections to the servers 
	 */
	CRNetServer *netServer;    /**< array net[num_servers] */

	/**
	 * Array of buffers for packing state changes for servers.
	 * Usually just used during state differencing.
	 */
	CRPackBuffer *buffer;  /**< array buffer[num_servers] */
};

/**
 * Tilesort spu state
 */
typedef struct {
	/**
	 *  SPU stuff 
	 */
	int id;
	SPUDispatchTable self;

	/**
	 * \name for display lists 
	 */
	/*@{*/
	SPUDispatchTable packerDispatch; /**< for display list uploads */
	SPUDispatchTable stateDispatch;  /**< for display list state reconciliation */
	/*@}*/

	/**
	 * \name Threads 
	 */
	/*@{*/
	int numThreads;
	ThreadInfo thread[MAX_THREADS];
	/*@}*/

	/**
	 *  \name Contexts 
	 */
	/*@{*/
	CRHashTable *contextTable;
	/*@}*/

	/**
	 * \name Windows 
	 */
	/*@{*/
	CRHashTable *windowTable; /**< map SPU window IDs to WindowInfo */
	/*@}*/

	/**
	 * \name config options 
	 */
	/*@{*/
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
	int listTrack;
	/*@}*/

	/**
	 * \name stereo config 
	 */
	/*@{*/
	StereoMode stereoMode;
	GlassesType glassesType;
	CRmatrix stereoProjMatrices[2]; /**< 0 = left, 1 = right */
	CRmatrix stereoViewMatrices[2]; /**< 0 = left, 1 = right */
	TileSortBucketMode defaultBucketMode;
	unsigned int fakeWindowWidth, fakeWindowHeight;
	int scaleImages;
	GLboolean anaglyphMask[2][4]; 	/**< [eye][channel] */

	int swap;  /**< byte swapping */
	/*RenderSide renderSide;*/  /* left/right side rendering */
	unsigned int MTU;
	unsigned int buffer_size;
	unsigned int geom_buffer_size;
	unsigned int geom_buffer_mtu;
	int num_servers;
	int replay;
	int forceQuadBuffering;
	CRHashTable *listTable; /**< map display list ID to sentFlags for each server */
	/*@}*/

	/**
	 * \name WGL/GLX interface for DMX 
	 */
	/*@{*/
	crOpenGLInterface ws;
	/*@}*/

	/**
	 * \name warped display   XXX this might be per-window state 
	 */
	/*@{*/
	WarpDisplayInfo *displays;
	/*@}*/

	CRLimitsState limits;  /**< OpenGL limits computed from children */

	CRDLM *dlm;  /**< Display list manager */
} TileSortSPU;

/**
 * tilesorter bucket info
 */
typedef struct {
	CRbitvalue hits[CR_MAX_BITARRAY];
	GLvectorf  objectMin;
	GLvectorf  objectMax;
	CRrecti    pixelBounds;
} TileSortBucketInfo;


extern TileSortSPU tilesort_spu;


#ifdef CHROMIUM_THREADSAFE
extern CRmutex _TileSortMutex;
extern CRtsd _ThreadTSD;
#if 1
#define GET_THREAD(T) ThreadInfo *T = crGetTSD(&_ThreadTSD)
#else
#define GET_THREAD(T) ThreadInfo *T = crGetTSD(&_ThreadTSD); fprintf(stderr,"%s %d T = %p\n",__FILE__,__LINE__,T);
#endif
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
void tilesortspuSetAnaglyphMask(TileSortSPU *tilesort_spu);

/* tilesortspu_net.c */
void tilesortspuConnectToServers( void );

/* tilesortspu_flush.c */
void tilesortspuSendServerBuffer( int server_index );
void tilesortspuSendServerBufferThread( int server_index, ThreadInfo *thread );
void tilesortspuHuge( CROpcode opcode, void *buf );
void tilesortspuFlush( ThreadInfo *thread );
void tilesortspuFlush_callback( void *arg );
void tilesortspuFlushToServers(ThreadInfo *thread, TileSortBucketInfo *bucket_info);
void tilesortspuBroadcastGeom( GLboolean send_state_anyway );
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

/* tilesortspu_context.c */
void tilesortspuInitThreadPacking( ThreadInfo *thread );

/* tilesortspu_misc.c */
void TILESORTSPU_APIENTRY tilesortspu_ChromiumParametervCR(GLenum target, GLenum type, GLsizei count, const GLvoid *values);

/* tilesortspu_get.c */
const GLubyte *tilesortspuGetExtensionsString(void);
GLfloat tilesortspuGetVersionNumber(void);

/* tilesortspu_list_gen.c */
void tilesortspuLoadListTable(void);
void tilesortspuLoadSortTable(void);
void tilesortspuLoadStateTable(SPUDispatchTable *t);
void tilesortspuLoadPackTable(SPUDispatchTable *t);

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

void TILESORTSPU_APIENTRY
tilesortspu_PackZPixCR(GLsizei width, GLsizei height, GLenum format,
										 GLenum type, GLenum ztype, GLint zparm, GLint length,
										 const GLvoid *pixels);

/* tilesortspu_stereo.c */
void tilesortspuStereoContextInit(ContextInfo *ctx);
void tilesortspuSetupStereo(GLenum buffer);


#endif /* TILESORT_SPU_H */
