/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#ifdef WINDOWS
#pragma warning( push, 3 )
#endif
#include <gm.h>
#ifdef WINDOWS
#pragma warning( pop )
#endif
#ifndef WINDOWS
#include <ctype.h> /* for tolower */
#endif

#include "cr_error.h"
#include "cr_mem.h"
#include "cr_string.h"
#include "cr_bufpool.h"
#include "cr_net.h"
#include "cr_protocol.h"
#include "cr_string.h"
#include "net_internals.h"

#define CR_GM_USE_CREDITS 1
#define CR_GM_SEND_CREDITS_THRESHOLD ( 1 << 18 )

/* Versions of GM prior to 1.4pre37 have a bug involving short sends
 * that straddle a page boundary.  Set CR_GM_BROKEN_SHORT_SENDS to
 * 1 to enable a work-around. */

#define CR_GM_BROKEN_SHORT_SENDS 0

#if CR_GM_BROKEN_SHORT_SENDS
#define CR_GM_PAGE_SIZE     4096
#define CR_GM_PAGE_ALIGN(x) ((void *) (((unsigned long) (x) + CR_GM_PAGE_SIZE - 1) & ~(CR_GM_PAGE_SIZE-1)) )
#define CR_GM_BUFFER_SPANS_PAGE_BOUNDARY(x,l)	(((unsigned long) (x) & (CR_GM_PAGE_SIZE-1)) + (l) > CR_GM_PAGE_SIZE)
#endif

#ifndef _GM_NTOH
#define gm_ntoh_u32(x)	ntohl(x)
#define gm_ntoh_u16(x)	ntohs(x)
#define gm_ntoh_u8(x)	(x)
#endif

#define CR_GM_BUFFER_SEND_MAGIC 0x95718def
#define CR_GM_BUFFER_RECV_MAGIC 0x97518abc
#define CR_GM_SECONDS_PER_TICK  ( 5e-7 )

int crGmRecv();

typedef	enum { 
	CRGmMemoryPinned,
	CRGmMemoryUnpinned,
	CRGmMemoryBig 
} CRGmBufferKind;

typedef struct CRGmBuffer {
	unsigned int       magic;
	CRGmBufferKind kind;
	unsigned int       len;
	unsigned int       pad;
} CRGmBuffer;


typedef struct CRGmConnection {
	unsigned int       		   node_id;
	unsigned int       		   port_num;
	CRConnection   		  *conn;
	struct CRGmConnection *hash_next;
	struct CRGmConnection *credit_prev;
	struct CRGmConnection *credit_next;
} CRGmConnection;

/* Use a power of 2 to the lookup is just a mask -- this works fine
 * because we expect the gm node numbering to be dense */
#define CR_GM_CONN_HASH_SIZE	64

static struct {
	unsigned int          initialized;
	struct gm_port       *port;
	unsigned int          node_id;
	unsigned int          port_num;
	unsigned int          num_nodes;
	CRGmConnection       *gm_conn_hash[CR_GM_CONN_HASH_SIZE];
	CRGmConnection       *credit_head;
	CRGmConnection       *credit_tail;
	unsigned int          message_size;
	CRNetReceiveFunc      recv;
	CRNetCloseFunc        close;
	CRBufferPool          read_pool;
	CRBufferPool          write_pool;
	CRBufferPool          unpinned_read_pool;
	unsigned int          num_outstanding_sends;
	unsigned int          num_send_tokens;
	unsigned int          inside_recv;
} cr_gm;

#define CR_GM_PINNED_READ_MEMORY_SIZE   ( 8 << 20 )
#define CR_GM_PINNED_WRITE_MEMORY_SIZE  ( 8 << 20 )

#define CR_GM_API_VERSION          GM_API_VERSION_1_1
#define CR_GM_PRIORITY             GM_LOW_PRIORITY

#define CR_GM_ROUND_UP             1

#define CR_GM_CREDITS_DEBUG 	   0

#define CR_GM_DEBUG                0
#define CR_GM_SYNCHRONOUS          0

void crGmSend( CRConnection *conn, void **bufp, 
		void *start, unsigned int len );

#if CR_GM_DEBUG || CR_GM_CREDITS_DEBUG

static void cr_gm_debug( const char *format, ... )
{
	char txt[8192];
	va_list args;
	va_start( args, format );
	vsprintf( txt, format, args );
	va_end( args );

	crWarning( "%d%*cGM: %s", cr_gm.inside_recv,
			cr_gm.inside_recv * 2, ' ', txt );
}

#endif

static void cr_gm_info( unsigned int mtu )
{
	char name[ GM_MAX_HOST_NAME_LEN ];
	unsigned int node_id, max_node_id;
	unsigned int min_size;

	gm_get_host_name( cr_gm.port, name );
	gm_get_node_id( cr_gm.port, &node_id );
	crWarning( "GM: name=\"%s\" id=%u port=%u",
			name, node_id, cr_gm.port_num );

	gm_max_node_id_inuse( cr_gm.port, &max_node_id );
	crWarning( "GM: gm_max_node_id_inuse=%u",
			max_node_id );

	min_size = gm_min_size_for_length( mtu );
	crWarning( "GM: gm_min_size_for_length( "
			"mtu=%u ) = %u", mtu, min_size );

	crWarning( "GM: gm_max_length_for_size( "
			"size=%u ) = %u", min_size,
			(unsigned int) gm_max_length_for_size( min_size ) );

	crWarning( "GM: gm_min_message_size() = %u",
			(unsigned int) gm_min_message_size( cr_gm.port ) );

	crWarning( "GM: num_send_tokens=%u "
			"num_receive_tokens=%u",
			(unsigned int) gm_num_send_tokens( cr_gm.port ),
			(unsigned int) gm_num_receive_tokens( cr_gm.port ) );
}

