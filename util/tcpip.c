#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#pragma warning( push, 3 )
#include <winsock2.h>
#pragma warning( pop )
#pragma warning( disable : 4514 )
#pragma warning( disable : 4127 )
typedef int ssize_t;
#define write(a,b,c) send(a,b,c,0)
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "cr_error.h"
#include "cr_mem.h"
#include "cr_string.h"
#include "cr_bufpool.h"
#include "cr_net.h"

#ifdef WINDOWS
#define EADDRINUSE   WSAEADDRINUSE
#define ECONNREFUSED WSAECONNREFUSED
#endif

typedef enum {
	CRTCPIPMemory,
	CRTCPIPMemoryBig
} CRTCPIPBufferKind;

#define CR_TCPIP_BUFFER_MAGIC 0x89134532

typedef struct CRTCPIPBuffer {
	unsigned int          magic;
	CRTCPIPBufferKind     kind;
	unsigned int          len;
	unsigned int          pad;
} CRTCPIPBuffer;

#ifdef WINDOWS

#undef  ECONNRESET
#define ECONNRESET  WSAECONNRESET
#undef  EINTR
#define EINTR       WSAEINTR

int crTCPIPErrno( void )
{
	return WSAGetLastError( );
}

char *crTCPIPErrorString( int err )
{
	static char buf[512], *temp;

	sprintf( buf, "err=%d", err );

#define X(x)	crStrcpy(buf,x); break

	switch ( err )
	{
		case WSAECONNREFUSED: X( "connection refused" );
		case WSAECONNRESET:   X( "connection reset" );
		default:
													FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
															FORMAT_MESSAGE_FROM_SYSTEM |
															FORMAT_MESSAGE_MAX_WIDTH_MASK, NULL, err,
															MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
															(LPTSTR) &temp, 0, NULL );
													if ( temp )
													{
														crStrncpy( buf, temp, sizeof(buf)-1 );
													}
	}

#undef X

	temp = buf + crStrlen(buf) - 1;
	while ( temp > buf && isspace( *temp ) )
	{
		*temp = '\0';
		temp--;
	}

	return buf;
}

#else

int crTCPIPErrno( void )
{
	int err = errno;
	errno = 0;
	return err;
}

char *crTCPIPErrorString( int err )
{
	static char buf[512], *temp;
	
	temp = strerror( err );
	if ( temp )
	{
		crStrncpy( buf, temp, sizeof(buf)-1 );
	}
	else
	{
		sprintf( buf, "err=%d", err );
	}

	return buf;
}

#endif
void crCloseSocket( CRSocket sock )
{
	int fail;
#ifdef WINDOWS
    fail = ( closesocket( sock ) != 0 );
#else
	shutdown( sock, 2 /* RDWR */ );
	fail = ( close( sock ) != 0 );
#endif
	if ( fail )
	{
		int err = crTCPIPErrno( );
		crWarning( "crCloseSocket( sock=%d ): %s",
					   sock, crTCPIPErrorString( err ) );
	}
}

static struct {
	int                  initialized;
	int                  num_conns;
	CRConnection         **conns;
	CRBufferPool         bufpool;
	CRNetReceiveFunc     recv;
	CRNetCloseFunc       close;
	struct {
		CRSocket           sock;
		CRVoidFunc         connect;
	} server;
} cr_tcpip;

static int __read_exact( CRSocket sock, void *buf, unsigned int len )
{
	char *dst = (char *) buf;

	while ( len > 0 )
	{
		int num_read = recv( sock, dst, (int) len, 0 );

#ifdef WINDOWS_XXXX
		/* MWE: why is this necessary for windows???  Does it return a
			 "good" value for num_bytes despite having a reset
			 connection? */
		if ( crTCPIPErrno( ) == ECONNRESET )
			return -1;
#endif

		if ( num_read < 0 )
		{
			if ( crTCPIPErrno( ) == EINTR )
			{
				crWarning( "__read_exact(TCPIP): "
						"caught an EINTR, looping for more data" );
				continue;
			}
			return -1;
		}

		if ( num_read == 0 ) 
		{
			/* client exited gracefully */
			return 0;
		}

		dst += num_read;
		len -= num_read;
	}

	return 1;
}

