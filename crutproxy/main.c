/* Written by Dale Beermann (beermann@cs.virginia.edu) */

#include "crut_api.h"
#include "cr_error.h"
#include "cr_mothership.h"
#include "cr_string.h"
/**
 * \mainpage CrutProxy 
 *
 * \section CrutProxyIntroduction Introduction
 *
 */

typedef struct 
{
    int num_bytes;
    CRUTMessage *msg;
    CRConnection *recv_conn;
} CRUTProxy;

CRUTProxy crut_proxy;
CRUTAPI crut_api;

static int 
crutProxyRecv( CRConnection *conn, CRMessage *msg, unsigned int len )
{
    switch( msg->header.type )
    {
        case CR_MESSAGE_CRUT:

	    crut_proxy.msg = (CRUTMessage*) msg;

	    /* forward events */
	    if ( crut_proxy.msg->msg_type == CRUT_MOUSE_EVENT ) 
	    {
	        CRUTMouseMsg *msg1 = (CRUTMouseMsg*) msg;
		crutSendMouseEvent( &crut_api, msg1->button, msg1->state, msg1->x, msg1->y);
	    } 

	    else if ( crut_proxy.msg->msg_type == CRUT_RESHAPE_EVENT ) 
	    {
	        CRUTReshapeMsg *msg1 = (CRUTReshapeMsg*) msg;
		crutSendReshapeEvent( &crut_api, msg1->width, msg1->height);
	    } 

	    else if ( crut_proxy.msg->msg_type == CRUT_VISIBILITY_EVENT ) 
	    {
		 CRUTVisibilityMsg *msg1 = (CRUTVisibilityMsg*) msg;
		 crutSendVisibilityEvent( &crut_api, msg1->state);
	    } 

	    else if ( crut_proxy.msg->msg_type == CRUT_KEYBOARD_EVENT ) 
	    {
	        CRUTKeyboardMsg *msg1 = (CRUTKeyboardMsg*) msg;
		crutSendKeyboardEvent( &crut_api, msg1->key, msg1->x, msg1->y);
	    } 

	    else if ( crut_proxy.msg->msg_type == CRUT_MOTION_EVENT ) 
	    {
	        CRUTMotionMsg *msg1 = (CRUTMotionMsg*) msg;
		crutSendMotionEvent( &crut_api, msg1->x, msg1->y);
	    } 

	    else if ( crut_proxy.msg->msg_type == CRUT_PASSIVE_MOTION_EVENT ) 
	    {
	        CRUTPassiveMotionMsg *msg1 = (CRUTPassiveMotionMsg*) msg;
		crutSendPassiveMotionEvent( &crut_api, msg1->x, msg1->y);
	    } 

	    else if ( crut_proxy.msg->msg_type == CRUT_MENU_EVENT ) 
	    {
	        CRUTMenuMsg *msg1 = (CRUTMenuMsg*) msg;
		crutSendMenuEvent( &crut_api, msg1->menuID, msg1->value );
	    } 

	    return 1; /* HANDLED */
	    break;

        default:
	    crDebug("got a message NOT of type CRUT");
	    break;
    }
    (void) len;	
    return 0; /* HANDLED -- why weren't we doing this on Windows? Have to return something. */
}

static void 
crutProxyClose( CRConnection *conn )
{
    crError( "Client disconnected!" );
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
    crut_proxy.recv_conn = crNetConnectToServer( server, DEFAULT_CRUT_PORT, mtu, 1 );
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