#if CR_GM_DEBUG

static const char * cr_gm_str_event_type( gm_recv_event_t *event )
{
	const char *description;

#define CASE(error) case error : description = # error; break

	switch ( GM_RECV_EVENT_TYPE(event) )
	{
		CASE(GM_NO_RECV_EVENT);
		CASE(GM_ALARM_EVENT);
		CASE(GM_SENT_EVENT);
		CASE(_GM_SLEEP_EVENT);
		CASE(GM_RAW_RECV_EVENT);
		CASE(GM_RECV_EVENT);
		CASE(GM_HIGH_RECV_EVENT);
		CASE(GM_PEER_RECV_EVENT);
		CASE(GM_HIGH_PEER_RECV_EVENT);
		CASE(_GM_DIRECTED_SEND_NOTIFICATION_EVENT);
		CASE(_GM_FLUSHED_ALARM_EVENT);
		CASE(GM_SENT_TOKENS_EVENT);
		CASE(GM_IGNORE_RECV_EVENT);
		CASE(GM_FAST_RECV_EVENT);
		CASE(GM_FAST_HIGH_RECV_EVENT);
		CASE(GM_FAST_PEER_RECV_EVENT);
		CASE(GM_FAST_HIGH_PEER_RECV_EVENT);
		CASE(GM_SENDS_FAILED_EVENT);
		CASE(GM_BAD_SEND_DETECTED_EVENT);
		CASE(GM_SEND_TOKEN_VIOLATION_EVENT);
		CASE(GM_RECV_TOKEN_VIOLATION_EVENT);
		CASE(GM_BAD_RECV_TOKEN_EVENT);
		CASE(GM_ALARM_VIOLATION_EVENT);
		CASE(GM_REJECTED_SEND_EVENT);
		CASE(GM_ORPHANED_SEND_EVENT);
		CASE(GM_BAD_RESEND_DETECTED_EVENT);
		CASE(GM_DROPPED_SEND_EVENT);
		CASE(GM_BAD_SEND_VMA_EVENT);
		CASE(GM_BAD_RECV_VMA_EVENT);
		default:
		description = "unknown type";
	}

#undef CASE

	return description;
}

static void cr_gm_check_recv_event( gm_recv_event_t *event )
{
#if 0
	crWarning( "GM: gm_receive( type=%u )",
			GM_RECV_EVENT_TYPE(event) );
#endif

	switch ( GM_RECV_EVENT_TYPE(event) )
	{
		case GM_NO_RECV_EVENT:
		case GM_ALARM_EVENT:
		case GM_SENT_EVENT:
		case _GM_SLEEP_EVENT:
		case GM_RAW_RECV_EVENT:
		case GM_RECV_EVENT:
		case GM_HIGH_RECV_EVENT:
		case GM_PEER_RECV_EVENT:
		case GM_HIGH_PEER_RECV_EVENT:
		case _GM_DIRECTED_SEND_NOTIFICATION_EVENT:
		case _GM_FLUSHED_ALARM_EVENT:
		case GM_SENT_TOKENS_EVENT:
		case GM_IGNORE_RECV_EVENT:
			/* ignore */
			break;

		case GM_FAST_RECV_EVENT:
		case GM_FAST_HIGH_RECV_EVENT:
		case GM_FAST_PEER_RECV_EVENT:
		case GM_FAST_HIGH_PEER_RECV_EVENT:
#if 1
			crWarning( "GM fast event (type=%u)",
					GM_RECV_EVENT_TYPE(event) );
#endif
			break;

		case GM_SENDS_FAILED_EVENT:
		case GM_BAD_SEND_DETECTED_EVENT:
		case GM_SEND_TOKEN_VIOLATION_EVENT:
		case GM_RECV_TOKEN_VIOLATION_EVENT:
		case GM_BAD_RECV_TOKEN_EVENT:
		case GM_ALARM_VIOLATION_EVENT:
		case GM_REJECTED_SEND_EVENT:
		case GM_ORPHANED_SEND_EVENT:
		case GM_BAD_RESEND_DETECTED_EVENT:
		case GM_DROPPED_SEND_EVENT:
		case GM_BAD_SEND_VMA_EVENT:
		case GM_BAD_RECV_VMA_EVENT:
			crWarning( "GM error event (type=%u)",
					event->recv.type );
			break;

		default:
			crWarning( "GM unknown event (type=%u)",
					event->recv.type );
			break;
	}
}

#endif

static const char * cr_gm_str_error( gm_status_t status )
{
	const char *description;

#define CASE(error, desc) case error : description = desc; break

	switch ( status )
	{
		CASE (GM_SUCCESS, "success");
		CASE (GM_FAILURE, "unqualified error");
		CASE (GM_INPUT_BUFFER_TOO_SMALL, "input buffer is too small");
		CASE (GM_OUTPUT_BUFFER_TOO_SMALL, "output buffer is too small");
		CASE (GM_TRY_AGAIN, "try again");
		CASE (GM_BUSY, "busy");
		CASE (GM_MEMORY_FAULT, "memory fault");
		CASE (GM_INTERRUPTED, "interrupted");
		CASE (GM_INVALID_PARAMETER, "invalid parameter");
		CASE (GM_OUT_OF_MEMORY, "out of memory");
		CASE (GM_INVALID_COMMAND, "invalid command");
		CASE (GM_PERMISSION_DENIED, "permission denied");
		CASE (GM_INTERNAL_ERROR, "internal error");
		CASE (GM_UNATTACHED, "unattached");
		CASE (GM_UNSUPPORTED_DEVICE, "unsupported device");
		CASE (GM_SEND_TIMED_OUT, "send timed out");
		CASE (GM_SEND_REJECTED, "send was rejected by receiver");
		CASE (GM_SEND_TARGET_PORT_CLOSED, "target port was closed");
		CASE (GM_SEND_TARGET_NODE_UNREACHABLE, "target node was unreachable");
		CASE (GM_SEND_DROPPED, "send dropped as requested by the client");
		CASE (GM_SEND_PORT_CLOSED, "send port was closed");
		CASE (GM_NODE_ID_NOT_YET_SET,
				"node ID not set (node has not been configured by the mapper)");
		CASE (GM_STILL_SHUTTING_DOWN,
				"port is still shutting down from previous open");
		CASE (GM_CLONE_BUSY, "clone device is busy");
		default:
		description = "unknown error";
	}

#undef CASE

	return description;
}

