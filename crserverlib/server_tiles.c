/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "server.h"
#include "cr_mothership.h"
#include "cr_mem.h"
#include "cr_warp.h"
#include "cr_hull.h"
#include "cr_string.h"


/*
 * Given the mural->extents[i].imagewindow parameters, compute the
 * mural->extents[i].bounds and outputwindow parameters.
 * This includes allocating space in the render SPU's window for each
 * of the tiles.
 */
static void
initializeExtents(CRMuralInfo *mural)
{
	int i;
	int x, y, w, h, y_max;

	x = cr_server.useL2 ? 2 : 0;
	y = 0;
	y_max = 0;

	/* Basically just copy the server's list of tiles to the RunQueue
	 * and compute some derived tile information.
	 */
	for ( i = 0; i < mural->numExtents; i++ )
	{
		CRExtent *extent = &mural->extents[i];

		/* extent->display = find_output_display( extent->imagewindow ); */

		/* Compute normalized tile bounds.
		 * That is, x1, y1, x2, y2 will be in the range [-1, 1] where
		 * x1=-1, y1=-1, x2=1, y2=1 corresponds to the whole mural.
		 */
		extent->bounds.x1 = ( (GLfloat) (2*extent->imagewindow.x1) /
				mural->width - 1.0f );
		extent->bounds.y1 = ( (GLfloat) (2*extent->imagewindow.y1) /
				mural->height - 1.0f );
		extent->bounds.x2 = ( (GLfloat) (2*extent->imagewindow.x2) /
				mural->width - 1.0f );
		extent->bounds.y2 = ( (GLfloat) (2*extent->imagewindow.y2) /
				mural->height - 1.0f );

		/* Width and height of tile, in mural pixels */
		w = mural->extents[i].imagewindow.x2 - mural->extents[i].imagewindow.x1;
		h = mural->extents[i].imagewindow.y2 - mural->extents[i].imagewindow.y1;

		if (cr_server.useDMX) {
			/* Tile layout is easy!
			 * Remember, the X window we're drawing into (tileosort::xsubwin)
			 * is already adjusted to the right mural position.
			 */
			extent->outputwindow.x1 = 0;
			extent->outputwindow.y1 = 0;
			extent->outputwindow.x2 = w;
			extent->outputwindow.y2 = h;
#if 0
			printf("OutputWindow %d, %d .. %d, %d\n",
						 extent->outputwindow.x1,
						 extent->outputwindow.y1,
						 extent->outputwindow.x2,
						 extent->outputwindow.y2);
#endif
		}
		else {
			/* Carve space out of the underlying (render SPU) window for this tile.
			 */
			if ( x + w > (int) mural->underlyingDisplay[2] )
			{
				y += y_max;
				x = ((cr_server.useL2) ? 2 : 0);
				y_max = 0;
			}

			extent->outputwindow.x1 = x;
			extent->outputwindow.y1 = ( mural->underlyingDisplay[3] - mural->maxTileHeight - y );
			extent->outputwindow.x2 = x + w;
			extent->outputwindow.y2 = ( mural->underlyingDisplay[3] - mural->maxTileHeight - y + h );

			if (extent->outputwindow.y1 < 0)
				crWarning("Ran out of room for tiles in this server's window (%d x %d)!!!", mural->underlyingDisplay[2], mural->underlyingDisplay[3]);

			if ( y_max < h )
				y_max = h;

			x += w + ((cr_server.useL2) ? 2 : 0);
		} /* useDMX */
	}
}


/*
 * This needs to be called when the tiling changes.  Compute max tile
 * height and check if optimized bucketing can be used, etc.
 */
void
crServerInitializeTiling(CRMuralInfo *mural)
{
	int i;

	/* The imagespace rect is useful in a few places (but redundant) */
	mural->imagespace.x1 = 0;
	mural->imagespace.y1 = 0;
	mural->imagespace.x2 = mural->width;
	mural->imagespace.y2 = mural->height;

	/* find max tile height */
	mural->maxTileHeight = 0;
	for (i = 0; i < mural->numExtents; i++)
	{
		const int h = mural->extents[i].imagewindow.y2 -
			mural->extents[i].imagewindow.y1;

		if (h > mural->maxTileHeight)
			mural->maxTileHeight = h;
	}

	/* compute extent bounds, outputwindow, etc */
	initializeExtents(mural);

	/* optimized hash-based bucketing setup */
	if (cr_server.optimizeBucket) {
		 mural->optimizeBucket = crServerInitializeBucketing(mural);
	}
	else {
		 mural->optimizeBucket = GL_FALSE;
	}
}


