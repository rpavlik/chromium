/* Written by Dale Beermann (beermann@cs.virginia.edu) */

#include "crutserver.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_mothership.h"
#include "cr_string.h"

/**
 * \mainpage CrutServer 
 *
 * \section CrutServerIntroduction Introduction
 *
 */
CRUTServer crut_server;
CRUTAPI crut_api;

int curMenuID;

static int 
crutServerRecv( CRConnection *conn, void *buf, unsigned int len )
{
    CRMessage *msg = (CRMessage *) buf;
    (void) len;
    
    switch( msg->header.type )
    {
    case CR_MESSAGE_CRUT:
	return 0; /* NOT HANDLED - server shouldn't receive anything */

    default:
	return 0; /* NOT HANDLED */
    }
#if !( defined(WINDOWS) || defined(DARWIN) )
  return 0; /* HANDLED */
#endif
}

static void 
crutServerClose( unsigned int id )
{
	crError( "Client disconnected!" );
	(void) id;
}

/* GLUT needs a function for the display callback */
static void 
showWin( void ) 
{

}


static void
handleMenu( int value )
{
    int menuID = crut_server.valueTable[value].menuID;
    int retValue = crut_server.valueTable[value].value;

    crutSendMenuEvent( &crut_api, menuID, retValue );
}

#ifdef USE_CRUT_MENUS
static void
pushMenu( apiMenu* newMenu )
{
    newMenu->sLast = crut_server.endStack;
    
    crut_server.endStack->sNext = newMenu;
    crut_server.endStack = newMenu;
}

static apiMenu* 
popMenu( void )
{
    apiMenu* ret = crut_server.endStack;
    crut_server.endStack = crut_server.endStack->sLast;

    return ret;
}

static void
addItem( char* name, int value )
{

    if ( !crut_server.endStack->beginItems )
    {
	crut_server.endStack->beginItems = crut_server.endStack->lastItem = 
	    crAlloc( sizeof( apiMenuItem ) );
    }
    else
    {
	crut_server.endStack->lastItem->next = crAlloc( sizeof( apiMenuItem ) );
	crut_server.endStack->lastItem = crut_server.endStack->lastItem->next;
    }

    crut_server.endStack->lastItem->name = crAlloc( sizeof( strlen(name) ) );
    crStrcpy( crut_server.endStack->lastItem->name, name );
    
    crut_server.endStack->lastItem->value = value;
}

static void
createSubMenu( apiMenu* newMenu )
{
    int i;

    /* begin copy subMenus - should re-implement this as a linked list*/
    apiMenu** tmp = crAlloc( sizeof(apiMenu*) * (crut_server.endStack->subMenus + 1) );

    for ( i=0; i < crut_server.endStack->subMenus; i++ )
	tmp[i]  = crut_server.endStack->tree[i];
    /* end copy subMenus */
    
    tmp[crut_server.endStack->subMenus] = newMenu;

    crut_server.endStack->tree = tmp;
    
    crut_server.endStack->subMenus++;

}

static void
start_hndl(void *data, const char *el, const char **attr) 
{
    static int topMenu = 0;
    static int button = 0;
    char* name;
    int value, i;
    apiMenu* newMenu;
    
    if ( strcmp( el, "button" ) == 0 )
    {
	button = crStrToInt( attr[1] );
    }
    else if ( strcmp( el, "menu") == 0 )
    {
	if ( !topMenu )
	{
	    crut_server.stack = crut_server.endStack = crAlloc( sizeof(apiMenu) );
	    crut_server.stack->subMenus = 0;
	    crut_server.stack->sLast = crut_server.stack->sNext = NULL;
	    crut_server.stack->att_button = button;

	    crut_server.stack->menuID = crStrToInt( attr[1] );

	    topMenu = 1;
	}
	else
	{
	    newMenu = crAlloc( sizeof(apiMenu) );
	    
	    newMenu->name = crAlloc( strlen( attr[1] ) );
	    crStrcpy( newMenu->name, attr[1] );

	    newMenu->menuID = crStrToInt( attr[3] );
	    
	    pushMenu( newMenu );
	}
    }
    else if ( strcmp( el, "item" ) == 0 )
    {
	for ( i=0; i<=4; i+=2 )
	{
	    if ( strcmp( attr[i], "value" ) == 0 )
		value = crStrToInt( attr[i+1] );

	    if ( strcmp( attr[i], "name" ) == 0 )
		name = (char*) attr[i+1];
	}
	
	addItem( name, value);
    }
}