static double cr_gm_clock( void )
{
#if defined(WINDOWS)
	/* Windows can't convert u64 -> double, only s64 */
	gm_s64_t ticks = (gm_s64_t) gm_ticks( cr_gm.port );
#else
	gm_u64_t ticks = gm_ticks( cr_gm.port );
#endif
	return CR_GM_SECONDS_PER_TICK * (double) ticks;
}

static void * cr_gm_dma_malloc( unsigned long length )
{
	void *buf;

	buf = gm_dma_malloc( cr_gm.port, length );
	if ( buf == NULL )
	{
		crError( "gm_dma_malloc( port=%p, len=%ld ) : failed",
				cr_gm.port, length );
	}

#if CR_GM_DEBUG
	crWarning( "GM: dma_malloc, length=%d "
			"range=%p:%p", length, buf,
			(unsigned char*) buf + length );
#endif

	return buf;
}

/* All the functions that shouldn't get called */

void crGmSendExact( CRConnection *conn, void *buf, unsigned int len )
{
	crError( "crGmSendExact shouldn't ever get called." );
	(void) conn;
	(void) buf;
	(void) len;
}

void crGmBogusRecv( CRConnection *conn, void *buf, unsigned int len )
{
	crError( "crGmSingleRecv shouldn't ever get called." );
	(void) conn;
	(void) buf;
	(void) len;
}

static void crGmCreditIncrease( CRGmConnection *gm_conn )
{
	CRGmConnection *parent;
	int my_credits;

	/* move this node up the doubly-linked, if it isn't already in the
	 * right position */

	parent = gm_conn->credit_prev;
	my_credits = gm_conn->conn->recv_credits;
	if ( parent && parent->conn->recv_credits < my_credits )
	{
		/* we can't be at the head of the list, because our parent is
		 * non-NULL */

		/* are we the tail of the list? */
		if ( cr_gm.credit_tail == gm_conn )
		{
			/* yes, pull up the tail pointer, and fix our parent to be tail */
			cr_gm.credit_tail = parent;
			parent->credit_next = NULL;
		}
		else
		{
			/* no, just link around us */
			parent->credit_next = gm_conn->credit_next;
			gm_conn->credit_next->credit_prev = parent;
		}

		/* walk up the list until we find a guy with more credits than
		 * us or we reach the head */
		while ( parent && parent->conn->recv_credits < my_credits )
		{
			parent = parent->credit_prev;
		}

		/* reinsert us -- we know there is somebody else in the list
		 * (and thus we won't be the tail, because we moved up),
		 * otherwise we would have been the head, and never bothered
		 * trying to move ourself */
		if ( parent )
		{
			CRGmConnection *child = parent->credit_next;
			gm_conn->credit_next = child;
			child->credit_prev = gm_conn;

			parent->credit_next = gm_conn;
			gm_conn->credit_prev = parent;
		}
		else
		{
			CRGmConnection *child = cr_gm.credit_head;
			gm_conn->credit_next = child;
			child->credit_prev = gm_conn;

			cr_gm.credit_head = gm_conn;
			gm_conn->credit_prev = NULL;
		}
	}
}

	static void
crGmCreditZero( CRGmConnection *gm_conn )
{
	CRASSERT( cr_gm.credit_head == gm_conn );

	/* if we aren't already at the tail of the list, */
	if ( cr_gm.credit_tail != gm_conn )
	{
		/* then pull us off the head, */
		cr_gm.credit_head = gm_conn->credit_next;
		cr_gm.credit_head->credit_prev = NULL;

		/* and put us on the tail */
		cr_gm.credit_tail->credit_next = gm_conn;
		gm_conn->credit_prev = cr_gm.credit_tail;
		gm_conn->credit_next = NULL;

		cr_gm.credit_tail = gm_conn;
	}
}
#define CR_GM_HASH(n,p)	cr_gm.gm_conn_hash[ (n) & (CR_GM_CONN_HASH_SIZE-1) ]