/*
 * After we've received the tile parameters from the mothership
 * we do all the initialization to perform tile sorting.
 * NOTE: we need to have a current rendering context at this point!!!
 */
void crServerBeginTiling(CRMuralInfo *mural)
{
	if (mural->numExtents > 0)
	{
		unsigned int j;
		for ( j = 0 ; j < cr_server.numClients ; j++)
		{
			crServerRecomputeBaseProjection( &(cr_server.clients[j].baseProjection),
																			 0, 0,
																			 mural->width,
																			 mural->height );
		}
		cr_server.head_spu->dispatch_table.MatrixMode( GL_PROJECTION );
		cr_server.head_spu->dispatch_table.LoadMatrixf( (GLfloat *) &(cr_server.clients[0].baseProjection) );
	}
}


/*
 * Change the tiling for a mural.
 * The boundaries are specified in mural space.
 * Input: muralWidth/muralHeight - new window/mural size
 *        numTiles - number of tiles
 * Input: tileBounds[0] = bounds[0].x
 *        tileBounds[1] = bounds[0].y
 *        tileBounds[2] = bounds[0].width
 *        tileBounds[3] = bounds[0].height
 *        tileBounds[4] = bounds[1].x
 *        ...
 */
void
crServerNewMuralTiling(CRMuralInfo *mural,
											 int muralWidth, int muralHeight,
											 int numTiles, const int *tileBounds)
{
	int i;

	CRASSERT(numTiles >= 0);

	crDebug("Reconfiguring tiles in crServerNewTiles:");
	crDebug("  New mural size: %d x %d", muralWidth, muralHeight);
	for (i = 0; i < numTiles; i++)
	{
		crDebug("  Tile %d: %d, %d  %d x %d", i,
						tileBounds[i*4], tileBounds[i*4+1],
						tileBounds[i*4+2], tileBounds[i*4+3]);
	}

	/*
	 * This section basically mimics what's done during crServerGetTileInfo()
	 */
	mural->width = muralWidth;
	mural->height = muralHeight;
	mural->numExtents = numTiles;
	for (i = 0; i < numTiles; i++)
	{
		const int x = tileBounds[i * 4 + 0];
		const int y = tileBounds[i * 4 + 1];
		const int w = tileBounds[i * 4 + 2];
		const int h = tileBounds[i * 4 + 3];
		mural->extents[i].imagewindow.x1 = x;
		mural->extents[i].imagewindow.y1 = y;
		mural->extents[i].imagewindow.x2 = x + w;
		mural->extents[i].imagewindow.y2 = y + h;
	}

	crServerInitializeTiling(mural);
}




