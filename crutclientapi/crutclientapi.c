/* Written by Dale Beermann (beermann@cs.virginia.edu) */

#include "crut_clientapi.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_mothership.h"
#include "cr_string.h"

/**
 * \mainpage CrutClientApi 
 *
 * \section CrutClientApiIntroduction Introduction
 *
 */
CRUTClient crut_client;
CRUTAPI crut_api;

#define LOAD( x ) x##_ptr = (x##Proc) crGetProcAddress( #x )


static int
__getCRUTMessageSize(int type)
{
    int msg_size = 0;

    switch( type )
    {
    case CRUT_MOUSE_EVENT:
	msg_size = sizeof( CRUTMouseMsg );
	break;
    case CRUT_RESHAPE_EVENT:
	msg_size = sizeof( CRUTReshapeMsg );
	break;
    case CRUT_KEYBOARD_EVENT:
	msg_size = sizeof( CRUTKeyboardMsg );
	break;
    case CRUT_MOTION_EVENT:
	msg_size = sizeof( CRUTMotionMsg );
	break;
    case CRUT_PASSIVE_MOTION_EVENT:
	msg_size = sizeof( CRUTPassiveMotionMsg );
	break;
    case CRUT_MENU_EVENT:
	msg_size = sizeof( CRUTMenuMsg );
	break;
    case CRUT_VISIBILITY_EVENT:
	msg_size = sizeof( CRUTVisibilityMsg );
	break;
    default:
	crError("Getting message size for unknown type!");
	break;
    }

    return msg_size;
}
 
/* Add a new buffer to the end of the list */
static void
__newEventBuffer(void) 
{
    EventBuffer* tmp;
    if (!crut_client.last_buffer)
	tmp = crut_client.last_buffer = crCalloc(sizeof(EventBuffer));
    else
    {
	tmp = crut_client.last_buffer->next = crCalloc(sizeof(EventBuffer));
	crut_client.last_buffer = tmp;
    }
 
    tmp->buffer = tmp->empty = crCalloc(EVENT_BUFFER_SIZE);
    tmp->buffer_end = (char*)(tmp->buffer+EVENT_BUFFER_SIZE);
}

/* Add a new event to the last buffer */
static void
__addToEventBuffer(CRMessage* msg)
{
    CRUTMessage *crut_msg = (CRUTMessage *) msg;

    int msg_size = __getCRUTMessageSize(crut_msg->msg_type);

    unsigned long int dist = crut_client.last_buffer->buffer_end - crut_client.last_buffer->empty;

    if (0 <= dist && dist < MAX_MSG_SIZE)
	__newEventBuffer();

    crMemcpy( crut_client.last_buffer->empty, msg, msg_size );

    crut_client.last_buffer->empty += msg_size;
}

static void 
__crutSetMouseParam(void)
{
    crut_client.callbacks.mouse_param = 1;
}

static void 
__crutSetKeyboardParam(void)
{
    crut_client.callbacks.keyboard_param = 1;
}

static void 
__crutSetReshapeParam(void)
{
    crut_client.callbacks.reshape_param = 1;
}

static void 
__crutSetMotionParam(void)
{
    crut_client.callbacks.motion_param = 1;
}

static void 
__crutSetPassiveMotionParam(void)
{
    crut_client.callbacks.passivemotion_param = 1;
}


static void 
__crutSetVisibilityParam(void)
{
    crut_client.callbacks.visibility_param = 1;
}

/* Tells the library what events it wants */
void 
CRUT_CLIENT_APIENTRY 
crutReceiveEventType(int type) 
{
    if (type == CRUT_MOUSE_EVENT)
	__crutSetMouseParam();
    else if (type == CRUT_KEYBOARD_EVENT)
	__crutSetKeyboardParam();
    else if (type == CRUT_RESHAPE_EVENT)
	__crutSetReshapeParam();
    else if (type == CRUT_MOTION_EVENT)
	__crutSetMotionParam();
    else if (type == CRUT_PASSIVE_MOTION_EVENT)
	__crutSetPassiveMotionParam();
    else if (type == CRUT_VISIBILITY_EVENT)
	__crutSetVisibilityParam();
    /* do we need to add menu events here? */
    else
	crError("Telling mothership we want to receive an unknown event type!");
}

void 
CRUT_CLIENT_APIENTRY 
crutMouseFunc( void (*func)(int button, int state, int x, int y) )
{
    crut_client.callbacks.mouse = func;
    __crutSetMouseParam();
}

