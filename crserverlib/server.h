/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_SERVER_H
#define CR_SERVER_H

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
} CRServerBarrier;

typedef struct {
	GLuint count;
	wqnode *waiting, *tail;
} CRServerSemaphore;

void crServerGatherConfiguration(char *mothership);
void crServerGetTileInfoFromMothership( CRConnection *conn, CRMuralInfo *mural );
void crServerInitializeTiling(CRMuralInfo *mural);
void crServerBeginTiling(CRMuralInfo *mural);
void crServerInitDispatch(void);
void crServerReturnValue( const void *payload, unsigned int payload_len );
void crServerWriteback(void);
int crServerRecv( CRConnection *conn, void *buf, unsigned int len );
void crServerSerializeRemoteStreams(void);
void crServerAddToRunQueue( CRClient *client );

void crServerRecomputeBaseProjection(CRmatrix *base, GLint x, GLint y, GLint w, GLint h);
void crServerApplyBaseProjection(void);

void crServerComputeOutputBounds( CRMuralInfo *mural, CRContext *ctx );

void crServerSetOutputBounds( CRContext *ctx, const CRrecti *outputwindow, const CRrecti *imagespace, const CRrecti *imagewindow, CRrecti *clippedImagewindow );
void crServerSetViewportBounds( CRViewportState *v, const CRrecti *outputwindow, const CRrecti *imagespace, const CRrecti *imagewindow, CRrecti *p, CRrecti *q );
void crServerSetTransformBounds( CRTransformState *t, const CRrecti *outputwindow, CRrecti *p, CRrecti *q );
GLboolean crServerInitializeBucketing(CRMuralInfo *mural);

void crServerNewMuralTiling(CRMuralInfo *mural, int muralWidth, int muralHeight, int numTiles, const int *tileBounds);

void crComputeOverlapGeom(double *quads, int nquad, CRPoly ***res);
void crComputeKnockoutGeom(double *quads, int nquad, int my_quad_idx, CRPoly **res);

#endif /* CR_SERVER_H */
