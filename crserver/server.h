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

#include "cr_server.h"

extern CRServer cr_server;

typedef struct {
	GLrecti outputwindow;   /* coordinates in mural space */
	GLrecti imagewindow;    /* coordinates in render window */
	GLrectf bounds;         /* coordinates in [-1,11] x [1,1], I think */
	int     display;        /* not used (historical?) */
} CRRunQueueExtent;

typedef struct __runqueue {
	CRClient *client;
	GLrecti imagespace;
	int numExtents;
	CRRunQueueExtent extent[CR_MAX_EXTENTS];
	int blocked;
	int number;
	struct __runqueue *next;
	struct __runqueue *prev;
} RunQueue;

extern RunQueue *run_queue;

extern CRHashTable *cr_barriers, *cr_semaphores;

void crServerGatherConfiguration(char *mothership);
void crServerGetTileInfo( CRConnection *conn, int nonFileClient );
void crServerInitDispatch(void);
void crServerReturnValue( const void *payload, unsigned int payload_len );
void crServerWriteback(void);
int crServerRecv( CRConnection *conn, void *buf, unsigned int len );
void crServerSerializeRemoteStreams(void);
void crServerAddToRunQueue( CRClient *client );

void crServerRecomputeBaseProjection(GLmatrix *base, GLint x, GLint y, GLint w, GLint h);
void crServerApplyBaseProjection(void);

void crServerSetOutputBounds( CRContext *ctx, const GLrecti *outputwindow, const GLrecti *imagespace, const GLrecti *imagewindow );
void crServerSetViewportBounds( CRViewportState *v, const GLrecti *outputwindow, const GLrecti *imagespace, const GLrecti *imagewindow, GLrecti *p, GLrecti *q );
void crServerSetTransformBounds( CRTransformState *t, const GLrecti *outputwindow, GLrecti *p, GLrecti *q );
void crServerFillBucketingHash(void);

#endif /* CR_SERVER_H */
