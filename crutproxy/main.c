/* Written by Dale Beermann (beermann@cs.virginia.edu) */

#include "crut_api.h"
#include "cr_error.h"
#include "cr_mothership.h"
#include "cr_string.h"

#define DEFAULT_PORT 9000

typedef struct 
{
    int num_bytes;
    CRUTMessage *msg;
    CRConnection *recv_conn;
} CRUTProxy;

CRUTProxy crut_proxy;
CRUTAPI crut_api;

static int 
crutProxyRecv( CRConnection *conn, void *buf, unsigned int len )
{
    CRMessage *msg = (CRMessage *) buf;
    switch( msg->header.type )
    {
        case CR_MESSAGE_CRUT:

	    crut_proxy.msg = (CRUTMessage*) buf;

	    /* forward events */
	    if ( crut_proxy.msg->msg_type == CRUT_MOUSE_EVENT ) 
	    {
	        CRUTMouseMsg *msg = (CRUTMouseMsg*) buf;
		crutSendMouseEvent( &crut_api, msg->button, msg->state, msg->x, msg->y);
	    } 

	    else if ( crut_proxy.msg->msg_type == CRUT_RESHAPE_EVENT ) 
	    {
	        CRUTReshapeMsg *msg = (CRUTReshapeMsg*) buf;
		crutSendReshapeEvent( &crut_api, msg->width, msg->height);
	    } 

	    else if ( crut_proxy.msg->msg_type == CRUT_KEYBOARD_EVENT ) 
	    {
	        CRUTKeyboardMsg *msg = (CRUTKeyboardMsg*) buf;
		crutSendKeyboardEvent( &crut_api, msg->key, msg->x, msg->y);
	    } 

	    else if ( crut_proxy.msg->msg_type == CRUT_MOTION_EVENT ) 
	    {
	        CRUTMotionMsg *msg = (CRUTMotionMsg*) buf;
		crutSendMotionEvent( &crut_api, msg->x, msg->y);
	    } 

	    else if ( crut_proxy.msg->msg_type == CRUT_PASSIVE_MOTION_EVENT ) 
	    {
	        CRUTPassiveMotionMsg *msg = (CRUTPassiveMotionMsg*) buf;
		crutSendPassiveMotionEvent( &crut_api, msg->x, msg->y);
	    } 

	    else if ( crut_proxy.msg->msg_type == CRUT_MENU_EVENT ) 
	    {
	        CRUTMenuMsg *msg = (CRUTMenuMsg*) buf;
		crutSendMenuEvent( &crut_api, msg->menuID, msg->value );
	    } 

	    return 1; /* HANDLED */
	    break;

        default:
	    crDebug("got a message NOT of type CRUT");
	    return 0; /* NOT HANDLED */
    }
    (void) len;	
#ifndef WINDOWS
    return 0; /* HANDLED */
#endif
}

static void 
crutProxyClose( unsigned int id )
{
    crError( "Client disconnected!" );
    (void) id;
}

static void 
crutInitProxy(char *mothership)
{
    char response[8096];
    char **newserver;
    char* server;
    int mtu;
    
    crutInitAPI(&crut_api, mothership);

    crMothershipIdentifyCRUTProxy( crut_api.mothershipConn, response );

    crMothershipGetCRUTServer( crut_api.mothershipConn, response );
    newserver = crStrSplit(response, " ");
    server = newserver[1];

    mtu = crMothershipGetMTU(crut_api.mothershipConn);
 
    /* set up the connection to recv on */
    crut_proxy.recv_conn = crNetConnectToServer( server, DEFAULT_PORT, mtu, 1 );
    crutConnectToClients( &crut_api );

}

int 
main( int argc, char *argv[] )
{
    char *mothership = NULL;
        
    crNetInit(crutProxyRecv, crutProxyClose);
    
    crutInitProxy(mothership);
  
    for (;;) 
	crNetRecv();	

#ifndef WINDOWS
    return 0;
#endif
}


