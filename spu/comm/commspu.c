/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_spu.h"
#include "cr_error.h"
#include "cr_net.h"
#include "commspu.h"

CommSPU comm_spu;

void COMMSPU_APIENTRY commSwapBuffers( GLint window, GLint flags )
{
	static unsigned int frame_counter = 0;
	CRMessage *incoming_msg;
	CommSPUPing *ping;

	frame_counter++;

	comm_spu.msg->header.type = CR_MESSAGE_OOB;
	comm_spu.msg->frame_counter = frame_counter;
	crNetSend( comm_spu.peer_send, NULL, comm_spu.msg, comm_spu.num_bytes );
	if (comm_spu.mtu > comm_spu.peer_send->mtu)
		comm_spu.mtu = comm_spu.peer_send->mtu;

	crNetGetMessage( comm_spu.peer_recv, &incoming_msg );
	ping = (CommSPUPing *) incoming_msg;

	if (ping->header.type != CR_MESSAGE_OOB)
	{
		crError( "Comm SPU Got a message of type 0x%x!", ping->header.type );
	}
	crDebug( "COMM SPU Got pinged with frame counter %d!", ping->frame_counter );
	crNetFree( comm_spu.peer_recv, incoming_msg );

	comm_spu.super.SwapBuffers( window, flags );
}

SPUNamedFunctionTable comm_table[] = {
	{ "SwapBuffers", (SPUGenericFunction) commSwapBuffers },
	{ NULL, NULL }
};
