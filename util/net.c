/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <memory.h>

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <process.h>
#else
#include <unistd.h>
#endif

#include "cr_mem.h"
#include "cr_error.h"
#include "cr_string.h"
#include "cr_url.h"
#include "cr_net.h"
#include "cr_netserver.h"
#include "cr_bufpool.h"
#include "cr_environment.h"
#include "cr_endian.h"
#include "net_internals.h"

#define CR_INITIAL_RECV_CREDITS ( 1 << 21 ) /* 2MB */

static struct {
	int                  initialized; /* flag */
	CRNetReceiveFunc     recv;        /* what to do with arriving packets */
	CRNetCloseFunc       close;       /* what to do when a client goes down */
	int                  use_gm;      /* count the number of people using GM */
	int                  num_clients; /* count the number of total clients (unused?) */
	CRBufferPool         message_list_pool;
} cr_net;

/* This the common interface that every networking type should export in order 
 * to work with this abstraction.  An initializer, a 'start up connection' function, 
 * and a function to recieve work on that interface. */

#define NETWORK_TYPE(x) \
	extern void cr##x##Init(CRNetReceiveFunc, CRNetCloseFunc, unsigned int); \
	extern void cr##x##Connection(CRConnection *); \
	extern int cr##x##Recv(void)

/* Now, all the appropriate interfaces are defined simply by listing the supported 
 * networking types here. */

NETWORK_TYPE( TCPIP );
NETWORK_TYPE( Devnull );
#ifdef GM_SUPPORT
NETWORK_TYPE( Gm );
#endif

/* Clients call this function to connect to a server.  The "server" argument is
 * expected to be a URL type specifier "protocol://servername:port", where the port 
 * specifier is optional, and if the protocol is missing it is assumed to be 
 * "tcpip".  
 * 
 * Not sure if the MTU argument should be here -- maybe in crNetInit()? */

CRConnection *crNetConnectToServer( char *server, 
		unsigned short default_port, int mtu, int broker )
{
	char hostname[4096], protocol[4096];
	unsigned short port;
	CRConnection *conn;

	CRASSERT( cr_net.initialized );


	/* Tear the URL apart into relevant portions. */
	if ( !crParseURL( server, protocol, hostname, &port, default_port ) )
	{
		crError( "Malformed URL: \"%s\"", server );
	}
	crDebug( "Connecting to server %s on port %d, with protocol %s", 
			hostname, port, protocol );

	conn = (CRConnection *) crAlloc( sizeof(*conn) );

	conn->type               = CR_NO_CONNECTION; /* we don't know yet */
	conn->id                 = 0;                /* Each connection has an id */
	conn->total_bytes        = 0;                    /* how many bytes have we sent? */
	conn->send_credits       = 0;
	conn->recv_credits       = CR_INITIAL_RECV_CREDITS;
	conn->hostname           = crStrdup( hostname ); 
	conn->port               = port;
	conn->Alloc              = NULL;                 /* How do we allocate buffers to send? */
	conn->Send               = NULL;                 /* How do we send things? */
	conn->Free               = NULL;                 /* How do we free things? */
	conn->tcp_socket         = 0;
	conn->gm_node_id         = 0;
	conn->mtu                = mtu;
	conn->broker             = broker;
	conn->swap               = 0;
	conn->endianness         = crDetermineEndianness();

	conn->multi.len = 0;
	conn->multi.max = 0;
	conn->multi.buf = NULL;

	conn->messageList        = NULL;
	conn->messageTail        = NULL;

	/* now, just dispatch to the appropriate protocol's initialization functions. */
	
	if ( !strcmp( protocol, "devnull" ) )
	{
		crDevnullInit( cr_net.recv, cr_net.close, mtu );
		crDevnullConnection( conn );
	}
#ifdef GM_SUPPORT
	else if ( !strcmp( protocol, "gm" ) )
	{
		cr_net.use_gm++;
		crGmInit( cr_net.recv, cr_net.close, mtu );
		crGmConnection( conn );
	}
#endif
	else if ( !strcmp( protocol, "tcpip" ) )
	{
		crTCPIPInit( cr_net.recv, cr_net.close, mtu );
		crTCPIPConnection( conn );
	}
	else
	{
		crError( "Unknown Protocol: \"%s\"", protocol );
	}

	if (!crNetConnect( conn ))
	{
		crFree( conn );
		return NULL;
	}

	if (conn->swap)
	{
		crWarning( "Creating a byte-swapped connection!" );
	}
	crDebug( "Done connecting to server." );
	return conn;
}