static void crGmConnectionAdd( CRConnection *conn )
{
	CRGmConnection *gm_conn, **bucket;

	bucket = &CR_GM_HASH( conn->gm_node_id, conn->gm_port_id );

	for ( gm_conn = *bucket; gm_conn != NULL; gm_conn = gm_conn->hash_next )
	{
		if ( gm_conn->node_id  == conn->gm_node_id && 
				gm_conn->port_num == conn->gm_port_num )
		{
			crError( "GM: I've already got a connection from node=%u "
					"port=%u (\"%s\"), so why is it connecting again?",
					conn->gm_node_id, conn->gm_port_num, conn->hostname );
		}
	}

	gm_conn = (CRGmConnection*)crAlloc( sizeof(*gm_conn) );
	gm_conn->node_id   = conn->gm_node_id;
	gm_conn->port_num  = conn->gm_port_num;
	gm_conn->conn      = conn;
	gm_conn->hash_next = *bucket;

	*bucket = gm_conn;

	gm_conn->credit_next = NULL;
	if ( cr_gm.credit_head )
	{
		gm_conn->credit_prev  = cr_gm.credit_tail;
		cr_gm.credit_tail->credit_next = gm_conn;
		cr_gm.credit_tail = gm_conn;
	}
	else
	{
		gm_conn->credit_prev  = NULL;
		cr_gm.credit_head = gm_conn;
		cr_gm.credit_tail = gm_conn;
	}

	/* we're on the tail of the list now, so we might need to move
	 * up the list, we certainly can't move down it */
	crGmCreditIncrease( gm_conn );
}


void crGmAccept( CRConnection *conn, unsigned short port )
{
	CRConnection *mother;
	char response[8096];
  char my_hostname[256];
	crWarning( "crGmAccept is being called -- brokering the connection through the mothership!." );

	mother = __copy_of_crMothershipConnect( );

	if ( crGetHostname( my_hostname, sizeof( my_hostname ) ) )
	{
		crError( "Couldn't determine my own hostname in crGmAccept!" );
	}
	
	/* Tell the mothership I'm willing to receive a client, and what my GM info is */
	if (!__copy_of_crMothershipSendString( mother, response, "acceptrequest gm %s %d %d %d %d", my_hostname, conn->port, cr_gm.node_id, cr_gm.port_num, conn->endianness ) )
	{
		crError( "Mothership didn't like my accept request request" );
	}

	/* The response will contain the GM information for the guy who accepted 
	 * this connection.  The mothership will sit on the acceptrequest 
	 * until someone connects. */
	
	sscanf( response, "%d %d", &(conn->gm_node_id), &(conn->gm_port_num) );
	
	/* NOW, we can add the connection, since we have enough information 
	 * to uniquely determine the sender when we get a packet! */
	crGmConnectionAdd( conn );

	__copy_of_crMothershipDisconnect( mother );
	
	(void) port;
}

int crGmDoConnect( CRConnection *conn )
{
	CRConnection *mother;
	char response[8096];
	int remote_endianness;
	crWarning( "crGmDoConnect is being called -- brokering the connection through the mothership!." );

	mother = __copy_of_crMothershipConnect( );

	/* Tell the mothership who I want to connect to, and what my GM info is */
	if (!__copy_of_crMothershipSendString( mother, response, "connectrequest %s %d %d %d %d", conn->hostname, conn->port, cr_gm.node_id, cr_gm.port_num, conn->endianness ) )
	{
		crError( "Mothership didn't like my connect request request" );
	}

	/* The response will contain the GM information for the guy who accepted 
	 * this connection.  The mothership will sit on the connectrequest 
	 * until someone accepts. */
	
	sscanf( response, "%d %d %d", &(conn->gm_node_id), &(conn->gm_port_num), &(remote_endianness) );

	if (remote_endianness != conn->endianness)
	{
		conn->swap = 1;
	}

	/* NOW, we can add the connection, since we have enough information 
	 * to uniquely determine the sender when we get a packet! */
	crGmConnectionAdd( conn );

	__copy_of_crMothershipDisconnect( mother );
	
	return 1;
}

void crGmDoDisconnect( CRConnection *conn )
{
	crError( "crGmDoDisconnect shouldn't ever get called." );
	(void) conn;
}



	static __inline CRGmConnection *
crGmConnectionLookup( unsigned int node_id, unsigned int port_num )
{
	CRGmConnection *gm_conn;

	CRASSERT( node_id < cr_gm.num_nodes );

	gm_conn = CR_GM_HASH( node_id, port_id );
	while ( gm_conn )
	{
		if ( gm_conn->node_id == node_id && gm_conn->port_num == port_num )
		{
			return gm_conn;
		}
		gm_conn = gm_conn->hash_next;
	}

	crError( "GM: lookup on unknown source: node=%u port=%u",
			node_id, port_num );

	/* unreached */
	return NULL;
}


void *crGmAlloc( CRConnection *conn )
{
	CRGmBuffer *gm_buffer = (CRGmBuffer *)
		crBufferPoolPop( &cr_gm.write_pool );
	while ( gm_buffer == NULL )
	{
		if ( cr_gm.num_outstanding_sends == 0 )
		{
			crError( "crGmAlloc: no available buffers, "
					"and none outstanding." );
		}
		crGmRecv( );
		gm_buffer = (CRGmBuffer *)
			crBufferPoolPop( &cr_gm.write_pool );
	}

	return (void *)( gm_buffer + 1 );
}

	static void
cr_gm_provide_receive_buffer( void *buf )
{
	CRMessage *msg = (CRMessage *) buf;

	CRASSERT( ((CRGmBuffer *) msg - 1)->magic 
			== CR_GM_BUFFER_RECV_MAGIC );

	msg->type = CR_MESSAGE_ERROR;

	gm_provide_receive_buffer( cr_gm.port, buf,
			cr_gm.message_size,
			CR_GM_PRIORITY );
}

void crGmHandleNewMessage( CRConnection *conn, CRMessage *msg,
		unsigned int len )
{
	CRGmBuffer *gm_buffer = ((CRGmBuffer *) msg) - 1;

	/* build a header so we can delete the message later */
	gm_buffer->magic = CR_GM_BUFFER_RECV_MAGIC;
	gm_buffer->kind  = CRGmMemoryBig;
	gm_buffer->len   = len;
	gm_buffer->pad   = 0;

	if (!cr_gm.recv( conn, msg, len ))
	{
		crNetDefaultRecv( conn, msg, len );
	}
}

