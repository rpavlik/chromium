/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "packspu.h"
#include "cr_string.h"
#include "cr_error.h"
#include "cr_mothership.h"
#include "cr_spu.h"

#include <stdio.h>

static void __setDefaults( void )
{
}

void packspuGatherConfiguration( const SPU *child_spu )
{
	CRConnection *conn;
	char response[8096];
	char servername[8096];
	int num_servers;

	conn = crMothershipConnect();
	if (!conn)
	{
		crError( "Couldn't connect to the mothership -- I have no idea what to do!" );
	}
	crMothershipIdentifySPU( conn, pack_spu.id );

	__setDefaults();

	crMothershipGetServers( conn, response );

	sscanf( response, "%d %s", &num_servers, servername );

	if (num_servers == 1)
	{
		pack_spu.server.name = crStrdup( servername );
	}
	else
	{
		crError( "Bad server specification for Pack SPU %d", pack_spu.id );
	}

	crMothershipGetMTU( conn, response );
	sscanf( response, "%d", &(pack_spu.server.buffer_size) );

	fprintf(stderr, "******** pack spu %d begin propogate\n", pack_spu.id);
	crSPUPropogateGLLimits( conn, pack_spu.id, child_spu, &pack_spu.limits );
	fprintf(stderr, "******** pack spu %d end propogate\n", pack_spu.id);

	crMothershipDisconnect( conn );
}



//
// XXX NEW BrianP
//
// A resuable SPU function to query OpenGL limits from the caller's servers
// (i.e. downstream SPUs).
//

extern void crSPUGetServerGLLimits( CRConnection *conn );

void crSPUGetServerGLLimits( CRConnection *conn )
{
	char response[8096];

	char **serverids;
	int num_servers;
	int i;

	// Connect to the mothership and identify ourselves.
	

	// The response to this tells us how many servers, what the
	// SPU id of their first SPU is, and where they are.
	//
	// For example:  2 1 tcpip://foo 2 tcpip://bar
	//
	// We need the SPU ID since the tiling information is actually
	// associated with the render SPU that must be at the head of the
	// server chain for tile/sorting to work properly.

#if 111
	if (crMothershipSendString( conn, response, "serverids" ))
		fprintf(stderr, ">>>> Get server ids = %s\n", response);
	else
		fprintf(stderr, ">>>> Get server ids failed!\n");
	serverids = crStrSplit( response, " " );
#endif

	num_servers = crStrToInt( serverids[0] );

	if (num_servers == 0)
	{
		crError( "No servers specified for pack SPU?!" );
		return;
	}

	for (i = 0 ; i < num_servers ; i++)
	{
		int server_id;
		CRLimitsState limits;

		server_id = crStrToInt(serverids[i+1]);

		crSPUQueryGLLimits( conn, server_id, &limits );
	}
}
