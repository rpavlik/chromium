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
#include <X11/Xlib.h>
#endif

#include "cr_glstate.h"
#include "cr_net.h"
#include "cr_netserver.h"
#include "cr_pack.h"
#include "cr_spu.h"
#include "cr_threads.h"

#include "state/cr_limits.h"
#include "state/cr_statetypes.h"

#define MAX_WINDOWS 1

#define END_FLUFF 4 /* space for phantom GLEND opcode for splitting */

typedef struct {
	int num_extents;
	int display_ndx[CR_MAX_EXTENTS];
	GLfloat world_extents[CR_MAX_EXTENTS][8]; /* x1, y1, x2, y2, x3, y3, ... */
	CRrecti extents[CR_MAX_EXTENTS];
	CRContext *context[CR_MAX_CONTEXTS];
	GLint serverCtx[CR_MAX_CONTEXTS];

	GLint vertexCount;  /* for debug/validation purposes */
} TileSortSPUServer;

typedef struct {
	GLboolean    isLoop;
	GLint        numRestore;
	GLint        wind;
	CRVertex     vtx[3];
	unsigned char *beginOp, *beginData;
} TileSortSPUPinchState;

typedef enum {
	BROADCAST,       /* send all geometry to all servers */
	TEST_ALL_TILES,  /* test bounding box against all tiles */
	UNIFORM_GRID,    /* all columns are equal width, all rows are equal height */
	NON_UNIFORM_GRID,/* columns and rows are of varying width, height */
	RANDOM,          /* randomly bucket geometry */
	WARPED_GRID      /* warped tiles (Karl Rasche) */
} TileSortBucketMode;

typedef struct thread_info_t ThreadInfo;
typedef struct context_info_t ContextInfo;

struct thread_info_t {
	int geom_pack_size;
	CRPackBuffer geometry_pack;
	CRPackContext *packer;
	ContextInfo *currentContext;
	int currentContextIndex;
	TileSortSPUPinchState pinchState;
	TileSortSPUServer *state_server;  /* only used during __doFlush() */
	int state_server_index;           /* only used during __doFlush() */

	CRNetServer *net;    /* array net[num_servers] */
	CRPackBuffer *pack;  /* array pack[num_servers] */
};

struct context_info_t {
	CRContext *State;
};

typedef struct
{
	GLint id, width, height;
	GLint num_tiles;
	
	GLfloat correct[9];		
	GLfloat correct_inv[9];
} display_t;

typedef struct {
	int id;

	int numThreads;
	ThreadInfo thread[MAX_THREADS];

	int numContexts;
	ContextInfo context[CR_MAX_CONTEXTS];

	GLboolean windows_inuse[MAX_WINDOWS];

	SPUDispatchTable self;

	/* config options */
	int splitBeginEnd;
	int broadcast;
	int drawBBOX;
	float bboxLineWidth;
	int syncOnFinish, syncOnSwap;
	int scaleToMuralSize;
	int localTileSpec;
	int emit_GATHER_POST_SWAPBUFFERS;
	int optimizeBucketing;

	int swap;  /* byte swapping */

	TileSortBucketMode bucketMode;

	float viewportCenterX, viewportCenterY;
	float halfViewportWidth, halfViewportHeight;

	GLenum providedBBOX;
	int inDrawPixels;
	int ReadPixels;

	unsigned int muralWidth, muralHeight;
	unsigned int muralColumns, muralRows;

	float widthScale, heightScale; /* muralSize / windowSize */

	unsigned int MTU;
	unsigned int buffer_size;
	int num_servers;
	TileSortSPUServer *servers;
	display_t *displays;
	double world_bbox[4];

#ifdef WINDOWS
	HDC client_hdc;
	HWND client_hwnd;
#else
	Display *glx_display;
	Drawable glx_drawable;
#endif
	unsigned int fakeWindowWidth, fakeWindowHeight;

	CRLimitsState limits;  /* OpenGL limits computed from children */

} TileSortSPU;

typedef struct {
	CRbitvalue hits[CR_MAX_BITARRAY];
	GLvectorf  screenMin;
	GLvectorf  screenMax;
	GLvectorf  objectMin;
	GLvectorf  objectMax;
	CRrecti    pixelBounds;
} TileSortBucketInfo;


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



extern TileSortSPU tilesort_spu;

void tilesortspuCreateFunctions( void );
void tilesortspuGatherConfiguration( const SPU *child_spu );
void tilesortspuConnectToServers( void );
void tilesortspuGetTileInformation(CRConnection *conn);
void tilesortspuComputeRowsColumns(void);
GLboolean tilesortspuInitGridBucketing(void);

void tilesortspuComputeMaxViewport( void );
void tilesortspuInitThreadPacking( ThreadInfo *thread );
void tilesortspuHuge( CROpcode opcode, void *buf );
void tilesortspuFlush( ThreadInfo *thread );
void tilesortspuFlush_callback( void *arg );
void tilesortspuBroadcastGeom( int send_state_anyway );
void tilesortspuShipBuffers( void );
void tilesortspuCreateDiffAPI( void );
void tilesortspuSetBucketingBounds( int x, int y, unsigned int w, unsigned int h );
void tilesortspuBucketingInit( void );
void tilesortspuPinch( void );
void tilesortspuPinchRestoreTriangle( void );

void tilesortspuDebugOpcodes( CRPackBuffer *pack );

void TILESORTSPU_APIENTRY tilesortspu_ChromiumParametervCR(GLenum target, GLenum type, GLsizei count, const GLvoid *values);
void TILESORTSPU_APIENTRY tilesortspu_Begin(GLenum prim);
void TILESORTSPU_APIENTRY tilesortspu_End(void);

void tilesortspuSendServerBuffer( int server_index );
void tilesortspuBucketGeometry(TileSortBucketInfo *info);

extern void _math_init_eval(void);


#endif /* TILESORT_SPU_H */