/* Accept a client on various interfaces. */

CRConnection *crNetAcceptClient( char *protocol, unsigned short port, unsigned int mtu, int broker )
{
	CRConnection *conn;

	CRASSERT( cr_net.initialized );

	conn = (CRConnection *) crAlloc( sizeof( *conn ) );
	conn->type               = CR_NO_CONNECTION; /* we don't know yet */
	conn->id                 = 0;                /* Each connection has an id */
	conn->total_bytes        = 0;                    /* how many bytes have we sent? */
	conn->send_credits       = 0;
	conn->recv_credits       = CR_INITIAL_RECV_CREDITS;
	/*conn->hostname           = crStrdup( hostname ); */
	conn->port               = port;
	conn->Alloc              = NULL;                 /* How do we allocate buffers to send? */
	conn->Send               = NULL;                 /* How do we send things? */
	conn->Free               = NULL;                 /* How do we receive things? */
	conn->tcp_socket         = 0;
	conn->gm_node_id         = 0;
	conn->mtu                = mtu;
	conn->broker             = broker;
	conn->swap               = 0;
	conn->endianness         = crDetermineEndianness();

	conn->multi.len = 0;
	conn->multi.max = 0;
	conn->multi.buf = NULL;

	conn->messageList        = NULL;
	conn->messageTail        = NULL;

	/* now, just dispatch to the appropriate protocol's initialization functions. */
	
	if ( !strcmp( protocol, "devnull" ) )
	{
		crDevnullInit( cr_net.recv, cr_net.close, mtu );
		crDevnullConnection( conn );
	}
#ifdef GM_SUPPORT
	else if ( !strcmp( protocol, "gm" ) )
	{
		cr_net.use_gm++;
		crGmInit( cr_net.recv, cr_net.close, mtu );
		crGmConnection( conn );
	}
#endif
	else if ( !strcmp( protocol, "tcpip" ) )
	{
		crTCPIPInit( cr_net.recv, cr_net.close, mtu );
		crTCPIPConnection( conn );
	}
	else
	{
		crError( "Unknown Protocol: \"%s\"", protocol );
	}

	crNetAccept( conn, port );
	return conn;
}

/* Start the ball rolling.  give functions to handle incoming traffic 
 * (usually placing blocks on a queue), and a handler for dropped 
 * connections. */

void crNetInit( CRNetReceiveFunc recvFunc, CRNetCloseFunc closeFunc )
{
	if ( cr_net.initialized )
	{
		/* This way, networking can be initialized before anyone's really 
		 * ready to play -- i.e., to talk to the configuration server. 
		 * 
		 * Basically, the OpenGL stub will initialize networking because 
		 * it needs to get the damn WSAStartup in, but it has no clue 
		 * what the recvFunc and closeFunc should be later. 
		 * 
		 * So, the stub explicitly them to NULL, meaning they can be overridden 
		 * later. */
		
		if (cr_net.recv == NULL && cr_net.close == NULL) 
		{
			cr_net.recv = recvFunc;
			cr_net.close = closeFunc;
			return;
		}
		else
		{
			crWarning( "Networking already initialized!" );
			return;
		}
	}
	else
	{
#ifdef WINDOWS
		WORD wVersionRequested = MAKEWORD(2, 0);
		WSADATA wsaData;
		int err;

		err = WSAStartup(wVersionRequested, &wsaData);
		if (err != 0)
			crError("Couldn't initialize sockets on WINDOWS");
#endif

		cr_net.recv        = recvFunc;
		cr_net.close       = closeFunc;
		cr_net.use_gm      = 0;
		cr_net.num_clients = 0;
		crBufferPoolInit( &cr_net.message_list_pool, 16 );

		cr_net.initialized = 1;
	}
}

