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

	readback_spu->extract_depth = 0;
	readback_spu->extract_alpha = 0;
	readback_spu->local_visualization = 1;
	readback_spu->visualize_depth = 0;
	readback_spu->depthType = GL_FLOAT;
	readback_spu->drawX = 0;
	readback_spu->drawY = 0;
	readback_spu->barrierCount = 0;
}

void readbackspuGatherConfiguration( ReadbackSPU *readback_spu )
{
	CRConnection *conn;
	char response[8096];

	__setDefaults( readback_spu );

	/* Connect to the mothership and identify ourselves. */
	
	conn = crMothershipConnect( );
	if (!conn)
	{
		/* The mothership isn't running.  Some SPU's can recover gracefully, some 
		 * should issue an error here. */
		return;
	}
	crMothershipIdentifySPU( conn, readback_spu->id );

	if (crMothershipGetSPUParam( conn, response, "extract_depth" ))
	{
		readback_spu->extract_depth = crStrToInt( response );
	}
	if (crMothershipGetSPUParam( conn, response, "extract_alpha" ))
	{
		readback_spu->extract_alpha = crStrToInt( response );
	}
	if (crMothershipGetSPUParam( conn, response, "local_visualization" ))
	{
		readback_spu->local_visualization = crStrToInt( response );
	}
	if (crMothershipGetSPUParam( conn, response, "visualize_depth" ))
	{
		readback_spu->visualize_depth = crStrToInt( response );
	}
	if (crMothershipGetSPUParam( conn, response, "drawpixels_pos" ) )
	{
		sscanf( response, "%d %d", &readback_spu->drawX, &readback_spu->drawY );
	}

	/* we can either composite with alpha or Z, but not both */
	if (readback_spu->extract_depth && readback_spu->extract_alpha) {
		crWarning("Readback SPU can't extract both depth and alpha, using depth");
		readback_spu->extract_alpha = 0;
	}

	crMothershipDisconnect( conn );
}
