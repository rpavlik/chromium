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
	/* default window */
	readback_spu->windows[0].inUse = GL_TRUE;
	readback_spu->windows[0].renderWindow = 0;
	readback_spu->windows[0].childWindow = 0;
	readback_spu->depthType = GL_FLOAT;
	readback_spu->barrierCount = 0;

	/* This will make the viewport be computed later */
	readback_spu->halfViewportWidth = 0;
	readback_spu->cleared_this_frame = 0;
	readback_spu->bbox = NULL;
}

void set_extract_depth( ReadbackSPU *readback_spu, const char *response )
{
   readback_spu->extract_depth = crStrToInt( response );
}

void set_extract_alpha( ReadbackSPU *readback_spu, const char *response )
{
   readback_spu->extract_alpha = crStrToInt( response );
}

void set_local_visualization( ReadbackSPU *readback_spu, const char *response )
{
   readback_spu->local_visualization = crStrToInt( response );
}

void set_visualize_depth( ReadbackSPU *readback_spu, const char *response )
{
   readback_spu->visualize_depth = crStrToInt( response );
}

void set_drawpixels_pos( ReadbackSPU *readback_spu, const char *response )
{
   sscanf( response, "%d %d", &readback_spu->drawX, &readback_spu->drawY );
}


void set_display_resolution( ReadbackSPU *readback_spu, const char *response )
{
   sscanf( response, "%d %d", &readback_spu->resX, &readback_spu->resY );
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

   { "drawpixels_pos", CR_INT, 2, "0, 0", "0, 0", NULL,
     "glDrawPixels Position (x,y)", (SPUOptionCB)set_drawpixels_pos },

   { "display_resolution", CR_INT, 2, "0", "0", NULL,
     "resolution of the output device (x,y)", (SPUOptionCB)set_display_resolution },

   { NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },

};


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

	crMothershipDisconnect( conn );
}
