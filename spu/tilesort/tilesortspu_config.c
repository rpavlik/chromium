/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "tilesortspu.h"

#include "cr_mothership.h"
#include "cr_string.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_spu.h"

static void
__setDefaults(void)
{
	tilesort_spu.numThreads = 1;
	tilesort_spu.num_servers = 0;

/*  	tilesort_spu.splitBeginEnd = 1; */
/*  	tilesort_spu.drawBBOX = 0; */
/*  	tilesort_spu.bboxLineWidth = 5; */
/*  	tilesort_spu.syncOnFinish = 1; */
/*  	tilesort_spu.syncOnSwap = 1; */
/*  	tilesort_spu.fakeWindowWidth = 0; */
/*  	tilesort_spu.fakeWindowHeight = 0; */
/*  	tilesort_spu.scaleToMuralSize = GL_TRUE; */
}


static void
set_split_begin_end(TileSortSPU *tilesort_spu, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu->splitBeginEnd));
}

static void
set_sync_on_swap(TileSortSPU *tilesort_spu, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu->syncOnSwap));
}

static void
set_sync_on_finish(TileSortSPU *tilesort_spu, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu->syncOnFinish));
}

static void
set_draw_bbox(TileSortSPU *tilesort_spu, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu->drawBBOX));
}

static void
set_bbox_line_width(TileSortSPU *tilesort_spu, const char *response)
{
	sscanf(response, "%f", &(tilesort_spu->bboxLineWidth));
}

static void
set_fake_window_dims(TileSortSPU *tilesort_spu, const char *response)
{
	float w, h;
	if (response[0] == '[')
		sscanf(response, "[ %f, %f ]", &w, &h);
	else
		sscanf(response, "%f %f", &w, &h);
	tilesort_spu->fakeWindowWidth = (unsigned int) w;
	tilesort_spu->fakeWindowHeight = (unsigned int) h;
}

static void
set_scale_to_mural_size(TileSortSPU *tilesort_spu, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu->scaleToMuralSize));
}

static void
set_emit(TileSortSPU *tilesort_spu, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu->emit_GATHER_POST_SWAPBUFFERS));
}

static void
set_local_tile_spec(TileSortSPU *tilesort_spu, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu->localTileSpec));
}

static void
set_bucket_mode(TileSortSPU *tilesort_spu, const char *response)
{
	if (crStrcmp(response, "Broadcast") == 0)
		tilesort_spu->defaultBucketMode = BROADCAST;
	else if (crStrcmp(response, "Test All Tiles") == 0)
		tilesort_spu->defaultBucketMode = TEST_ALL_TILES;
	else if (crStrcmp(response, "Uniform Grid") == 0)
		tilesort_spu->defaultBucketMode = UNIFORM_GRID;
	else if (crStrcmp(response, "Non-Uniform Grid") == 0)
		tilesort_spu->defaultBucketMode = NON_UNIFORM_GRID;
	else if (crStrcmp(response, "Random") == 0)
		tilesort_spu->defaultBucketMode = RANDOM;
	else if (crStrcmp(response, "Warped Grid") == 0)
		tilesort_spu->defaultBucketMode = WARPED_GRID;
	else
	{
		crWarning("Bad value (%s) for tilesort bucket_mode", response);
		tilesort_spu->defaultBucketMode = BROADCAST;
	}
}

static void
set_use_dmx(TileSortSPU *tilesort_spu, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu->useDMX));
}

static void
retile_on_resize(TileSortSPU *tilesort_spu, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu->retileOnResize));
}


/* option, type, nr, default, min, max, title, callback
 */