void crTCPIPReadExact( CRSocket sock, void *buf, unsigned int len )
{
	int retval = __read_exact( sock, buf, len );
	if ( retval <= 0 )
	{
		int err = crTCPIPErrno( );
		crError( "crTCPIPReadExact: %s", crTCPIPErrorString( err ) );
	}
}

static int __write_exact( CRSocket sock, void *buf, unsigned int len )
{
	char *src = (char *) buf;

	while ( len > 0 )
	{
		int num_written = write( sock, src, len );
		if ( num_written <= 0 )
		{
			if ( crTCPIPErrno( ) == EINTR )
			{
				crWarning( "__write_exact(TCPIP): "
						"caught an EINTR, continuing" );
				continue;
			}
			return -1;
		}

		len -= num_written;
		src += num_written;
	}

	return 1;
}

void crTCPIPWriteExact( CRConnection *conn, void *buf, unsigned int len )
{
	int retval = __write_exact( conn->tcp_socket, buf, len );
	if ( retval <= 0 )
	{
		int err = crTCPIPErrno( );
		crError( "crTCPIPWriteExact: %s", 
						   crTCPIPErrorString( err ) );
	}
}

void *crTCPIPAlloc( void )
{
	CRTCPIPBuffer *buf = (CRTCPIPBuffer *)
		crBufferPoolPop( &cr_tcpip.bufpool );
	if ( buf == NULL )
	{
		buf = (CRTCPIPBuffer *) 
			crAlloc( sizeof(CRTCPIPBuffer) + crNetMTU() );
		buf->magic = CR_TCPIP_BUFFER_MAGIC;
		buf->kind  = CRTCPIPMemory;
		buf->pad   = 0;
	}
	return (void *)( buf + 1 );
}

void crTCPIPSingleRecv( CRConnection *conn, void *buf, unsigned int len )
{
	crTCPIPReadExact( conn->tcp_socket, buf, len );
}

void crTCPIPSend( CRConnection *conn, void **bufp,
				 void *start, unsigned int len )
{
	CRTCPIPBuffer *tcpip_buffer;
	unsigned int      *lenp;

	if ( bufp == NULL )
	{
		/* we are doing synchronous sends from user memory, so no need
		 * to get fancy.  Simply write the length & the payload and
		 * return. */
		crTCPIPWriteExact( conn, &len, sizeof(len) );
		crTCPIPWriteExact( conn, start, len );
		return;
	}

	tcpip_buffer = (CRTCPIPBuffer *)(*bufp) - 1;

	CRASSERT( tcpip_buffer->magic == CR_TCPIP_BUFFER_MAGIC );

	/* All of the buffers passed to the send function were allocated
	 * with crTCPIPAlloc(), which includes a header with a 4 byte
	 * length field, to insure that we always have a place to write
	 * the length field, even when start == *bufp. */
	lenp = (unsigned int *) start - 1;
	*lenp = len;

	if ( __write_exact( conn->tcp_socket, lenp, len + sizeof(int) ) < 0 )
	{
		int err = crTCPIPErrno( );
		crError( "crTCPIPSend: %s", crTCPIPErrorString( err ) );
	}

	/* reclaim this pointer for reuse and try to keep the client from
		 accidentally reusing it directly */
	crBufferPoolPush( &cr_tcpip.bufpool, tcpip_buffer );
	*bufp = NULL;
}

static void __dead_connection( CRConnection *conn )
{
	crWarning( "Dead connection (sock=%d, host=%s)",
				   conn->tcp_socket, conn->hostname );
	if ( cr_tcpip.server.connect )
	{
		crWarning( "Closing server socket" );
		crCloseSocket( cr_tcpip.server.sock );
	}
	exit( 0 );
}

