/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "tilesortspu.h"

#include "cr_mothership.h"
#include "cr_string.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_spu.h"

static void
setDefaults(void)
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
set_bounding_box_scale(TileSortSPU *tilesort_spu, const char *response)
{
	sscanf(response, "%f", &(tilesort_spu->bboxScale));
}

static void
set_auto_dlist_bbox(TileSortSPU *tilesort_spu, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu->autoDListBBoxes));
}

static void
set_lazy_send_dlists(TileSortSPU *tilesort_spu, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu->lazySendDLists));
}

static void
list_track(TileSortSPU *tilesort_spu, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu->listTrack));
}

static void
set_fake_window_dims(TileSortSPU *tilesort_spu, const char *response)
{
	float w = 0.0, h = 0.0;
	CRASSERT(response[0] == '[');
	sscanf(response, "[ %f, %f ]", &w, &h);
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
	else if (crStrcmp(response, "Frustum") == 0)
		tilesort_spu->defaultBucketMode = FRUSTUM;
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


static void
scale_images(TileSortSPU *tilesort_spu, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu->scaleImages));
}

static void
set_stereomode(TileSortSPU *server, const char *response)
{
	char buffer[32], mode[16], *bptr;
	const char *rptr;

	if (response) {
		for (rptr = response, bptr = buffer; *rptr; rptr++) {
			if (*rptr != ' ') {
				*bptr = *rptr;
				bptr++;
			}
		}
		*bptr = '\0';
		sscanf(buffer, "%s", mode);
		if (!crStrcmp(mode, "None"))
			server->stereoMode = NONE;
		else if (!crStrcmp(mode, "Passive"))
			server->stereoMode = PASSIVE;
		else if (!crStrcmp(mode, "CrystalEyes"))
			server->stereoMode = CRYSTAL;
		else if (!crStrcmp(mode, "Anaglyph"))
			server->stereoMode = ANAGLYPH;
		else if (!crStrcmp(mode, "SideBySide"))
			server->stereoMode = SIDE_BY_SIDE;
		else {
			crWarning
				("stereo_mode '%s' is not supported.  Currently supported modes are 'None', 'Passive', 'CrystalEyes', 'Anaglyph' and 'SideBySide'.\n", 
				 mode);
			server->stereoMode = NONE;
		}
	}
}

static void
set_force_quad_buffering(TileSortSPU *tilesort_spu, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu->forceQuadBuffering));
}

void
tilesortspuSetAnaglyphMask(TileSortSPU *tilesort_spu)
{
	/* init left mask */
	tilesort_spu->anaglyphMask[0][0] = GL_FALSE;
	tilesort_spu->anaglyphMask[0][1] = GL_FALSE;
	tilesort_spu->anaglyphMask[0][2] = GL_FALSE;
	tilesort_spu->anaglyphMask[0][3] = GL_TRUE;
	/* init right mask */
	tilesort_spu->anaglyphMask[1][0] = GL_FALSE;
	tilesort_spu->anaglyphMask[1][1] = GL_FALSE;
	tilesort_spu->anaglyphMask[1][2] = GL_FALSE;
	tilesort_spu->anaglyphMask[1][3] = GL_TRUE;
	
	switch (tilesort_spu->glassesType) {
	case RED_BLUE:
		/* left */
		tilesort_spu->anaglyphMask[0][0] = GL_TRUE;
		/* right */
		tilesort_spu->anaglyphMask[1][2] = GL_TRUE;
		break;
	case RED_GREEN:
		/* left */
		tilesort_spu->anaglyphMask[0][0] = GL_TRUE;
		/* right */
		tilesort_spu->anaglyphMask[1][1] = GL_TRUE;
		break;
	case RED_CYAN:
		/* left */
		tilesort_spu->anaglyphMask[0][0] = GL_TRUE;
		/* right */
		tilesort_spu->anaglyphMask[1][1] = GL_TRUE;
		tilesort_spu->anaglyphMask[1][2] = GL_TRUE;
		break;
	case BLUE_RED:
		/* left */
		tilesort_spu->anaglyphMask[0][2] = GL_TRUE;
		/* right */
		tilesort_spu->anaglyphMask[1][0] = GL_TRUE;
		break;
	case GREEN_RED:
		/* left */
		tilesort_spu->anaglyphMask[0][1] = GL_TRUE;
		/* right */
		tilesort_spu->anaglyphMask[1][0] = GL_TRUE;
		break;
	case CYAN_RED:
		/* left */
		tilesort_spu->anaglyphMask[0][1] = GL_TRUE;
		tilesort_spu->anaglyphMask[0][2] = GL_TRUE;
		/* right */
		tilesort_spu->anaglyphMask[1][0] = GL_TRUE;
		break;
	}
}