static void
crGmRecvOther( CRGmConnection *gm_conn, CRMessage *msg,
		unsigned int len )
{
	CRGmBuffer *temp;

	temp = (CRGmBuffer *) crBufferPoolPop( &cr_gm.read_pool );

	if ( temp == NULL )
	{
#if CR_GM_DEBUG
		cr_gm_debug( "crGmRecv: ran out of pinned memory, "
				"copying to unpinned" );
#endif

		/* we're out of pinned buffers, copy this message into
		 * an unpinned buffer so we can jam the just received
		 * buffer back in the hardware */
		temp = (CRGmBuffer *) 
			crBufferPoolPop( &cr_gm.unpinned_read_pool );
		if ( temp == NULL )
		{
			temp = (CRGmBuffer*)crAlloc( sizeof(CRGmBuffer) + gm_conn->conn->mtu );
			temp->magic = CR_GM_BUFFER_RECV_MAGIC;
			temp->kind  = CRGmMemoryUnpinned;
			temp->pad   = 0;
		}
		temp->len = len;
		memcpy( temp+1, msg, len );

		cr_gm_provide_receive_buffer( msg );

		if (!cr_gm.recv( gm_conn->conn, temp+1, len ))
		{
			crNetDefaultRecv( gm_conn->conn, temp+1, len );
		}
	}
	else
	{
		cr_gm_provide_receive_buffer( temp + 1 );

		temp = (CRGmBuffer *) msg - 1;
		temp->len = len;

		if (!cr_gm.recv( gm_conn->conn, msg, len ))
		{
			crNetDefaultRecv( gm_conn->conn, msg, len );
		}
	}
}

	static void 
cr_gm_send_callback( struct gm_port *port, void *ctx, gm_status_t status )
{
	CRGmBuffer *buf = (CRGmBuffer *) ctx - 1;

	if ( status != GM_SUCCESS )
	{
		crError( "cr_gm_send_callback error: %s (%d)",
				cr_gm_str_error( status ), status );
	}

	CRASSERT( buf->magic == CR_GM_BUFFER_SEND_MAGIC );

#if CR_GM_DEBUG
	cr_gm_debug( "send callback, ctx=%p", ctx );
#endif

	CRASSERT( cr_gm.num_outstanding_sends > 0 );
	cr_gm.num_outstanding_sends--;
	cr_gm.num_send_tokens++;
	(void)(port);

	crBufferPoolPush( &cr_gm.write_pool, buf );
}

static void
cr_gm_send( CRConnection *conn, void *buf, unsigned int len,
		void *ctx )
{
#if CR_GM_DEBUG
	cr_gm_debug( "gm_send: dst=%u:%u buf=%p len=%d ctx=%p",
			conn->gm_node_id, conn->gm_port_num, buf, len, ctx );
#endif

	/* get a send token */
	while ( cr_gm.num_send_tokens == 0 )
	{
		crGmRecv( );
	}

#if CR_GM_BROKEN_SHORT_SENDS
	if ( len <= 256 && CR_GM_BUFFER_SPANS_PAGE_BOUNDARY(buf,len) )
	{
		/* This is a short send that straddles a page boundary.  Some
		 * versions of GM (prior to gm-1.4pre37) will not handle the
		 * page crossing correctly in some cases.  Move the data
		 * (within the same buffer) so that it doesn't cross a page
		 * boundary. */
		void *temp = ctx;
		if ( CR_GM_BUFFER_SPANS_PAGE_BOUNDARY(temp,len) )
		{
			temp = CR_GM_PAGE_ALIGN(temp);
		}
#if 0
		crWarning( "GM: copying len=%u send from "
				"start=%p to %p in ctx=%p", len, buf, temp, ctx );
#endif

		/* the source and destination may overlap, but memmove handles
			 that cases correctly */
		memmove( temp, buf, len );
		buf = temp;
	}
#endif

	cr_gm.num_outstanding_sends++;
	cr_gm.num_send_tokens--;

	if ( conn->gm_port_num == cr_gm.port_num )
	{
		/* only when both are on the same port... */
		gm_send_to_peer_with_callback( cr_gm.port, buf,
				cr_gm.message_size, len,
				CR_GM_PRIORITY, conn->gm_node_id,
				cr_gm_send_callback, ctx );
	}
	else
	{
		gm_send_with_callback( cr_gm.port, buf,
				cr_gm.message_size, len,
				CR_GM_PRIORITY, conn->gm_node_id,
				conn->gm_port_num,
				cr_gm_send_callback, ctx );
	}

#if CR_GM_SYNCHRONOUS
	/* force synchronous sends */
	while ( cr_gm.num_outstanding_sends > 0 )
		crGmRecv( );
#endif

#if CR_GM_DEBUG
	cr_gm_debug( "send complete" );
#endif
}

	static void
crGmSendCredits( CRConnection *conn )
{
	CRGmBuffer *gm_buffer;
	CRMessageFlowControl *msg;

	CRASSERT( cr_gm.num_send_tokens > 0 );
	CRASSERT( conn->recv_credits > 0 );

	gm_buffer = (CRGmBuffer *)
		crBufferPoolPop( &cr_gm.write_pool );

	CRASSERT( gm_buffer );

	msg = (CRMessageFlowControl *) ( gm_buffer + 1 );

	msg->type    = CR_MESSAGE_FLOW_CONTROL;
	msg->credits = conn->recv_credits;

#if CR_GM_CREDITS_DEBUG
	cr_gm_debug( "sending %d credits to host %s",
			conn->recv_credits, conn->hostname );
#endif

	conn->recv_credits = 0;

	cr_gm_send( conn, msg, sizeof(*msg), msg );
}

	static void
