#ifndef CR_NETSERVER_H
#define CR_NETSERVER_H

#include "cr_net.h"

typedef struct {
	char *name;
	int buffer_size;
	CRConnection *conn;
} CRNetServer;

void crNetServerConnect( CRNetServer *ns );

#endif /* CR_NETSERVER_H */