void 
CRUT_CLIENT_APIENTRY 
crutKeyboardFunc( void (*func) (unsigned char key, int x, int y) )
{
    crut_client.callbacks.keyboard = func;
    __crutSetKeyboardParam();
}

void 
CRUT_CLIENT_APIENTRY 
crutReshapeFunc( void (*func) (int width, int height) )
{
    crut_client.callbacks.reshape = func;
    __crutSetReshapeParam();
}

void 
CRUT_CLIENT_APIENTRY 
crutMotionFunc( void (*func) ( int x, int y) )
{
    crut_client.callbacks.motion = func;
    __crutSetMotionParam();
}

void 
CRUT_CLIENT_APIENTRY 
crutPassiveMotionFunc( void (*func) (int x, int y) )
{
    crut_client.callbacks.passivemotion = func;
    __crutSetPassiveMotionParam();
}

void 
CRUT_CLIENT_APIENTRY 
crutVisibilityFunc( void (*func) (int state) )
{
    crut_client.callbacks.visibility = func;
    __crutSetVisibilityParam();
}

void 
CRUT_CLIENT_APIENTRY 
crutIdleFunc( void (*func)(void))
{
    crut_client.callbacks.idle = func;
}

void 
CRUT_CLIENT_APIENTRY 
crutDisplayFunc( void (*func)(void) )
{
    crut_client.callbacks.display = func;
}

void
CRUT_CLIENT_APIENTRY
crutPostRedisplay( void )
{
    crut_client.redisplay = 1;
}

int 
CRUT_CLIENT_APIENTRY 
crutCreateContext(unsigned int visual) 
{
    crCreateContextProc crCreateContext_ptr;
    crMakeCurrentProc   crMakeCurrent_ptr;

    int ctx;
    const char *dpy = NULL;

    LOAD( crCreateContext );
    LOAD( crMakeCurrent );

    ctx = crCreateContext_ptr(dpy, visual);
    if (ctx < 0) {
	crError("crCreateContext_ptr() call failed!\n");
    }
    
    return ctx;
}

int 
CRUT_CLIENT_APIENTRY 
crutCreateWindow(unsigned int visual) 
{
    crWindowCreateProc crWindowCreate_ptr;
    int win;
    const char *dpy = NULL;

    LOAD( crWindowCreate );

    win = crWindowCreate_ptr(dpy, visual);
    if (win < 0) {
	crError("crWindowCreate_ptr() call failed!\n");
    }
    
    return win;
}

static int
__addMenuToList( void (*func) (int val) )
{
    static int clientMenuID = 1;
    int temp;

    if (!crut_client.lastMenu)
    {
	crut_client.lastMenu = crut_client.beginMenuList = 
	    crCalloc(sizeof(clientMenuList));
    }
    else
    {
	crut_client.lastMenu->next = crCalloc(sizeof(clientMenuList));
	crut_client.lastMenu = crut_client.lastMenu->next;
    }
    
    crut_client.lastMenu->menu = func;
    crut_client.lastMenu->clientMenuID = clientMenuID;

    temp = clientMenuID;
    clientMenuID++;
    return temp;
}

int 
CRUT_CLIENT_APIENTRY
crutCreateMenu( void (*func) (int val) )
{
    return __addMenuToList( func );
}

static void
__addItemToList( char* name, int value , int menu_type)
{

    if (!crut_client.lastMenu->lastItem)
    {
	crut_client.lastMenu->lastItem = crut_client.lastMenu->beginItems = 
	    crCalloc(sizeof(clientMenuItem));
    }
    else
    {
	crut_client.lastMenu->lastItem->next = crCalloc(sizeof(clientMenuItem));
	crut_client.lastMenu->lastItem = crut_client.lastMenu->lastItem->next;
    }
    
    crut_client.lastMenu->lastItem->name = crCalloc( crStrlen(name) + 1);
    crMemcpy(crut_client.lastMenu->lastItem->name, name, crStrlen(name));
    crut_client.lastMenu->lastItem->name[ crStrlen( name ) ] = '\0';
    
    crut_client.lastMenu->lastItem->value = value;
    crut_client.lastMenu->lastItem->type = menu_type;
}

void 
CRUT_CLIENT_APIENTRY
crutAddMenuEntry( char* name, int value )
{

    __addItemToList( name, value , MENU_ITEM_REGULAR );

}

