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

void tilesortspuCreateFunctions();
void tilesortspuGatherConfiguration();
void tilesortspuConnectToServers();

typedef struct {
	CRNetServer net;
	CRPackBuffer server_pack;
	int num_extents;
	int mural_x[CR_MAX_EXTENTS], mural_y[CR_MAX_EXTENTS];
	int mural_w[CR_MAX_EXTENTS], mural_h[CR_MAX_EXTENTS];
	CRContext *ctx;
} TileSortSPUServer;

typedef struct {
	int id;

	CRPackBuffer geometry_pack;
	CRContext *ctx;

	int num_servers;
	TileSortSPUServer *servers;
} TileSortSPU;

extern TileSortSPU tilesort_spu;

#endif /* TILESORT_SPU_H */
