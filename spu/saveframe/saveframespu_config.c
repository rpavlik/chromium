/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifndef WINDOWS
#include <unistd.h>
#endif
#include "saveframespu.h"

#include "cr_mothership.h"
#include "cr_string.h"
#include "cr_mem.h"

static void
__setDefaults(void)
{
	saveframe_spu.framenum = 0;
	saveframe_spu.buffer = NULL;
	saveframe_spu.spec = NULL;

#ifdef JPEG
	saveframe_spu.cinfo.err = jpeg_std_error(&saveframe_spu.jerr);
	jpeg_create_compress(&saveframe_spu.cinfo);
#endif
}

void
set_stride(void *foo, const char *response)
{
	sscanf(response, "%d", &saveframe_spu.stride);
}

void
set_binary(void *foo, const char *response)
{
	sscanf(response, "%d", &saveframe_spu.binary);
}

void
set_spec(void *foo, const char *response)
{
	if (saveframe_spu.spec && (!crStrcmp(response, "frame%d.ppm")))
		return;											/* Specified basename and only default spec. */

	if (saveframe_spu.spec)
		crFree(saveframe_spu.spec);

	saveframe_spu.spec = crStrdup(response);
}

void
set_basename(void *foo, const char *response)
{
	int rl = crStrlen(response);
	if (rl)
	{
		saveframe_spu.spec = crAlloc(rl + 7);
		/* This relies on saveframe_spu.format being initialized first! */
		sprintf(saveframe_spu.spec, "%s%%d.%s", response, saveframe_spu.format);
	}
}

void
set_format(void *foo, const char *response)
{
	saveframe_spu.format = crStrdup(response);
}

void
set_single(void *foo, const char *response)
{
	sscanf(response, "%d", &saveframe_spu.single);
}

void
set_geometry(void *foo, const char *response)
{
	int x, y, w, h, result;
	result = sscanf(response, "%d, %d, %d, %d", &x, &y, &w, &h);
	saveframe_spu.x = result > 0 ? x : 0;
	saveframe_spu.y = result > 1 ? y : 0;
	saveframe_spu.width = result > 2 ? w : -1;
	saveframe_spu.height = result > 3 ? h : -1;

	if ((saveframe_spu.width != -1) && (saveframe_spu.height != -1))
		ResizeBuffer();
}

void
set_enabled(void *foo, const char *response)
{
	sscanf(response, "%d", &saveframe_spu.enabled);
}



/* option, type, nr, default, min, max, title, callback
 */
SPUOptions saveframeSPUOptions[] = {
	{"stride", CR_INT, 1, "1", "1", NULL,
	 "Filename Number Stride", (SPUOptionCB) set_stride},

	/* Do not move "format" below "basename"... set_basename uses format */
	{"format", CR_ENUM, 1, "ppm",
#ifdef JPEG
	 "'ppm', 'jpeg'",
#else
	 "'ppm'",
#endif
	 NULL,
	 "Image Format", (SPUOptionCB) set_format},

	{"basename", CR_STRING, 1, "", NULL, NULL,
	 "File Basename (obsolete, use spec)", (SPUOptionCB) set_basename},

	{"spec", CR_STRING, 1, "frame%d.ppm", NULL, NULL,
	 "Filename Specification", (SPUOptionCB) set_spec},

	{"single", CR_INT, 1, "-1", "-1", NULL,
	 "Single Frame Number", (SPUOptionCB) set_single},

	{"binary", CR_BOOL, 1, "1", NULL, NULL,
	 "Binary PPM format", (SPUOptionCB) set_binary},

	{"geometry", CR_INT, 4, "0, 0, -1, -1", "0, 0, 1, 1", NULL,
	 "Geometry (x, y, w, h)", (SPUOptionCB) set_geometry},

	{"enabled", CR_INT, 1, "1", "1", NULL,
	 "Start Enabled", (SPUOptionCB) set_enabled},

	{NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL},
};

void
saveframespuGatherConfiguration( const SPU *child_spu )
{
	CRConnection *conn;

	__setDefaults();

	/* Connect to the mothership and identify ourselves. */

	conn = crMothershipConnect();
	if (!conn)
	{
		/* The mothership isn't running.  Some SPU's can recover gracefully,
		 * some should issue an error here. */
		crSPUSetDefaultParams(&saveframe_spu, saveframeSPUOptions);
		return;
	}
	crMothershipIdentifySPU(conn, saveframe_spu.id);

	crSPUGetMothershipParams(conn, &saveframe_spu, saveframeSPUOptions);

        crSPUPropogateGLLimits( conn, saveframe_spu.id, child_spu, NULL );

	crMothershipDisconnect(conn);
}
