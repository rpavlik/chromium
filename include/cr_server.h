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
	CRmatrix baseProjection;
	CRConnection *conn;

	CRContext *currentCtx;
	GLint currentWindow;
} CRClient;

typedef struct st_CRPoly
{
	int npoints;
	double *points;
	struct st_CRPoly *next;
} CRPoly;

typedef struct {
	CRrecti outputwindow;   /* coordinates in server's rendering window */
	CRrecti imagewindow;    /* coordinates in mural space */
	CRrectf bounds;         /* normalized coordinates in [-1,-1] x [1,1] */
	int     display;        /* not used (historical?) */
} CRRunQueueExtent;

typedef struct __runqueue {
	CRClient *client;
	CRrecti imagespace;     /* the whole mural rectangle */
	int numExtents;
	CRRunQueueExtent extent[CR_MAX_EXTENTS];
	int blocked;
	int number;
	struct __runqueue *next;
	struct __runqueue *prev;
} RunQueue;

typedef struct {
	unsigned short tcpip_port;

	unsigned int numClients;
	CRClient *clients;
	CRClient *curClient;
	CRCurrentStatePointers current;

	GLboolean firstCallCreateContext;
	GLboolean firstCallMakeCurrent;

	int optimizeBucket;
	int numExtents;  /* number of tiles */
	int curExtent;
	/* coordinates of each tile's rectangle in mural coord space */
	CRrecti extents[CR_MAX_EXTENTS];
	/* coords of the tile in the server's rendering window */
	CRrecti outputwindow[CR_MAX_EXTENTS];
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
	int sharedDisplayLists;
	int sharedTextureObjects;
	int sharedPrograms;
	int localTileSpec;
	int overlapBlending;
	GLfloat alignment_matrix[16], unnormalized_alignment_matrix[16];
	
	CRPoly **overlap_geom;
	CRPoly *overlap_knockout;
	float *overlap_intens;
	int num_overlap_intens;
	int num_overlap_levels;

	CRHashTable *barriers, *semaphores;

	RunQueue *run_queue;
} CRServer;


#ifdef __cplusplus
}
#endif

#endif