static void
end_hndl(void *data, const char *el) 
{
    apiMenu* popped;

    if ( strcmp( el, "menu" ) == 0)
    {   
	if ( !(crut_server.endStack->sLast == NULL) )
	{
	    popped = popMenu();
	    createSubMenu( popped );
	}
    }
}
#endif /* USE_CRUT_MENUS */


static int 
addToValueList( int menuID, int value )
{
    static int index = 0;

    if ( !crut_server.values )
    	crut_server.values = crut_server.endValue = crAlloc( sizeof( itemValue ) );
    else
    {
	crut_server.endValue->next = crAlloc( sizeof( itemValue ) );
	crut_server.endValue = crut_server.endValue->next;
    }

    crut_server.endValue->index = index;
    crut_server.endValue->menuID = menuID;
    crut_server.endValue->value = value;
    index++;
    
    crut_server.numValues++;
    
    return crut_server.endValue->index;
}

#ifdef USE_CRUT_MENUS
static void
buildValueArray(void)
{
    int i;
    itemValue* item = crut_server.values;
    itemValue* temp;

    crut_server.valueTable = crAlloc( crut_server.numValues * sizeof( itemValue ) );

    for ( i=0; i < crut_server.numValues; i++ )
    {
	crut_server.valueTable[i].index = item->index;
	crut_server.valueTable[i].value = item->value;
	crut_server.valueTable[i].menuID = item->menuID;

	temp = item;
	item = item->next;
	crFree(temp);
    } 
}
#endif /* USE_CRUT_MENUS */

static int
buildGLUTMenu( apiMenu* node)
{
    int i, menu, id;
    apiMenuItem* items;

    for ( i=0; i < node->subMenus; i++)
	node->tree[i]->glutMenuID = buildGLUTMenu( node->tree[i] );
   
    /* XXX glutCreateMenu seems to segfault when the item name is too long??? */
    menu = glutCreateMenu( handleMenu );

    for( items = node->beginItems; items; items=items->next )
    {
	id = addToValueList( node->menuID, items->value );
	glutAddMenuEntry( items->name, id ); 
    }

    for ( i=0; i < node->subMenus; i++)
	glutAddSubMenu( node->tree[i]->name, node->tree[i]->glutMenuID );

    if (node == crut_server.stack)
	glutAttachMenu( node->att_button );

    return menu;
}

#ifdef USE_CRUT_MENUS
static void
buildMenu(void)
{
    /* XML parsing is done using expat */
    int done = 1;
    int len = strlen(crut_api.menuBuffer);

    XML_Parser p = XML_ParserCreate(NULL);

    if (! p) 
    {
	fprintf(stderr, "Couldn't allocate memory for parser\n");
	exit(-1);
    }
    
    XML_UseParserAsHandlerArg(p);
    XML_SetElementHandler(p, start_hndl, end_hndl);

    if (! XML_Parse(p, crut_api.menuBuffer, len, done)) 
    {
	crError("Parse error at line %d:\n%s\n",
	      XML_GetCurrentLineNumber(p),
	      XML_ErrorString(XML_GetErrorCode(p)));
    }

    buildGLUTMenu( crut_server.stack );

    buildValueArray();
}
#endif

