/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_string.h"
#include "cr_error.h"
#include "cr_net.h"
#include "cr_mothership.h"
#include "cr_environment.h"
#include "cr_mem.h"

#include <stdio.h>
#include <stdarg.h>

#define MOTHERPORT 10000

CRConnection *crMothershipConnect( void )
{
	char *mother_server = NULL;

	crNetInit( NULL, NULL );

	mother_server = crGetenv( "CRMOTHERSHIP" );
	if (!mother_server)
	{
		crWarning( "Couldn't find the CRMOTHERSHIP environment variable, defaulting to localhost" );
		mother_server = "localhost";
	}

	return crNetConnectToServer( mother_server, MOTHERPORT, 8096, 0 );
}

void crMothershipDisconnect( CRConnection *conn )
{
	crMothershipSendString( conn, NULL, "disconnect" );
	crNetDisconnect( conn );
	crFree( conn->hostname );
	crFree( conn );
}

int crMothershipSendString( CRConnection *conn, char *response_buf, const char *str, ... )
{
	va_list args;
	static char txt[8092];

	va_start(args, str);
	vsprintf( txt, str, args );
	va_end(args);

	crStrcat( txt, "\n" );
	crNetSendExact( conn, txt, crStrlen(txt) );
	if (response_buf)
	{
		return crMothershipReadResponse( conn, response_buf );
	}
	else
	{
		char devnull[1024];
		return crMothershipReadResponse( conn, devnull );
	}
}

int crMothershipReadResponse( CRConnection *conn, void *buf )
{
	char codestr[4];
	int code;

	crNetSingleRecv( conn, codestr, 4 );
	crNetReadline( conn, buf );

	code = crStrToInt( codestr );
	return (code == 200);
}



/**********************************************************************
 * crMothershipIdentify*() functions.
 * Called by nodes and SPUs to identify themselves to the mothership.
 * This usually has to be done before any other mothership communications.
 */

/* Called by SPUs to identify themselves to the mothership */
void crMothershipIdentifySPU( CRConnection *conn, int spu )
{
	if (!crMothershipSendString( conn, NULL, "spu %d", spu ))
	{
		crError( "Server said it hadn't heard of SPU %d", spu );
	}
}

#define INSIST(x) if (!x) crError( "Bad Mothership response: %s", response )

/* Called by app faker nodes to identify themselves to the mothership */
void crMothershipIdentifyFaker( CRConnection *conn, char *response )
{
	char hostname[1024];
	if ( crGetHostname( hostname, sizeof(hostname) ) )
	{
		crError( "Couldn't get my own hostname?" );
	}
	INSIST( crMothershipSendString( conn, response, "faker %s", hostname ));
}

/* Called by OpenGL faker library to identify itself to the mothership */
void crMothershipIdentifyOpenGL( CRConnection *conn, char *response, char *app_id )
{
	char hostname[1024];
	if ( crGetHostname( hostname, sizeof(hostname) ) )
	{
		crError( "Couldn't get my own hostname?" );
	}
	/* This returns the client's SPU chain, if nothing goes wrong */
	INSIST( crMothershipSendString( conn, response, "opengldll %s %s", app_id, hostname ));
}

/* Called by crserver/network nodes to identify themselves to the mothership */
void crMothershipIdentifyServer( CRConnection *conn, char *response )
{
	char hostname[1024];
	if ( crGetHostname( hostname, sizeof(hostname) ) )
	{
		crError( "Couldn't get my own hostname?" );
	}
	INSIST( crMothershipSendString( conn, response, "server %s", hostname ));
}

/* Called by CRUT client nodes to identify themselves to the mothership */
void crMothershipIdentifyCRUTClient( CRConnection *conn, char *response )
{
	char hostname[1024];
	if ( crGetHostname( hostname, sizeof(hostname) ) )
	{
		crError( "Couldn't get my own hostname?" );
	}
	INSIST( crMothershipSendString( conn, response, "crutclient %s", hostname ));
}

/* Called by CRUT server nodes to identify themselves to the mothership */
void crMothershipIdentifyCRUTServer( CRConnection *conn, char *response )
{
	char hostname[1024];
	if ( crGetHostname( hostname, sizeof(hostname) ) )
	{
		crError( "Couldn't get my own hostname?" );
	}
	INSIST( crMothershipSendString( conn, response, "crutserver %s", hostname ));
}

/* Called by CRUT proxy nodes to identify themselves to the mothership */
void crMothershipIdentifyCRUTProxy( CRConnection *conn, char *response )
{
	char hostname[1024];
	if ( crGetHostname( hostname, sizeof(hostname) ) )
	{
		crError( "Couldn't get my own hostname?" );
	}
	INSIST( crMothershipSendString( conn, response, "crutproxy %s", hostname ));
}



