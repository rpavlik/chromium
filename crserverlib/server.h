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

/* Semaphore wait queue node */
typedef struct _wqnode {
	RunQueue *q;
	struct _wqnode *next;
} wqnode;

typedef struct {
	GLuint count;
	GLuint num_waiting;
	RunQueue **waiting;
} CRBarrier;

typedef struct {
	GLuint count;
	wqnode *waiting, *tail;
} CRSemaphore;

void crServerGatherConfiguration(char *mothership);
void crServerGetTileInfo( CRConnection *conn );
void crServerInitializeTiling(void);
void crServerInitDispatch(void);
void crServerReturnValue( const void *payload, unsigned int payload_len );
void crServerWriteback(void);
int crServerRecv( CRConnection *conn, void *buf, unsigned int len );
void crServerSerializeRemoteStreams(void);
void crServerAddToRunQueue( CRClient *client );
void crServerInitializeQueueExtents(RunQueue *q);

void crServerRecomputeBaseProjection(CRmatrix *base, GLint x, GLint y, GLint w, GLint h);
void crServerApplyBaseProjection(void);

void crServerSetOutputBounds( CRContext *ctx, const CRrecti *outputwindow, const CRrecti *imagespace, const CRrecti *imagewindow );
void crServerSetViewportBounds( CRViewportState *v, const CRrecti *outputwindow, const CRrecti *imagespace, const CRrecti *imagewindow, CRrecti *p, CRrecti *q );
void crServerSetTransformBounds( CRTransformState *t, const CRrecti *outputwindow, CRrecti *p, CRrecti *q );
void crServerFillBucketingHash(void);

void crServerNewTiles(int muralWidth, int muralHeight, int numTiles, const int *tileBounds);
GLboolean crServerCheckTileLayout(void);

void crComputeOverlapGeom(double *quads, int nquad, CRPoly ***res);
void crComputeKnockoutGeom(double *quads, int nquad, int my_quad_idx, CRPoly **res);

#endif /* CR_SERVER_H */
