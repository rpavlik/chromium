/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_SERVER_H
#define CR_SERVER_H

#include "cr_spu.h"
#include "cr_net.h"
#include "cr_hash.h"
#include "cr_protocol.h"
#include "cr_glstate.h"
#include "spu_dispatch_table.h"

#include "state/cr_currentpointers.h"

typedef struct {
	int spu_id;
	GLmatrix baseProjection;
	CRConnection *conn;
	CRContext *ctx;
} CRClient;

typedef struct {
	unsigned short tcpip_port;

	int numClients;
	CRClient *clients;
	CRClient *curClient;
	CRCurrentStatePointers current;

	int optimizeBucket;
	int numExtents, curExtent;
	int x1[CR_MAX_EXTENTS], y1[CR_MAX_EXTENTS];
	int x2[CR_MAX_EXTENTS], y2[CR_MAX_EXTENTS];
	int maxTileHeight;

	int useL2;

	unsigned int muralWidth, muralHeight;
	unsigned int underlyingDisplay[4]; // needed for laying out the extents

	SPU *head_spu;
	SPUDispatchTable dispatch;

	CRNetworkPointer return_ptr;
	CRNetworkPointer writeback_ptr;
} CRServer;

extern CRServer cr_server;

typedef struct {
	GLrecti outputwindow;
	GLrecti imagewindow;
	GLrectf bounds;
	int     display;
} CRRunQueueExtent;

typedef struct __runqueue {
	CRClient *client;
	GLrecti imagespace;
	int numExtents;
	CRRunQueueExtent extent[CR_MAX_EXTENTS];
	int blocked;
	struct __runqueue *next;
	struct __runqueue *prev;
} RunQueue;

extern RunQueue *run_queue;

extern CRHashTable *cr_barriers, *cr_semaphores;

void crServerGatherConfiguration(char *mothership);
void crServerInitDispatch(void);
void crServerReturnValue( const void *payload, unsigned int payload_len );
void crServerWriteback(void);
int crServerRecv( CRConnection *conn, void *buf, unsigned int len );
void crServerSerializeRemoteStreams(void);
void crServerAddToRunQueue( int index );

void crServerRecomputeBaseProjection(GLmatrix *base);
void crServerApplyBaseProjection(void);

void crServerSetOutputBounds( CRContext *ctx, const GLrecti *outputwindow, const GLrecti *imagespace, const GLrecti *imagewindow );
void crServerSetViewportBounds( CRViewportState *v, const GLrecti *outputwindow, const GLrecti *imagespace, const GLrecti *imagewindow, GLrecti *p, GLrecti *q );
void crServerSetTransformBounds( CRTransformState *t, const GLrecti *outputwindow, GLrecti *p, GLrecti *q );
void crServerFillBucketingHash(void);

#endif /* CR_SERVER_H */