crGmMaybeSendCredits( void )
{
	if ( cr_gm.num_send_tokens == 0 || cr_gm.write_pool.num == 0 )
		return;

	if ( cr_gm.credit_head && 
			cr_gm.credit_head->conn->recv_credits >= 
			CR_GM_SEND_CREDITS_THRESHOLD )
	{
		crGmSendCredits( cr_gm.credit_head->conn );
		crGmCreditZero( cr_gm.credit_head );
	}
}

int crGmRecv( void )
{
	gm_recv_event_t *event;

	/* MWE: Note that we are inside a recv so we can catch the error
		 of calling crGmSend or crGmFlowControl within a recv
		 callback.  This prevents the observed problem of the send
		 function checking for any pending recvs and having those
		 processed as well, and a resulting recv->send->recv recursion
		 that eventually explodes. */
	cr_gm.inside_recv++;

#if CR_GM_USE_CREDITS
#if CR_GM_SYNCHRONOUS_XXX
	/* HACK - don't do this for now, it changes the bug we are trying
		 to find... */
	/* if we are doing synchronous sends, only ever send credits if we
		 don't already have an outstanding send */
	if ( cr_gm.num_outstanding_sends == 0 )
	{
		crGmMaybeSendCredits( );
	}
#else
	crGmMaybeSendCredits( );
#endif
#endif

	event = gm_receive( cr_gm.port );

	if ( GM_RECV_EVENT_TYPE(event) == GM_NO_RECV_EVENT )
	{
		cr_gm.inside_recv--;
		return 0;
	}

#if CR_GM_DEBUG
	/* cr_gm_check_recv_event( event ); */
#endif

	switch ( GM_RECV_EVENT_TYPE(event) ) 
	{
		case GM_RECV_EVENT:
		case GM_HIGH_RECV_EVENT:
		case GM_PEER_RECV_EVENT:
		case GM_HIGH_PEER_RECV_EVENT:
			{
				gm_u32_t len = gm_ntoh_u32( event->recv.length );
				CRMessage *msg = (CRMessage *)
					gm_ntohp( event->recv.buffer );
				gm_u16_t src_node = gm_ntoh_u16( event->recv.sender_node_id );
				gm_u8_t  src_port = gm_ntoh_u8( event->recv.sender_port_id );
				CRGmConnection *gm_conn = 
					crGmConnectionLookup( src_node, src_port );

#if CR_GM_DEBUG
				cr_gm_debug( "gm_receive: src=%d:%d buf=%p len=%u (%s)",
						src_node, src_port, msg, len, 
						cr_gm_str_event_type( event ) );
#endif

				crGmRecvOther( gm_conn, msg, len );
			}
			break;

		case GM_FAST_RECV_EVENT:
		case GM_FAST_HIGH_RECV_EVENT:
		case GM_FAST_PEER_RECV_EVENT:
		case GM_FAST_HIGH_PEER_RECV_EVENT:
			/* can't handle these yet, I don't know if we need to call
			 * gm_provide_receive_buffer() after handling these or not */

		default:
#if CR_GM_DEBUG
			cr_gm_debug( "gm_unknown: %s", cr_gm_str_event_type( event ) );
#endif
			gm_unknown( cr_gm.port, event );
	}

	cr_gm.inside_recv--;

	return 1;
}

	static void
crGmWaitForSendCredits( CRConnection *conn )
{
	double start, elapsed;

	start = cr_gm_clock( );
	do
	{
		crGmRecv( );
		elapsed = cr_gm_clock( ) - start;
	}
	while ( conn->send_credits <= 0 && elapsed < 1.0 );

	if ( conn->send_credits <= 0 )
	{
		crWarning( "GM: waiting for credits to "
				"talk to \"%s\"", conn->hostname );

		while ( conn->send_credits <= 0 )
		{
			crGmRecv( );
		}

		elapsed = cr_gm_clock( ) - start;

		crWarning( "GM: waited %.1f seconds for "
				"credits to talk to \"%s\"", elapsed, conn->hostname );
	}
}

	static void
crGmSendMulti( CRConnection *conn, void *buf, unsigned int len )
{
	unsigned char *src;
	CRASSERT( buf != NULL && len > 0 );

	if ( len <= conn->mtu )
	{
		/* the user is doing a send from memory not allocated by the
		 * network layer, but it does fit within a single message, so
		 * don't bother with fragmentation */
		void *pack = crGmAlloc( conn );
		memcpy( pack, buf, len );
		crGmSend( conn, &pack, pack, len );
		return;
	}

#if CR_GM_USE_CREDITS
	/* first get some credits */
	if ( conn->send_credits <= 0 )
	{
		crGmWaitForSendCredits( conn );
	}

	conn->send_credits -= len;
#endif

	src = (unsigned char *) buf;
	while ( len > 0 )
	{
		CRMessageMulti *msg = (CRMessageMulti *) crGmAlloc( conn );
		unsigned int        n_bytes;

		if ( len + sizeof(*msg) > conn->mtu )
		{
			msg->type = CR_MESSAGE_MULTI_BODY;
			n_bytes   = conn->mtu - sizeof(*msg);
		}
		else
		{
			msg->type = CR_MESSAGE_MULTI_TAIL;
			n_bytes   = len;
		}
		memcpy( msg + 1, src, n_bytes );

		cr_gm_send( conn, msg, n_bytes + sizeof(*msg), msg );

		src += n_bytes;
		len -= n_bytes;
	}
}