/**********************************************************************
 * Mothership functions.
 */

/* Send exit message to the mothership */
void crMothershipExit( CRConnection *conn )
{
	(void)crMothershipSendString( conn, NULL, "exit" );
}

/* Send reset message to the mothership */
void crMothershipReset( CRConnection *conn )
{
	if (!crMothershipSendString( conn, NULL, "reset" ))
	{
		crError( "Couldn't reset the server!" );
	}
}

/* Set a mothership configuration parameter */
void crMothershipSetParam( CRConnection *conn, const char *param, const char *value )
{
	(void) crMothershipSendString( conn, NULL, "setparam %s %s", param, value );
}

/* Get a mothership configuration parameter */
int crMothershipGetParam( CRConnection *conn, const char *param, char *response )
{
	return crMothershipSendString( conn, response, "getparam %s", param );
}

/* Get the Chromium MTU.
 * This is just a convenience function.
 */
int crMothershipGetMTU( CRConnection *conn )
{
	char response[4096];
	int mtu;
	INSIST( crMothershipGetParam( conn, "MTU", response ) );
	sscanf( response, "%d", &mtu );
	return mtu;
}



/**********************************************************************
 * SPU functions  (i.e. called from SPUs)
 */

/* Called by (terminal) SPUs to get a list of their servers */
void crMothershipGetServers( CRConnection *conn, char *response )
{
	INSIST( crMothershipSendString( conn, response, "servers" ));
}

/* Called by SPUs to get their config parameters */
int crMothershipGetSPUParam( CRConnection *conn, char *response, const char *param )
{
	return crMothershipSendString( conn, response, "spuparam %s", param );
}

/* Called by SPUs to get server config parameters */
int crMothershipGetServerParamFromSPU( CRConnection *conn, int server_num,
																			 const char *param, char *response )
{
	return crMothershipSendString( conn, response, "server_param %d %s", server_num, param );
}


/* Called by SPUs to get the list of tiles from a specific server */
int crMothershipGetTiles( CRConnection *conn, char *response, int server_num )
{
	return crMothershipSendString( conn, response, "tiles %d", server_num );
}

/* Called by SPUs to get list of displays (for warped tiles) */
int crMothershipGetDisplays( CRConnection *conn, char *response )
{
	return crMothershipSendString( conn, response, "displays");
}

/* Called by SPUs to get list of tiles for the given server.
 * Each set of tile parameters is preceeded by a display number.
 */
int crMothershipGetDisplayTiles( CRConnection *conn, char *response, int server_num )
{
	return crMothershipSendString( conn, response, "display_tiles %d", server_num );
}


/**********************************************************************
 * CRServer/Network node functions  (i.e. called from network nodes)
 */

/* Called by crserver nodes to get list of its clients */
void crMothershipGetClients( CRConnection *conn, char *response )
{
	INSIST( crMothershipSendString( conn, response, "clients" ));
}

/* Called by crserver nodes to get their config parameters */
int crMothershipGetServerParam( CRConnection *conn, char *response, const char *param)
{
	return crMothershipSendString( conn, response, "serverparam %s", param );
}

int crMothershipGetServerTiles( CRConnection *conn, char *response )
{
	return crMothershipSendString( conn, response, "servertiles" );
}

int crMothershipGetServerDisplayTiles( CRConnection *conn, char *response )
{
	return crMothershipSendString( conn, response, "serverdisplaytiles" );
}

int crMothershipRequestTileLayout( CRConnection *conn, char *response,
                             int muralWidth, int muralHeight )
{
	return crMothershipSendString( conn, response, "gettilelayout %d %d",
																 muralWidth, muralHeight );
}



/**********************************************************************
 * Faker/client node functions.
 */

/* Called by app nodes to get their config parameters */
int crMothershipGetFakerParam( CRConnection *conn, char *response, const char *param )
{
	return crMothershipSendString( conn, response, "fakerparam %s", param );
}


/**********************************************************************
 * Server or app node functions.
 */

/* Called by nodes to get their rank (for Quadrics networking) */
int crMothershipGetRank( CRConnection *conn, char *response )
{
	return crMothershipSendString( conn, response, "rank" );
}



/**********************************************************************
 * CRUT functions
 */

int crMothershipGetCRUTServerParam( CRConnection *conn, char *response, const char *param )
{
	return crMothershipSendString( conn, response, "crutserverparam %s", param );
}

void crMothershipGetCRUTServer( CRConnection *conn, char *response )
{
	INSIST( crMothershipSendString( conn, response, "crutservers" ));
}

void crMothershipGetCRUTClients( CRConnection *conn, char *response )
{
	INSIST( crMothershipSendString( conn, response, "crutclients" ));
}
