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
#include <signal.h>

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
#include "cr_environment.h"
#include "cr_endian.h"
#include "cr_bufpool.h"
#include "cr_threads.h"
#include "net_internals.h"

#define CR_INITIAL_RECV_CREDITS ( 1 << 21 ) /* 2MB */

/* Allow up to four processes per node. . . */
#define CR_QUADRICS_LOWEST_RANK  0
#define CR_QUADRICS_HIGHEST_RANK 3

static struct {
	int                  initialized; /* flag */
	CRNetReceiveFuncList *recv_list;  /* what to do with arriving packets */
	CRNetCloseFuncList   *close_list; /* what to do when a client goes down */
	int                  use_gm;      /* count the number of people using GM */
  int                  use_teac;    /* count the number of people using teac */
  int                  use_tcscomm; /* count the number of people using tcscomm 
*/

	int                  num_clients; /* count the number of total clients (unused?) */
	CRBufferPool         *message_list_pool;
#ifdef CHROMIUM_THREADSAFE
	CRmutex		     mutex;
#endif
  int                  my_rank;
} cr_net;

/* This the common interface that every networking type should export in order 
 * to work with this abstraction.  An initializer, a 'start up connection' function, 
 * and a function to recieve work on that interface. */

#define NETWORK_TYPE(x) \
	extern void cr##x##Init(CRNetReceiveFuncList *, CRNetCloseFuncList *, unsigned int); \
	extern void cr##x##Connection(CRConnection *); \
	extern int cr##x##Recv(void)

/* Now, all the appropriate interfaces are defined simply by listing the supported 
 * networking types here. */

NETWORK_TYPE( TCPIP );
NETWORK_TYPE( UDPTCPIP );
NETWORK_TYPE( Devnull );
NETWORK_TYPE( File );
#ifdef GM_SUPPORT
NETWORK_TYPE( Gm );
#endif
#ifdef TEAC_SUPPORT
NETWORK_TYPE( Teac );
extern void crTeacSetRank( int );
extern void crTeacSetContextRange( int, int );
extern void crTeacSetNodeRange( const char *, const char * );
#endif
#ifdef TCSCOMM_SUPPORT
NETWORK_TYPE( Tcscomm );
extern void crTcscommSetRank( int );
extern void crTcscommSetContextRange( int, int );
extern void crTcscommSetNodeRange( const char *, const char * );
#endif

/* Clients call this function to connect to a server.  The "server" argument is
 * expected to be a URL type specifier "protocol://servername:port", where the port 
 * specifier is optional, and if the protocol is missing it is assumed to be 
 * "tcpip".  */

#define CR_MINIMUM_MTU 1024

