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

#include "state/cr_statetypes.h"

#define END_FLUFF 4 // space for phantom GLEND opcode for splitting

void tilesortspuCreateFunctions( void );
void tilesortspuGatherConfiguration( void );
void tilesortspuConnectToServers( void );

typedef struct {
	CRNetServer net;
	CRPackBuffer pack;
	int num_extents;
	int x1[CR_MAX_EXTENTS], y1[CR_MAX_EXTENTS];
	int x2[CR_MAX_EXTENTS], y2[CR_MAX_EXTENTS];
	CRContext *ctx;
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
	CRContext *ctx;
	TileSortSPUPinchState pinchState;

	int sendBounds;
	int splitBeginEnd;
	int broadcast;
	int optimizeBucketing;
	int drawBBOX;
	float bboxLineWidth;

	unsigned int fake_window_width, fake_window_height;

	int syncOnFinish, syncOnSwap;

	float viewportCenterX, viewportCenterY;
	float halfViewportWidth, halfViewportHeight;

	GLenum providedBBOX;

	unsigned int muralWidth, muralHeight;

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
} TileSortSPU;

typedef struct {
	GLbitvalue hits;
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
void tilesortspuBroadcastGeom( void );
void tilesortspuShipBuffers( void );
void tilesortspuCreateDiffAPI( void );
void tilesortspuSetBucketingBounds( int x, int y, unsigned int w, unsigned int h );
void tilesortspuBucketingInit( void );
void tilesortspuPinch( void );
void tilesortspuPinchRestoreTriangle( void );

void tilesortspuDebugOpcodes( CRPackBuffer *pack );

void TILESORTSPU_APIENTRY tilesortspu_Hint( GLenum target, GLenum mode );

#endif /* TILESORT_SPU_H */
