/* Copyright (c) 2004, Stanford University
 * Copyright (c) 2004, GraphStream
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */


/* sometimes the default IB install doesn't setup the includes correctly,
	 so we are going to manually define AF_INET_SDP */
/* if you use the Mellanox Gold 1.6.x or the OpenIB gen1 stack this
 * will work, but for OpenIB gen2 and the corresponding vendor stacks,
 * you will need to use the enum in the kernel tree which is currently
 * 27, but should be defined by AF_INET_SDP */

#ifndef AF_INET_SDP
#define AF_INET_SDP 26
#endif

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

#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>

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

typedef enum {
	CRSDPMemory,
	CRSDPMemoryBig
} CRSDPBufferKind;

#define CR_SDP_BUFFER_MAGIC 0x89134532

typedef struct CRSDPBuffer {
	unsigned int          magic;
	CRSDPBufferKind     kind;
	unsigned int          len;
	unsigned int          allocated;
	unsigned int          pad;
} CRSDPBuffer;

typedef struct {
	int                  initialized;
	int                  num_conns;
	CRConnection         **conns;
	CRBufferPool         *bufpool;
#ifdef CHROMIUM_THREADSAFE
	CRmutex              mutex;
	CRmutex              recvmutex;
#endif
	CRNetReceiveFuncList *recv_list;
	CRNetCloseFuncList *close_list;
	CRSocket             server_sock;
} cr_sdp_data;


/* XXX these could be removed by reordering the functions below */
static void
__sdp_dead_connection( CRConnection *conn );

static void
crSDPDoDisconnect( CRConnection *conn );



#ifdef WINDOWS
#define EADDRINUSE   WSAEADDRINUSE
#define ECONNREFUSED WSAECONNREFUSED
#endif


#ifdef WINDOWS

#undef  ECONNRESET
#define ECONNRESET  WSAECONNRESET
#undef  EINTR
#define EINTR       WSAEINTR

static int
crSDPErrno( void )
{
	return WSAGetLastError( );
}