CRConnection *crNetConnectToServer( char *server, 
		unsigned short default_port, int mtu, int broker )
{
	char hostname[4096], protocol[4096];
	unsigned short port;
	CRConnection *conn;

	crDebug( "calling crNetConnectToServer( \"%s\", %d, %d, %d )", server, default_port, mtu, broker );

	CRASSERT( cr_net.initialized );
	if (mtu < CR_MINIMUM_MTU)
	{
		crError( "You tried to connect to server \"%s\" with an mtu of %d, but the minimum MTU is %d", server, mtu, CR_MINIMUM_MTU );
	}

	/* Tear the URL apart into relevant portions. */
	if ( !crParseURL( server, protocol, hostname, &port, default_port ) )
	  {
	    crError( "Malformed URL: \"%s\"", server );
	  }
	
	/* If the host name is "localhost" replace it with the _real_ name
	 * of the localhost.  If we don't do this, there seems to be
	 * confusion in the mothership as to whether or not "localhost" and
	 * "foo.bar.com" are the same machine.
	 */
	if (crStrcmp(hostname, "localhost") == 0) {
		int rv = crGetHostname(hostname, 4096);
		CRASSERT(rv == 0);
		(void) rv;
	}

	if ( !crStrcmp( protocol, "quadrics" ) ||
	     !crStrcmp( protocol, "quadrics-tcscomm" ) ) {
	  /* For Quadrics protocols, treat "port" as "rank" */
	  if ( port < CR_QUADRICS_HIGHEST_RANK ) {
	    crWarning( "Invalid crserver rank, %d, defaulting to %d\n",
		       port, CR_QUADRICS_LOWEST_RANK );
	    port = CR_QUADRICS_LOWEST_RANK;
	  }
	  crDebug( "Connecting to server %s:%d via Quadrics",
		   hostname, port );
	}
	else {
	  crDebug( "Connecting to server %s on port %d, with protocol %s", 
		   hostname, port, protocol );
	}

	conn = (CRConnection *) crCalloc( sizeof(*conn) );
	if (!conn)
		return NULL;

	conn->type               = CR_NO_CONNECTION; /* we don't know yet */
	conn->id                 = 0;                /* Each connection has an id */
	conn->total_bytes_sent   = 0;                /* how many bytes have we sent? */
	conn->total_bytes_recv   = 0;                /* how many bytes have we recv? */
	conn->send_credits       = 0;
	conn->recv_credits       = CR_INITIAL_RECV_CREDITS;
	conn->hostname           = crStrdup( hostname ); 
	conn->port               = port;
	conn->Alloc              = NULL;                 /* How do we allocate buffers to send? */
	conn->Send               = NULL;                 /* How do we send things? */
	conn->Barf               = NULL;                 /* How do we barf things? */
	conn->Free               = NULL;                 /* How do we free things? */
	conn->tcp_socket         = 0;
	conn->udp_socket         = 0;
	crMemset(&conn->remoteaddr, 0, sizeof(conn->remoteaddr));
	conn->gm_node_id         = 0;
	conn->mtu                = mtu;
	conn->buffer_size        = mtu;
	conn->broker             = broker;
	conn->swap               = 0;
	conn->endianness         = crDetermineEndianness();
	conn->actual_network     = 0;
	conn->teac_id            = -1;
	conn->teac_rank          = port;
	conn->tcscomm_id         = -1;
	conn->tcscomm_rank       = port;

	conn->multi.len = 0;
	conn->multi.max = 0;
	conn->multi.buf = NULL;

	conn->messageList        = NULL;
	conn->messageTail        = NULL;

	conn->userbuf            = NULL;
	conn->userbuf_len        = 0;

	/* now, just dispatch to the appropriate protocol's initialization functions. */
	
	if ( !crStrcmp( protocol, "devnull" ) )
	{
		crDevnullInit( cr_net.recv_list, cr_net.close_list, mtu );
		crDevnullConnection( conn );
	}
	else if ( !crStrcmp( protocol, "file" ) )
	{
		crFileInit( cr_net.recv_list, cr_net.close_list, mtu );
		crFileConnection( conn );
	}
	else if ( !crStrcmp( protocol, "swapfile" ) )
	{
		crFileInit( cr_net.recv_list, cr_net.close_list, mtu );
		crFileConnection( conn );
		conn->swap = 1;
	}
#ifdef GM_SUPPORT
	else if ( !crStrcmp( protocol, "gm" ) )
	{
		cr_net.use_gm++;
		crGmInit( cr_net.recv_list, cr_net.close_list, mtu );
		crGmConnection( conn );
	}
#endif
#ifdef TEAC_SUPPORT
	else if ( !crStrcmp( protocol, "quadrics" ) )
	{
		cr_net.use_teac++;
		crTeacInit( cr_net.recv_list, cr_net.close_list, mtu );
		crTeacConnection( conn );
	}
#endif
#ifdef TCSCOMM_SUPPORT
	else if ( !crStrcmp( protocol, "quadrics-tcscomm" ) )
	{
		cr_net.use_tcscomm++;
		crTcscommInit( cr_net.recv_list, cr_net.close_list, mtu );
		crTcscommConnection( conn );
	}
#endif
	else if ( !crStrcmp( protocol, "tcpip" ) )
	{
	    crDebug("Calling crTCPIPInit()");
		crTCPIPInit( cr_net.recv_list, cr_net.close_list, mtu );
		crDebug("Calling crTCPIPConnection");
		crTCPIPConnection( conn );
		crDebug("Done calling crTCPIPConnection");
	}
	else if ( !crStrcmp( protocol, "udptcpip" ) )
	{
		crUDPTCPIPInit( cr_net.recv_list, cr_net.close_list, mtu );
		crUDPTCPIPConnection( conn );
	}
	else
	{
		crError( "Unknown Protocol: \"%s\"", protocol );
	}

	if (!crNetConnect( conn ))
	{
	    crDebug("Uh oh, freeing the connection");
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

/* Create a new client */
void crNetNewClient( CRConnection *conn, CRNetServer *ns )
{
	unsigned int len = sizeof(CRMessageNewClient*);
	CRMessageNewClient msg;
        
	CRASSERT( conn );

	if (conn->swap)
		msg.header.type = (CRMessageType) SWAP32(CR_MESSAGE_NEWCLIENT);
	else
		msg.header.type = CR_MESSAGE_NEWCLIENT;

	crNetSend( conn, NULL, &msg, len );
	crNetServerConnect( ns );
}


/* Accept a client on various interfaces. */

CRConnection *crNetAcceptClient( const char *protocol, char *hostname, unsigned short port, unsigned int mtu, int broker )
{
	CRConnection *conn;

	CRASSERT( cr_net.initialized );

	conn = (CRConnection *) crCalloc( sizeof( *conn ) );
	if (!conn)
		return NULL;

	conn->type               = CR_NO_CONNECTION; /* we don't know yet */
	conn->id                 = 0;                /* Each connection has an id */
	conn->total_bytes_sent   = 0;              /* how many bytes have we sent? */
	conn->total_bytes_recv   = 0;              /* how many bytes have we recv? */
	conn->send_credits       = 0;
	conn->recv_credits       = CR_INITIAL_RECV_CREDITS;
	/*conn->hostname           = crStrdup( hostname ); */
	conn->port               = port;
	conn->Alloc              = NULL;    /* How do we allocate buffers to send? */
	conn->Send               = NULL;    /* How do we send things? */
	conn->Barf               = NULL;    /* How do we barf things? */
	conn->Free               = NULL;    /* How do we receive things? */
	conn->tcp_socket         = 0;
	conn->udp_socket         = 0;
	crMemset(&conn->remoteaddr, 0, sizeof(conn->remoteaddr));
	conn->gm_node_id         = 0;
	conn->mtu                = mtu;
	conn->buffer_size        = mtu;
	conn->broker             = broker;
	conn->swap               = 0;
	conn->endianness         = crDetermineEndianness();
	conn->actual_network     = 0;
	conn->teac_id            = -1;
	conn->teac_rank          = -1;
	conn->tcscomm_id         = -1;
	conn->tcscomm_rank       = -1;

	conn->multi.len = 0;
	conn->multi.max = 0;
	conn->multi.buf = NULL;

	conn->messageList        = NULL;
	conn->messageTail        = NULL;
	
	conn->userbuf            = NULL;
	conn->userbuf_len        = 0;
	

	/* now, just dispatch to the appropriate protocol's initialization functions. */
	
	if ( !crStrcmp( protocol, "devnull" ) )
	{
		crDevnullInit( cr_net.recv_list, cr_net.close_list, mtu );
		crDevnullConnection( conn );
	}
	if ( !crStrncmp( protocol, "file", crStrlen( "file" ) ) ||
	     !crStrncmp( protocol, "swapfile", crStrlen( "swapfile" ) ) )
	{
		char filename[4096];
		if (!crParseURL( protocol, NULL, filename, NULL, 0 ))
		{
			crError( "Malformed URL: \"%s\"", protocol );
		}
		conn->hostname = crStrdup( filename );
		crFileInit( cr_net.recv_list, cr_net.close_list, mtu );
		crFileConnection( conn );
	}
#ifdef GM_SUPPORT
	else if ( !crStrcmp( protocol, "gm" ) )
	{
		cr_net.use_gm++;
		crGmInit( cr_net.recv_list, cr_net.close_list, mtu );
		crGmConnection( conn );
	}
#endif
#ifdef TEAC_SUPPORT
  else if ( !crStrcmp( protocol, "quadrics" ) )
    {
      cr_net.use_teac++;
      crTeacInit( cr_net.recv_list, cr_net.close_list, mtu );
      crTeacConnection( conn );
    }
#endif
#ifdef TCSCOMM_SUPPORT
  else if ( !crStrcmp( protocol, "quadrics-tcscomm" ) )
    {
      cr_net.use_tcscomm++;
      crTcscommInit( cr_net.recv_list, cr_net.close_list, mtu );
      crTcscommConnection( conn );
    }
#endif
	else if ( !crStrcmp( protocol, "tcpip" ) )
	{
		crTCPIPInit( cr_net.recv_list, cr_net.close_list, mtu );
		crTCPIPConnection( conn );
	}
	else if ( !crStrcmp( protocol, "udptcpip" ) )
	{
		crUDPTCPIPInit( cr_net.recv_list, cr_net.close_list, mtu );
		crUDPTCPIPConnection( conn );
	}
	else
	{
		crError( "Unknown Protocol: \"%s\"", protocol );
	}

	crNetAccept( conn, hostname, port );
	return conn;
}

/* Start the ball rolling.  give functions to handle incoming traffic 
 * (usually placing blocks on a queue), and a handler for dropped 
 * connections. */

void crNetInit( CRNetReceiveFunc recvFunc, CRNetCloseFunc closeFunc )
{
	CRNetReceiveFuncList *rfl;
	CRNetCloseFuncList *cfl;

	if ( cr_net.initialized )
	{
		crDebug( "Networking already initialized!" );
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

		cr_net.use_gm      = 0;
		cr_net.num_clients = 0;
#ifdef CHROMIUM_THREADSAFE
		crInitMutex(&cr_net.mutex);
#endif
		cr_net.message_list_pool = crBufferPoolInit(16);

		cr_net.initialized = 1;
		cr_net.recv_list = NULL;
		cr_net.close_list = NULL;
	}
	if (recvFunc != NULL)
	{
		for (rfl = cr_net.recv_list ; rfl ; rfl = rfl->next )
		{
			if (rfl->recv == recvFunc) 
			{
				/* we've already seen this function -- do nothing */
				break;
			}
		}
		if (!rfl)
		{
			rfl = (CRNetReceiveFuncList *) malloc( sizeof (*rfl ));
			rfl->recv = recvFunc;
			rfl->next = cr_net.recv_list;
			cr_net.recv_list = rfl;
		}
	}
	if (closeFunc != NULL)
	{
		for (cfl = cr_net.close_list ; cfl ; cfl = cfl->next )
		{
			if (cfl->close == closeFunc) 
			{
				/* we've already seen this function -- do nothing */
				break;
			}
		}
		if (!cfl)
		{
			cfl = (CRNetCloseFuncList *) malloc( sizeof (*cfl ));
			cfl->close = closeFunc;
			cfl->next = cr_net.close_list;
			cr_net.close_list = cfl;
		}
	}
}

extern CRConnection** crTCPIPDump( int * num );
extern CRConnection** crDevnullDump( int * num );
#ifdef GM_SUPPORT
extern CRConnection** crGmDump( int * num );
#endif
extern CRConnection** crFileDump( int * num );

CRConnection** crNetDump( int* num ) 
{
	CRConnection **c;

	c = crTCPIPDump( num );
	if ( c ) return c;

	c = crDevnullDump( num );
	if ( c ) return c;

	c = crFileDump( num );
	if ( c ) return c;

#ifdef GM_SUPPORT
	c = crGmDump( num );
	if ( c ) return c;
#endif

	*num = 0;
	return NULL;
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
				(unsigned char *) *bufp + conn->buffer_size );
	}

#ifndef NDEBUG
	if ( conn->send_credits > CR_INITIAL_RECV_CREDITS )
	{
		crError( "crNetSend: send_credits=%u, looks like there is a "
				"leak (max=%u)", conn->send_credits,
				CR_INITIAL_RECV_CREDITS );
	}
#endif

	conn->total_bytes_sent += len;

	msg->header.conn_id = conn->id;
	conn->Send( conn, bufp, start, len );
}

