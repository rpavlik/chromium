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

#include "state/cr_limits.h"
#include "state/cr_statetypes.h"

#define END_FLUFF 4 /* space for phantom GLEND opcode for splitting */

void tilesortspuCreateFunctions( void );
void tilesortspuGatherConfiguration( const SPU *child_spu );
void tilesortspuConnectToServers( void );

typedef struct {
	CRNetServer net;
	CRPackBuffer pack;
	int num_extents;
	int x1[CR_MAX_EXTENTS], y1[CR_MAX_EXTENTS];
	int x2[CR_MAX_EXTENTS], y2[CR_MAX_EXTENTS];

	CRContext *currentContext;
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

typedef struct {
	int id;

	int geom_pack_size;
	CRPackBuffer geometry_pack;

	CRContext *context[CR_MAX_CONTEXTS];
	CRContext *currentContext;

	TileSortSPUPinchState pinchState;
	SPUDispatchTable self;

	/* config options */
	int splitBeginEnd;
	int broadcast;
	int optimizeBucketing;
	int drawBBOX;
	float bboxLineWidth;
	int syncOnFinish, syncOnSwap;
	int scaleToMuralSize;

	int swap;

	float viewportCenterX, viewportCenterY;
	float halfViewportWidth, halfViewportHeight;

	GLenum providedBBOX;
	int inDrawPixels;
	int ReadPixels;

	unsigned int muralWidth, muralHeight;

	float widthScale, heightScale; /* muralSize / windowSize */

	unsigned int MTU;
	int num_servers;
	TileSortSPUServer *servers;

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
	GLbitvalue hits[CR_MAX_BITARRAY];
	GLvectorf  screenMin;
	GLvectorf  screenMax;
	GLvectorf  objectMin;
	GLvectorf  objectMax;
	GLrecti    pixelBounds;
} TileSortBucketInfo;

extern TileSortBucketInfo *tilesortspuBucketGeometry(void);

extern TileSortSPU tilesort_spu;

void tilesortspuHuge( CROpcode opcode, void *buf );
void tilesortspuFlush( void *arg );
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

void tilesortspuSendServerBuffer( TileSortSPUServer *server );

#endif /* TILESORT_SPU_H */