/* Buffers that will eventually be transmitted on a connection 
 * *must* be allocated using this interface.  This way, we can 
 * automatically pin memory and tag blocks, and we can also use 
 * our own buffer pool management. */

void *crNetAlloc( CRConnection *conn )
{
	CRASSERT( conn );
	return conn->Alloc( conn );
}

/* Send a set of commands on a connection.  Pretty straightforward, just 
 * error checking, byte counting, and a dispatch to the protocol's 
 * "send" implementation. */

void crNetSend( CRConnection *conn, void **bufp, 
		            void *start, unsigned int len )
{
	CRMessage *msg = (CRMessage *) start;
	CRASSERT( conn );
	CRASSERT( len > 0 );
	if ( bufp ) {
		CRASSERT( start >= *bufp );
		CRASSERT( (unsigned char *) start + len <= 
				(unsigned char *) *bufp + conn->mtu );
	}

#ifndef NDEBUG
	if ( conn->send_credits > CR_INITIAL_RECV_CREDITS )
	{
		crError( "crNetSend: send_credits=%u, looks like there is a "
				"leak (max=%u)", conn->send_credits,
				CR_INITIAL_RECV_CREDITS );
	}
#endif

	conn->total_bytes += len;

	msg->header.conn_id = conn->id;
	conn->Send( conn, bufp, start, len );
}

/* Send something exact on a connection without the message length 
 * header. */

void crNetSendExact( CRConnection *conn, void *buf, unsigned int len )
{
	conn->SendExact( conn, buf, len );
}

/* Since the networking layer allocates memory, it needs to free it as 
 * well.  This will return things to the correct buffer pool etc. */

void crNetFree( CRConnection *conn, void *buf )
{
	conn->Free( conn, buf );
}

/* Actually do the connection implied by the argument */

int crNetConnect( CRConnection *conn )
{
	return conn->Connect( conn );
}

/* Tear it down */

void crNetDisconnect( CRConnection *conn )
{
	conn->Disconnect( conn );
}

void crNetAccept( CRConnection *conn, unsigned short port )
{
	conn->Accept( conn, port );
}

/* Do a blocking receive on a particular connection.  This only 
 * really works for TCPIP, but it's really only used (right now) by 
 * the mothership client library. */

void crNetSingleRecv( CRConnection *conn, void *buf, unsigned int len )
{
	if (conn->type != CR_TCPIP)
	{
		crError( "Can't do a crNetSingleReceive on anything other than TCPIP." );
	}
	conn->Recv( conn, buf, len );
}

static void crNetRecvMulti( CRConnection *conn, CRMessageMulti *msg, unsigned int len )
{
	CRMultiBuffer *multi = &(conn->multi);
	unsigned char *src, *dst;

	CRASSERT( len > sizeof(*msg) );
	len -= sizeof(*msg);

	if ( len + multi->len > multi->max )
	{
		if ( multi->max == 0 )
		{
			multi->len = conn->sizeof_buffer_header;
			multi->max = 8192;
		}
		while ( len + multi->len > multi->max )
		{
			multi->max <<= 1;
		}
		crRealloc( &multi->buf, multi->max );
	}

	dst = (unsigned char *) multi->buf + multi->len;
	src = (unsigned char *) msg + sizeof(*msg);
	memcpy( dst, src, len );
	multi->len += len;

	conn->InstantReclaim( conn, (CRMessage *) msg );

	if (msg->header.type == CR_MESSAGE_MULTI_TAIL)
	{
		conn->HandleNewMessage( 
				conn, 
				(CRMessage *) (((char *) multi->buf) + conn->sizeof_buffer_header), 
				multi->len - conn->sizeof_buffer_header );

		/* clean this up before calling the user */
		multi->buf = NULL;
		multi->len = 0;
		multi->max = 0;
	}
}

