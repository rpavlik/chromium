#ifndef CR_SERVER_H
#define CR_SERVER_H

#include "cr_spu.h"
#include "cr_net.h"

typedef struct {
	int spu_id;
	CRConnection *conn;
} CRClient;

typedef struct {
	unsigned short tcpip_port;

	int num_clients;
	CRClient *clients;

	SPU *head_spu;
} CRServer;

extern CRServer cr_server;

void crServerGatherConfiguration( void );

#endif /* CR_SERVER_H */