static void
set_glasses_type(TileSortSPU *tilesort_spu, const char *response)
{
	if (crStrcmp(response, "RedBlue") == 0)
		tilesort_spu->glassesType = RED_BLUE;
	else if (crStrcmp(response, "RedGreen") == 0)
		tilesort_spu->glassesType = RED_GREEN;
	else if (crStrcmp(response, "RedCyan") == 0)
		tilesort_spu->glassesType = RED_CYAN;
	else if (crStrcmp(response, "BlueRed") == 0)
		tilesort_spu->glassesType = BLUE_RED;
	else if (crStrcmp(response, "GreenRed") == 0)
		tilesort_spu->glassesType = GREEN_RED;
	else if (crStrcmp(response, "CyanRed") == 0)
		tilesort_spu->glassesType = CYAN_RED;
	else
	{
		crWarning("Bad value (%s) for tilesort glasses_type", response);
		tilesort_spu->glassesType = RED_BLUE;
	}
	tilesortspuSetAnaglyphMask(tilesort_spu);
}


static void
set_left_view_matrix(TileSortSPU *tilesort_spu, const char *response)
{
	crMatrixInitFromString(&tilesort_spu->stereoViewMatrices[0], response);
}

static void
set_right_view_matrix(TileSortSPU *tilesort_spu, const char *response)
{
	crMatrixInitFromString(&tilesort_spu->stereoViewMatrices[1], response);
}

static void
set_left_projection_matrix(TileSortSPU *tilesort_spu, const char *response)
{
	crMatrixInitFromString(&tilesort_spu->stereoProjMatrices[0], response);
}

static void
set_right_projection_matrix(TileSortSPU *tilesort_spu, const char *response)
{
	crMatrixInitFromString(&tilesort_spu->stereoProjMatrices[1], response);
}


/* option, type, nr, default, min, max, title, callback
 */
