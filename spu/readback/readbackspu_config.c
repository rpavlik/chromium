/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "readbackspu.h"
#include "cr_mothership.h"
#include "cr_string.h"

static void __setDefaults( ReadbackSPU *readback_spu )
{
	/* config options */
	readback_spu->extract_depth = 0;
	readback_spu->extract_alpha = 0;
	readback_spu->local_visualization = 0;
	readback_spu->visualize_depth = 0;
	readback_spu->resizable = 0;
	readback_spu->drawBboxOutlines = 0;
	readback_spu->drawOffsetX = 0;
	readback_spu->drawOffsetY = 0;
	readback_spu->readSizeX = 0;
	readback_spu->readSizeY = 0;

	/* misc */
	readback_spu->barrierSize = 0;
}

static void set_extract_depth( ReadbackSPU *readback_spu, const char *response )
{
	readback_spu->extract_depth = crStrToInt( response );
}

static void set_extract_alpha( ReadbackSPU *readback_spu, const char *response )
{
	readback_spu->extract_alpha = crStrToInt( response );
}

static void set_local_visualization( ReadbackSPU *readback_spu, const char *response )
{
	readback_spu->local_visualization = crStrToInt( response );
}

static void set_visualize_depth( ReadbackSPU *readback_spu, const char *response )
{
	readback_spu->visualize_depth = crStrToInt( response );
}

static void set_gather_url( ReadbackSPU *readback_spu, const char *response )
{
	if (crStrlen(response) > 0)
		readback_spu->gather_url = crStrdup( response );
	else
		readback_spu->gather_url = NULL;
}

static void set_gather_mtu( ReadbackSPU *readback_spu, const char *response )
{
	sscanf( response, "%d", &readback_spu->gather_mtu );
}

static void set_draw_offset( ReadbackSPU *readback_spu, const char *response )
{
	int x, y;
	if (sscanf( response, "[ %d, %d ]", &x, &y) == 2) {
		readback_spu->drawOffsetX = x;
		readback_spu->drawOffsetY = y;
	}
	else {
		crWarning("readback SPU: illegal draw offset %s ignored.", response);
	}
}

static void set_read_size( ReadbackSPU *readback_spu, const char *response )
{
	int x, y;
	if (sscanf( response, "[ %d, %d ]", &x, &y) == 2) {
		readback_spu->readSizeX = x;
		readback_spu->readSizeY = y;
	}
	else {
		crWarning("readback SPU: illegal read size %s ignored.", response);
	}
}

static void set_draw_bbox_outlines( ReadbackSPU *readback_spu, const char *response )
{
	readback_spu->drawBboxOutlines = crStrToInt( response );
}


/* option, type, nr, default, min, max, title, callback
 */
SPUOptions readbackSPUOptions[] = {

	/* Really a multiple choice:  Extract depth or alpha.
	 */
	{ "extract_depth", CR_BOOL, 1, "0", NULL, NULL,
	  "Extract Depth Values", (SPUOptionCB)set_extract_depth },

	{ "extract_alpha", CR_BOOL, 1, "0", NULL, NULL,
	  "Extract Alpha Values", (SPUOptionCB)set_extract_alpha },

	{ "local_visualization", CR_BOOL, 1, "1", NULL, NULL,
	  "Local Visualization", (SPUOptionCB)set_local_visualization },

	{ "visualize_depth", CR_BOOL, 1, "0", NULL, NULL,
	  "Visualize Depth as Grayscale", (SPUOptionCB)set_visualize_depth },

	{ "gather_url", CR_STRING, 1, "", NULL, NULL,
	  "URL to Connect to for Gathering", (SPUOptionCB)set_gather_url },

	{ "gather_mtu", CR_INT, 1, "1048576", "1024", NULL,
	  "MTU for Gathering", (SPUOptionCB)set_gather_mtu },

	{ "draw_offset", CR_INT, 2, "[0,0]", NULL, NULL,
	  "DrawPixels offsets", (SPUOptionCB)set_draw_offset },

	{ "read_size", CR_INT, 2, "[0,0]", NULL, NULL,
	  "ReadPixels size", (SPUOptionCB)set_read_size },

	{ "draw_bbox_outlines", CR_BOOL, 1, "0", NULL, NULL,
	  "Draw Bounding Box Outlines", (SPUOptionCB)set_draw_bbox_outlines },

	{ NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};


static int
ParseVisString( const char *visString )
{
	int mask = 0;
	if (crStrlen(visString) > 0) {
		if (crStrstr(visString, "rgb"))
			mask |= CR_RGB_BIT;
		if (crStrstr(visString, "alpha"))
			mask |= CR_ALPHA_BIT;
		if (crStrstr(visString, "z") || crStrstr(visString, "depth"))
			mask |= CR_DEPTH_BIT;
		if (crStrstr(visString, "stencil"))
			mask |= CR_STENCIL_BIT;
		if (crStrstr(visString, "accum"))
			mask |= CR_ACCUM_BIT;
		if (crStrstr(visString, "stereo"))
			mask |= CR_STEREO_BIT;
		if (crStrstr(visString, "multisample"))
			mask |= CR_MULTISAMPLE_BIT;
		if (crStrstr(visString, "double"))
			mask |= CR_DOUBLE_BIT;
		if (crStrstr(visString, "pbuffer"))
			mask |= CR_PBUFFER_BIT;
	}
	return mask;
}


void readbackspuGatherConfiguration( ReadbackSPU *readback_spu )
{
	CRConnection *conn;

	__setDefaults( readback_spu );

	/* Connect to the mothership and identify ourselves. */
	
	conn = crMothershipConnect( );
	if (!conn)
	{
		/* The mothership isn't running.  Some SPU's can recover gracefully, some 
		 * should issue an error here. */
		crSPUSetDefaultParams( readback_spu, readbackSPUOptions );
		return;
	}
	crMothershipIdentifySPU( conn, readback_spu->id );

	crSPUGetMothershipParams( conn, (void *)readback_spu, readbackSPUOptions );

	/* we can either composite with alpha or Z, but not both */
	if (readback_spu->extract_depth && readback_spu->extract_alpha) {
		crWarning("Readback SPU can't extract both depth and alpha, using depth");
		readback_spu->extract_alpha = 0;
	}

	/* Get the a few options from the Render SPU from which we inherit */
	{
		char response[1000];
		if (crMothershipGetSPUParam( conn, response, "resizable" )) {
			int resizable = 0;
			sscanf(response, "%d", &resizable);
			readback_spu->resizable = resizable;
		}
		if (crMothershipGetSPUParam( conn, response, "render_to_app_window" )) {
			int renderToAppWindow = 0;
			sscanf(response, "%d", &renderToAppWindow);
			readback_spu->renderToAppWindow = renderToAppWindow;
		}
		if (crMothershipGetSPUParam( conn, response, "default_visual" )) {
			readback_spu->default_visual = ParseVisString(response);
		}
		else {
			/* This *MUST* match the default in the Render SPU */
			readback_spu->default_visual = CR_RGB_BIT | CR_DOUBLE_BIT | CR_DEPTH_BIT;
		}

	}

	crMothershipDisconnect( conn );
}