void
CRUT_CLIENT_APIENTRY
crutAddSubMenu( char* name, int clientMenuID )
{
    __addItemToList( name, clientMenuID, MENU_ITEM_SUBMENU );

}

void
CRUT_CLIENT_APIENTRY
crutAttachMenu( int button )
{
    crut_client.beginMenuList->att_button = button;
}

static char*
__createSubMenuXML( int clientMenuID , char* name)
{
    clientMenuList *tmpMenu;
    clientMenuItem *tmpItem;
    char *retBuf = crAlloc(1024); /* so we can return a buffer */
    char tmpBuf[1024];

    retBuf[0] = '\0';

    tmpMenu = crut_client.beginMenuList;
    while( tmpMenu->clientMenuID != clientMenuID && tmpMenu != NULL )
	tmpMenu = tmpMenu->next;

    if ( tmpMenu == NULL )
	crError( "Could not find clientMenuID passed to  __createMenuXML" );

    if ( name == NULL )
    {
	sprintf( tmpBuf, "<menu id=\"%i\">", clientMenuID ); 
	crStrcat( retBuf, tmpBuf );
    }
    else
    {
	sprintf( tmpBuf, "<menu name=\"%s\" id=\"%i\">", name, clientMenuID ); 
	crStrcat( retBuf, tmpBuf );
    }

    for( tmpItem = tmpMenu->beginItems; tmpItem; tmpItem = tmpItem->next )
    {
	if ( tmpItem->type == MENU_ITEM_SUBMENU)
	    crStrcat( retBuf, __createSubMenuXML( tmpItem->value , tmpItem->name) );
	else if ( tmpItem->type == MENU_ITEM_REGULAR )
	{
	    sprintf( tmpBuf, "<item type=\"%i\" name=\"%s\" value=\"%i\"></item>",
		     tmpItem->type, tmpItem->name, tmpItem->value );
	    crStrcat( retBuf, tmpBuf );
	}
    }
    crStrcat( retBuf, "</menu>" ); 

    return retBuf;
}

/* Not sure of how this should be formatted yet */
static void
__createMenuXML(void)
{
    char buffer[32];

    char* header = "<?xml version=\"1.0\" standalone=\"yes\"?><!DOCTYPE CRUTmenu [ <!ELEMENT CRUTmenu (button, menu+)> <!ELEMENT button (#PCDATA)> <!ATTLIST button id CDATA #REQUIRED><!ELEMENT menu (item+, menu?)> <!ATTLIST menu name CDATA #IMPLIED> <!ELEMENT item (#PCDATA)> <!ATTLIST item type CDATA #REQUIRED> <!ATTLIST item name CDATA #REQUIRED> <!ATTLIST item value CDATA #REQUIRED> ]>";
    crStrcat( crut_client.menuBuffer, header );
    crStrcat( crut_client.menuBuffer, "<CRUTmenu>" );

    sprintf( buffer, "<button id=\"%i\" />", crut_client.beginMenuList->att_button );
    crStrcat( crut_client.menuBuffer, buffer );
    crStrcat( crut_client.menuBuffer, 
	      __createSubMenuXML( crut_client.lastMenu->clientMenuID , NULL ) );
    crStrcat( crut_client.menuBuffer, "</CRUTmenu>" );
}

/* Check to see if there are currently messages in an event buffer */
int 
CRUT_CLIENT_APIENTRY 
crutCheckEvent(void)
{
    if (crut_client.next_event != crut_client.event_buffer->empty) 
	return 1;
    
    crNetRecv();
    return 0;
}

/* Peek ahead to the next event, return the type */
int
CRUT_CLIENT_APIENTRY
crutPeekNextEvent(void)
{
    CRUTMessage *msg;
    int msgsize;

    if (crut_client.next_event == crut_client.event_buffer->empty)
	return CRUT_NO_EVENT;

    msg = (CRUTMessage*) crut_client.next_event;
    msgsize = __getCRUTMessageSize(msg->msg_type);

    /* see if there is only one event in the current buffer */
    if ((crut_client.event_buffer->empty - 
	 (crut_client.next_event + msgsize)) < (int) MAX_MSG_SIZE)
    {
	if (crut_client.event_buffer != crut_client.last_buffer)
	{
	    msg = (CRUTMessage*) crut_client.event_buffer->next->buffer;
	    return msg->msg_type;
	}

	return CRUT_NO_EVENT;
    }
    else 
    {
	msg = (CRUTMessage*) (crut_client.next_event + msgsize);
	return msg->msg_type;
    }
}

