/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mothership.h"
#include "cr_mem.h"
#include "cr_environment.h"
#include "cr_string.h"
#include "cr_error.h"

#include "server.h"

#ifdef WINDOWS
#pragma warning( disable: 4706 )
#endif


static void
__setDefaults(void)
{
	unsigned int i;

	cr_server.tcpip_port = 7000;
	cr_server.optimizeBucket = 1;
	cr_server.useL2 = 0;
	cr_server.maxBarrierCount = 0;
	cr_server.ignore_papi = 0;
	cr_server.only_swap_once = 0;
	cr_server.overlapBlending = 0;
	cr_server.debug_barriers = 0;
	cr_server.sharedDisplayLists = 1;
	cr_server.sharedTextureObjects = 1;
	cr_server.sharedPrograms = 1;
	cr_server.useDMX = 0;

	cr_server.num_overlap_intens = 0;
	cr_server.overlap_intens = 0;
	cr_server.SpuContext = 0;

	for (i = 0; i < CR_MAX_CONTEXTS; i++)
		cr_server.context[i] = NULL;
}

void
crServerGatherConfiguration(char *mothership)
{
	CRMuralInfo *defaultMural;
	CRConnection *conn;
	char response[8096];

	char **spuchain;
	int num_spus;
	int *spu_ids;
	char **spu_names;
	char *spu_dir = NULL;
	int i;
	/* Quadrics defaults */
	int my_rank = 0;
	int low_context = CR_QUADRICS_DEFAULT_LOW_CONTEXT;
	int high_context = CR_QUADRICS_DEFAULT_HIGH_CONTEXT;
	char *low_node = "none";
	char *high_node = "none";

	char **clientchain, **clientlist;
	int numClients;

	defaultMural = (CRMuralInfo *) crHashtableSearch(cr_server.muralTable, 0);
	CRASSERT(defaultMural);

	__setDefaults();

	if (mothership)
	{
		crSetenv("CRMOTHERSHIP", mothership);
	}

	conn = crMothershipConnect();

	if (!conn)
	{
		crError
			("Couldn't connect to the mothership -- I have no idea what to do!");
	}

	/* The response will tell which SPUs to load */
	crMothershipIdentifyServer(conn, response);

	spuchain = crStrSplit(response, " ");
	num_spus = crStrToInt(spuchain[0]);
	spu_ids = (int *) crAlloc(num_spus * sizeof(*spu_ids));
	spu_names = (char **) crAlloc((num_spus + 1) * sizeof(*spu_names));
	for (i = 0; i < num_spus; i++)
	{
		spu_ids[i] = crStrToInt(spuchain[2 * i + 1]);
		spu_names[i] = crStrdup(spuchain[2 * i + 2]);
		crDebug("SPU %d/%d: (%d) \"%s\"", i + 1, num_spus, spu_ids[i],
						spu_names[i]);
	}
	spu_names[i] = NULL;

	/*
	 * Gather configuration options.
	 */
	if (crMothershipGetServerParam( conn, response, "spu_dir" ) && crStrlen(response) > 0)
	{
		spu_dir = crStrdup(response);
	}

	if (crMothershipGetRank(conn, response))
	{
		my_rank = crStrToInt(response);
	}
	crNetSetRank(my_rank);

	if (crMothershipGetParam(conn, "low_context", response))
	{
		low_context = crStrToInt(response);
	}
	if (crMothershipGetParam(conn, "high_context", response))
	{
		high_context = crStrToInt(response);
	}
	crNetSetContextRange(low_context, high_context);

	if (crMothershipGetParam(conn, "low_node", response))
	{
		low_node = crStrdup(response);
	}
	if (crMothershipGetParam(conn, "high_node", response))
	{
		high_node = crStrdup(response);
	}
	crNetSetNodeRange(low_node, high_node);
	if (low_node)
		crFree(low_node);
	if (high_node)
		crFree(high_node);

	if (crMothershipGetServerParam(conn, response, "port"))
	{
		cr_server.tcpip_port = crStrToInt(response);
	}
	if (crMothershipGetServerParam(conn, response, "optimize_bucket"))
	{
		cr_server.optimizeBucket = crStrToInt(response);
	}
	if (crMothershipGetServerParam(conn, response, "local_tile_spec"))
	{
		cr_server.localTileSpec = crStrToInt(response);
	}
	if (crMothershipGetServerParam(conn, response, "lightning2"))
	{
		cr_server.useL2 = crStrToInt(response);
	}
	if (crMothershipGetServerParam(conn, response, "ignore_papi"))
	{
		cr_server.ignore_papi = crStrToInt(response);
	}
	if (crMothershipGetServerParam(conn, response, "overlap_blending"))
	{
		if (!crStrcmp(response, "blend"))
			cr_server.overlapBlending = 1;
		else if (!crStrcmp(response, "knockout"))
			cr_server.overlapBlending = 2;
	}
	if (crMothershipGetServerParam(conn, response, "overlap_levels"))
	{
		int a;
		char **levels, *found;

		/* remove the silly []'s */
		while ((found = crStrchr(response, '[')))
			*found = ' ';
		while ((found = crStrchr(response, ']')))
			*found = ' ';

		levels = crStrSplit(response, ",");

		a = 0;
		while (levels[a] != NULL)
		{
			crDebug("%d: %s", a, levels[a]);
			cr_server.num_overlap_intens++;
			a++;
		}

		cr_server.overlap_intens = (float *)crAlloc(cr_server.num_overlap_intens*sizeof(float));
		for (a=0; a<cr_server.num_overlap_intens; a++)
			cr_server.overlap_intens[a] = crStrToFloat(levels[a]);

		crFreeStrings(levels);
	}
	if (crMothershipGetServerParam(conn, response, "only_swap_once"))
	{
		cr_server.only_swap_once = crStrToInt(response);
	}
	if (crMothershipGetServerParam(conn, response, "debug_barriers"))
	{
		cr_server.debug_barriers = crStrToInt(response);
	}
	if (crMothershipGetServerParam(conn, response, "shared_display_lists"))
	{
		cr_server.sharedDisplayLists = crStrToInt(response);
	}
	if (crMothershipGetServerParam(conn, response, "shared_texture_objects"))
	{
		cr_server.sharedTextureObjects = crStrToInt(response);
	}
	if (crMothershipGetServerParam(conn, response, "shared_programs"))
	{
		cr_server.sharedPrograms = crStrToInt(response);
	}
	if (crMothershipGetServerParam(conn, response, "use_dmx"))
	{
		cr_server.useDMX = crStrToInt(response);
	}

	/*
	 * Load the SPUs
	 */
	cr_server.head_spu =
		crSPULoadChain(num_spus, spu_ids, spu_names, spu_dir, &cr_server);

	/* Need to do this as early as possible */

	/* XXX DMX get window size instead? */
	cr_server.head_spu->dispatch_table.GetIntegerv(GL_VIEWPORT,
															 (GLint *) defaultMural->underlyingDisplay);

	crFree(spu_ids);
	crFreeStrings(spu_names);
	crFreeStrings(spuchain);
	if (spu_dir)
		crFree(spu_dir);

	/*
	 * NOTICE:
	 * if you add new network node config options, please add them to the
	 * configuration options list in mothership/tools/crtypes.py
	 */

	cr_server.mtu = crMothershipGetMTU( conn );

	/* The response will tell us what protocols we need to serve 
	 * example: "3 tcpip 1,gm 2,via 10" */

	crMothershipGetClients(conn, response);

	clientchain = crStrSplitn(response, " ", 1);
	numClients = crStrToInt(clientchain[0]);
	if (numClients == 0)
	{
		crError("I have no clients!  What's a poor server to do?");
	}
	clientlist = crStrSplit(clientchain[1], ",");

	cr_server.numClients = numClients;

	/*
	 * Allocate and initialize the cr_server.clients[] array.
	 * Call crNetAcceptClient() for each client.
	 * Also, look for a client that's _not_ using the file: protocol.
	 */
	cr_server.clients = (CRClient *) crAlloc(sizeof(CRClient) * numClients);
	for (i = 0; i < numClients; i++)
	{
		CRClient *client = &cr_server.clients[i];

		crMemZero(client, sizeof(CRClient));

		cr_server.clients[i].number = i;

		sscanf(clientlist[i], "%s %d", cr_server.protocol, &(client->spu_id));
		client->conn = crNetAcceptClient(cr_server.protocol, NULL,
																		 cr_server.tcpip_port, cr_server.mtu, 1);

	}

	crFreeStrings(clientchain);
	crFreeStrings(clientlist);

	/* Ask the mothership for the tile info */
	crServerGetTileInfoFromMothership(conn, defaultMural);

	crMothershipDisconnect(conn);
}