SPUOptions tilesortSPUOptions[] = {
	{"split_begin_end", CR_BOOL, 1, "1", NULL, NULL,
	 "Split glBegin/glEnd", (SPUOptionCB) set_split_begin_end},

	{"sync_on_swap", CR_BOOL, 1, "1", NULL, NULL,
	 "Sync on SwapBuffers", (SPUOptionCB) set_sync_on_swap},

	{"sync_on_finish", CR_BOOL, 1, "1", NULL, NULL,
	 "Sync on glFinish", (SPUOptionCB) set_sync_on_finish},

	{"draw_bbox", CR_BOOL, 1, "0", NULL, NULL,
	 "Draw Bounding Boxes", (SPUOptionCB) set_draw_bbox},

	{"bbox_line_width", CR_INT, 1, "5", "0", "10",
	 "Bounding Box Line Width", (SPUOptionCB) set_bbox_line_width},

	{"fake_window_dims", CR_INT, 2, "0, 0", "0, 0", NULL,
	 "Fake Window Dimensions (w, h)", (SPUOptionCB) set_fake_window_dims},

	{"scale_to_mural_size", CR_BOOL, 1, "1", NULL, NULL,
	 "Scale to Mural Size", (SPUOptionCB) set_scale_to_mural_size},

	{"emit_GATHER_POST_SWAPBUFFERS", CR_BOOL, 1, "0", NULL, NULL,
	 "Emit a glParameteri After SwapBuffers", (SPUOptionCB) set_emit},

	{"local_tile_spec", CR_BOOL, 1, "0", NULL, NULL,
	 "Specify Tiles Relative to Displays", (SPUOptionCB) set_local_tile_spec},

	{"bucket_mode", CR_ENUM, 1, "Test All Tiles",
	 "'Broadcast', 'Test All Tiles', 'Uniform Grid', 'Non-Uniform Grid', 'Random', 'Warped Grid'", NULL,
	 "Geometry Bucketing Method", (SPUOptionCB) set_bucket_mode},

	{"use_dmx", CR_BOOL, 1, "0", NULL, NULL,
	 "Use DMX display", (SPUOptionCB) set_use_dmx},

	{"retile_on_resize", CR_BOOL, 1, "1", NULL, NULL,
	 "Retile when Window Resizes", (SPUOptionCB) retile_on_resize},

	{NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL},
};


static int
tilesortspuGetNumServers(CRConnection *conn)
{
	char response[8096], **serverchain;
	int n;

	crMothershipGetServers(conn, response);

	serverchain = crStrSplitn(response, " ", 1);
	n = crStrToInt(serverchain[0]);

	if (n == 0) {
		crError("No servers specified for a tile/sort SPU?!");
	}

	crFreeStrings(serverchain);
	return n;
}


void
tilesortspuGatherConfiguration(const SPU * child_spu)
{
	CRConnection *conn;
	WindowInfo *winInfo;

	__setDefaults();

	/* Connect to the mothership and identify ourselves. */

	conn = crMothershipConnect();
	if (!conn)
	{
		crError
			("Couldn't connect to the mothership -- I have no idea what to do!");
	}
	crMothershipIdentifySPU(conn, tilesort_spu.id);

	/* Process SPU config options */
	crSPUGetMothershipParams(conn, (void *) &tilesort_spu, tilesortSPUOptions);

	tilesort_spu.MTU = crMothershipGetMTU(conn);
	crDebug("Got the MTU as %d", tilesort_spu.MTU);

	/* Need to get this, before we create initial window! */
	tilesort_spu.num_servers = tilesortspuGetNumServers(conn);
	crDebug("Got %d servers!", tilesort_spu.num_servers);

	/* get a buffer which can hold one big big opcode (counter computing
	 * against packer/pack_buffer.c)
	 */
	tilesort_spu.buffer_size =
		((((tilesort_spu.MTU - sizeof(CRMessageOpcodes)) * 5 + 3) / 4 +
			0x3) & ~0x3) + sizeof(CRMessageOpcodes);

	/* Create initial/default window (id=0) */
	winInfo = tilesortspuCreateWindowInfo(0,
														CR_RGB_BIT | CR_DOUBLE_BIT /*| CR_DEPTH_BIT*/);
	CRASSERT(winInfo);

	/* The initial tile layout is obtained from the servers */
	tilesortspuGetTilingFromServers(conn, winInfo);
	crMothershipDisconnect(conn);

	crWarning("Total output dimensions = (%d, %d)",
						winInfo->lastWidth, winInfo->lastHeight);

	/* XXX we should really query all the servers to compute the limits! */
	crStateLimitsInit(&tilesort_spu.limits);
}


