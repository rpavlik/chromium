/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef INCLUDE_CR_SERVER_H
#define INCLUDE_CR_SERVER_H

#include "cr_spu.h"
#include "cr_net.h"
#include "cr_hash.h"
#include "cr_protocol.h"
#include "cr_glstate.h"
#include "spu_dispatch_table.h"

#include "state/cr_currentpointers.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int spu_id;
	int number;
	GLmatrix baseProjection;
	CRConnection *conn;

	CRContext *currentCtx;
	GLint currentWindow;
} CRClient;

typedef struct {
	unsigned short tcpip_port;

	unsigned int numClients;
	CRClient *clients;
	CRClient *curClient;
	CRCurrentStatePointers current;

	int optimizeBucket;
	int numExtents;  /* number of tiles */
	int curExtent;
	/* coordinates of each tile's rectangle in mural coord space */
	GLrecti extents[CR_MAX_EXTENTS];
	/* coords of the tile in the server's rendering window */
	GLrecti outputwindow[CR_MAX_EXTENTS];
	int maxTileHeight; /* the tallest tile's height */

	int useL2;
	int mtu;
	int buffer_size;
	char protocol[1024];

	unsigned int muralWidth, muralHeight;
	unsigned int underlyingDisplay[4]; /* needed for laying out the extents */

	SPU *head_spu;
	SPUDispatchTable dispatch;

	CRNetworkPointer return_ptr;
	CRNetworkPointer writeback_ptr;

	CRLimitsState limits; /* GL limits for any contexts we create */

	int SpuContext; /* Rendering context for the head SPU */

	CRContext *context[CR_MAX_CONTEXTS];

	int ignore_papi;

	unsigned int maxBarrierCount;
	unsigned int clearCount;
	int only_swap_once;
	int debug_barriers;
	int localTileSpec;
	GLfloat alignment_matrix[16];
} CRServer;


#ifdef __cplusplus
}
#endif

#endif