static int __select( int n, fd_set *readfds, struct timeval *timeout )
{
	for ( ; ; ) 
	{
		int err, num_ready;

		num_ready = select( n, readfds, NULL, NULL, timeout );

		if ( num_ready >= 0 )
		{
			return num_ready;
		}

		err = crTCPIPErrno( );
		if ( err == EINTR )
		{
			crWarning( "select interruped by an unblocked signal, trying again" );
		}
		else
		{
			crError( "select failed: %s", crTCPIPErrorString( err ) );
		}
	}
}

int crTCPIPRecv( void )
{
	int    num_ready, max_fd;
	fd_set read_fds;
	int i;

	max_fd = 0;
	FD_ZERO( &read_fds );
	if ( cr_tcpip.server.connect )
	{
		FD_SET( cr_tcpip.server.sock, &read_fds );
		max_fd = (int) cr_tcpip.server.sock + 1;
	}
	for ( i = 0; i < cr_tcpip.num_conns; i++ )
	{
		CRConnection *conn = cr_tcpip.conns[i];
		if ( conn->recv_credits > 0 || conn->type != CR_TCPIP )
		{
			/* NOTE: may want to always put the FD in the descriptor
               set so we'll notice broken connections.  Down in the
               loop that iterates over the ready sockets only peek
               (MSG_PEEK flag to recv()?) if the connection isn't
               enabled. */
			CRSocket sock = conn->tcp_socket;
			if ( (int) sock + 1 > max_fd )
				max_fd = (int) sock + 1;
			FD_SET( sock, &read_fds );
		}
	}

	if ( cr_tcpip.num_conns )
	{
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		num_ready = __select( max_fd, &read_fds, &timeout );
	}
	else
	{
		crWarning( "Waiting for first connection..." );
		num_ready = __select( max_fd, &read_fds, NULL );
	}

	if ( num_ready == 0 )
		return 0;

	if ( cr_tcpip.server.connect && 
		 FD_ISSET( cr_tcpip.server.sock, &read_fds ) )
	{
		cr_tcpip.server.connect( );
	}

	for ( i = 0; i < cr_tcpip.num_conns; i++ )
	{
		CRTCPIPBuffer *tcpip_buffer;
		unsigned int   len;
		int            read_ret;
		CRConnection  *conn = cr_tcpip.conns[i];
		CRSocket       sock = conn->tcp_socket;

		if ( !FD_ISSET( sock, &read_fds ) )
			continue;

		read_ret = __read_exact( sock, &len, sizeof(len) );
		if ( read_ret <= 0 )
		{
			__dead_connection( conn );
			i--;
			continue;
		}

		CRASSERT( len > 0 );

		if ( len <= crNetMTU() )
		{
			tcpip_buffer = (CRTCPIPBuffer *) crTCPIPAlloc( ) - 1;
		}
		else
		{
			tcpip_buffer = (CRTCPIPBuffer *) 
				crAlloc( sizeof(*tcpip_buffer) + len );
			tcpip_buffer->magic = CR_TCPIP_BUFFER_MAGIC;
			tcpip_buffer->kind  = CRTCPIPMemoryBig;
			tcpip_buffer->pad   = 0;
		}

		tcpip_buffer->len = len;

		read_ret = __read_exact( sock, tcpip_buffer + 1, len );
		if ( read_ret <= 0 )
		{
			crFree( tcpip_buffer );
			__dead_connection( conn );
			i--;
			continue;
		}

#if 0
		crLogRead( len );
#endif

		conn->recv_credits -= len;
		cr_tcpip.recv( conn, tcpip_buffer + 1, len );
	}

	return 1;
}

void crTCPIPFree( CRConnection *conn, void *buf )
{
	CRTCPIPBuffer *tcpip_buffer = (CRTCPIPBuffer *) buf - 1;

	CRASSERT( tcpip_buffer->magic == CR_TCPIP_BUFFER_MAGIC );
	conn->recv_credits += tcpip_buffer->len;

	switch ( tcpip_buffer->kind )
	{
		case CRTCPIPMemory:
			crBufferPoolPush( &cr_tcpip.bufpool, tcpip_buffer );
			break;

		case CRTCPIPMemoryBig:
			crFree( tcpip_buffer );
			break;

		default:
			crError( "Weird buffer kind trying to free in crTCPIPFree: %d", tcpip_buffer->kind );
	}
}