void crGmSend( CRConnection *conn, void **bufp, 
		void *start, unsigned int len )
{
	CRGmBuffer *gm_buffer;

	if ( cr_gm.inside_recv )
	{
		crError( "crGmSend: can not be called within crGmRecv" );
	}

	if ( bufp == NULL )
	{
		crGmSendMulti( conn, start, len );
		return;
	}

	gm_buffer = (CRGmBuffer *) (*bufp) - 1;

	CRASSERT( gm_buffer->magic == CR_GM_BUFFER_SEND_MAGIC );

#if CR_GM_USE_CREDITS && CR_GM_CREDITS_DEBUG
	if ( conn->send_credits <= 0 )
	{
		cr_gm_debug( "need %d credits to talk to host %s, only have %d",
				len, conn->hostname, conn->send_credits );
	}
#endif

	/* Every time the client calls send we try to be polite and grab
	 * any pending GM messages.  We also have to block if we don't
	 * have any credits. */
	while ( crGmRecv( ) )
		;

#if CR_GM_USE_CREDITS
	if ( conn->send_credits <= 0 )
	{
		crGmWaitForSendCredits( conn );
	}

	conn->send_credits -= len;
#endif

#if CR_GM_CREDITS_DEBUG
	cr_gm_debug( "sending a len=%d message to host=%s, have %d credits "
			"remaining", len, conn->hostname, conn->send_credits );
#endif

	cr_gm_send( conn, start, len, *bufp );

	*bufp = NULL;
}

void crGmInstantReclaim( CRConnection *conn, CRMessage *msg )
{
	CRGmBuffer *gm_buffer = (CRGmBuffer *) msg - 1;

	CRASSERT( gm_buffer->magic == CR_GM_BUFFER_RECV_MAGIC );

	switch ( gm_buffer->kind )
	{
		case CRGmMemoryPinned:
			cr_gm_provide_receive_buffer( msg );
			break;

		case CRGmMemoryUnpinned:
		case CRGmMemoryBig:
			crFree( gm_buffer );
			break;

		default:
			crError( "Bad buffer kind in crGmInstantReclaim: %d", gm_buffer->kind );
			break;
	}
}

	void
crGmFree( CRConnection *conn, void *buf )
{
	CRGmBuffer *gm_buffer = (CRGmBuffer *) buf - 1;

	CRASSERT( gm_buffer->magic == CR_GM_BUFFER_RECV_MAGIC );
	conn->recv_credits += gm_buffer->len;

	switch ( gm_buffer->kind )
	{
		case CRGmMemoryPinned:
			crBufferPoolPush( &cr_gm.read_pool, gm_buffer );
			break;

		case CRGmMemoryUnpinned:
			crBufferPoolPush( &cr_gm.unpinned_read_pool, gm_buffer );
			break;

		case CRGmMemoryBig:
			crFree( gm_buffer );
			break;

		default:
			crError( "Bad buffer kind in crGmFree: %d", gm_buffer->kind );
			break;
	}

	crGmCreditIncrease( crGmConnectionLookup( conn->gm_node_id, 
				conn->gm_port_num ) );
}

#if 0
	static int 
cr_gm_looks_like_same_host( const char *a, const char *b )
{
	CRASSERT( a && b );
	while ( *a && *b ) {
		if ( tolower(*a) != tolower(*b) )
			return 0;
		a++;
		b++;
	}

	/* they're the same if the names match, or if one is a DNS prefix
		 of the other */
	return ( ( *a == '\0' && *b == '\0' ) ||
			( *a == '.'  && *b == '\0' ) ||
			( *a == '\0' && *b == '.'  ) );
}
#endif

	static void
cr_gm_set_acceptable_sizes( void )
{
	gm_status_t      status;
	enum gm_priority priority;
	unsigned long    mask;

	for ( priority = 0; priority < GM_NUM_PRIORITIES; priority++ )
	{
		mask = 0;
		if ( priority == CR_GM_PRIORITY )
			mask = 1 << cr_gm.message_size;

		status = gm_set_acceptable_sizes( cr_gm.port, priority, mask );
		if ( status != GM_SUCCESS )
		{
			crError( "GM: gm_set_acceptable_sizes( port=%u, pri=%u, "
					"mask=%ld ) failed: %s (%d)", cr_gm.port_num,
					priority, mask, 
					cr_gm_str_error( status ), status );
		}
	}
}

