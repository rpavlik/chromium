#ifndef CR_SERVER_H
#define CR_SERVER_H

#include "cr_spu.h"
#include "cr_net.h"
#include "cr_protocol.h"
#include "cr_glstate.h"
#include "spu_dispatch_table.h"

typedef struct {
	int spu_id;
	CRConnection *conn;
	CRContext *ctx;
} CRClient;

typedef struct {
	unsigned short tcpip_port;

	int num_clients;
	int cur_client;
	CRClient *clients;
	CRConnection *current_connection;

	int num_extents;
	int x1[CR_MAX_EXTENTS], y1[CR_MAX_EXTENTS];
	int x2[CR_MAX_EXTENTS], y2[CR_MAX_EXTENTS];

	SPU *head_spu;
	SPUDispatchTable dispatch;

	CRNetworkPointer return_ptr;
	CRNetworkPointer writeback_ptr;
} CRServer;

extern CRServer cr_server;

void crServerGatherConfiguration( void );
void crServerInitDispatch(void);
void crServerReturnValue( const void *payload, unsigned int payload_len );
void crServerWriteback(void);
int crServerRecv( CRConnection *conn, void *buf, unsigned int len );
void crServerSerializeRemoteStreams(void);
void crServerAddToRunQueue( int index );

#endif /* CR_SERVER_H */