/* Barf a set of commands on a connection.  Pretty straightforward, just 
 * error checking, byte counting, and a dispatch to the protocol's 
 * "barf" implementation. */

void crNetBarf( CRConnection *conn, void **bufp, 
		            void *start, unsigned int len )
{
	CRMessage *msg = (CRMessage *) start;
	CRASSERT( conn );
	CRASSERT( len > 0 );
	CRASSERT( conn->Barf );
	if ( bufp ) {
		CRASSERT( start >= *bufp );
		CRASSERT( (unsigned char *) start + len <= 
				(unsigned char *) *bufp + conn->buffer_size );
	}

#ifndef NDEBUG
	if ( conn->send_credits > CR_INITIAL_RECV_CREDITS )
	{
		crError( "crNetBarf: send_credits=%u, looks like there is a "
				"leak (max=%u)", conn->send_credits,
				CR_INITIAL_RECV_CREDITS );
	}
#endif

	conn->total_bytes_sent += len;

	msg->header.conn_id = conn->id;
	conn->Barf( conn, bufp, start, len );
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

void crNetAccept( CRConnection *conn, char *hostname, unsigned short port )
{
	conn->Accept( conn, hostname, port );
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
	crMemcpy( dst, src, len );
	multi->len += len;

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
	
	/* Don't do this too early! */
	conn->InstantReclaim( conn, (CRMessage *) msg );
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

static void crNetRecvWriteback( CRMessageWriteback *wb )
{
	int *writeback;
	crMemcpy( &writeback, &(wb->writeback_ptr), sizeof( writeback ) );
	(*writeback)--;
}

static void crNetRecvReadback( CRMessageReadback *rb, unsigned int len )
{
	/* minus the header, the destination pointer, 
	 * *and* the implicit writeback pointer at the head. */

	int payload_len = len - sizeof( *rb );
	int *writeback;
	void *dest_ptr; 
	crMemcpy( &writeback, &(rb->writeback_ptr), sizeof( writeback ) );
	crMemcpy( &dest_ptr, &(rb->readback_ptr), sizeof( dest_ptr ) );

	(*writeback)--;
	crMemcpy( dest_ptr, ((char *)rb) + sizeof(*rb), payload_len );
}

void crNetDefaultRecv( CRConnection *conn, void *buf, unsigned int len )
{
	CRMessageList *msglist;
	
	CRMessage *msg = (CRMessage *) buf;

	switch( msg->header.type )
	{
		case CR_MESSAGE_GATHER:
			break;	
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
	        case CR_MESSAGE_CRUT:
		  {
		  }
		  break;
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
	
#ifdef CHROMIUM_THREADSAFE
	crLockMutex(&cr_net.mutex);
#endif
	msglist = (CRMessageList *) crBufferPoolPop( cr_net.message_list_pool, conn->buffer_size );
	if ( msglist == NULL )
	{
		msglist = (CRMessageList *) crAlloc( sizeof( *msglist ) );
	}
#ifdef CHROMIUM_THREADSAFE
	crUnlockMutex(&cr_net.mutex);
#endif
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

void crNetDispatchMessage( CRNetReceiveFuncList *rfl, CRConnection *conn, void *buf, unsigned int len )
{
	for ( ; rfl ; rfl = rfl->next)
	{
		if (rfl->recv( conn, buf, len ))
		{
			return;
		}
	}
	crNetDefaultRecv( conn, buf, len );
}

unsigned int crNetPeekMessage( CRConnection *conn, CRMessage **message )
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
#ifdef CHROMIUM_THREADSAFE
		crLockMutex(&cr_net.mutex);
#endif
		crBufferPoolPush( cr_net.message_list_pool, temp, conn->buffer_size );
#ifdef CHROMIUM_THREADSAFE
		crUnlockMutex(&cr_net.mutex);
#endif
		return len;
	}
	return 0;
}

unsigned int crNetGetMessage( CRConnection *conn, CRMessage **message )
{
	/* Keep getting work to do */
	for (;;)
	{
		int len = crNetPeekMessage( conn, message );
		if (len) return len;
		crNetRecv();
	}

#ifndef WINDOWS
	/* silence compiler */
	return 0;
#endif
}

/* Read a line from a socket.  Useful for reading from the mothership. */

void crNetReadline( CRConnection *conn, void *buf )
{
	char *temp, c;

	if (!conn || conn->type == CR_NO_CONNECTION) 
		return;
	
	if (conn->type != CR_TCPIP)
	{
		crError( "Can't do a crNetReadline on anything other than TCPIP (%d).",conn->type );
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

	found_work += crUDPTCPIPRecv( );

	found_work += crFileRecv( );

#ifdef GM_SUPPORT
	if ( cr_net.use_gm )
		found_work += crGmRecv( );
#endif

#ifdef TEAC_SUPPORT
  	if ( cr_net.use_teac )
    		found_work += crTeacRecv( );
#endif

#ifdef TCSCOMM_SUPPORT
  	if ( cr_net.use_tcscomm )
    		found_work += crTcscommRecv( );
#endif

	return found_work;
}

int crGetPID( void )
{
	return (int) getpid();
}

void
crNetSetRank( int my_rank )
{
	cr_net.my_rank = my_rank;
	crDebug( "crNetSetRank:  set my_rank to %d", cr_net.my_rank );
#ifdef TEAC_SUPPORT
	crTeacSetRank( cr_net.my_rank );      
#endif
#ifdef TCSCOMM_SUPPORT
	crTcscommSetRank( cr_net.my_rank );
#endif
}

void
crNetSetContextRange( int low_context, int high_context )
{
#ifdef TEAC_SUPPORT
	crTeacSetContextRange( low_context, high_context );
#endif
#ifdef TCSCOMM_SUPPORT
	crTcscommSetContextRange( low_context, high_context );
#endif
}

void
crNetSetNodeRange( const char *low_node, const char *high_node )
{
#ifdef TEAC_SUPPORT
	crTeacSetNodeRange( low_node, high_node );
#endif
#ifdef TCSCOMM_SUPPORT
	crTcscommSetNodeRange( low_node, high_node );
#endif
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
	CRConnection *conn;
	char *mother_server = NULL;

	crNetInit( NULL, NULL );

	mother_server = crGetenv( "CRMOTHERSHIP" );
	if (!mother_server)
	{
		crWarning( "Couldn't find the CRMOTHERSHIP environment variable, defaulting to localhost" );
		mother_server = "localhost";
	}

	conn = crNetConnectToServer( mother_server, MOTHERPORT, 8096, 0 );

	if (!conn)
		crError("Failed to connect to mothership\n");

	return conn;
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