void crGmInit( CRNetReceiveFunc recvFunc, CRNetCloseFunc closeFunc, unsigned int mtu )
{
	gm_status_t status;
	unsigned int port, min_port, max_port;
	unsigned int node_id, max_node_id, i;
	unsigned int stride, count, n_bytes, num_recv_tokens;
	unsigned char *mem;

	if ( cr_gm.initialized )
	{
		CRASSERT( cr_gm.recv == recvFunc );
		CRASSERT( cr_gm.close == closeFunc );
		return;
	}

	crWarning( "GM: initializing" );

	status = gm_init( );
	if ( status != GM_SUCCESS )
	{
		crError( "GM: gm_init() failed: %s (%d)",
				cr_gm_str_error( status ), status );
	}

	/* open a reasonable port */
	min_port = 4;
	max_port = gm_num_ports( NULL /* known unused */ ) - 1;
	for ( port = min_port; port <= max_port; port++ )
	{
		status = gm_open( &cr_gm.port, 0, port,
				"CR GM", CR_GM_API_VERSION );
		if ( status == GM_SUCCESS )
		{
			break;
		}
		if ( status != GM_BUSY )
		{
			crError( "GM: gm_open( port=%u ) failed: %s (%d)",
					port, cr_gm_str_error( status ), status );
		}
	}
	if ( port > max_port )
	{
		crError( "GM: all ports busy (tried port %u through %u)",
				min_port, max_port );
	}
	cr_gm.port_num = port;

	cr_gm.message_size = gm_min_size_for_length( mtu );

	cr_gm_set_acceptable_sizes( );

	cr_gm_info( mtu );

	gm_max_node_id_inuse( cr_gm.port, &max_node_id );

	cr_gm.num_nodes = max_node_id + 1;
	for ( i = 0; i < CR_GM_CONN_HASH_SIZE; i++ )
	{
		cr_gm.gm_conn_hash[i] = NULL;
	}

	cr_gm.credit_head = NULL;
	cr_gm.credit_tail = NULL;

	gm_get_node_id( cr_gm.port, &node_id );
	cr_gm.node_id = node_id;

	stride  = mtu;
#if CR_GM_ROUND_UP
	stride  = gm_max_length_for_size( gm_min_size_for_length( stride ) );
#endif
	stride += sizeof(CRGmBuffer);
	count   = CR_GM_PINNED_READ_MEMORY_SIZE / stride;
	n_bytes = count * stride;
	crWarning( "GM: read pool: bufsize=%u, "
			"%u buffers, %u bytes", stride, count, n_bytes );
	mem     = cr_gm_dma_malloc( n_bytes );

	num_recv_tokens = gm_num_receive_tokens( cr_gm.port );

	crBufferPoolInit( &cr_gm.read_pool, count );
	for ( i = 0; i < count; i++ )
	{
		CRGmBuffer *buf = (CRGmBuffer *) ( mem + i * stride );
		buf->magic = CR_GM_BUFFER_RECV_MAGIC;
		buf->kind  = CRGmMemoryPinned;
		buf->len   = mtu;
		buf->pad   = 0;
		if ( i < num_recv_tokens )
		{
			cr_gm_provide_receive_buffer( buf + 1 );
		}
		else
		{
			crBufferPoolPush( &cr_gm.read_pool, buf );
		}
	}

	crBufferPoolInit( &cr_gm.unpinned_read_pool, 16 );

	stride  = mtu;
#if CR_GM_ROUND_UP
	stride  = gm_max_length_for_size( gm_min_size_for_length( stride ) );
#endif
	stride += sizeof(CRGmBuffer);
	count   = CR_GM_PINNED_WRITE_MEMORY_SIZE / stride;
	n_bytes = count * stride;
	crWarning( "GM: write pool: bufsize=%u, "
			"%u buffers, %u bytes", stride, count, n_bytes );
	mem     = cr_gm_dma_malloc( n_bytes );

	crBufferPoolInit( &cr_gm.write_pool, count );
	for ( i = 0; i < count; i++ )
	{
		CRGmBuffer *buf = (CRGmBuffer *) ( mem + i * stride );
		buf->magic = CR_GM_BUFFER_SEND_MAGIC;
		buf->kind  = CRGmMemoryPinned;
		buf->len   = mtu;
		buf->pad   = 0;

		crBufferPoolPush( &cr_gm.write_pool, buf );
	}

	cr_gm.recv  = recvFunc;
	cr_gm.close = closeFunc;

	cr_gm.num_outstanding_sends = 0;
	cr_gm.num_send_tokens = gm_num_send_tokens( cr_gm.port );

	cr_gm.inside_recv = 0;

	cr_gm.initialized = 1;
}

unsigned int crGmNodeId( void )
{
	CRASSERT( cr_gm.initialized );
	return cr_gm.node_id;
}

unsigned int crGmPortNum( void )
{
	CRASSERT( cr_gm.initialized );
	return cr_gm.port_num;
}

void crGmConnection( CRConnection *conn )
{
	/* The #if 0'ed out code here did some sanity checking on 
	 * the integrity of the GM network.  This doesn't quite work 
	 * any more because we don't have an initial TCPIP handshake 
	 * to establish the GM node ID of the other party.  Perhaps 
	 * we can do this validation in GmAccept once the mothership 
	 * is brokering connections? */
#if 0
	char *actual_name;
#endif

	CRASSERT( cr_gm.initialized );

#if 0
	if ( conn->gm_node_id == GM_NO_SUCH_NODE_ID )
	{
		crError( "GM: there's no host called \"%s\"?",
				conn->hostname );
	}

	actual_name = gm_node_id_to_host_name( cr_gm.port, conn->gm_node_id );
	if ( !actual_name )
	{
		crError( "GM: gm_node_id_to_host_name( id=%u ) failed",
				conn->gm_node_id );
	}

	if ( conn->hostname && 
			!cr_gm_looks_like_same_host( conn->hostname, actual_name ) )
	{
		crError( "GM: \"%s\" says id=%u, but I think \"%s\" "
				"is on that ID", conn->hostname,
				conn->gm_node_id, actual_name );
	}
#endif


	conn->type  = CR_GM;
	conn->Alloc = crGmAlloc;
	conn->Send  = crGmSend;
	conn->SendExact = crGmSendExact;
	conn->Recv = crGmBogusRecv;
	conn->Free  = crGmFree;
	conn->InstantReclaim = crGmInstantReclaim;
	conn->HandleNewMessage = crGmHandleNewMessage;
	conn->Accept = crGmAccept;
	conn->Connect = crGmDoConnect;
	conn->Disconnect = crGmDoDisconnect;
	conn->sizeof_buffer_header = sizeof( CRGmBuffer );

#if 0
	crWarning( "GM: accepted connection from "
			"host=%s id=%d (gm_name=%s)", conn->hostname,
			conn->gm_node_id, actual_name );
#endif
}
