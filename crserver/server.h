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

	int num_clients;
	int cur_client;
	CRClient *clients;
	CRClient *current_client;
	CRCurrentStatePointers current;

	int num_extents, current_extent;
	int x1[CR_MAX_EXTENTS], y1[CR_MAX_EXTENTS];
	int x2[CR_MAX_EXTENTS], y2[CR_MAX_EXTENTS];

	unsigned int muralWidth, muralHeight;

	SPU *head_spu;
	SPUDispatchTable dispatch;

	CRNetworkPointer return_ptr;
	CRNetworkPointer writeback_ptr;
} CRServer;

extern CRServer cr_server;

typedef struct __runqueue {
	CRClient *client;
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

#endif /* CR_SERVER_H */
