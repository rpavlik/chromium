/* Copyright (c) 2003, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mothership.h"
#include "cr_string.h"
#include "zpixspu.h"


static void
__setDefaults(ZpixSPU * zpix_spu)
{
	zpix_spu->verbose = 0;
	zpix_spu->debug = 0;
	zpix_spu->client_id = 0;
	zpix_spu->no_diff = 0;     /* use  buffer differencing */
	zpix_spu->ztype = GL_ZLIB_COMPRESSION_CR;    /* gnu zlib                 */
	zpix_spu->ztype_parm = -1; /* zlib's default parm */
}

static void
set_verbose(ZpixSPU * zpix_spu, const char *response)
{
	zpix_spu->verbose = crStrToInt(response);
	crDebug("Zpix SPU config: verbose = %d", zpix_spu->verbose);
}

static void
set_debug(ZpixSPU * zpix_spu, const char *response)
{
	zpix_spu->debug = crStrToInt(response);
	crDebug("Zpix SPU config: debug = %d", zpix_spu->debug);
}

static void
set_client_id(ZpixSPU * zpix_spu, const char *response)
{
	zpix_spu->client_id = crStrToInt(response);
	crDebug("Zpix SPU config: client_id = %d", zpix_spu->client_id);
}

static void
set_no_diff(ZpixSPU * zpix_spu, const char *response)
{
	zpix_spu->no_diff = crStrToInt(response);
	crDebug("Zpix SPU config: no_diff = %d", zpix_spu->no_diff);
}

static void
set_ztype(ZpixSPU * zpix_spu, const char *response)
{
	if (crStrcmp(response, "None") == 0)
		zpix_spu->ztype = GL_NONE;
	else if (crStrcmp(response, "Zlib") == 0)
		zpix_spu->ztype = GL_ZLIB_COMPRESSION_CR;
	else if (crStrcmp(response, "RLE") == 0)
		zpix_spu->ztype = GL_RLE_COMPRESSION_CR;
	else if (crStrcmp(response, "PLE") == 0)
		zpix_spu->ztype = GL_PLE_COMPRESSION_CR;
	else
	{
		crWarning("Bad value (%s) for ZPix compression ", response);
		zpix_spu->ztype = GL_ZLIB_COMPRESSION_CR;
	}

	crDebug("Zpix SPU config: ztype = %d", zpix_spu->ztype);
}

static void
set_ztype_parm(ZpixSPU * zpix_spu, const char *response)
{
	zpix_spu->ztype_parm = crStrToInt(response);
	crDebug("Zpix SPU config: ztype_parm = %d", zpix_spu->ztype_parm);
}


/* option, type, nr, default, min, max, title, callback
 */
SPUOptions zpixSPUOptions[] = {
	{"ztype", CR_ENUM, 1, "Zlib", "'None', 'Zlib', 'RLE', 'PLE'", NULL,
	 "Compression type", (SPUOptionCB) set_ztype},
	{"ztype_parm", CR_INT, 1, "-1", "-1", NULL,
	 "Compression parameter", (SPUOptionCB) set_ztype_parm},
	{"client_id", CR_INT, 1, "0", NULL, NULL,
	 "Client ID when using multiple clients", (SPUOptionCB) set_client_id},
	{"no_diff", CR_BOOL, 1, "0", NULL, NULL,
	 "Suppress inter-frame differencing", (SPUOptionCB) set_no_diff},
	{"debug", CR_BOOL, 1, "0", NULL, NULL,
	 "Debug mode", (SPUOptionCB) set_debug},
	{"verbose", CR_BOOL, 1, "0", NULL, NULL,
	 "Verbose messages", (SPUOptionCB) set_verbose},
	{NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL},
};


void
zpixspuGatherConfiguration(ZpixSPU * zpix_spu)
{
	CRConnection *conn;

	__setDefaults(zpix_spu);

	/* Connect to the mothership and identify ourselves. */

	conn = crMothershipConnect();
	if (!conn)
	{
		/* The mothership isn't running.  Some SPU's can recover gracefully, some 
		 * should issue an error here. */
		crSPUSetDefaultParams(zpix_spu, zpixSPUOptions);
		return;
	}
	crMothershipIdentifySPU(conn, zpix_spu->id);

	crSPUGetMothershipParams(conn, (void *) zpix_spu, zpixSPUOptions);

	crMothershipDisconnect(conn);
}