static char *
crSDPErrorString( int err )
{
	static char buf[512], *temp;
	
	sprintf( buf, "err=%d", err );
	
#define X(x)	crStrcpy(buf,x); break
  
	switch ( err )
	{
	case WSAECONNREFUSED:
		X( "connection refused" );
	case WSAECONNRESET:
		X( "connection reset" );
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

static int
crSDPErrno( void )
{
	int err = errno;
	errno = 0;
	return err;
}

static char *
crSDPErrorString( int err )
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

static unsigned short last_port = 0;
static cr_sdp_data cr_sdp;

/**
 * Return the suffix to put onto hostnames to address the IB/SDP interface.
 * We'll check the CR_SDP_SUFFIX env var the first time called.
 * Note: "sdp" is the default.
 */
const char *
crGetSDPHostnameSuffix(void)
{
	static const char *suffix = NULL;
	if (!suffix) {
		suffix = crGetenv("CR_SDP_SUFFIX");
		if (!suffix)
			suffix = "sdp";
	}
	return suffix;
}

static int
__sdp_read_exact( CRSocket sock, void *buf, unsigned int len )
{
	char *dst = (char *) buf;
	/* 
	 * Shouldn't write to a non-existent socket, ie when 
	 * crSDPDoDisconnect has removed it from the pool
	 */
	if ( sock <= 0 )
		return 1;

	while ( len > 0 )
	{
		int num_read = recv( sock, dst, (int) len, 0 );
    
		if ( num_read < 0 )
		{
			int error = crSDPErrno();
			switch( error )
			{
			case EINTR:
				crWarning( "__sdp_read_exact(SDP): "
						   "caught an EINTR, looping for more data" );
				continue;
			case EFAULT:
				crWarning( "EFAULT" );
				break;
			case EINVAL:
				crWarning( "EINVAL" );
				break;
			default:
				break;
			}
			crWarning( "Bad bad bad socket error: %s", crSDPErrorString( error ) );
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

static void
crSDPReadExact( CRConnection *conn, void *buf, unsigned int len )
{
	if ( __sdp_read_exact( conn->sdp_socket, buf, len ) <= 0 )
	{
		__sdp_dead_connection( conn );
	}
}

static int
__sdp_write_exact( CRSocket sock, const void *buf, unsigned int len )
{
	int err;
	char *src = (char *) buf;

	/* 
	 * Shouldn't write to a non-existent socket, ie when 
	 * crSDPDoDisconnect has removed it from the pool
	 */
	if ( sock <= 0 )
		return 1;

	while ( len > 0 )
	{
		int num_written = send( sock, src, len, 0 );
		if ( num_written <= 0 )
		{
		  if ( (err = crSDPErrno( )) == EINTR )
		  {
				crWarning( "__sdp_write_exact(SDP): "
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

static void
crSDPWriteExact( CRConnection *conn, const void *buf, unsigned int len )
{
	if ( __sdp_write_exact( conn->sdp_socket, buf, len) <= 0 )
	{
		__sdp_dead_connection( conn );
	}
}


/* 
 * Make sockets do what we want: 
 * 
 * 1) Change the size of the send/receive buffers to 1MB 
 * 2) Turn off Nagle's algorithm
 */
static void
spankSocket( CRSocket sock )
{

	int sndbuf = 1*1024*1024;

	int rcvbuf = sndbuf;
	int so_reuseaddr = 1;

	if ( setsockopt( sock, SOL_SOCKET, SO_SNDBUF, 
			 (char *) &sndbuf, sizeof(sndbuf) ) )
	{
		int err = crSDPErrno( );
		crWarning( "setsockopt( SO_SNDBUF=%d ) : %s",
			   sndbuf, crSDPErrorString( err ) );
	}
	
	if ( setsockopt( sock, SOL_SOCKET, SO_RCVBUF,
			 (char *) &rcvbuf, sizeof(rcvbuf) ) )
	{
		int err = crSDPErrno( );
		crWarning( "setsockopt( SO_RCVBUF=%d ) : %s",
			   rcvbuf, crSDPErrorString( err ) );
  }
	
	if ( setsockopt( sock, SOL_SOCKET, SO_REUSEADDR,
			 (char *) &so_reuseaddr, sizeof(so_reuseaddr) ) )
	{
		int err = crSDPErrno( );
		crWarning( "setsockopt( SO_REUSEADDR=%d ) : %s",
			   so_reuseaddr, crSDPErrorString( err ) );
	}
}


#if defined( WINDOWS ) || defined( IRIX ) || defined( IRIX64 )
typedef int socklen_t;
#endif

static void
crSDPAccept( CRConnection *conn, const char *hostname, unsigned short port )
{
	int err;
	socklen_t		addr_length;
	struct sockaddr_in	servaddr;
	struct sockaddr		addr;
	struct hostent		*host;
	struct in_addr		sin_addr;

	if (port != last_port)
	{
	     /* with the new OOB stuff, we can have multiple ports being 
	      * accepted on, so we need to redo the server socket every time.
	      */
		cr_sdp.server_sock = socket( AF_INET_SDP, SOCK_STREAM, 0 );
		if ( cr_sdp.server_sock == -1 )
		{
			err = crSDPErrno( );
			crError( "Couldn't create socket: %s", crSDPErrorString( err ) );
		}
		spankSocket( cr_sdp.server_sock );

		servaddr.sin_family = AF_INET_SDP;
		servaddr.sin_addr.s_addr = INADDR_ANY;
		servaddr.sin_port = htons( port );

		if ( bind( cr_sdp.server_sock, (struct sockaddr *) &servaddr, sizeof(servaddr) ) )
		{
			err = crSDPErrno( );
			crError( "Couldn't bind to socket (port=%d): %s", port, crSDPErrorString( err ) );
		}
		last_port = port;
		if ( listen( cr_sdp.server_sock, 100 /* max pending connections */ ) )
		{
			err = crSDPErrno( );
			crError( "Couldn't listen on socket: %s", crSDPErrorString( err ) );
		}
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
				crError( "Couldn't determine my own hostname in crSDPAccept!" );
			}
		}
		else
			crStrcpy(my_hostname, hostname);
    
		/* Hack hostname, YUCK!!! */
		temp = strtok(my_hostname, ".");
		crStrcat(temp, crGetSDPHostnameSuffix());
		(void) temp;
		if (!__copy_of_crMothershipSendString( mother, response, "acceptrequest sdp %s %d %d", temp, conn->port, conn->endianness ) )
		{
			crError( "Mothership didn't like my accept request" );
		}
    
		__copy_of_crMothershipDisconnect( mother );
    
		sscanf( response, "%u", &(conn->id) );
	}
	
	addr_length =	sizeof( addr );
	conn->sdp_socket = accept( cr_sdp.server_sock, (struct sockaddr *) &addr, &addr_length );
	if (conn->sdp_socket == -1)
	{
		err = crSDPErrno( );
		crError( "Couldn't accept client: %s", crSDPErrorString( err ) );
	}
	
	sin_addr = ((struct sockaddr_in *) &addr)->sin_addr;
	host = gethostbyaddr( (char *) &sin_addr, sizeof( sin_addr), AF_INET_SDP );
	if (host == NULL )
	{
		char *temp = inet_ntoa( sin_addr );
		conn->hostname = crStrdup( temp );
	}
	else
	{
		char *temp;
		conn->hostname = crStrdup( host->h_name );
    
		temp = conn->hostname;
		while (*temp && *temp != '.' )
			temp++;
		*temp = '\0';
	}
	
#ifdef RECV_BAIL_OUT 
	err = sizeof(unsigned int);
	if ( getsockopt( conn->sdp_socket, SOL_SOCKET, SO_RCVBUF,
									 (char *) &conn->krecv_buf_size, &err ) )
	{
		conn->krecv_buf_size = 0;	
	}
#endif
	crDebug( "Accepted connection from \"%s\".", conn->hostname );
}


static void *
crSDPAlloc( CRConnection *conn )
{
  CRSDPBuffer *buf;
  
#ifdef CHROMIUM_THREADSAFE
  crLockMutex(&cr_sdp.mutex);
#endif
  
	buf = (CRSDPBuffer *) crBufferPoolPop( cr_sdp.bufpool, conn->buffer_size );
  
	if ( buf == NULL )
	{
		crDebug( "Buffer pool %p was empty, so I allocated %d bytes.\n\tI did so from the buffer: %p", 
             cr_sdp.bufpool,
             (unsigned int)sizeof(CRSDPBuffer) + conn->buffer_size, &cr_sdp.bufpool );
		buf = (CRSDPBuffer *) 
			crAlloc( sizeof(CRSDPBuffer) + conn->buffer_size );
		buf->magic = CR_SDP_BUFFER_MAGIC;
		buf->kind  = CRSDPMemory;
		buf->pad   = 0;
		buf->allocated = conn->buffer_size;
	}
	else {
		CRASSERT(buf->magic == CR_SDP_BUFFER_MAGIC);
	}

#ifdef CHROMIUM_THREADSAFE
	crUnlockMutex(&cr_sdp.mutex);
#endif
  
	return (void *)( buf + 1 );
}


static void
crSDPSingleRecv( CRConnection *conn, void *buf, unsigned int len )
{
	crSDPReadExact( conn, buf, len );
}


static void
crSDPSend( CRConnection *conn, void **bufp,
           const void *start, unsigned int len )
{
	CRSDPBuffer *sdp_buffer;
	unsigned int      *lenp;
  
	if ( !conn || conn->type == CR_NO_CONNECTION )
		return;
  
	if ( bufp == NULL )
	{
		/* we are doing synchronous sends from user memory, so no need
		 * to get fancy.  Simply write the length & the payload and
		 * return. */
		const int sendable_len = conn->swap ? SWAP32(len) : len;
    
		crSDPWriteExact( conn, &sendable_len, sizeof(len) );
		if ( !conn || conn->type == CR_NO_CONNECTION) return;
		crSDPWriteExact( conn, start, len );
		return;
	}
	sdp_buffer = (CRSDPBuffer *)(*bufp) - 1;
  
	CRASSERT( sdp_buffer->magic == CR_SDP_BUFFER_MAGIC );
  
	/* All of the buffers passed to the send function were allocated
	 * with crSDPAlloc(), which includes a header with a 4 byte
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
  
	if ( __sdp_write_exact( conn->sdp_socket, lenp, len + sizeof(int) ) < 0 )
	{
		__sdp_dead_connection( conn );
	}
	/* reclaim this pointer for reuse and try to keep the client from
		 accidentally reusing it directly */
#ifdef CHROMIUM_THREADSAFE
	crLockMutex(&cr_sdp.mutex);
#endif
	crBufferPoolPush( cr_sdp.bufpool, sdp_buffer, sdp_buffer->allocated );
	/* Since the buffer's now in the 'free' buffer pool, the caller can't
	 * use it any more.  Setting bufp to NULL will make sure the caller
	 * doesn't try to re-use the buffer.
	 */
	*bufp = NULL;
#ifdef CHROMIUM_THREADSAFE
	crUnlockMutex(&cr_sdp.mutex);
#endif
}


static void
__sdp_dead_connection( CRConnection *conn )
{
	crDebug( "Dead connection (sock=%d, host=%s), removing from pool",
           conn->sdp_socket, conn->hostname );
  
	/* remove from connection pool */
	crSDPDoDisconnect( conn );
}


static int
__crSDPSelect( int n, fd_set *readfds, int sec, int usec )
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
    
		err = crSDPErrno( );
		if ( err == EINTR )
		{
			crWarning( "select interruped by an unblocked signal, trying again" );
		}
		else
		{
			crError( "select failed: %s", crSDPErrorString( err ) );
		}
	}
}


static void
crSDPFree( CRConnection *conn, void *buf )
{
	CRSDPBuffer *sdp_buffer = (CRSDPBuffer *) buf - 1;

	CRASSERT( sdp_buffer->magic == CR_SDP_BUFFER_MAGIC );
	conn->recv_credits += sdp_buffer->len;

	switch ( sdp_buffer->kind )
	{
	case CRSDPMemory:
#ifdef CHROMIUM_THREADSAFE
		crLockMutex(&cr_sdp.mutex);
#endif
		if (cr_sdp.bufpool) {
			/* pool may have been deallocated just a bit earlier in response
			 * to a SIGPIPE (Broken Pipe) signal.
			 */
			crBufferPoolPush( cr_sdp.bufpool, sdp_buffer, sdp_buffer->allocated );
		}
#ifdef CHROMIUM_THREADSAFE
		crUnlockMutex(&cr_sdp.mutex);
#endif
		break;

	case CRSDPMemoryBig:
		crFree( sdp_buffer );
		break;

	default:
		crError( "Weird buffer kind trying to free in crSDPFree: %d", sdp_buffer->kind );
	}
}


/* returns the amt of pending data which was handled */ 
static int
crSDPUserbufRecv(CRConnection *conn, CRMessage *msg)
{
	unsigned long buf[2];
	int len;
  
	switch (msg->header.type)
	{
	case CR_MESSAGE_GATHER:
		/* grab the offset and the length */
		len = 2*sizeof(unsigned long);
		if (__sdp_read_exact(conn->sdp_socket, buf, len) <= 0)
		{
			__sdp_dead_connection( conn );
		}
		msg->gather.offset = buf[0];
		msg->gather.len = buf[1];
    
		/* read the rest into the userbuf */
		if (buf[0]+buf[1] > (unsigned long) conn->userbuf_len)
		{
			crDebug("userbuf for Gather Message is too small!");
			return len;
		}
		
		if (__sdp_read_exact(conn->sdp_socket, conn->userbuf+buf[0], buf[1]) <= 0)
		{
			__sdp_dead_connection( conn );
		}
		return len+buf[1];
    
	default:
		return 0;
	}
}


/**
 * Receive the next message on the given connection.
 * If we're being called by crSDPRecv(), we already know there's
 * something to receive.
 */
static void
crSDPReceiveMessage(CRConnection *conn)
{
	CRMessage *msg;
	CRMessageType cached_type;
	CRSDPBuffer *sdp_buffer;
	unsigned int len, total, leftover;
	const CRSocket sock = conn->sdp_socket;

	/* this reads the length of the message */
	if ( __sdp_read_exact( sock, &len, sizeof(len)) <= 0 )
	{
		__sdp_dead_connection( conn );
		return;
	}

	if (conn->swap)
		len = SWAP32(len);

	CRASSERT( len > 0 );

	if ( len <= conn->buffer_size )
	{
		sdp_buffer = (CRSDPBuffer *) crSDPAlloc( conn ) - 1;
		CRASSERT(sdp_buffer->magic == CR_SDP_BUFFER_MAGIC);
		CRASSERT(sdp_buffer->allocated > 0);
	}
	else
	{
		crWarning("Sending as BIG, the performance is going to tank!!!.  You need larger buffers!!!");
		sdp_buffer = (CRSDPBuffer *) crAlloc( sizeof(*sdp_buffer) + len );
		sdp_buffer->magic = CR_SDP_BUFFER_MAGIC;
		sdp_buffer->kind  = CRSDPMemoryBig;
		sdp_buffer->pad   = 0;
	}

	sdp_buffer->len = len;

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
	if ( __sdp_read_exact( sock, sdp_buffer + 1, total) <= 0 )
	{
		crWarning( "Bad juju: %d %d on sock %x", sdp_buffer->allocated, total, sock );
		crFree( sdp_buffer );
		__sdp_dead_connection( conn );
		return;
	}

	conn->recv_credits -= total;
	conn->total_bytes_recv +=  total;
    
	msg = (CRMessage *) (sdp_buffer + 1);
	cached_type = msg->header.type;
	if (conn->swap)
	{
		msg->header.type = (CRMessageType) SWAP32( msg->header.type );
		msg->header.conn_id = (CRMessageType) SWAP32( msg->header.conn_id );
	}
    
	/* if there is still data pending, it should go into the user buffer */
	if (leftover)
	{
		unsigned int handled = crSDPUserbufRecv(conn, msg);
      
		/* if there is anything left, plop it into the recv_buffer */
		if (leftover - handled)
		{
			if ( __sdp_read_exact( sock, sdp_buffer + 1 + total, leftover-handled) <= 0 )
			{
				crWarning( "Bad juju: %d %d", sdp_buffer->allocated, leftover-handled);
				crFree( sdp_buffer );
				__sdp_dead_connection( conn );
				return;
			}
		}

		conn->recv_credits -= handled;
		conn->total_bytes_recv +=  handled;
	}

	crNetDispatchMessage( cr_sdp.recv_list, conn, msg, len );

	/* CR_MESSAGE_OPCODES is freed in crserverlib/server_stream.c with crNetFree.
	 * OOB messages are the programmer's problem.  -- Humper 12/17/01
	 */
	if (cached_type != CR_MESSAGE_OPCODES && cached_type != CR_MESSAGE_OOB
			&&	cached_type != CR_MESSAGE_GATHER) 
	{
		crSDPFree( conn, sdp_buffer + 1 );
	}
}



int
crSDPRecv( void )
{
	int    num_ready, max_fd;
	fd_set read_fds;
	int i;
	/* ensure we don't get caught with a new thread connecting */
	int num_conns = cr_sdp.num_conns;
#if CRAPPFAKER_SHOULD_DIE
	int none_left = 1;
#endif
  
#ifdef CHROMIUM_THREADSAFE
	crLockMutex(&cr_sdp.recvmutex);
#endif

	/*
	 * First determine which connections are ready with data.
	 */
	max_fd = 0;
	FD_ZERO( &read_fds );
	for ( i = 0; i < num_conns; i++ )
	{
		CRConnection *conn = cr_sdp.conns[i];
		if ( !conn || conn->type == CR_NO_CONNECTION )
			continue;

#if CRAPPFAKER_SHOULD_DIE
		none_left = 0;
#endif

		if ( conn->recv_credits > 0 || conn->type != CR_SDP )
		{
			CRSocket sock = conn->sdp_socket;
			if ( (int) sock + 1 > max_fd ){
				max_fd = (int) sock + 1;
			}
			FD_SET( sock, &read_fds );	
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
		crUnlockMutex(&cr_sdp.recvmutex);
#endif
		crError("No more connections to process, terminating...\n");
		exit(0); /* shouldn't get here */
	}
#endif

	if (!max_fd) {
#ifdef CHROMIUM_THREADSAFE
		crUnlockMutex(&cr_sdp.recvmutex);
#endif
		return 0;
	}

	if ( num_conns )
	{
		num_ready = __crSDPSelect( max_fd, &read_fds, 0, 500 );
	}
	else
	{
		crWarning( "Waiting for first connection..." );
		num_ready = __crSDPSelect( max_fd, &read_fds, 0, 0 );
	}

	if ( num_ready == 0 ) {
#ifdef CHROMIUM_THREADSAFE
		crUnlockMutex(&cr_sdp.recvmutex);
#endif
		return 0;
	}

	/*
	 * Now do the actual socket reads.
	 */
	for ( i = 0; i < num_conns; i++ )
	{
		CRConnection  *conn = cr_sdp.conns[i];
		CRSocket       sock;

		if ( !conn || conn->type == CR_NO_CONNECTION )
			continue;

		sock = conn->sdp_socket;
		if ( !FD_ISSET( sock, &read_fds ) )
			continue;

		if (conn->threaded)
			continue;

		crSDPReceiveMessage(conn);
	}
  
#ifdef CHROMIUM_THREADSAFE
	crUnlockMutex(&cr_sdp.recvmutex);
#endif
  
	return 1;
}


static void
crSDPHandleNewMessage( CRConnection *conn, CRMessage *msg,
                       unsigned int len )
{
	CRSDPBuffer *buf = ((CRSDPBuffer *) msg) - 1;
  
	/* build a header so we can delete the message later */
	buf->magic = CR_SDP_BUFFER_MAGIC;
	buf->kind  = CRSDPMemory;
	buf->len   = len;
	buf->pad   = 0;
  
	crNetDispatchMessage( cr_sdp.recv_list, conn, msg, len );
}


static void
crSDPInstantReclaim( CRConnection *conn, CRMessage *mess )
{
	crSDPFree( conn, mess );
}


void
crSDPInit( CRNetReceiveFuncList *rfl, CRNetCloseFuncList *cfl,
           unsigned int mtu )
{
	(void) mtu;
  
	cr_sdp.recv_list = rfl;
	cr_sdp.close_list = cfl;
	if ( cr_sdp.initialized )
	{
		return;
	}
  
	cr_sdp.initialized = 1;
	crDebug("Initializing SDP");
  
	cr_sdp.num_conns = 0;
	cr_sdp.conns     = NULL;
	
	cr_sdp.server_sock    = -1;
  
#ifdef CHROMIUM_THREADSAFE
	crInitMutex(&cr_sdp.mutex);
	crInitMutex(&cr_sdp.recvmutex);
#endif
	cr_sdp.bufpool = crBufferPoolInit(16);
}

/* The function that actually connects.  This should only be called by clients 
 * Servers have another way to set up the socket. */
static int
crSDPDoConnect( CRConnection *conn )
{
	int err;
	struct sockaddr_in servaddr;
	struct hostent *hp;
	int i;
  
	conn->sdp_socket = socket( AF_INET_SDP, SOCK_STREAM, 0 );
	if ( conn->sdp_socket < 0 )
	{
		int err = crSDPErrno( );
		crWarning( "socket error: %s", crSDPErrorString( err ) );
		cr_sdp.conns[conn->index] = NULL; /* remove from table */
		return 0;
	}
  
	/* Set up the socket the way *we* want. */
	spankSocket( conn->sdp_socket );
  
	/* Standard Berkeley sockets mumbo jumbo */
	hp = gethostbyname( conn->hostname );
	if ( !hp )
	{
		crWarning( "Unknown host: \"%s\"", conn->hostname );
		cr_sdp.conns[conn->index] = NULL; /* remove from table */
		return 0;
	}
  
	crMemset( &servaddr, 0, sizeof(servaddr) );
	servaddr.sin_family = AF_INET_SDP;
	servaddr.sin_port = htons( (short) conn->port );
  
	crMemcpy( (char *) &servaddr.sin_addr, hp->h_addr,
            sizeof(servaddr.sin_addr) );
  
	if (conn->broker)
	{
		CRConnection *mother;
		char response[8096];
		int remote_endianness;
		mother = __copy_of_crMothershipConnect( );
    
		if (!__copy_of_crMothershipSendString( mother, response, "connectrequest sdp %s %d %d", 
                                           conn->hostname, conn->port, conn->endianness) )
		{
			crError( "Mothership didn't like my connect request" );
		}
    
		__copy_of_crMothershipDisconnect( mother );
    
		sscanf( response, "%u %d", &(conn->id), &(remote_endianness) );
    
		if (conn->endianness != remote_endianness)
		{
			conn->swap = 1;
		}
	}
	for (i=1;i;)
	{
#ifdef RECV_BAIL_OUT		
		err = sizeof(unsigned int);
		if ( getsockopt( conn->sdp_socket, SOL_SOCKET, SO_RCVBUF,
                     (char *) &conn->krecv_buf_size, &err ) )
		{
			conn->krecv_buf_size = 0;	
		}
#endif
		if ( !connect( conn->sdp_socket, (struct sockaddr *) &servaddr,
                   sizeof(servaddr) ) )
			return 1;
    
		err = crSDPErrno( );
		if ( err == EADDRINUSE || err == ECONNREFUSED )
			crWarning( "Couldn't connect to %s:%d, %s",
                 conn->hostname, conn->port, crSDPErrorString( err ) );
    
		else if ( err == EINTR )
		{
			crWarning( "connection to %s:%d "
                 "interruped, trying again", conn->hostname, conn->port );
			continue;
		}
		else
			crWarning( "Couldn't connect to %s:%d, %s",
                 conn->hostname, conn->port, crSDPErrorString( err ) );
		i=0;
	}
	cr_sdp.conns[conn->index] = NULL; /* remove from table */
	return 0;
}

static void
crSDPDoDisconnect( CRConnection *conn )
{
	int num_conns = cr_sdp.num_conns;
	int none_left = 1;
	int i;
  
	crCloseSocket( conn->sdp_socket );
	conn->sdp_socket = 0;
	conn->type = CR_NO_CONNECTION;
	cr_sdp.conns[conn->index] = NULL;
  
	for (i = 0; i < num_conns; i++) 
	{
		if ( cr_sdp.conns[i] && cr_sdp.conns[i]->type != CR_NO_CONNECTION )
			none_left = 0; /* found a live connection */
	}
  
	if (none_left && cr_sdp.server_sock != -1)
	{
		crDebug("Closing master socket (probably quitting).");
		crCloseSocket( cr_sdp.server_sock );
#ifdef CHROMIUM_THREADSAFE
		crFreeMutex(&cr_sdp.mutex);
		crFreeMutex(&cr_sdp.recvmutex);
#endif
		crBufferPoolFree( cr_sdp.bufpool );
		cr_sdp.bufpool = NULL;
		last_port = 0;
		cr_sdp.initialized = 0;
	}
}

void
crSDPConnection( CRConnection *conn )
{
	int i, found = 0;
	int n_bytes;
  
	CRASSERT( cr_sdp.initialized );
  
	conn->type  = CR_SDP;
	conn->Alloc = crSDPAlloc;
	conn->Send  = crSDPSend;
	conn->SendExact  = crSDPWriteExact;
	conn->Recv  = crSDPSingleRecv;
	conn->RecvMsg = crSDPReceiveMessage;
	conn->Free  = crSDPFree;
	conn->Accept = crSDPAccept;
	conn->Connect = crSDPDoConnect;
	conn->Disconnect = crSDPDoDisconnect;
	conn->InstantReclaim = crSDPInstantReclaim;
	conn->HandleNewMessage = crSDPHandleNewMessage;
	conn->index = cr_sdp.num_conns;
	conn->sizeof_buffer_header = sizeof( CRSDPBuffer );
	conn->actual_network = 1;
	conn->krecv_buf_size = 0;
  
	/* Find a free slot */
	for (i = 0; i < cr_sdp.num_conns; i++) {
		if (cr_sdp.conns[i] == NULL) {
			conn->index = i;
			cr_sdp.conns[i] = conn;
			found = 1;
			break;
		}
	}
	
	/* Realloc connection stack if we couldn't find a free slot */
	if (found == 0) {
		n_bytes = ( cr_sdp.num_conns + 1 ) * sizeof(*cr_sdp.conns);
		crRealloc( (void **) &cr_sdp.conns, n_bytes );
		cr_sdp.conns[cr_sdp.num_conns++] = conn;
	}
}

CRConnection** crSDPDump( int *num )
{
	*num = cr_sdp.num_conns;
  
	return cr_sdp.conns;
}
