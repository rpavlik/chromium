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
#include "cr_warp.h"
#include "cr_hull.h"

#include "server.h"


static void
__setDefaults(void)
{
	cr_server.tcpip_port = 7000;
	cr_server.numExtents = 0;
	cr_server.curExtent = 0;
	cr_server.muralWidth = 0;
	cr_server.muralHeight = 0;
	cr_server.optimizeBucket = 1;
	cr_server.useL2 = 0;
	cr_server.maxBarrierCount = 0;
	cr_server.only_swap_once = 0;
	cr_server.debug_barriers = 0;
	cr_server.SpuContext = 0;
}

void
crServerGatherConfiguration(char *mothership)
{
	CRConnection *conn;
	char response[8096];

	char **spuchain;
	int num_spus;
	int *spu_ids;
	char **spu_names;
	char *spu_dir;
	int i;
	/* Quadrics defaults */
	int my_rank = 0;
	int low_context = CR_QUADRICS_DEFAULT_LOW_CONTEXT;
	int high_context = CR_QUADRICS_DEFAULT_HIGH_CONTEXT;
	char *low_node = "none";
	char *high_node = "none";

	char **clientchain, **clientlist;
	int numClients;

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
	spu_names = (char **) crAlloc(num_spus * sizeof(*spu_names));
	for (i = 0; i < num_spus; i++)
	{
		spu_ids[i] = crStrToInt(spuchain[2 * i + 1]);
		spu_names[i] = crStrdup(spuchain[2 * i + 2]);
		crDebug("SPU %d/%d: (%d) \"%s\"", i + 1, num_spus, spu_ids[i],
						spu_names[i]);
	}

	if (crMothershipGetServerParam( conn, response, "spu_dir" ) && crStrlen(response) > 0)
	{
		spu_dir = response;
	}
	else
	{
		spu_dir = NULL;
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

	cr_server.head_spu =
		crSPULoadChain(num_spus, spu_ids, spu_names, spu_dir, &cr_server);

	/* Need to do this as early as possible */
	cr_server.head_spu->dispatch_table.GetIntegerv(GL_VIEWPORT,
																								 (GLint *) cr_server.
																								 underlyingDisplay);

	/* Get OpenGL limits from first SPU */
	crSPUQueryGLLimits(conn, spu_ids[0], &cr_server.limits);

	crFree(spu_ids);
	crFree(spu_names);
	crFree(spuchain);


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
	if (crMothershipGetServerParam(conn, response, "only_swap_once"))
	{
		cr_server.only_swap_once = crStrToInt(response);
	}
	if (crMothershipGetServerParam(conn, response, "debug_barriers"))
	{
		cr_server.debug_barriers = crStrToInt(response);
	}


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
		client->conn = crNetAcceptClient(cr_server.protocol,
																		 cr_server.tcpip_port, cr_server.mtu, 1);

	}

	/* Ask the mothership for the tile info */
	crServerGetTileInfo(conn);

	crMothershipDisconnect(conn);
}

float
absf(float a)
{
	if (a < 0)
		return -a;
	return a;
}

/*
 * Ask the mothership for our tile information.
 * Also, pose as one of the tilesort SPUs and query all servers for their
 * tile info in order to compute the total mural size.
 */