static float
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
crServerGetTileInfoFromMothership( CRConnection *conn, CRMuralInfo *mural )
{
	char response[8096];

	/* default alignment is the identity */
	crMemset(cr_server.alignment_matrix, 0, 16*sizeof(GLfloat));
	cr_server.alignment_matrix[0]  = 1;
	cr_server.alignment_matrix[5]  = 1;
	cr_server.alignment_matrix[10] = 1;
	cr_server.alignment_matrix[15] = 1;

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
		tilelist = crStrSplit(tilechain[1], ",");

		/* save list of tiles/extents for this mural */
		mural->numExtents = crStrToInt(tilechain[0]);
		for (i = 0; i < mural->numExtents; i++)
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

			mural->extents[i].imagewindow.x1 = (int) x;
			mural->extents[i].imagewindow.y1 = (int) y;
			mural->extents[i].imagewindow.x2 = (int) x + (int) w;
			mural->extents[i].imagewindow.y2 = (int) y + (int) h;
			crDebug("Added tile: %d, %d .. %d, %d",
							mural->extents[i].imagewindow.x1, mural->extents[i].imagewindow.y1,
							mural->extents[i].imagewindow.x2, mural->extents[i].imagewindow.y2);
		}

		if ((cr_server.localTileSpec) && (num_displays))
		{
			int w, h, id, idx, our_idx = 0;
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
					our_idx = (idx / 8) - 1;
				}
			}

			/* now compute the bounding box of the display area */
			crHullInteriorBox(corners, idx / 2, bbox);

			cr_server.num_overlap_levels = idx / 8;
			crComputeOverlapGeom(corners, cr_server.num_overlap_levels, 
								&cr_server.overlap_geom);
			crComputeKnockoutGeom(corners, cr_server.num_overlap_levels, our_idx,
								&cr_server.overlap_knockout);

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

			Sx = Sy = 1;
			cent[0] = cent[1] = 0;

			cr_server.unnormalized_alignment_matrix[0] = tmp[0] * Sx;
			cr_server.unnormalized_alignment_matrix[1] = tmp[3] * Sx;
			cr_server.unnormalized_alignment_matrix[2] = 0;
			cr_server.unnormalized_alignment_matrix[3] = tmp[6] * Sx;
			cr_server.unnormalized_alignment_matrix[4] = tmp[1] * Sy;
			cr_server.unnormalized_alignment_matrix[5] = tmp[4] * Sy;
			cr_server.unnormalized_alignment_matrix[6] = 0;
			cr_server.unnormalized_alignment_matrix[7] = tmp[7] * Sy;
			cr_server.unnormalized_alignment_matrix[8] = 0;
			cr_server.unnormalized_alignment_matrix[9] = 0;
			cr_server.unnormalized_alignment_matrix[10] = tmp[6] * Sx * cent[0] + 
																			 tmp[7] * Sy * cent[1] + 
																			 tmp[8] -
																			 absf(tmp[6] * Sx) -
																			 absf(tmp[7] * Sy);
			cr_server.unnormalized_alignment_matrix[11] = 0;
			cr_server.unnormalized_alignment_matrix[12] =
				tmp[2] + tmp[0] * Sx * cent[0] + tmp[1] * Sy * cent[1];
			cr_server.unnormalized_alignment_matrix[13] =
				tmp[5] + tmp[3] * Sx * cent[0] + tmp[4] * Sy * cent[1];
			cr_server.unnormalized_alignment_matrix[14] = 0;
			cr_server.unnormalized_alignment_matrix[15] =
				tmp[8] + tmp[6] * Sx * cent[0] + tmp[7] * Sy * cent[1];

			crFree(corners);

			crFreeStrings(displaychain);
			crFreeStrings(displaylist);
		}

		crFreeStrings(tilechain);
		crFreeStrings(tilelist);
	}


	if (cr_server.clients[0].spu_id != -1) /* out-of-nowhere file client */
	{
		/* Sigh -- the servers need to know how big the whole mural is if we're 
		 * doing tiling, so they can compute their base projection.  For now, 
		 * just have them pretend to be one of their client SPU's, and redo 
		 * the configuration step of the tilesort SPU.  Basically this is a dirty 
		 * way to figure out who the other servers are.  It *might* matter 
		 * which SPU we pick for certain graph configurations, but we'll cross 
		 * that bridge later.
		 */
		int reqTileWidth = 0, reqTileHeight = 0;
		int num_servers, i;
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
					break;
			}
			else
			{
				if (!crMothershipGetTiles(conn, response, i))
					break;
			}

			tilechain = crStrSplitn(response, " ", 1);
			numExtents = crStrToInt(tilechain[0]);

			tilelist = crStrSplit(tilechain[1], ",");

			/* Examine tile extents to determine total mural size */
			for (tile = 0; tile < numExtents; tile++)
			{
				int w, h, id, x1, y1, x2, y2;
				if (cr_server.localTileSpec)
					sscanf(tilelist[tile], "%d %d %d %d %d", &id, &x1, &y1, &w, &h);
				else
					sscanf(tilelist[tile], "%d %d %d %d", &x1, &y1, &w, &h);

				/* update mural size */
				x2 = x1 + w;
				y2 = y1 + h;
				if (x2 > (int) mural->width)
					mural->width = x2;
				if (y2 > (int) mural->height)
					mural->height = y2;

				/* if optimize_bucket is set, make sure all tiles are same size */
				if (cr_server.optimizeBucket) {
					if (reqTileWidth == 0 && reqTileHeight == 0) {
						reqTileWidth = w;
						reqTileHeight = h;
					}
					else {
						if (w != reqTileWidth || h != reqTileHeight) {
							crWarning("Server: optimize_bucket is set, but tiles aren't "
												"all the same size.  Disabling optimize_bucket.");
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
							mural->width, mural->height);
	}
	else
	{
		crWarning
			("It looks like there are nothing but file clients.  That suits me just fine.");
	}

	/* Prepare data structures for tiling/bucketing */
	crServerInitializeTiling(mural);
}