static void 
crutInitServer(char *mothership, int argc, char *argv[])
{
    char response[8096];
#ifndef WINDOWS
    int drawable;
#endif

    crNetInit( crutServerRecv, crutServerClose );

    crutInitAPI( &crut_api, mothership ); /* here? */

    crMothershipIdentifyCRUTServer( crut_api.mothershipConn, response );

    crutGetWindowParams( &crut_api );
    
    /* set up GLUT window */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition( crut_api.winX,crut_api.winY );
    glutInitWindowSize( crut_api.winWidth,crut_api.winHeight );
    
    glutCreateWindow("CRUTServer Interactive Window"); 
    
    glutDisplayFunc(showWin);
    
    /* give window id to mothership */
#ifndef WINDOWS
#ifdef DARWIN
	/* XXX \todo crut get current drawable (this only gets glut win_num) */
	drawable = glutGetWindow();
#else
    /* XXX is this cast safe? */
    drawable = (int) glXGetCurrentDrawable();
#endif
    crutSetWindowID( &crut_api, drawable);
#endif

    /* use API to connect to clients */
    crutConnectToClients( &crut_api );

    /* Retrieve events to send out */
    crMothershipGetParam( crut_api.mothershipConn, "crut_callbacks_mouse", response );
    crut_server.callbacks.mouse = crStrToInt(response);

    crMothershipGetParam( crut_api.mothershipConn, "crut_callbacks_keyboard", response );
    crut_server.callbacks.keyboard = crStrToInt(response);

    crMothershipGetParam( crut_api.mothershipConn, "crut_callbacks_motion", response );
    crut_server.callbacks.motion = crStrToInt(response);

    crMothershipGetParam( crut_api.mothershipConn, "crut_callbacks_passivemotion", response );
    crut_server.callbacks.passivemotion = crStrToInt(response);

    crMothershipGetParam( crut_api.mothershipConn, "crut_callbacks_reshape", response );
    crut_server.callbacks.reshape = crStrToInt(response);

    crMothershipGetParam( crut_api.mothershipConn, "crut_callbacks_visibility", response );
    crut_server.callbacks.visibility = crStrToInt(response);

    crMothershipGetParam( crut_api.mothershipConn, "crut_callbacks_menu", response );
    crut_server.callbacks.menu = crStrToInt(response);

#ifdef USE_CRUT_MENUS
    crutGetMenuXML();

    buildMenu();
#endif
 
}

static void 
handleMouse(int button, int state, int x, int y)
{
    crutSendMouseEvent( &crut_api, button, state, x, y);
}

static void 
handleReshape(int width, int height)
{
    crutSendReshapeEvent( &crut_api, width, height );
}

static void 
handleKeyboard(unsigned char key, int x, int y) 
{
    crutSendKeyboardEvent( &crut_api, key, x, y );
}

static void 
handleMotion(int x, int y)
{
    crutSendMotionEvent(  &crut_api, x, y );
}

static void 
handlePassiveMotion(int x, int y)
{
    crutSendPassiveMotionEvent( &crut_api, x, y );
}

static void 
handleVisibility(int state)
{
    crutSendVisibilityEvent( &crut_api, state );
}

int 
main( int argc, char *argv[] )
{
    int i;
    char *mothership = NULL;
#ifdef DARWIN
#elif !defined(WINDOWS)
    Display *disp = NULL;
#endif

    for (i = 1 ; i < argc ; i++)
    {
        if (!crStrcmp( argv[i], "-mothership" ))
	{
	    if (i == argc - 1)
	    {
	        crError( "-mothership requires an argument" );
	    }
	    mothership = argv[i+1];
	    i++;
	}
    } 

    crutInitServer(mothership, argc, argv);

    if (crut_server.callbacks.mouse)
        glutMouseFunc(handleMouse);

    if (crut_server.callbacks.keyboard)
        glutKeyboardFunc(handleKeyboard);

    if (crut_server.callbacks.motion)
        glutMotionFunc(handleMotion);
  
    if (crut_server.callbacks.passivemotion)
        glutPassiveMotionFunc(handlePassiveMotion);

    if (crut_server.callbacks.visibility)
        glutVisibilityFunc(handleVisibility);

    if (crut_server.callbacks.reshape)
        glutReshapeFunc(handleReshape);
  
    /* Need to 'release' the current drawable so render SPU can make current to it*/
#ifdef DARWIN
#elif !defined(WINDOWS)
    disp = glXGetCurrentDisplay();
    glXMakeCurrent(disp, None, NULL);
#endif
    
    glutMainLoop();
  
    return 0;
}


