/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#pragma warning( push, 3 )
#include <winsock2.h>
#pragma warning( pop )
#pragma warning( disable : 4514 )
#pragma warning( disable : 4127 )
typedef int ssize_t;
#else
#include <sys/types.h>
#include <sys/wait.h>
#ifdef DARWIN
#elif defined(OSF1)
typedef int socklen_t;
#endif
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#ifndef DARWIN
#include <netinet/tcp.h>
#endif
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#ifdef AIX
#include <strings.h>
#endif

#ifdef LINUX
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include "cr_error.h"
#include "cr_mem.h"
#include "cr_string.h"
#include "cr_bufpool.h"
#include "cr_net.h"
#include "cr_endian.h"
#include "cr_threads.h"
#include "cr_environment.h"
#include "net_internals.h"

#ifdef ADDRINFO
#define PF PF_UNSPEC
#endif

#ifdef WINDOWS
#define EADDRINUSE   WSAEADDRINUSE
#define ECONNREFUSED WSAECONNREFUSED
#endif

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
				buf[sizeof(buf)-1] = 0;
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

#else /* WINDOWS */

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
		buf[sizeof(buf)-1] = 0;
	}
	else
	{
		sprintf( buf, "err=%d", err );
	}

	return buf;
}

#endif /* WINDOWS */