static void crNetRecvFlowControl( CRConnection *conn,
		CRMessageFlowControl *msg, unsigned int len )
{
	CRASSERT( len == sizeof(CRMessageFlowControl) );

	/*crWarning ("Getting %d credits!", msg->credits); */
	if (conn->swap)
	{
		conn->send_credits += SWAP32(msg->credits);
	}
	else
	{
		conn->send_credits += msg->credits;
	}

	conn->InstantReclaim( conn, (CRMessage *) msg );
}

void crNetRecvWriteback( CRMessageWriteback *wb )
{
	int *writeback;
	memcpy( &writeback, &(wb->writeback_ptr), sizeof( writeback ) );
	(*writeback)--;
}

void crNetRecvReadback( CRMessageReadback *rb, unsigned int len )
{
	/* minus the header, the destination pointer, 
	 * *and* the implicit writeback pointer at the head. */

	int payload_len = len - sizeof( *rb );
	int *writeback;
	void *dest_ptr; 
	memcpy( &writeback, &(rb->writeback_ptr), sizeof( writeback ) );
	memcpy( &dest_ptr, &(rb->readback_ptr), sizeof( dest_ptr ) );

	(*writeback)--;
	memcpy( dest_ptr, ((char *)rb) + sizeof(*rb), payload_len );
}

void crNetDefaultRecv( CRConnection *conn, void *buf, unsigned int len )
{
	CRMessageList *msglist;
	
	CRMessage *msg = (CRMessage *) buf;

	switch( msg->header.type )
	{
		case CR_MESSAGE_MULTI_BODY:
		case CR_MESSAGE_MULTI_TAIL:
			crNetRecvMulti( conn, &(msg->multi), len );
			return;
		case CR_MESSAGE_FLOW_CONTROL:
			crNetRecvFlowControl( conn, &(msg->flowControl), len );
			return;
		case CR_MESSAGE_OPCODES:
		case CR_MESSAGE_OOB:
			{
				/*CRMessageOpcodes *ops = (CRMessageOpcodes *) msg; 
				 *unsigned char *data_ptr = (unsigned char *) ops + sizeof( *ops) + ((ops->numOpcodes + 3 ) & ~0x03); 
				 *crDebugOpcodes( stdout, data_ptr-1, ops->numOpcodes ); */
			}
			break;
		case CR_MESSAGE_READ_PIXELS:
			crError( "Can't handle read pixels" );
			return;
		case CR_MESSAGE_WRITEBACK:
			crNetRecvWriteback( &(msg->writeback) );
			return;
		case CR_MESSAGE_READBACK:
			crNetRecvReadback( &(msg->readback), len );
			return;
		default:
			/* We can end up here if anything strange happens in
			 * the GM layer.  In particular, if the user tries to
			 * send unpinned memory over GM it gets sent as all
			 * 0xAA instead.  This can happen when a program exits
			 * ungracefully, so the GM is still DMAing memory as
			 * it is disappearing out from under it.  We can also
			 * end up here if somebody adds a message type, and
			 * doesn't put it in the above case block.  That has
			 * an obvious fix. */
			{
				char string[128];
				crBytesToString( string, sizeof(string), msg, len );
				crWarning( "\n\nI'm ABOUT TO EXPLODE!  Did you add a new\n"
						       "message type and forget to tell crNetDefaultRecv\n"
									 "about it?\n\n" );
				crError( "crNetDefaultRecv: received a bad message: "
						"buf=[%s]", string );
			}
	}

	/* If we make it this far, it's not a special message, so 
	 * just tack it on to the end of the connection's list of 
	 * work blocks. */
	
	msglist = (CRMessageList *) crBufferPoolPop( &cr_net.message_list_pool );
	if ( msglist == NULL )
	{
		msglist = (CRMessageList *) crAlloc( sizeof( *msglist ) );
	}
	msglist->mesg = (CRMessage*)buf;
	msglist->len = len;
	msglist->next = NULL;
	if (conn->messageTail)
	{
		conn->messageTail->next = msglist;
	}
	else
	{
		conn->messageList = msglist;
	}
	conn->messageTail = msglist;
}

