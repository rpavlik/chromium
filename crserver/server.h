#ifndef CR_SERVER_H
#define CR_SERVER_H

#include "cr_spu.h"
#include "cr_net.h"
#include "cr_protocol.h"
#include "spu_dispatch_table.h"

typedef struct {
	int spu_id;
	CRConnection *conn;
} CRClient;

typedef struct {
	unsigned short tcpip_port;

	int num_clients;
	int cur_client;
	CRClient *clients;

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

#endif /* CR_SERVER_H */
