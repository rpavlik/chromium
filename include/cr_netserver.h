/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_NETSERVER_H
#define CR_NETSERVER_H

#include "cr_net.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	char *name;
	int buffer_size;
	CRConnection *conn;
} CRNetServer;

void crNetServerConnect( CRNetServer *ns );

#ifdef __cplusplus
}
#endif

#endif /* CR_NETSERVER_H */