void crTCPIPInit( CRNetReceiveFunc recvFunc, CRNetCloseFunc closeFunc )
{
	if ( cr_tcpip.initialized )
	{
		CRASSERT( cr_tcpip.recv == recvFunc );
		CRASSERT( cr_tcpip.close == closeFunc );
		return;
	}

	cr_tcpip.num_conns = 0;
	cr_tcpip.conns     = NULL;
	
	cr_tcpip.server.sock    = 0;
	cr_tcpip.server.connect = NULL;

	crBufferPoolInit( &cr_tcpip.bufpool, 16 );

	cr_tcpip.recv = recvFunc;
	cr_tcpip.close = closeFunc;

	cr_tcpip.initialized = 1;
}

//
// Make sockets do what we want:
//
// 1) Change the size of the send/receive buffers to 64K
// 2) Turn off Nagle's algorithm

static void __crSpankSocket( CRSocket sock )
{
	int sndbuf = 64*1024;
	int rcvbuf = sndbuf;
	int tcp_nodelay = 1;

	if ( setsockopt( sock, SOL_SOCKET, SO_SNDBUF, 
				(char *) &sndbuf, sizeof(sndbuf) ) )
	{
		int err = crTCPIPErrno( );
		crWarning( "setsockopt( SO_SNDBUF=%d ) : %s",
				sndbuf, crTCPIPErrorString( err ) );
	}

	if ( setsockopt( sock, SOL_SOCKET, SO_RCVBUF,
				(char *) &rcvbuf, sizeof(rcvbuf) ) )
	{
		int err = crTCPIPErrno( );
		crWarning( "setsockopt( SO_RCVBUF=%d ) : %s",
				rcvbuf, crTCPIPErrorString( err ) );
	}

	if ( setsockopt( sock, IPPROTO_TCP, TCP_NODELAY,
				(char *) &tcp_nodelay, sizeof(tcp_nodelay) ) )
	{
		int err = crTCPIPErrno( );
		crWarning( "setsockopt( TCP_NODELAY=%d )"
				" : %s", tcp_nodelay, crTCPIPErrorString( err ) );
	}
}

// The function that actually connects.  This should only be called by clients
// Servers have another way to set up the socket.

void crTCPIPDoConnect( CRConnection *conn )
{
	struct sockaddr_in servaddr;
	struct hostent *hp;
	int err;

	conn->tcp_socket = socket( AF_INET, SOCK_STREAM, 0 );
	if ( conn->tcp_socket < 0 )
	{
		int err = crTCPIPErrno( );
		crError( "socket error: %s", 
				crTCPIPErrorString( err ) );
	}

	// Set up the socket the way *we* want.
	__crSpankSocket( conn->tcp_socket );

	// Standard Berkeley sockets mumbo jumbo
	hp = gethostbyname( conn->hostname );
	if ( !hp )
	{
		crError( "Unknown host: \"%s\"", conn->hostname );
	}

	memset( &servaddr, 0, sizeof(servaddr) );
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons( (short) conn->port );

	memcpy( (char *) &servaddr.sin_addr, hp->h_addr,
			sizeof(servaddr.sin_addr) );
	while (1)
	{
		if ( !connect( conn->tcp_socket, (struct sockaddr *) &servaddr,
					sizeof(servaddr) ) )
			break;

		err = crTCPIPErrno( );
		if ( err == EADDRINUSE || err == ECONNREFUSED )
		{
			// Here's where we might try again on another
			// port -- don't think we'll do that any more.
			crError( "Couldn't connect to %s:%d, %s",
					conn->hostname, conn->port, crTCPIPErrorString( err ) );
		}
		else if ( err == EINTR )
		{
			crWarning( "connection to %s:%d "
					"interruped, trying again", conn->hostname, conn->port );
		}
		else
		{
			crError( "Couldn't connect to %s:%d, %s",
					conn->hostname, conn->port, crTCPIPErrorString( err ) );
		}
	}
}