/* Wait until we have an event to return */
void
CRUT_CLIENT_APIENTRY 
crutReceiveEvent(CRUTMessage **msg)
{
    unsigned long int dist;

    while (crut_client.next_event == crut_client.event_buffer->empty)
	crNetRecv();

    *msg = (CRUTMessage*) crut_client.next_event;

    crut_client.next_event += __getCRUTMessageSize((*msg)->msg_type);
    
    dist = crut_client.event_buffer->buffer_end - crut_client.next_event;

    /* If we've reached the last message in the buffer, deal accordingly */
    if (0 <= dist && dist < MAX_MSG_SIZE) 
    {
	/* If there's another buffer, free the first one and point to the second */
	if (crut_client.event_buffer != crut_client.last_buffer)
	{
	    EventBuffer* tmp = crut_client.event_buffer;
	    crut_client.event_buffer = crut_client.event_buffer->next;
	    crut_client.next_event = crut_client.event_buffer->buffer;
	    crFree(tmp);
	}
	else /* reuse the current buffer */
	{
	    crut_client.event_buffer->empty = crut_client.event_buffer->buffer;
	    crut_client.next_event = crut_client.event_buffer->buffer;
	}
    } /* end if 0 <= dist < MAX_MSG_SIZE */
}

static int 
crutClientRecv( CRConnection *conn, CRMessage *msg, unsigned int len )
{
    switch( msg->header.type )
    {
    case CR_MESSAGE_CRUT:
	__addToEventBuffer(msg);

       	return 1; /* HANDLED */
	break;
    default:
	return 0; /* NOT HANDLED */
    }
    (void) len;	
#ifndef WINDOWS
    return 0; /* NOT HANDLED */
#endif
}

static void 
crutClientClose( unsigned int id )
{
    crError( "Client disconnected!" );
    (void) id;
}

void 
CRUT_CLIENT_APIENTRY 
crutInitClient(void)
{
    char *mothership = NULL;
    char response[8096];
    char* server;
    char **newserver;
    int mtu;

    /* crMemZero( &crut_client, sizeof(crut_client) );  XXX init somewhere! */
    crut_client.num_bytes = sizeof( CRUTMouseMsg );
    crut_client.msg = crCalloc( crut_client.num_bytes );
     
    /* create and initialize event buffer */
    __newEventBuffer();
    crut_client.event_buffer = crut_client.last_buffer;
    crut_client.next_event = crut_client.last_buffer->buffer;
    
    crNetInit(crutClientRecv, crutClientClose);
    
    crutInitAPI( &crut_api, mothership);
    
    crMothershipIdentifyCRUTClient(crut_api.mothershipConn, response);
    
    /* set mothership parameters for server to use */
    if (crut_client.callbacks.mouse_param)
	crMothershipSetParam( crut_api.mothershipConn, "crut_callbacks_mouse", "1" );
    
    if (crut_client.callbacks.keyboard_param) 
	crMothershipSetParam( crut_api.mothershipConn, "crut_callbacks_keyboard", "1" );

    if (crut_client.callbacks.motion_param) 
	crMothershipSetParam( crut_api.mothershipConn, "crut_callbacks_motion", "1" );

    if (crut_client.callbacks.passivemotion_param) 
	crMothershipSetParam( crut_api.mothershipConn, "crut_callbacks_passivemotion", "1" );    
    
    if (crut_client.callbacks.visibility_param) 
	crMothershipSetParam( crut_api.mothershipConn, "crut_callbacks_visibility", "1" );

    if (crut_client.callbacks.reshape_param) 
	crMothershipSetParam( crut_api.mothershipConn, "crut_callbacks_reshape", "1" );

    if (crut_client.beginMenuList) 
    {
	__createMenuXML();
		
	crMothershipSetParam( crut_api.mothershipConn, "crut_callbacks_menu", "1");
	crMothershipSetParam( crut_api.mothershipConn, "crut_menu_xml", 
			      crut_client.menuBuffer );
    }
    crMothershipGetCRUTServer(crut_api.mothershipConn, response);
    newserver = crStrSplit(response, " ");
    server = newserver[1];
    
    mtu = crMothershipGetMTU(crut_api.mothershipConn);

    /* set up the connection to recv on */
    crut_client.recv_conn = crNetConnectToServer( server, DEFAULT_PORT, mtu, 0 );

    crutConnectToClients( &crut_api );
    crutPostRedisplay();
}