void crCloseSocket( CRSocket sock )
{
	int fail;

	if (sock <= 0)
		return;

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

static unsigned short last_port = 0;

cr_tcpip_data cr_tcpip;

int
__tcpip_read_exact( CRSocket sock, void *buf, unsigned int len )
{
	char *dst = (char *) buf;
	/* 
	 * Shouldn't write to a non-existent socket, ie when 
	 * crTCPIPDoDisconnect has removed it from the pool
	 */
	if ( sock <= 0 )
		return 1;

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
			int error = crTCPIPErrno();
			switch( error )
			{
				case EINTR:
					crWarning( "__tcpip_read_exact(TCPIP): "
							"caught an EINTR, looping for more data" );
					continue;
				case EFAULT: crWarning( "EFAULT" ); break;
				case EINVAL: crWarning( "EINVAL" ); break;
				default: break;
			}
			crWarning( "Bad bad bad socket error: %s", crTCPIPErrorString( error ) );
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

void
crTCPIPReadExact( CRConnection *conn, void *buf, unsigned int len )
{
	if ( __tcpip_read_exact( conn->tcp_socket, buf, len ) <= 0 )
	{
		__tcpip_dead_connection( conn );
	}
}

int
__tcpip_write_exact( CRSocket sock, const void *buf, unsigned int len )
{
	int err;
	char *src = (char *) buf;

	/* 
	 * Shouldn't write to a non-existent socket, ie when 
	 * crTCPIPDoDisconnect has removed it from the pool
	 */
	if ( sock <= 0 )
		return 1;

	while ( len > 0 )
	{
		int num_written = send( sock, src, len, 0 );
		if ( num_written <= 0 )
		{
		  if ( (err = crTCPIPErrno( )) == EINTR )
		  {
				crWarning( "__tcpip_write_exact(TCPIP): "
									 "caught an EINTR, continuing" );
				continue;
		  }
		  
		  return -err;
		}
	     
		len -= num_written;
		src += num_written;
	}
	     
	return 1;
}

void
crTCPIPWriteExact( CRConnection *conn, const void *buf, unsigned int len )
{
	if ( __tcpip_write_exact( conn->tcp_socket, buf, len) <= 0 )
	{
		__tcpip_dead_connection( conn );
	}
}


/* 
 * Make sockets do what we want: 
 * 
 * 1) Change the size of the send/receive buffers to 64K 
 * 2) Turn off Nagle's algorithm
 */
static void
spankSocket( CRSocket sock )
{
	/* why do we do 1) ? things work much better for me to push the
	 * the buffer size way up -- karl
	 */
#ifdef LINUX
	int sndbuf = 1*1024*1024;
#else
	int sndbuf = 64*1024;
#endif	

	int rcvbuf = sndbuf;
	int so_reuseaddr = 1;
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
	
	
	if ( setsockopt( sock, SOL_SOCKET, SO_REUSEADDR,
			 (char *) &so_reuseaddr, sizeof(so_reuseaddr) ) )
	{
		int err = crTCPIPErrno( );
		crWarning( "setsockopt( SO_REUSEADDR=%d ) : %s",
			   so_reuseaddr, crTCPIPErrorString( err ) );
	}
	
	if ( setsockopt( sock, IPPROTO_TCP, TCP_NODELAY,
			 (char *) &tcp_nodelay, sizeof(tcp_nodelay) ) )
	{
		int err = crTCPIPErrno( );
		crWarning( "setsockopt( TCP_NODELAY=%d )"
			   " : %s", tcp_nodelay, crTCPIPErrorString( err ) );
	}
}


#if defined( WINDOWS ) || defined( IRIX ) || defined( IRIX64 )
typedef int socklen_t;
#endif

void
crTCPIPAccept( CRConnection *conn, char *hostname, unsigned short port )
{
	int err;
	socklen_t		addr_length;
#ifndef ADDRINFO
	struct sockaddr_in	servaddr;
	struct sockaddr		addr;
	struct hostent		*host;
	struct in_addr		sin_addr;
#else
	struct sockaddr_storage	addr;
	char			host[NI_MAXHOST];
#endif

	if (port != last_port)
	{
		/* with the new OOB stuff, we can have multiple ports being 
		 * accepted on, so we need to redo the server socket every time.
		 */
#ifndef ADDRINFO
		cr_tcpip.server_sock = socket( AF_INET, SOCK_STREAM, 0 );
		if ( cr_tcpip.server_sock == -1 )
		{
			err = crTCPIPErrno( );
			crError( "Couldn't create socket: %s", crTCPIPErrorString( err ) );
		}
		spankSocket( cr_tcpip.server_sock );

		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = INADDR_ANY;
		servaddr.sin_port = htons( port );

		if ( bind( cr_tcpip.server_sock, (struct sockaddr *) &servaddr, sizeof(servaddr) ) )
		{
			err = crTCPIPErrno( );
			crError( "Couldn't bind to socket (port=%d): %s", port, crTCPIPErrorString( err ) );
		}
		last_port = port;

		if ( listen( cr_tcpip.server_sock, 100 /* max pending connections */ ) )
		{
			err = crTCPIPErrno( );
			crError( "Couldn't listen on socket: %s", crTCPIPErrorString( err ) );
		}
#else
		char port_s[NI_MAXSERV];
		struct addrinfo *res,*cur;
		struct addrinfo hints;

		sprintf(port_s, "%u", (short unsigned) port);

		crMemset(&hints, 0, sizeof(hints));
		hints.ai_flags = AI_PASSIVE;
		hints.ai_family = PF;
		hints.ai_socktype = SOCK_STREAM;

		err = getaddrinfo( NULL, port_s, &hints, &res );
		if ( err )
			crError( "Couldn't find local TCP port %s: %s", port_s, gai_strerror(err) );

		for (cur=res;cur;cur=cur->ai_next)
		{
			cr_tcpip.server_sock = socket( cur->ai_family, cur->ai_socktype, cur->ai_protocol );
			if ( cr_tcpip.server_sock == -1 )
			{
				err = crTCPIPErrno( );
				if (err != EAFNOSUPPORT)
					crWarning( "Couldn't create socket of family %i: %s, trying another", 
						   cur->ai_family, crTCPIPErrorString( err ) );
				continue;
			}
			spankSocket( cr_tcpip.server_sock );

			if ( bind( cr_tcpip.server_sock, cur->ai_addr, cur->ai_addrlen ) )
			{
				err = crTCPIPErrno( );
				crWarning( "Couldn't bind to socket (port=%d): %s", 
					   port, crTCPIPErrorString( err ) );
				crCloseSocket( cr_tcpip.server_sock );
				continue;
			}
			last_port = port;

			if ( listen( cr_tcpip.server_sock, 100 /* max pending connections */ ) )
			{
				err = crTCPIPErrno( );
				crWarning( "Couldn't listen on socket: %s", crTCPIPErrorString( err ) );
				crCloseSocket( cr_tcpip.server_sock );
				continue;
			}
			break;
		}
		freeaddrinfo(res);
		if (!cur)
			crError( "Couldn't find local TCP port %s", port_s);
#endif
	}
	
	if (conn->broker)
	{
		CRConnection *mother;
		char response[8096];
		char my_hostname[256];
		char *temp;
		mother = __copy_of_crMothershipConnect( );
		
		if (!hostname)
		{
			if ( crGetHostname( my_hostname, sizeof( my_hostname ) ) )
			{
				crError( "Couldn't determine my own hostname in crTCPIPAccept!" );
			}
		}
		else
			crStrcpy(my_hostname, hostname);
		
#if 0
		/* XXX why is this done??? (Brian asks) */
		temp = crStrchr( my_hostname, '.' );
		if (temp) *temp='\0';
#else
		(void) temp;
#endif
		if (!__copy_of_crMothershipSendString( mother, response, "acceptrequest tcpip %s %d %d", my_hostname, conn->port, conn->endianness ) )
		{
			crError( "Mothership didn't like my accept request request" );
		}
		
		__copy_of_crMothershipDisconnect( mother );
		
		sscanf( response, "%u", &(conn->id) );
	}
	
	addr_length =	sizeof( addr );
	conn->tcp_socket = accept( cr_tcpip.server_sock, (struct sockaddr *) &addr, &addr_length );
	if (conn->tcp_socket == -1)
	{
		err = crTCPIPErrno( );
		crError( "Couldn't accept client: %s", crTCPIPErrorString( err ) );
	}
	
#ifndef ADDRINFO
	sin_addr = ((struct sockaddr_in *) &addr)->sin_addr;
	host = gethostbyaddr( (char *) &sin_addr, sizeof( sin_addr), AF_INET );
	if (host == NULL )
	{
		char *temp = inet_ntoa( sin_addr );
		conn->hostname = crStrdup( temp );
	}
#else
	err = getnameinfo ( (struct sockaddr *) &addr, addr_length,
			    host, sizeof( host),
			    NULL, 0, NI_NAMEREQD);
	if ( err )
	{
		err = getnameinfo ( (struct sockaddr *) &addr, addr_length,
				    host, sizeof( host),
				    NULL, 0, NI_NUMERICHOST);
		if ( err )	/* shouldn't ever happen */
			conn->hostname = "unknown ?!";
		else
			conn->hostname = crStrdup( host );
	}
#endif
	else
	{
		char *temp;
#ifndef ADDRINFO
		conn->hostname = crStrdup( host->h_name );
#else
		conn->hostname = crStrdup( host );
#endif

		temp = conn->hostname;
		while (*temp && *temp != '.' )
			temp++;
		*temp = '\0';
	}

#ifdef RECV_BAIL_OUT 
	err = sizeof(unsigned int);
	if ( getsockopt( conn->tcp_socket, SOL_SOCKET, SO_RCVBUF,
			(char *) &conn->krecv_buf_size, &err ) )
	{
		conn->krecv_buf_size = 0;	
	}
#endif
	crDebug( "Accepted connection from \"%s\".", conn->hostname );
}

void *
crTCPIPAlloc( CRConnection *conn )
{
	CRTCPIPBuffer *buf;

#ifdef CHROMIUM_THREADSAFE
	crLockMutex(&cr_tcpip.mutex);
#endif

	buf = (CRTCPIPBuffer *) crBufferPoolPop( cr_tcpip.bufpool, conn->buffer_size );

	if ( buf == NULL )
	{
		crDebug( "Buffer pool %p was empty, so I allocated %d bytes.\n\tI did so from the buffer: %p", 
			 cr_tcpip.bufpool,
			 (unsigned int)sizeof(CRTCPIPBuffer) + conn->buffer_size, &cr_tcpip.bufpool );
		crDebug("sizeof(CRTCPIPBuffer): %d", (unsigned int)sizeof(CRTCPIPBuffer));
		crDebug("sizeof(conn->buffer_size): %d", conn->buffer_size);
		buf = (CRTCPIPBuffer *) 
			crAlloc( sizeof(CRTCPIPBuffer) + conn->buffer_size );
		buf->magic = CR_TCPIP_BUFFER_MAGIC;
		buf->kind  = CRTCPIPMemory;
		buf->pad   = 0;
		buf->allocated = conn->buffer_size;
	}
	
#ifdef CHROMIUM_THREADSAFE
	crUnlockMutex(&cr_tcpip.mutex);
#endif

	return (void *)( buf + 1 );
}


static void
crTCPIPSingleRecv( CRConnection *conn, void *buf, unsigned int len )
{
	crTCPIPReadExact( conn, buf, len );
}


static void
crTCPIPSend( CRConnection *conn, void **bufp,
						 const void *start, unsigned int len )
{
	CRTCPIPBuffer *tcpip_buffer;
	unsigned int      *lenp;

	if ( !conn || conn->type == CR_NO_CONNECTION )
		return;

	if ( bufp == NULL )
	{
		/* we are doing synchronous sends from user memory, so no need
		 * to get fancy.  Simply write the length & the payload and
		 * return. */
		const int sendable_len = conn->swap ? SWAP32(len) : len;
#if 0
		int sendable_len=len;/* can't swap len then use it for length*/
		if (conn->swap)
		{
			sendable_len = SWAP32(len);
		}
#endif
		crTCPIPWriteExact( conn, &sendable_len, sizeof(len) );
		if ( !conn || conn->type == CR_NO_CONNECTION) return;
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
	if (conn->swap)
	{
		*lenp = SWAP32(len);
	}
	else
	{
		*lenp = len;
	}

	if ( __tcpip_write_exact( conn->tcp_socket, lenp, len + sizeof(int) ) < 0 )
	{
		__tcpip_dead_connection( conn );
	}

	/* reclaim this pointer for reuse and try to keep the client from
		 accidentally reusing it directly */
#ifdef CHROMIUM_THREADSAFE
	crLockMutex(&cr_tcpip.mutex);
#endif
	crBufferPoolPush( cr_tcpip.bufpool, tcpip_buffer, tcpip_buffer->allocated );
	/* Since the buffer's now in the 'free' buffer pool, the caller can't
	 * use it any more.  Setting bufp to NULL will make sure the caller
	 * doesn't try to re-use the buffer.
	 */
	*bufp = NULL;
#ifdef CHROMIUM_THREADSAFE
	crUnlockMutex(&cr_tcpip.mutex);
#endif
}


void
__tcpip_dead_connection( CRConnection *conn )
{
	crDebug( "Dead connection (sock=%d, host=%s), removing from pool",
  				   conn->tcp_socket, conn->hostname );
  
	/* remove from connection pool */
	crTCPIPDoDisconnect( conn );
}


int
__crSelect( int n, fd_set *readfds, int sec, int usec )
{
	for ( ; ; ) 
	{ 
		int err, num_ready;

		if (sec || usec)
		{
			/* We re-init everytime for Linux, as it corrupts
			 * the timeout structure, but other OS's
			 * don't have a problem with it.
			 */
			struct timeval timeout;
			timeout.tv_sec = sec;
			timeout.tv_usec = usec;
			num_ready = select( n, readfds, NULL, NULL, &timeout );
		} 
		else
			num_ready = select( n, readfds, NULL, NULL, NULL );

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


void crTCPIPFree( CRConnection *conn, void *buf )
{
	CRTCPIPBuffer *tcpip_buffer = (CRTCPIPBuffer *) buf - 1;

	CRASSERT( tcpip_buffer->magic == CR_TCPIP_BUFFER_MAGIC );
	conn->recv_credits += tcpip_buffer->len;

	switch ( tcpip_buffer->kind )
	{
		case CRTCPIPMemory:
#ifdef CHROMIUM_THREADSAFE
			crLockMutex(&cr_tcpip.mutex);
#endif
			if (cr_tcpip.bufpool) {
				/* pool may have been deallocated just a bit earlier in response
				 * to a SIGPIPE (Broken Pipe) signal.
				 */
				crBufferPoolPush( cr_tcpip.bufpool, tcpip_buffer, tcpip_buffer->allocated );
			}
#ifdef CHROMIUM_THREADSAFE
			crUnlockMutex(&cr_tcpip.mutex);
#endif
			break;

		case CRTCPIPMemoryBig:
			crFree( tcpip_buffer );
			break;

		default:
			crError( "Weird buffer kind trying to free in crTCPIPFree: %d", tcpip_buffer->kind );
	}
}


/* returns the amt of pending data which was handled */ 
static int
crTCPIPUserbufRecv(CRConnection *conn, CRMessage *msg)
{
	unsigned int buf[2];
	int len;

	switch (msg->header.type)
	{
		case CR_MESSAGE_GATHER:
			/* grab the offset and the length */
			len = 2*sizeof(unsigned long);
			if (__tcpip_read_exact(conn->tcp_socket, buf, len) <= 0)
			{
				__tcpip_dead_connection( conn );
			}
			msg->gather.offset = buf[0];
			msg->gather.len = buf[1];

			/* read the rest into the userbuf */
			if (buf[0]+buf[1] > (unsigned int)conn->userbuf_len)
			{
				crDebug("userbuf for Gather Message is too small!");
				return len;
			}

			if (__tcpip_read_exact(conn->tcp_socket, conn->userbuf+buf[0], buf[1]) <= 0)
			{
				__tcpip_dead_connection( conn );
			}
			return len+buf[1];

		default:
			return 0;
	}
}


int
crTCPIPRecv( void )
{
	CRMessage *msg;
	CRMessageType cached_type;
	int    num_ready, max_fd;
	fd_set read_fds;
	int i;
	int msock = -1; /* assumed mothership socket */
	/* ensure we don't get caught with a new thread connecting */
	int num_conns = cr_tcpip.num_conns;
#if CRAPPFAKER_SHOULD_DIE
	int none_left = 1;
#endif

#ifdef CHROMIUM_THREADSAFE
	crLockMutex(&cr_tcpip.recvmutex);
#endif

	max_fd = 0;
	FD_ZERO( &read_fds );
	for ( i = 0; i < num_conns; i++ )
	{
		CRConnection *conn = cr_tcpip.conns[i];
		if ( !conn || conn->type == CR_NO_CONNECTION ) continue;

#if CRAPPFAKER_SHOULD_DIE
		none_left = 0;
#endif

		if ( conn->recv_credits > 0 || conn->type != CR_TCPIP )
		{
			/* 
			 * NOTE: may want to always put the FD in the descriptor
               		 * set so we'll notice broken connections.  Down in the
               		 * loop that iterates over the ready sockets only peek
               		 * (MSG_PEEK flag to recv()?) if the connection isn't
               		 * enabled. 
			 */
#ifndef ADDRINFO
			struct sockaddr s;
#else
			struct sockaddr_storage s;
#endif
			socklen_t slen;
			fd_set only_fd; /* testing single fd */
			CRSocket sock = conn->tcp_socket;

			if ( (int) sock + 1 > max_fd )
				max_fd = (int) sock + 1;
			FD_SET( sock, &read_fds );

			/* KLUDGE CITY......
			 *
			 * With threads there's a race condition between
			 * TCPIPRecv and TCPIPSingleRecv when new
			 * clients are connecting, thus new mothership
			 * connections are also being established.
			 * This code below is to check that we're not
			 * in a state of accepting the socket without
			 * connecting to it otherwise we fail with
			 * ENOTCONN later. But, this is really a side
			 * effect of this routine catching a motherships
			 * socket connection and reading data that wasn't
			 * really meant for us. It was really meant for
			 * TCPIPSingleRecv. So, if we detect an
			 * in-progress connection we set the msock id
			 * so that we can assume the motherships socket
			 * and skip over them.
			 */
			
			FD_ZERO(&only_fd);
			FD_SET( sock, &only_fd );
			slen = sizeof( s );
			/* Check that the socket is REALLY connected */
			if (getpeername(sock, (struct sockaddr *) &s, &slen) < 0) {
				/* Another kludge.....
				 * If we disconnect a socket without writing
				 * anything to it, we end up here. Detect
				 * the disconnected socket by checking if
				 * we've ever sent something and then
				 * disconnect it.
				 * 
				 * I think the networking layer needs
				 * a bit of a re-write.... Alan.
				 */
				if (conn->total_bytes_sent > 0) {
					crTCPIPDoDisconnect( conn );
				}
				FD_CLR(sock, &read_fds);
				msock = sock;
			}
			/* 
			 * Nope, that last socket we've just caught in
			 * the connecting phase. We've probably found
			 * a mothership connection here, and we shouldn't
			 * process it 
			 */
			if ((int)sock == msock+1)
				FD_CLR(sock, &read_fds);
		}
	}

#if CRAPPFAKER_SHOULD_DIE
	if (none_left) {
		/*
		 * Caught no more connections.
		 * Review this if we want to try 
		 * restarting crserver's dynamically.
		 */
#ifdef CHROMIUM_THREADSAFE
		crUnlockMutex(&cr_tcpip.recvmutex);
#endif
		crError("No more connections to process, terminating...\n");
		exit(0); /* shouldn't get here */
	}
#endif

	if (!max_fd) {
#ifdef CHROMIUM_THREADSAFE
		crUnlockMutex(&cr_tcpip.recvmutex);
#endif
		return 0;
	}

	if ( num_conns )
	{
		num_ready = __crSelect( max_fd, &read_fds, 0, 500 );
	}
	else
	{
		crWarning( "Waiting for first connection..." );
		num_ready = __crSelect( max_fd, &read_fds, 0, 0 );
	}

	if ( num_ready == 0 ) {
#ifdef CHROMIUM_THREADSAFE
		crUnlockMutex(&cr_tcpip.recvmutex);
#endif
		return 0;
	}

	for ( i = 0; i < num_conns; i++ )
	{
		CRTCPIPBuffer *tcpip_buffer;
		unsigned int   len, total, handled, leftover;
#ifdef RECV_BAIL_OUT
		int inbuf;
#endif
		CRConnection  *conn = cr_tcpip.conns[i];
		CRSocket       sock;

		if ( !conn || conn->type == CR_NO_CONNECTION ) continue;

		/* Added by Samuel Thibault during TCP/IP / UDP code factorization */
		if ( conn->type != CR_TCPIP )
			continue;

		sock = conn->tcp_socket;

		if ( !FD_ISSET( sock, &read_fds ) )
			continue;

		/* Our gigE board is acting odd. If we recv() an amount
		 * less than what is already in the RECVBUF, performance
		 * goes into the toilet (somewhere around a factor of 3).
		 * This is an ugly hack, but seems to get around whatever
		 * funk is being produced  
		 *
		 * Remember to set your kernel recv buffers to be bigger
		 * than the framebuffer 'chunk' you are sending (see
		 * sysctl -a | grep rmem) , or this will really have no
		 * effect.   --karl 
		 */		 
#ifdef RECV_BAIL_OUT 
		(void) recv(sock, &len, sizeof(len), MSG_PEEK);
		ioctl(conn->tcp_socket, FIONREAD, &inbuf);

		if ((conn->krecv_buf_size > len) && (inbuf < len)) continue;
#endif
		/* this reads the length of the message */
		if ( __tcpip_read_exact( sock, &len, sizeof(len)) <= 0 )
		{
			__tcpip_dead_connection( conn );
			i--;
			continue;
		}

		if (conn->swap)
		{
			len = SWAP32(len);
		}

		CRASSERT( len > 0 );

		if ( len <= conn->buffer_size )
		{
			tcpip_buffer = (CRTCPIPBuffer *) crTCPIPAlloc( conn ) - 1;
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

		/* if we have set a userbuf, and there is room in it, we probably 
		 * want to stick the message into that, instead of our allocated
		 * buffer.  */
		leftover = 0;
		total = len;
		if ((conn->userbuf != NULL) && (conn->userbuf_len >= (int) sizeof(CRMessageHeader)))
		{
			leftover = len - sizeof(CRMessageHeader);
			total = sizeof(CRMessageHeader);
		}
		if ( __tcpip_read_exact( sock, tcpip_buffer + 1, total) <= 0 )
		{
			crWarning( "Bad juju: %d %d on sock %x", tcpip_buffer->allocated, total, sock );
			crFree( tcpip_buffer );
			__tcpip_dead_connection( conn );
			i--;
			continue;
		}
		
		conn->recv_credits -= total;
		conn->total_bytes_recv +=  total;

		msg = (CRMessage *) (tcpip_buffer + 1);
		cached_type = msg->header.type;
		if (conn->swap)
		{
			msg->header.type = (CRMessageType) SWAP32( msg->header.type );
			msg->header.conn_id = (CRMessageType) SWAP32( msg->header.conn_id );
		}
	
		/* if there is still data pending, it should go into the user buffer */
		if (leftover)
		{
			handled = crTCPIPUserbufRecv(conn, msg);

			/* if there is anything left, plop it into the recv_buffer */
			if (leftover-handled)
			{
				if ( __tcpip_read_exact( sock, tcpip_buffer + 1 + total, leftover-handled) <= 0 )
				{
					crWarning( "Bad juju: %d %d", tcpip_buffer->allocated, leftover-handled);
					crFree( tcpip_buffer );
					__tcpip_dead_connection( conn );
					i--;
					continue;
				}
			}
			
			conn->recv_credits -= handled;
			conn->total_bytes_recv +=  handled;
		}

		
		crNetDispatchMessage( cr_tcpip.recv_list, conn, tcpip_buffer + 1, len );
#if 0
		crLogRead( len );
#endif


		/* CR_MESSAGE_OPCODES is freed in
		 * crserverlib/server_stream.c 
		 *
		 * OOB messages are the programmer's problem.  -- Humper 12/17/01 */
		if (cached_type != CR_MESSAGE_OPCODES && cached_type != CR_MESSAGE_OOB
		    &&	cached_type != CR_MESSAGE_GATHER) 
		{
			crTCPIPFree( conn, tcpip_buffer + 1 );
		}
		
	}

#ifdef CHROMIUM_THREADSAFE
	crUnlockMutex(&cr_tcpip.recvmutex);
#endif

	return 1;
}


static void
crTCPIPHandleNewMessage( CRConnection *conn, CRMessage *msg,
		unsigned int len )
{
	CRTCPIPBuffer *buf = ((CRTCPIPBuffer *) msg) - 1;

	/* build a header so we can delete the message later */
	buf->magic = CR_TCPIP_BUFFER_MAGIC;
	buf->kind  = CRTCPIPMemory;
	buf->len   = len;
	buf->pad   = 0;

	crNetDispatchMessage( cr_tcpip.recv_list, conn, msg, len );
}


static void
crTCPIPInstantReclaim( CRConnection *conn, CRMessage *mess )
{
	crTCPIPFree( conn, mess );
}


void
crTCPIPInit( CRNetReceiveFuncList *rfl, CRNetCloseFuncList *cfl,
						 unsigned int mtu )
{
	(void) mtu;

	cr_tcpip.recv_list = rfl;
	cr_tcpip.close_list = cfl;
	if ( cr_tcpip.initialized )
	{
		return;
	}

	cr_tcpip.initialized = 1;
	crDebug("Initializing TCPIP\n");

	cr_tcpip.num_conns = 0;
	cr_tcpip.conns     = NULL;
	
	cr_tcpip.server_sock    = -1;

#ifdef CHROMIUM_THREADSAFE
	crInitMutex(&cr_tcpip.mutex);
	crInitMutex(&cr_tcpip.recvmutex);
#endif
	cr_tcpip.bufpool = crBufferPoolInit(16);
}

/* The function that actually connects.  This should only be called by clients 
 * Servers have another way to set up the socket. */
int
crTCPIPDoConnect( CRConnection *conn )
{
	int err;
#ifndef ADDRINFO
	struct sockaddr_in servaddr;
	struct hostent *hp;
	int i;

	conn->tcp_socket = socket( AF_INET, SOCK_STREAM, 0 );
	if ( conn->tcp_socket < 0 )
	{
		int err = crTCPIPErrno( );
		crWarning( "socket error: %s", crTCPIPErrorString( err ) );
		cr_tcpip.conns[conn->index] = NULL; /* remove from table */
		return 0;
	}

	/* Set up the socket the way *we* want. */
	spankSocket( conn->tcp_socket );

	/* Standard Berkeley sockets mumbo jumbo */
	hp = gethostbyname( conn->hostname );
	if ( !hp )
	{
		crWarning( "Unknown host: \"%s\"", conn->hostname );
		cr_tcpip.conns[conn->index] = NULL; /* remove from table */
		return 0;
	}

	crMemset( &servaddr, 0, sizeof(servaddr) );
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons( (short) conn->port );

	crMemcpy( (char *) &servaddr.sin_addr, hp->h_addr,
			sizeof(servaddr.sin_addr) );
#else
	char port_s[NI_MAXSERV];
	struct addrinfo *res,*cur;
	struct addrinfo hints;

	sprintf(port_s, "%u", (short unsigned) conn->port);

	crMemset(&hints, 0, sizeof(hints));
	hints.ai_family = PF;
	hints.ai_socktype = SOCK_STREAM;

	err = getaddrinfo( conn->hostname, port_s, &hints, &res);
	if ( err )
	{
		crWarning( "Unknown host: \"%s\": %s", conn->hostname, gai_strerror(err) );
		cr_tcpip.conns[conn->index] = NULL; /* remove from table */
		return 0;
	}
#endif

	if (conn->broker)
	{
		CRConnection *mother;
		char response[8096];
		int remote_endianness;
		mother = __copy_of_crMothershipConnect( );

		if (!__copy_of_crMothershipSendString( mother, response, "connectrequest tcpip %s %d %d", 
						       conn->hostname, conn->port, conn->endianness) )
		{
#ifdef ADDRINFO
			freeaddrinfo(res);
#endif
			crError( "Mothership didn't like my connect request request" );
		}

		__copy_of_crMothershipDisconnect( mother );

		sscanf( response, "%u %d", &(conn->id), &(remote_endianness) );

		if (conn->endianness != remote_endianness)
		{
			conn->swap = 1;
		}
	}
#ifndef ADDRINFO
	for (i=1;i;)
#else
	for (cur=res;cur;)
#endif
	{
#ifndef ADDRINFO

#ifdef RECV_BAIL_OUT		
		err = sizeof(unsigned int);
		if ( getsockopt( conn->tcp_socket, SOL_SOCKET, SO_RCVBUF,
				(char *) &conn->krecv_buf_size, &err ) )
		{
			conn->krecv_buf_size = 0;	
		}
#endif
		if ( !connect( conn->tcp_socket, (struct sockaddr *) &servaddr,
					sizeof(servaddr) ) )
			return 1;
#else

		conn->tcp_socket = socket( cur->ai_family, cur->ai_socktype, cur->ai_protocol );
		if ( conn->tcp_socket < 0 )
		{
			int err = crTCPIPErrno( );
			if (err != EAFNOSUPPORT)
				crWarning( "socket error: %s, trying another way", crTCPIPErrorString( err ) );
			cur=cur->ai_next;
			continue;
		}

		err = 1;
		setsockopt(conn->tcp_socket, SOL_SOCKET, SO_REUSEADDR,  &err, sizeof(int));

		/* Set up the socket the way *we* want. */
		spankSocket( conn->tcp_socket );

#if RECV_BAIL_OUT		
		err = sizeof(unsigned int);
		if ( getsockopt( conn->tcp_socket, SOL_SOCKET, SO_RCVBUF,
				(char *) &conn->krecv_buf_size, &err ) )
		{
			conn->krecv_buf_size = 0;	
		}
#endif

		if ( !connect( conn->tcp_socket, cur->ai_addr, cur->ai_addrlen ) ) {
			freeaddrinfo(res);
			return 1;
		}
#endif

		err = crTCPIPErrno( );
		if ( err == EADDRINUSE || err == ECONNREFUSED )
			crWarning( "Connection refused to %s:%d, %s",
					conn->hostname, conn->port, crTCPIPErrorString( err ) );

		else if ( err == EINTR )
		{
			crWarning( "connection to %s:%d "
					"interruped, trying again", conn->hostname, conn->port );
			continue;
		}
		else
			crWarning( "Couldn't connect to %s:%d, %s",
					conn->hostname, conn->port, crTCPIPErrorString( err ) );
		crCloseSocket( conn->tcp_socket );
#ifndef ADDRINFO
		i=0;
#else
		cur=cur->ai_next;
#endif
	}
#ifdef ADDRINFO
	freeaddrinfo(res);
	crWarning( "Couln't find any suitable way to connect to %s", conn->hostname );
#endif
	cr_tcpip.conns[conn->index] = NULL; /* remove from table */
	return 0;
}

void
crTCPIPDoDisconnect( CRConnection *conn )
{
	int num_conns = cr_tcpip.num_conns;
	int none_left = 1;
	int i;

	crCloseSocket( conn->tcp_socket );
	conn->tcp_socket = 0;
	conn->type = CR_NO_CONNECTION;
	cr_tcpip.conns[conn->index] = NULL;

	for (i = 0; i < num_conns; i++) 
	{
		if ( cr_tcpip.conns[i] && cr_tcpip.conns[i]->type != CR_NO_CONNECTION )
			none_left = 0; /* found a live connection */
	}

	if (none_left && cr_tcpip.server_sock != -1)
	{
		crDebug("Closing master socket (probably quitting).");
		crCloseSocket( cr_tcpip.server_sock );
#ifdef CHROMIUM_THREADSAFE
		crFreeMutex(&cr_tcpip.mutex);
		crFreeMutex(&cr_tcpip.recvmutex);
#endif
		crBufferPoolFree( cr_tcpip.bufpool );
		cr_tcpip.bufpool = NULL;
		last_port = 0;
		cr_tcpip.initialized = 0;
	}
}

void
crTCPIPConnection( CRConnection *conn )
{
	int i, found = 0;
	int n_bytes;

	CRASSERT( cr_tcpip.initialized );

	conn->type  = CR_TCPIP;
	conn->Alloc = crTCPIPAlloc;
	conn->Send  = crTCPIPSend;
	conn->SendExact  = crTCPIPWriteExact;
	conn->Recv  = crTCPIPSingleRecv;
	conn->Free  = crTCPIPFree;
	conn->Accept = crTCPIPAccept;
	conn->Connect = crTCPIPDoConnect;
	conn->Disconnect = crTCPIPDoDisconnect;
	conn->InstantReclaim = crTCPIPInstantReclaim;
	conn->HandleNewMessage = crTCPIPHandleNewMessage;
	conn->index = cr_tcpip.num_conns;
	conn->sizeof_buffer_header = sizeof( CRTCPIPBuffer );
	conn->actual_network = 1;

	conn->krecv_buf_size = 0;

	/* Find a free slot */
	for (i = 0; i < cr_tcpip.num_conns; i++) {
		if (cr_tcpip.conns[i] == NULL) {
			conn->index = i;
			cr_tcpip.conns[i] = conn;
			found = 1;
			break;
		}
	}
	
	/* Realloc connection stack if we couldn't find a free slot */
	if (found == 0) {
		n_bytes = ( cr_tcpip.num_conns + 1 ) * sizeof(*cr_tcpip.conns);
		crRealloc( (void **) &cr_tcpip.conns, n_bytes );
		cr_tcpip.conns[cr_tcpip.num_conns++] = conn;
	}
}

int crGetHostname( char *buf, unsigned int len )
{
	char *override = NULL;
	int ret;

 	override = crGetenv("CR_HOSTNAME");
	if (override)
	{
		crStrncpy(buf, override, len);
		ret = 0;	
	}
	else
		ret = gethostname( buf, len );
	/*if (ret) 
	 *{ 
		 *int err = crTCPIPErrno(); 
		 *crWarning( "Couldn't get hostname: %s", crTCPIPErrorString( err ) ); 
	 *} */

	return ret;
}

CRConnection** crTCPIPDump( int *num )
{
	*num = cr_tcpip.num_conns;

	return cr_tcpip.conns;
}