SPUOptions tilesortSPUOptions[] = {
	{"bucket_mode", CR_ENUM, 1, "Test All Tiles",
	 "'Broadcast', 'Test All Tiles', 'Uniform Grid', 'Non-Uniform Grid', 'Random', 'Warped Grid', 'Frustum'", NULL,
	 "Geometry Bucketing Method", (SPUOptionCB) set_bucket_mode},

	{"scale_to_mural_size", CR_BOOL, 1, "1", NULL, NULL,
	 "Scale to Mural Size", (SPUOptionCB) set_scale_to_mural_size},

	{"fake_window_dims", CR_INT, 2, "[0, 0]", "[0, 0]", NULL,
	 "Fake Window Dimensions (w, h)", (SPUOptionCB) set_fake_window_dims},

	{"retile_on_resize", CR_BOOL, 1, "1", NULL, NULL,
	 "Retile When Window Size Changes", (SPUOptionCB) retile_on_resize},

	{"scale_images", CR_BOOL, 1, "0", NULL, NULL,
	 "Scale glDraw/CopyPixels and glBitmap images", (SPUOptionCB) scale_images},

	{"sync_on_swap", CR_BOOL, 1, "1", NULL, NULL,
	 "Sync on SwapBuffers", (SPUOptionCB) set_sync_on_swap},

	{"sync_on_finish", CR_BOOL, 1, "1", NULL, NULL,
	 "Sync on glFinish", (SPUOptionCB) set_sync_on_finish},

	{"split_begin_end", CR_BOOL, 1, "1", NULL, NULL,
	 "Split glBegin/glEnd", (SPUOptionCB) set_split_begin_end},

	{"use_dmx", CR_BOOL, 1, "0", NULL, NULL,
	 "Use DMX display", (SPUOptionCB) set_use_dmx},

	{"draw_bbox", CR_BOOL, 1, "0", NULL, NULL,
	 "Draw Bounding Boxes", (SPUOptionCB) set_draw_bbox},

	{"bbox_line_width", CR_INT, 1, "5", "0", "10",
	 "Bounding Box Line Width", (SPUOptionCB) set_bbox_line_width},

	{"bbox_scale", CR_FLOAT, 1, "1.0", "0.1", "10.0",
	 "Bounding Box Scale Factor", (SPUOptionCB) set_bounding_box_scale},

	{"auto_dlist_bbox", CR_BOOL, 1, "1", NULL, NULL,
	 "Automatically compute/use bounding boxes for display lists",
	 (SPUOptionCB) set_auto_dlist_bbox},

	{"lazy_send_dlists", CR_BOOL, 1, "0", NULL, NULL,
	 "Send display lists to servers only when needed (lazy)",
	 (SPUOptionCB) set_lazy_send_dlists},

	{"dlist_state_tracking", CR_BOOL, 1, "0", NULL, NULL,
	 "Track state in display lists", (SPUOptionCB) list_track},

	{"emit_GATHER_POST_SWAPBUFFERS", CR_BOOL, 1, "0", NULL, NULL,
	 "Emit a glChromiumParameteri After SwapBuffers", (SPUOptionCB) set_emit},

	{"local_tile_spec", CR_BOOL, 1, "0", NULL, NULL,
	 "Specify Tiles Relative to Displays", (SPUOptionCB) set_local_tile_spec},

	{"stereo_mode", CR_ENUM, 1, "None",
	 "'None', 'Passive', 'CrystalEyes', 'SideBySide', 'Anaglyph'", NULL,
	 "Stereo Mode", (SPUOptionCB) set_stereomode },

	{"glasses_type", CR_ENUM, 1, "RedBlue",
	 "'RedBlue', 'RedGreen', 'RedCyan', 'BlueRed', 'GreenRed', 'CyanRed'", NULL, 
	 "Anaglyph Glasses Colors (Left/Right)", (SPUOptionCB) set_glasses_type },

	/* XXX perhaps this should be a string option which names window titles
	 * for the windows to force into stereo mode???
	 */
	{"force_quad_buffering", CR_BOOL, 1, "0", NULL, NULL,
	 "Force Quad-buffered Stereo", (SPUOptionCB) set_force_quad_buffering},

	{"left_view_matrix", CR_FLOAT, 16,
	 "[1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1]", NULL, NULL,
	 "Left Eye Viewing Matrix", (SPUOptionCB) set_left_view_matrix},

	{"right_view_matrix", CR_FLOAT, 16,
	 "[1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1]", NULL, NULL,
	 "Right Eye Viewing Matrix", (SPUOptionCB) set_right_view_matrix},

	{"left_projection_matrix", CR_FLOAT, 16,
	 "[1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1]", NULL, NULL,
	 "Left Eye Projection Matrix", (SPUOptionCB) set_left_projection_matrix},

	{"right_projection_matrix", CR_FLOAT, 16,
	 "[1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1]", NULL, NULL,
	 "Right Eye Projection Matrix", (SPUOptionCB) set_right_projection_matrix},

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

	setDefaults();

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

	tilesort_spu.buffer_size = crMothershipGetMTU(conn);
	tilesort_spu.MTU = tilesort_spu.buffer_size;
	crDebug("Got the MTU as %d", tilesort_spu.MTU);

	/* Need to get this, before we create initial window! */
	tilesort_spu.num_servers = tilesortspuGetNumServers(conn);

	crDebug("Got %d servers!", tilesort_spu.num_servers);

	/* Create initial/default window (id=0) */
	winInfo = tilesortspuCreateWindowInfo(0,
														CR_RGB_BIT | CR_DOUBLE_BIT /*| CR_DEPTH_BIT*/);
	CRASSERT(winInfo);

	/* The initial tile layout is obtained from the servers */
	tilesortspuGetTilingFromServers(conn, winInfo);
	crMothershipDisconnect(conn);

	crWarning("Total output dimensions = (%d, %d)",
						winInfo->muralWidth, winInfo->muralHeight);

	/* XXX we should really query all the servers to compute the limits! */
	crStateLimitsInit(&tilesort_spu.limits);
}