void
CRUT_CLIENT_APIENTRY 
crutMainLoop( )
{
    int lastEvent = CRUT_NO_EVENT;
    int nextEvent = CRUT_NO_EVENT;

    clientMenuList* menus;

    for (;;)
    {
	while (crutCheckEvent())
	{
	    /* Drop all of the mouse motion events that don't need to be processed.
	     * This happens here for a few reasons.  The event server can't tell how
	     * many events it should be dropping, and when the events are first received
	     * it is a little difficult to replace events already in the queue. */

	    for ( nextEvent = crutPeekNextEvent();
		  lastEvent == nextEvent && 
		      (nextEvent == CRUT_MOTION_EVENT ||
		       nextEvent == CRUT_PASSIVE_MOTION_EVENT);
		  nextEvent = crutPeekNextEvent()) {
		crutReceiveEvent(&crut_client.msg);
	    }
		
	    crutReceiveEvent(&crut_client.msg);
	    lastEvent = crut_client.msg->msg_type;

	    /* handle and forward events */
	    if (crut_client.msg->msg_type == CRUT_MOUSE_EVENT && crut_client.callbacks.mouse) 
	    {  
	        CRUTMouseMsg *msg = (CRUTMouseMsg*) crut_client.msg;
		crutSendMouseEvent( &crut_api, msg->button, msg->state, msg->x, msg->y);
		crut_client.callbacks.mouse(msg->button,msg->state,msg->x,msg->y);
	    }
	  
	    else if (crut_client.msg->msg_type == CRUT_RESHAPE_EVENT && crut_client.callbacks.reshape) 
	    {
	        CRUTReshapeMsg *msg = (CRUTReshapeMsg*) crut_client.msg;
		crutSendReshapeEvent( &crut_api, msg->width, msg->height);
		crut_client.callbacks.reshape(msg->width,msg->height);
	    }  

	    else if (crut_client.msg->msg_type == CRUT_VISIBILITY_EVENT && crut_client.callbacks.visibility) 
	    {
	        CRUTVisibilityMsg *msg = (CRUTVisibilityMsg*) crut_client.msg;
		crutSendVisibilityEvent( &crut_api, msg->state);
		crut_client.callbacks.visibility(msg->state);
	    }
	  
	    else if (crut_client.msg->msg_type == CRUT_KEYBOARD_EVENT && crut_client.callbacks.keyboard) 
	    {      
		CRUTKeyboardMsg *msg = (CRUTKeyboardMsg*) crut_client.msg;
		crutSendKeyboardEvent( &crut_api, msg->key, msg->x, msg->y);
		crut_client.callbacks.keyboard(msg->key,msg->x,msg->y);
	    }
	  
	    else if (crut_client.msg->msg_type == CRUT_MOTION_EVENT && crut_client.callbacks.motion) 
	    {
		CRUTMotionMsg *msg = (CRUTMotionMsg*) crut_client.msg;
		crutSendMotionEvent( &crut_api, msg->x, msg->y);
		crut_client.callbacks.motion(msg->x,msg->y);
	    }
	  
	    else if (crut_client.msg->msg_type == CRUT_PASSIVE_MOTION_EVENT && crut_client.callbacks.passivemotion) 
	    {
		CRUTPassiveMotionMsg *msg = (CRUTPassiveMotionMsg*) crut_client.msg;
		crutSendPassiveMotionEvent( &crut_api, msg->x, msg->y);
		crut_client.callbacks.passivemotion(msg->x,msg->y);
	    }

	    else if (crut_client.msg->msg_type == CRUT_MENU_EVENT) 
	    {
		CRUTMenuMsg *msg = (CRUTMenuMsg*) crut_client.msg;
		crutSendMenuEvent( &crut_api, msg->menuID, msg->value );
		
		for (menus = crut_client.beginMenuList; (menus); menus = menus->next)
		{
		    if (menus->clientMenuID == msg->menuID)
			menus->menu( msg->value );
		}
	    }
	} /* end while(crutCheckEvent()) */

	if (crut_client.redisplay && crut_client.callbacks.display)
	{
	    crut_client.redisplay = 0;
	    crut_client.callbacks.display();
	}
	else
	    if (crut_client.callbacks.idle) crut_client.callbacks.idle();

    } /* end for(;;) */
}

