/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef COMM_SPU_H
#define COMM_SPU_H

#ifdef WINDOWS
#define COMMSPU_APIENTRY __stdcall
#else
#define COMMSPU_APIENTRY
#endif

#include "cr_spu.h"
#include "cr_net.h"

#define COMM_SPU_PORT 8192

void commspuGatherConfiguration( void );

typedef struct {
	CRMessageHeader header;
	int frame_counter;
} CommSPUPing;

typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super;
	char *peer_name;
	int i_am_the_server;
	int mtu;
	CRConnection *peer_send, *peer_recv;
	CommSPUPing *msg;
} CommSPU;

extern CommSPU comm_spu;

#endif /* COMM_SPU_H */
