#ifndef TILESORT_SPU_H
#define TILESORT_SPU_H

#ifdef WINDOWS
#define TILESORTSPU_APIENTRY __stdcall
#else
#define TILESORTSPU_APIENTRY
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
	int mural_x[CR_MAX_EXTENTS], mural_y[CR_MAX_EXTENTS];
	int mural_w[CR_MAX_EXTENTS], mural_h[CR_MAX_EXTENTS];
	CRContext *ctx;
} TileSortSPUServer;

typedef struct {
	int id;

	int geom_pack_size;
	CRPackBuffer geometry_pack;
	CRContext *ctx;

	int apply_viewtransform;
	int splitBeginEnd;

	unsigned int MTU;
	int num_servers;
	TileSortSPUServer *servers;
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

void tilesortspuDebugOpcodes( CRPackBuffer *pack );

#endif /* TILESORT_SPU_H */