void
crServerGetTileInfo(CRConnection * conn)
{
	char response[8096];

	if ((!crMothershipGetServerTiles(conn, response)) &&
			(!crMothershipGetServerDisplayTiles(conn, response)))
	{
		crDebug("No tiling information for server!");
	}
	else
	{
		/* response is of the form: "N x y w h, x y w h, ..."
		 * where N is the number of tiles and each set of 'x y w h'
		 * values describes the tile location and size.
		 */
		char **tilechain, **tilelist;
		char **displaylist, **displaychain;
		int num_displays;
		int i;
		float our_id = 0.0f;

		/* first, grab all the displays so we know what the matricies are */
		crMothershipGetDisplays(conn, response);
		displaychain = crStrSplitn(response, " ", 1);
		displaylist = crStrSplit(displaychain[1], ",");
		num_displays = crStrToInt(displaychain[0]);

		if (cr_server.localTileSpec)
			crMothershipGetServerDisplayTiles(conn, response);
		else
			crMothershipGetServerTiles(conn, response);

		tilechain = crStrSplitn(response, " ", 1);
		cr_server.numExtents = crStrToInt(tilechain[0]);
		cr_server.maxTileHeight = 0;
		tilelist = crStrSplit(tilechain[1], ",");

		for (i = 0; i < cr_server.numExtents; i++)
		{
			float id, x, y, w, h;

			if (cr_server.localTileSpec)
			{
				sscanf(tilelist[i], "%f %f %f %f %f", &id, &x, &y, &w, &h);
				crDebug("got tile %f %f %f %f %f\n", id, x, y, w, h);
				if (i == 0)
					our_id = id;
				else
				{
					if (our_id != id)
						crError("Can't have multiple displays IDs on the same server!");
				}
			}
			else
				sscanf(tilelist[i], "%f %f %f %f", &x, &y, &w, &h);

			cr_server.extents[i].x1 = (int) x;
			cr_server.extents[i].y1 = (int) y;
			cr_server.extents[i].x2 = (int) x + (int) w;
			cr_server.extents[i].y2 = (int) y + (int) h;
			if (h > cr_server.maxTileHeight)
			{
				cr_server.maxTileHeight = (int) h;
			}
			crDebug("Added tile: %d %d %d %d",
							cr_server.extents[i].x1, cr_server.extents[i].y1,
							cr_server.extents[i].x2, cr_server.extents[i].y2);
		}

		if ((cr_server.localTileSpec) && (num_displays))
		{
			int w, h, id, idx;
			float pnt[2], tmp[9], hom[9], hom_inv[9], Sx, Sy;
			float cent[2], warped[2];
			double *corners, bbox[4];

			corners = (double *) crAlloc(num_displays * 8 * sizeof(double));
			idx = 0;

			for (i = 0; i < num_displays; i++)
			{
				sscanf(displaylist[i],
							 "%d %d %d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
							 &id, &w, &h,
							 &hom[0], &hom[1], &hom[2], &hom[3],
							 &hom[4], &hom[5], &hom[6], &hom[7], &hom[8],
							 &hom_inv[0], &hom_inv[1], &hom_inv[2], &hom_inv[3],
							 &hom_inv[4], &hom_inv[5], &hom_inv[6], &hom_inv[7],
							 &hom_inv[8]);

				pnt[0] = -1;
				pnt[1] = -1;
				crWarpPoint(hom_inv, pnt, warped);
				corners[idx] = warped[0];
				corners[idx + 1] = warped[1];
				idx += 2;

				pnt[0] = -1;
				pnt[1] = 1;
				crWarpPoint(hom_inv, pnt, warped);
				corners[idx] = warped[0];
				corners[idx + 1] = warped[1];
				idx += 2;

				pnt[0] = 1;
				pnt[1] = 1;
				crWarpPoint(hom_inv, pnt, warped);
				corners[idx] = warped[0];
				corners[idx + 1] = warped[1];
				idx += 2;

				pnt[0] = 1;
				pnt[1] = -1;
				crWarpPoint(hom_inv, pnt, warped);
				corners[idx] = warped[0];
				corners[idx + 1] = warped[1];
				idx += 2;

				if (id == our_id)
				{
					crMemcpy(tmp, hom, 9 * sizeof(float));
				}
			}

			/* now compute the bounding box of the display area */
			crHullInteriorBox(corners, idx / 2, bbox);

			Sx = (float)(bbox[2] - bbox[0]) * (float)0.5;
			Sy = (float)(bbox[3] - bbox[1]) * (float)0.5;

			cent[0] = (float)(bbox[0] + bbox[2]) / ((float)2.0 * Sx);
			cent[1] = (float)(bbox[1] + bbox[3]) / ((float)2.0 * Sy);

			cr_server.alignment_matrix[0] = tmp[0] * Sx;
			cr_server.alignment_matrix[1] = tmp[3] * Sx;
			cr_server.alignment_matrix[2] = 0;
			cr_server.alignment_matrix[3] = tmp[6] * Sx;
			cr_server.alignment_matrix[4] = tmp[1] * Sy;
			cr_server.alignment_matrix[5] = tmp[4] * Sy;
			cr_server.alignment_matrix[6] = 0;
			cr_server.alignment_matrix[7] = tmp[7] * Sy;
			cr_server.alignment_matrix[8] = 0;
			cr_server.alignment_matrix[9] = 0;
			cr_server.alignment_matrix[10] = tmp[6] * Sx * cent[0] + 
																			 tmp[7] * Sy * cent[1] + 
																			 tmp[8] -
																			 absf(tmp[6] * Sx) -
																			 absf(tmp[7] * Sy);
			cr_server.alignment_matrix[11] = 0;
			cr_server.alignment_matrix[12] =
				tmp[2] + tmp[0] * Sx * cent[0] + tmp[1] * Sy * cent[1];
			cr_server.alignment_matrix[13] =
				tmp[5] + tmp[3] * Sx * cent[0] + tmp[4] * Sy * cent[1];
			cr_server.alignment_matrix[14] = 0;
			cr_server.alignment_matrix[15] =
				tmp[8] + tmp[6] * Sx * cent[0] + tmp[7] * Sy * cent[1];

			crFree(corners);

			crFreeStrings(displaychain);
			crFreeStrings(displaylist);
		}

		crFreeStrings(tilechain);
		crFreeStrings(tilelist);
	}


	if (cr_server.clients[0].spu_id != -1) // out-of-nowhere file client
	{
		/* Sigh -- the servers need to know how big the whole mural is if we're 
		 * doing tiling, so they can compute their base projection.  For now, 
		 * just have them pretend to be one of their client SPU's, and redo 
		 * the configuration step of the tilesort SPU.  Basically this is a dirty 
		 * way to figure out who the other servers are.  It *might* matter 
		 * which SPU we pick for certain graph configurations, but we'll cross 
		 * that bridge later.
		 *
		 * As long as we're at it, we're going to verify that all the tile
		 * sizes are uniform when optimizeBucket is true.
		 */

		int optTileWidth = 0, optTileHeight = 0;
		int num_servers;
		int i;
		char **serverchain;

		crMothershipIdentifySPU(conn, cr_server.clients[0].spu_id);
		crMothershipGetServers(conn, response);

		/* crMothershipGetServers() response is of the form
		 * "N protocol://ip:port protocol://ipnumber:port ..."
		 * For example: "2 tcpip://10.0.0.1:7000 tcpip://10.0.0.2:7000"
		 */
		serverchain = crStrSplitn(response, " ", 1);
		num_servers = crStrToInt(serverchain[0]);

		if (num_servers == 0)
		{
			crError("No servers specified for SPU %d?!",
							cr_server.clients[0].spu_id);
		}

		for (i = 0; i < num_servers; i++)
		{
			char **tilechain, **tilelist;
			int numExtents;
			int tile;

			if (cr_server.localTileSpec)
			{
				if (!crMothershipGetDisplayTiles(conn, response, i))
				{
					break;
				}
			}
			else
			{
				if (!crMothershipGetTiles(conn, response, i))
				{
					break;
				}
			}

			tilechain = crStrSplitn(response, " ", 1);
			numExtents = crStrToInt(tilechain[0]);

			tilelist = crStrSplit(tilechain[1], ",");

			for (tile = 0; tile < numExtents; tile++)
			{
				int w, h, id;
				int x1, y1;
				int x2, y2;

				if (cr_server.localTileSpec)
					sscanf(tilelist[tile], "%d %d %d %d %d", &id, &x1, &y1, &w, &h);
				else
					sscanf(tilelist[tile], "%d %d %d %d", &x1, &y1, &w, &h);

				x2 = x1 + w;
				y2 = y1 + h;

				if (x2 > (int) cr_server.muralWidth)
					cr_server.muralWidth = x2;

				if (y2 > (int) cr_server.muralHeight)
					cr_server.muralHeight = y2;

				if (cr_server.optimizeBucket)
				{
					if (optTileWidth == 0 && optTileHeight == 0)
					{
						/* first tile */
						optTileWidth = w;
						optTileHeight = h;
					}
					else
					{
						/* subsequent tile - make sure it's the same size as first */
						if (w != optTileWidth || h != optTileHeight)
						{
							crWarning("Tile %d on server %d is not the right size!",
												tile, i);
							crWarning("All tiles must be same size with optimize_bucket.");
							crWarning("Turning off server's optimize_bucket.");
							cr_server.optimizeBucket = 0;
						}
						else if (x1 % optTileWidth != 0 || x2 % optTileWidth != 0 ||
										 y1 % optTileHeight != 0 || y2 % optTileHeight != 0)
						{
							crWarning
								("Tile %d on server %d is not positioned correctly to use optimize_bucket.",
								 tile, i);
							crWarning("Turning off server's optimize_bucket.");
							cr_server.optimizeBucket = 0;
						}
					}
				}
			}
			crFreeStrings(tilechain);
			crFreeStrings(tilelist);
		}
		crFreeStrings(serverchain);
		crWarning("Total output dimensions = (%d, %d)",
							cr_server.muralWidth, cr_server.muralHeight);
	}
	else
	{
		crWarning
			("It looks like there are nothing but file clients.  That suits me just fine.");
	}
}