unsigned int crNetGetMessage( CRConnection *conn, CRMessage **message )
{
	for (;;)
	{
		if (conn->messageList != NULL)
		{
			CRMessageList *temp;
			unsigned int len;
			*message = conn->messageList->mesg;
			len = conn->messageList->len;
			temp = conn->messageList;
			conn->messageList = conn->messageList->next;
			if (!conn->messageList)
			{
				conn->messageTail = NULL;
			}
			crBufferPoolPush( &(cr_net.message_list_pool), temp );
			return len;
		}
		crNetRecv();
	}
	/* NOTREACHED 
	 * return 0; */
}

/* Read a line from a socket.  Useful for reading from the mothership. */

void crNetReadline( CRConnection *conn, void *buf )
{
	char *temp, c;
	if (conn->type != CR_TCPIP)
	{
		crError( "Can't do a crNetReadline on anything other than TCPIP." );
	}
	temp = (char*)buf;
	for (;;)
	{
		conn->Recv( conn, &c, 1 );
		if (c == '\n')
		{
			*temp = '\0';
			return;
		}
		*(temp++) = c;
	}
}

/* The big boy -- call this function to see (non-blocking) if there is 
 * any pending work.  If there is, the networking layer's "work received" 
 * handler will be called, so this function only returns a flag.  Work 
 * is assumed to be placed on queues for processing by the handler. */

int crNetRecv( void )
{
	int found_work = 0;

	found_work += crTCPIPRecv( );
#ifdef GM_SUPPORT
	if ( cr_net.use_gm )
		found_work += crGmRecv( );
#endif

	return found_work;
}

int crGetPID( void )
{
	return (int) getpid();
}

void crNetServerConnect( CRNetServer *ns )
{
	ns->conn = crNetConnectToServer( ns->name, 7000, ns->buffer_size, 1 );
}

/* The fact that I've copied this function makes me ill. 
 * GM connections need to be brokered through the mothership, 
 * so I need to connect to the mothership, but I can't use the 
 * client library because it links against *this* library. 
 * Shoot me now.  No wonder academics have such a terrible 
 * reputation in industry. 
 * 
 *      --Humper */

#define MOTHERPORT 10000

CRConnection *__copy_of_crMothershipConnect( void )
{
	char *mother_server = NULL;
	int   mother_port = MOTHERPORT;
	char mother_url[1024];

	crNetInit( NULL, NULL );

	mother_server = crGetenv( "CRMOTHERSHIP" );
	if (!mother_server)
	{
		crWarning( "Couldn't find the CRMOTHERSHIP environment variable, defaulting to localhost" );
		mother_server = "localhost";
	}

	sprintf( mother_url, "%s:%d", mother_server, mother_port );

	return crNetConnectToServer( mother_server, 10000, 8096, 0 );
}

/* More code-copying lossage.  I sure hope no one ever sees this code.  Ever. */

int __copy_of_crMothershipReadResponse( CRConnection *conn, void *buf )
{
	char codestr[4];
	int code;

	crNetSingleRecv( conn, codestr, 4 );
	crNetReadline( conn, buf );

	code = crStrToInt( codestr );
	return (code == 200);
}

/* And, the final insult. */

int __copy_of_crMothershipSendString( CRConnection *conn, char *response_buf, char *str, ... )
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
		return __copy_of_crMothershipReadResponse( conn, response_buf );
	}
	else
	{
		char devnull[1024];
		return __copy_of_crMothershipReadResponse( conn, devnull );
	}
}

void __copy_of_crMothershipDisconnect( CRConnection *conn )
{
	__copy_of_crMothershipSendString( conn, NULL, "quit" );
	crNetDisconnect( conn );
}