void crTCPIPDoDisconnect( CRConnection *conn )
{
	crCloseSocket( conn->tcp_socket );
	conn->tcp_socket = 0;
	conn->type = CR_NO_CONNECTION;
}

void crTCPIPConnection( CRConnection *conn )
{
	int n_bytes;

	CRASSERT( cr_tcpip.initialized );

	conn->type  = CR_TCPIP;
	conn->Alloc = crTCPIPAlloc;
	conn->Send  = crTCPIPSend;
	conn->SendExact  = crTCPIPWriteExact;
	conn->Recv  = crTCPIPSingleRecv;
	conn->Free  = crTCPIPFree;
	conn->Connect = crTCPIPDoConnect;
	conn->Disconnect = crTCPIPDoDisconnect;

	n_bytes = ( cr_tcpip.num_conns + 1 ) * sizeof(*cr_tcpip.conns);
	crRealloc( (void **) &cr_tcpip.conns, n_bytes );

	cr_tcpip.conns[cr_tcpip.num_conns++] = conn;
}
void crTCPIPBecomeServer( CRSocket sock, CRVoidFunc connectFunc )
{
	CRASSERT( cr_tcpip.server.connect == NULL );
	CRASSERT( connectFunc != NULL );

	cr_tcpip.server.sock    = sock;
	cr_tcpip.server.connect = connectFunc;
}

int crGetHostname( char *buf, unsigned int len )
{
	int ret = gethostname( buf, len );
	if (ret)
	{
		int err = crTCPIPErrno();
		//crWarning( "Couldn't get hostname: %s", crTCPIPErrorString( err ) );
	}
	return ret;
}


#if 0
#define CR_BIND_TRIES 8
	for ( i = 0; i < WIREGL_BIND_TRIES; i++ )
	{
	}

	if ( i == WIREGL_BIND_TRIES )
	{
		crSimpleError( "Couldn't find server %s", hostname );
	}

	if ( i > 0 )
	{
		crWarning( WIREGL_WARN_VERBOSE_DEBUG, "Connected on alternate "
				"port %u", port );
	}

	if ( request->max_send < 1024 )
	{
		crSimpleError( "crConnectToServer: max_send too small (%d)\n",
				request->max_send );
	}


	/* calculate the request size */
	request_size = sizeof(*request);
	for (p = request; p->peer[0]; p++)
		request_size += sizeof(*request);

	crTCPIPWriteExact( conn->tcp_socket, request, request_size );

	crTCPIPReadExact( conn->tcp_socket, &response, sizeof(response) );

	if ( response.magic != WIREGL_CONNECTION_MAGIC )
	{
		crSimpleError( "crFinishConnectToServer: connection "
				"magic=0x%x, expected 0x%x",
				response.magic, WIREGL_CONNECTION_MAGIC );
	}

	if ( response.size != sizeof(response) )
	{
		crSimpleError( "crFinishConnectToServer: server claims "
				"response is %u bytes, I expect %u bytes\n",
				response.size, sizeof(response) );
	}

	if ( response.max_send < __wiregl_max_send )
	{
		crSimpleError( "crFinishConnectToServer: server has a "
				"max_send=%d, but client is using %d",
				response.max_send, __wiregl_max_send );
	}

	conn->sender_id = response.client_id;

	switch ( conn->type )
	{
#ifdef GM_SUPPORT
		case WIREGL_GM:
			/* set this up as a TCPIP connection first, so that we can
			 * detect a client disconnect via socket death */
			crTCPIPConnection( conn );

			/* and now set it up correctly for GM */
			conn->gm_node_id  = response.gm_node_id;
			conn->gm_port_num = response.gm_port_num;
			wiregl_net.use_gm++;
			crGmConnection( conn );
			break;
#endif
		case WIREGL_TCPIP:
			crTCPIPConnection( conn );
			break;

		default:
			crError( "crFinishConnectToServer: conn->type=%u?",
					conn->type );
	}
#endif
