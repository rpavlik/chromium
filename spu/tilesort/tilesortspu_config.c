/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "tilesortspu.h"

#include "cr_mothership.h"
#include "cr_string.h"
#include "cr_environment.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_spu.h"

/* fwd decl */
SPUOptions tilesortSPUOptions[];


static void
setDefaults(void)
{
	tilesort_spu.numThreads = 1;
	tilesort_spu.num_servers = 0;

	crMatrixInit(&tilesort_spu.stereoViewMatrices[0]);
	crMatrixInit(&tilesort_spu.stereoViewMatrices[1]);
	crMatrixInit(&tilesort_spu.stereoProjMatrices[0]);
	crMatrixInit(&tilesort_spu.stereoProjMatrices[1]);

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
	/* NOTE: list of enum values in the SPUOptions array _must_ match the
	 * order of the enum values for TileSortBucketMode in tilesortspu.h!
	 */
	int i = crSPUGetEnumIndex(tilesortSPUOptions, "bucket_mode", response);
	if (i < 0) {
		crWarning("Tilesort SPU: Bad value (%s) for tilesort bucket_mode",
							response);
		i = 0;
	}
	tilesort_spu->defaultBucketMode = BROADCAST + i;
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
set_stereo_mode(TileSortSPU *tilesort_spu, const char *response)
{
	int i = crSPUGetEnumIndex(tilesortSPUOptions, "stereo_mode", response);
	if (i < 0) {
		crWarning("Tilesort SPU: stereo_mode '%s' is not supported.  "
							"Currently supported modes are 'None', 'Passive', "
							"'CrystalEyes', 'Anaglyph' and 'SideBySide'.\n", 
							response);
		i = 0;
	}
	tilesort_spu->stereoMode = NONE + i;
}

static void
set_force_quad_buffering(TileSortSPU *tilesort_spu, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu->forceQuadBuffering));
}

static void
set_render_to_crut_window(TileSortSPU *tilesort_spu, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu->renderToCrutWindow));
}

static void
set_track_window_position(TileSortSPU *tilesort_spu, const char *response)
{
	sscanf(response, "%d", &(tilesort_spu->trackWindowPosition));
}

static void
set_display_string(TileSortSPU *tilesort_spu, const char *response)
{
	if (!crStrcmp(response, "DEFAULT")) {
		const char *display = crGetenv("DISPLAY");
		if (display)
			crStrncpy(tilesort_spu->displayString,
								display,
								sizeof(tilesort_spu->displayString));
		else
			crStrcpy(tilesort_spu->displayString, ""); /* empty string */
	}
	else {
		crStrncpy(tilesort_spu->displayString,
							response,
							sizeof(tilesort_spu->displayString));
	}
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
	int i = crSPUGetEnumIndex(tilesortSPUOptions, "glasses_type", response);
	if (i < 0) {
		crWarning("Tilesort SPU: Bad value (%s) for tilesort glasses_type",
							response);
		i = 0;
	}
	tilesort_spu->glassesType = RED_BLUE + i;
	tilesortspuSetAnaglyphMask(tilesort_spu);
}


#if 0
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
#endif


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

	{"auto_dlist_bbox", CR_BOOL, 1, "0", NULL, NULL,
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
	 "Stereo Mode", (SPUOptionCB) set_stereo_mode },

	{"glasses_type", CR_ENUM, 1, "RedBlue",
	 "'RedBlue', 'RedGreen', 'RedCyan', 'BlueRed', 'GreenRed', 'CyanRed'", NULL, 
	 "Anaglyph Glasses Colors (Left/Right)", (SPUOptionCB) set_glasses_type },

	/** XXX \todo perhaps this should be a string option which names window titles
	 * for the windows to force into stereo mode???
	 */
	{"force_quad_buffering", CR_BOOL, 1, "0", NULL, NULL,
	 "Force Quad-buffered Stereo", (SPUOptionCB) set_force_quad_buffering},

	{"render_to_crut_window", CR_BOOL, 1, "0", NULL, NULL,
	 "Render Into CRUT Window", (SPUOptionCB) set_render_to_crut_window},

	{"track_window_position", CR_BOOL, 1, "0", NULL, NULL,
	 "Track Window Positions", (SPUOptionCB) set_track_window_position},

	{ "display_string", CR_STRING, 1, "DEFAULT", NULL, NULL, 
		"X Display String (for DMX)", (SPUOptionCB) set_display_string },

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
		crError("Tilesort SPU: no servers found.");
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
		crError("Tilesort SPU: Couldn't connect to the mothership -- I have no idea what to do!");
	}
	crMothershipIdentifySPU(conn, tilesort_spu.id);

	tilesort_spu.rank = crMothershipGetSPURank(conn);

	/* Process SPU config options */
	crSPUGetMothershipParams(conn, (void *) &tilesort_spu, tilesortSPUOptions);

	tilesort_spu.buffer_size = crMothershipGetMTU(conn);
	tilesort_spu.MTU = tilesort_spu.buffer_size;
	crDebug("Tilesort SPU: Got the MTU as %d", tilesort_spu.MTU);

	/* Need to get this, before we create initial window! */
	tilesort_spu.num_servers = tilesortspuGetNumServers(conn);

	crDebug("Tilesort SPU: found %d servers.", tilesort_spu.num_servers);

	/* Create initial/default window (id=0) */
	winInfo = tilesortspuCreateWindowInfo(0,
														CR_RGB_BIT | CR_DOUBLE_BIT /*| CR_DEPTH_BIT*/);
	CRASSERT(winInfo);

	/* The initial tile layout is obtained from the servers */
	tilesortspuGetTilingFromServers(conn, winInfo);
	crMothershipDisconnect(conn);

	crDebug("Tilesort SPU: Total output dimensions = (%d, %d)",
				 winInfo->muralWidth, winInfo->muralHeight);

	/** XXX \todo we should really query all the servers to compute the limits! */
	crStateLimitsInit(&tilesort_spu.limits);
}


