/* cpg - 3/14/02 */

#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "cr_error.h"
#include "cr_mem.h"
#include "cr_string.h"
#include "cr_bufpool.h"
#include "cr_net.h"
#include "cr_endian.h"
#include "net_internals.h"

#include "tcscomm.h"

#define CR_TCSCOMM_BUFFER_PAD 8 /* sizeof( CRTcscommBuffer * ) */
#define CR_TCSCOMM_BUFFER_MAGIC 0x23543198
#define CR_TCSCOMM_SEND_CREDITS_THRESHOLD ( 1 << 18 )
#define CR_TCSCOMM_INITIAL_RECV_CREDITS ( 1 << 21 )
#define CR_TCSCOMM_TIMEOUT 0
#define CR_TCSCOMM_TCPIP_PORT 7000
#define CR_TCSCOMM_CRSERVER_RANK 0

typedef struct CRTcscommConnection {
  CRConnection            *conn;
  struct CRTcscommConnection *credit_prev;
  struct CRTcscommConnection *credit_next;
} CRTcscommConnection;

typedef struct CRTcscommBuffer {
  unsigned int         magic;
  unsigned int         len;
  void                *buf;
} CRTcscommBuffer;

static struct {
  int                  initialized;
  int                  num_conns;
  CRNetReceiveFuncList *recv_list;
  CRNetCloseFuncList *close_list;
  CRTcscommConnection *credit_head;
  CRTcscommConnection *credit_tail;
  unsigned int         inside_recv;
  void                *tcscomm_conn;
  char                 my_hostname[256];
  int                  my_rank;
  int                  low_context;
  int                  high_context;
  char                *low_node;
  char                *high_node;
} cr_tcscomm;

/* Forward declarations */
void crTcscommFree( CRConnection *, void * );
int  crTcscommRecv( void );
void crTCPIPReadExact( CRSocket sock, void *buf, unsigned int len );
void crTCPIPWriteExact( CRConnection *conn, void *buf, unsigned int len );
void crTCPIPAccept( CRConnection *conn, char *hostname, unsigned short port );
int  crTCPIPDoConnect( CRConnection *conn );

int
crTcscommErrno( void )
{
  int err = errno;
  errno = 0;
  return err;
}

char *
crTcscommErrorString( int err )
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

static int
__read_exact( int tcscomm_id, void *buf, unsigned int len )
{
  int   num_read = 0;
  void *dst = (void *) buf;

  num_read = tcscomm_Recv( tcscomm_id, dst, len );
  if ( num_read != len )
    return 0;

/*   crDebug( "__read_exact(Tcscomm):  tcscomm_id=%d, type=%x, len=%d", */
/* 	   tcscomm_id, ((CRMessage *) dst)->header.type, len ); */

  return num_read;
}

void
crTcscommReadExact( int tcscomm_id, void *buf, unsigned int len )
{
  int retval = __read_exact( tcscomm_id, buf, len );
  if ( retval <= 0 ) {
    int err = crTcscommErrno( );
    crError( "crTcscommReadExact: %s", crTcscommErrorString( err ) );
  }
}

static int
__write_exact( int tcscomm_id, void *buf, unsigned int len )
{
  int   num_written = 0;
  void *src = (void *) buf;

/*   crDebug( "__write_exact(Tcscomm):  tcscomm_id=%d, type=%x, len=%d", */
/* 	   tcscomm_id, ((CRMessage *) src)->header.type, len ); */

  num_written = tcscomm_Send( tcscomm_id, src, len );
  if ( num_written != len )
    return 0;

  return num_written;
}

void
crTcscommWriteExact( CRConnection *conn, void *buf, unsigned int len )
{
  int retval = __write_exact( conn->tcscomm_id, buf, len );
  if ( retval <= 0 ) {
    int err = crTcscommErrno( );
    crError( "crTcscommWriteExact: %s", crTcscommErrorString( err ) );
  }
}

static void
crTcscommCreditIncrease( CRTcscommConnection *tcscomm_conn )
{
  CRTcscommConnection *parent;
  int               my_credits;

  /* move this node up the doubly-linked, if it isn't already in the
   * right position */

  parent = tcscomm_conn->credit_prev;
  my_credits = tcscomm_conn->conn->recv_credits;
  if ( parent && parent->conn->recv_credits < my_credits )
    {
      /* we can't be at the head of the list, because our parent is
       * non-NULL */

      /* are we the tail of the list? */
      if ( cr_tcscomm.credit_tail == tcscomm_conn )
	{
	  /* yes, pull up the tail pointer, and fix our parent to be tail */
	  cr_tcscomm.credit_tail = parent;
	  parent->credit_next = NULL;
	}
      else
	{
	  /* no, just link around us */
	  parent->credit_next = tcscomm_conn->credit_next;
	  tcscomm_conn->credit_next->credit_prev = parent;
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
	  CRTcscommConnection *child = parent->credit_next;
	  tcscomm_conn->credit_next = child;
	  child->credit_prev = tcscomm_conn;

	  parent->credit_next = tcscomm_conn;
	  tcscomm_conn->credit_prev = parent;
	}
      else
	{
	  CRTcscommConnection *child = cr_tcscomm.credit_head;
	  tcscomm_conn->credit_next = child;
	  child->credit_prev = tcscomm_conn;

	  cr_tcscomm.credit_head = tcscomm_conn;
	  tcscomm_conn->credit_prev = NULL;
	}
    }
}

static void
crTcscommCreditZero( CRTcscommConnection *tcscomm_conn )
{
  CRASSERT( cr_tcscomm.credit_head == tcscomm_conn );

  /* if we aren't already at the tail of the list, */
  if ( cr_tcscomm.credit_tail != tcscomm_conn )
    {
      /* then pull us off the head, */
      cr_tcscomm.credit_head = tcscomm_conn->credit_next;
      cr_tcscomm.credit_head->credit_prev = NULL;

      /* and put us on the tail */
      cr_tcscomm.credit_tail->credit_next = tcscomm_conn;
      tcscomm_conn->credit_prev = cr_tcscomm.credit_tail;
      tcscomm_conn->credit_next = NULL;

      cr_tcscomm.credit_tail = tcscomm_conn;
    }
}

static void
crTcscommAddConnection( CRConnection *conn )
{
  CRTcscommConnection *tcscomm_conn = crAlloc( sizeof( CRTcscommConnection ) );

  CRASSERT( tcscomm_conn );

  tcscomm_conn->conn = conn;
  tcscomm_conn->credit_next = NULL;
  if ( cr_tcscomm.credit_head ) {
    tcscomm_conn->credit_prev = cr_tcscomm.credit_tail;
    cr_tcscomm.credit_tail->credit_next = tcscomm_conn;
    cr_tcscomm.credit_tail = tcscomm_conn;
  }
  else {
    tcscomm_conn->credit_prev = NULL;
    cr_tcscomm.credit_head = tcscomm_conn;
    cr_tcscomm.credit_tail = tcscomm_conn;
  }

  crTcscommCreditIncrease( tcscomm_conn );
}

static void
crTcscommSendCredits( CRConnection *conn )
{
  CRMessageFlowControl msg;
  int                  len = sizeof( CRMessageFlowControl );

  CRASSERT( conn->recv_credits > 0 );

/*   crDebug( "crTcscommSendCredits:  sending %d credits to %s", */
/* 	   cr_tcscomm.credit_head->conn->recv_credits, */
/* 	   cr_tcscomm.credit_head->conn->hostname ); */
  
  msg.header.type    = CR_MESSAGE_FLOW_CONTROL;
  msg.header.conn_id = conn->id;
  msg.credits        = conn->recv_credits;

  crTCPIPWriteExact( conn, &len, sizeof( int ) );
  crTCPIPWriteExact( conn, &msg, len );
  
/*   crDebug( "crTcscommSendCredits:  sent %d credits to %s", */
/* 	   conn->recv_credits, conn->hostname ); */

  conn->recv_credits = 0;
}

static void
crTcscommMaybeSendCredits( void )
{
  if ( cr_tcscomm.credit_head &&
       cr_tcscomm.credit_head->conn->recv_credits >=
       CR_TCSCOMM_SEND_CREDITS_THRESHOLD )
    {
      crTcscommSendCredits( cr_tcscomm.credit_head->conn );
      crTcscommCreditZero( cr_tcscomm.credit_head );
    }
}

static void
crTcscommWaitForSendCredits( CRConnection *conn )
{
/*   crDebug( "crTcscommWaitForSendCredits:  waiting for credits to" */
/* 	   " talk to %s (total=%d)", */
/* 	   conn->hostname, conn->send_credits ); */

  while ( conn->send_credits <= 0 ) {
    int   len;
    void *msg_buffer;
    CRMessage *msg;

    crTCPIPReadExact( conn->tcp_socket, &len, sizeof( int ) );
    CRASSERT( len > 0 );
    
    msg_buffer = (void *) crAlloc( len );
    crTCPIPReadExact( conn->tcp_socket, msg_buffer, len );
    
    msg = (CRMessage *) msg_buffer;
    switch ( msg->header.type ) {
    case CR_MESSAGE_FLOW_CONTROL:
      CRASSERT( len == sizeof( CRMessageFlowControl ) );
      
      conn->send_credits += ((CRMessageFlowControl *) msg)->credits;
      crFree( msg_buffer );
      break;
    default:
      crFree( msg_buffer );
      crError( "crTcscommWaitForSendCredits:  received non-flow control"
	       "message on out-of-band channel (type=%d)",
	       msg->header.type );
      break;
    }

/*     crDebug( "crTcscommWaitForSendCredits:  received %d credits" */
/* 	     " to talk to %s (total=%d)", */
/* 	     ((CRMessageFlowControl *) msg)->credits, */
/* 	     conn->hostname, */
/* 	     conn->send_credits ); */
  }
}

static CRTcscommConnection *
crTcscommConnectionLookup( int tcscomm_id )
{
  CRTcscommConnection *tcscomm_conn = cr_tcscomm.credit_head;
  CRConnection     *conn = NULL;

  while ( tcscomm_conn ) {
    conn = tcscomm_conn->conn;
    if ( conn->tcscomm_id == tcscomm_id )
      return tcscomm_conn;

    tcscomm_conn = tcscomm_conn->credit_next;
  }

  return NULL;
}

void *
crTcscommAlloc( CRConnection *conn )
{
  CRTcscommBuffer *tcscomm_buffer = (CRTcscommBuffer *)
    crAlloc( sizeof( CRTcscommBuffer ) );
  char *payload = NULL;

  tcscomm_buffer->buf = (CRTcscommBuffer *) 
    tcscomm_Malloc( cr_tcscomm.tcscomm_conn,
		    CR_TCSCOMM_BUFFER_PAD + conn->mtu );
  if ( !tcscomm_buffer->buf ) {
    crError( "crTcscommAlloc:  tcscomm_Malloc( 0x%p, %d ) failed! )",
	     cr_tcscomm.tcscomm_conn,
	     CR_TCSCOMM_BUFFER_PAD + conn->mtu );
  }
	     
  *((CRTcscommBuffer **) tcscomm_buffer->buf ) = tcscomm_buffer;

  payload = (char *) tcscomm_buffer->buf + CR_TCSCOMM_BUFFER_PAD;
  
  tcscomm_buffer->len = 0; 
  tcscomm_buffer->magic = CR_TCSCOMM_BUFFER_MAGIC;
  
  return (void *) payload;
}

void
crTcscommFree( CRConnection *conn, void *buf )
{
  char *payload = (char *) buf;
  char *buffer = payload - CR_TCSCOMM_BUFFER_PAD;
  CRTcscommBuffer *tcscomm_buffer = *((CRTcscommBuffer **) buffer );

  CRASSERT( tcscomm_buffer->magic == CR_TCSCOMM_BUFFER_MAGIC );

  conn->recv_credits += tcscomm_buffer->len;
/*   crDebug( "crTcscommFree:  incremented recv_credits by %d (total=%d)", */
/* 	   tcscomm_buffer->len, conn->recv_credits ); */

  tcscomm_Free( cr_tcscomm.tcscomm_conn,
		tcscomm_buffer->buf );
  crFree( tcscomm_buffer );

  crTcscommCreditIncrease( crTcscommConnectionLookup( conn->tcscomm_id ) );
}

void
crTcscommSend( CRConnection *conn, void **bufp,
	    void *start, unsigned int len )
{
  CRTcscommBuffer *tcscomm_buffer;
  char *buf = NULL;
  char *payload = NULL;

  if ( cr_tcscomm.inside_recv )
    crError( "crTcscommSend:  cannot be called with crTcscommRecv" );

  while ( crTcscommRecv( ) )
    ;

  if ( conn->send_credits <= 0 )
    crTcscommWaitForSendCredits( conn );

  if ( bufp == NULL ) {
    payload = (char *) crTcscommAlloc( conn );
    buf  = payload - CR_TCSCOMM_BUFFER_PAD; 
    tcscomm_buffer = *((CRTcscommBuffer **) buf );

    CRASSERT( tcscomm_buffer->magic == CR_TCSCOMM_BUFFER_MAGIC );

    tcscomm_buffer->len = len;
    conn->send_credits -= tcscomm_buffer->len;

    crMemcpy( payload, start, len );

    crTcscommWriteExact( conn, payload, len );

    tcscomm_Free( cr_tcscomm.tcscomm_conn, tcscomm_buffer->buf );

    return;
  }

  payload = (char *)( *bufp );
  buf = payload - CR_TCSCOMM_BUFFER_PAD; 
  tcscomm_buffer = *((CRTcscommBuffer **) buf );

  CRASSERT( tcscomm_buffer->magic == CR_TCSCOMM_BUFFER_MAGIC );

  tcscomm_buffer->len = len; 
  conn->send_credits -= tcscomm_buffer->len;
/*   crDebug( "crTcscommSend:  decremented send_credits by %d (total=%d)", */
/* 	   len, conn->send_credits ); */

  crTcscommWriteExact( conn, start, len );

  tcscomm_Free( cr_tcscomm.tcscomm_conn, tcscomm_buffer->buf );
}

static void
crTcscommCloseConnection( CRConnection *conn )
{
  crWarning( "crTcscommCloseConnection:  shutting down tcscomm_id=%d",
	   conn->tcscomm_id );
  /*   tcscomm_Close( conn->tcscomm_id ); */
}


void
crTcscommSingleRecv( CRConnection *conn, void *buf, unsigned int len )
{
  crTcscommReadExact( conn->tcscomm_id, buf, len );
}

static CRConnection *
crTcscommSelect( void )
{
  int               tcscomm_id = -1;
  CRTcscommConnection *tcscomm_conn = NULL;

  tcscomm_id = tcscomm_Poll( cr_tcscomm.credit_head->conn->tcscomm_id,
			  CR_TCSCOMM_TIMEOUT );

  if ( tcscomm_id == -1 ) {
/*     crDebug( "crTcscommSelect:  no data available!" ); */
    return NULL;
  }

  tcscomm_conn = crTcscommConnectionLookup( tcscomm_id );
  if ( tcscomm_conn ) {
/*     crDebug( "crTcscommSelect:  tcscomm_id=%d has data available", */
/*  	     tcscomm_conn->conn->tcscomm_id ); */
    return tcscomm_conn->conn;
  }

  return NULL;
}

int
crTcscommRecv( void )
{
  CRConnection *conn;
  char *payload = NULL;
  char *buf = NULL;
  CRTcscommBuffer *tcscomm_buffer = NULL;
  int          len;

  cr_tcscomm.inside_recv++;

  crTcscommMaybeSendCredits( );

  conn = crTcscommSelect( );
  if ( !conn ) {
    cr_tcscomm.inside_recv--;
    return 0;
  }
  
  len = tcscomm_Peek( conn->tcscomm_id );
  CRASSERT( len > 0 );

  payload = (char *) crTcscommAlloc( conn );
  buf = payload - CR_TCSCOMM_BUFFER_PAD;
  tcscomm_buffer = *((CRTcscommBuffer **) buf );
  
  tcscomm_buffer->len = len;
  
  crTcscommReadExact( conn->tcscomm_id, payload, len );

  crNetDispatchMessage( cr_tcscomm.recv_list, conn, payload, len );

  cr_tcscomm.inside_recv--;

  return 1;
}

void
crTcscommHandleNewMessage( CRConnection *conn, CRMessage *msg,
			unsigned int len )
{
  (void) conn;
  (void) msg;
  (void) len;
}

void
crTcscommInstantReclaim( CRConnection *conn, CRMessage *mess )
{
  crTcscommFree( conn, mess );
}

void crTcscommInit( CRNetReceiveFuncList *rfl, CRNetCloseFuncList *cfl,
		 unsigned int mtu )
{
  (void) mtu;
  cr_tcscomm.recv_list = rfl;
  cr_tcscomm.close_list = cfl;

  if ( cr_tcscomm.initialized )
    {
      return;
    }
  
  if ( crGetHostname( cr_tcscomm.my_hostname, sizeof( cr_tcscomm.my_hostname ) ) )
    {
      crError( "crTcscommInit:  couldn't determine my own hostname!" );
    }
  strtok( cr_tcscomm.my_hostname, "." );

  cr_tcscomm.tcscomm_conn = tcscomm_Init( cr_tcscomm.low_node, cr_tcscomm.high_node,
				   cr_tcscomm.low_context, cr_tcscomm.high_context,
				   cr_tcscomm.my_rank );
  if ( !cr_tcscomm.tcscomm_conn )
    crError( "crTcscommInit:  tcscomm_Init( %d ) failed",
	     cr_tcscomm.my_rank );

  cr_tcscomm.num_conns = 0;

  cr_tcscomm.credit_head = NULL;
  cr_tcscomm.credit_tail = NULL;

  cr_tcscomm.inside_recv = 0;

  cr_tcscomm.initialized = 1;
}

void
crTcscommAccept( CRConnection *conn, char *hostname, unsigned short port )
{
  CRConnection *mother;
  char          response[8096];
  char          client_hostname[256];
  int           client_rank;
  int           client_endianness;

  crDebug( "crTcscommAccept is being called -- "
	   "brokering the connection through the mothership!." );
  
  mother = __copy_of_crMothershipConnect( );

  /* Tell the mothership I'm willing to receive a client, and what my Tcscomm info is */
  if (!__copy_of_crMothershipSendString( mother, response,
					 "acceptrequest quadrics-tcscomm %s %d %d",
					 cr_tcscomm.my_hostname, cr_tcscomm.my_rank,
					 conn->endianness ) ) {
    crError( "Mothership didn't like my accept request request" );
  }
  
  /* The response will contain the Tcscomm information for the guy who accepted 
   * this connection.  The mothership will sit on the acceptrequest 
   * until someone connects. */
  sscanf( response, "%d %s %d %d",
	  &(conn->id), client_hostname, &(client_rank), &(client_endianness) );

  if (conn->endianness != client_endianness)
    conn->swap = 1;

  conn->hostname = crStrdup( client_hostname );
  crDebug( "crTcscommAccept:  opening connection to %s:%d",
	   conn->hostname, client_rank );
  
  conn->tcscomm_id = tcscomm_Open( cr_tcscomm.tcscomm_conn, conn->hostname, client_rank );
  if ( conn->tcscomm_id < 0 ) {
    crError( "crTcscommAccept:  couldn't establish an Tcscomm connection with %s",
	     conn->hostname );
  }
  
  /* NOW, we can add the connection, since we have enough information 
   * to uniquely determine the sender when we get a packet! */
  crTcscommAddConnection( conn );
  
  __copy_of_crMothershipDisconnect( mother );

  crDebug( "crTcscommAccept:  connecting to %s:%d on port %d "
	   "for out-of-band communication",
	   conn->hostname, client_rank, CR_TCSCOMM_TCPIP_PORT );
  crTCPIPAccept( conn, CR_TCSCOMM_TCPIP_PORT );
  conn->send_credits = 0;
  conn->recv_credits = CR_TCSCOMM_INITIAL_RECV_CREDITS;
  crDebug( "crTcscommAccept:  connected to %s:%d on port %d for "
	   "out-of-band communication",
	   conn->hostname, client_rank, CR_TCSCOMM_TCPIP_PORT );

  (void) port;
}

/* The function that actually connects.  This should only be called by clients 
 * Servers have another way to set up the connection. */
int
crTcscommDoConnect( CRConnection *conn )
{
  CRConnection *mother;
  char          response[8096];
  int           server_endianness;
	
  crDebug( "crTcscommDoConnect is being called -- "
	   "brokering the connection through the mothership!" );

  mother = __copy_of_crMothershipConnect( );

  /* Tell the mothership who I want to connect to, and what my Tcscomm info is */
  if (!__copy_of_crMothershipSendString( mother, response,
					 "connectrequest quadrics-tcscomm %s %d %s %d %d",
					 conn->hostname, CR_TCSCOMM_CRSERVER_RANK,
					 cr_tcscomm.my_hostname, cr_tcscomm.my_rank,
					 conn->endianness ) )
    {
      crError( "Mothership didn't like my connect request request" );
    }
  
  /* The response will contain the Tcscomm information for the guy who accepted 
   * this connection.  The mothership will sit on the connectrequest 
   * until someone accepts. */
  
  sscanf( response, "%d %d", &(conn->id), &(server_endianness) );

  if (conn->endianness != server_endianness)
    conn->swap = 1;
  
  crDebug( "crTcscommDoConnect:  opening connection to %s:%d",
	   conn->hostname, CR_TCSCOMM_CRSERVER_RANK );
  
  conn->tcscomm_id = tcscomm_Open( cr_tcscomm.tcscomm_conn,
				conn->hostname, CR_TCSCOMM_CRSERVER_RANK );
  if ( conn->tcscomm_id < 0 ) {
    crError( "crTcscommDoConnect:  couldn't establish an Tcscomm connection with %s",
	     conn->hostname );
  }

  /* NOW, we can add the connection, since we have enough information 
   * to uniquely determine the sender when we get a packet! */
  crTcscommAddConnection( conn );
  
  __copy_of_crMothershipDisconnect( mother );

  crDebug( "crTcscommDoConnect:  connecting to %s:%d on port %d "
	   "for out-of-band communication",
	   conn->hostname, CR_TCSCOMM_CRSERVER_RANK,
	   CR_TCSCOMM_TCPIP_PORT );
  /* reset connection's port */
  conn->port = CR_TCSCOMM_TCPIP_PORT;
  crTCPIPDoConnect( conn );
  conn->send_credits = 0;
  conn->recv_credits = CR_TCSCOMM_INITIAL_RECV_CREDITS;
  crDebug( "crTcscommDoConnect:  connected to %s:%d on port %d for "
	   "out-of-band communication",
	   conn->hostname, CR_TCSCOMM_CRSERVER_RANK,
	   CR_TCSCOMM_TCPIP_PORT );
  
  return 1;
}

void crTcscommDoDisconnect( CRConnection *conn )
{
  if ( cr_tcscomm.num_conns <= 0 ) {
    crTcscommCloseConnection( conn );
  }
}

void crTcscommConnection( CRConnection *conn, void *tcscomm_conn, int rank )
{
  CRASSERT( cr_tcscomm.initialized );

  conn->type  = CR_TCSCOMM;
  conn->Alloc = crTcscommAlloc;
  conn->Send  = crTcscommSend;
  conn->SendExact  = crTcscommWriteExact;
  conn->Recv  = crTcscommSingleRecv;
  conn->Free  = crTcscommFree;
  conn->Accept = crTcscommAccept;
  conn->Connect = crTcscommDoConnect;
  conn->Disconnect = crTcscommDoDisconnect;
  conn->InstantReclaim = crTcscommInstantReclaim;
  conn->HandleNewMessage = crTcscommHandleNewMessage;
  conn->index = cr_tcscomm.num_conns;
  conn->sizeof_buffer_header = sizeof( CRTcscommBuffer );
  conn->actual_network = 1;

  cr_tcscomm.num_conns++;
}

void
crTcscommSetRank( int rank )
{
  cr_tcscomm.my_rank = rank;
}

void
crTcscommSetContextRange( int low_context, int high_context )
{
  cr_tcscomm.low_context  = low_context;
  cr_tcscomm.high_context = high_context;
  crDebug( "crTcscommSetContextRange:  using contexts %d-%d",
	   cr_tcscomm.low_context, cr_tcscomm.high_context );
}

void
crTcscommSetNodeRange( const char *low_node, const char *high_node )
{
  char* low= crStrdup( low_node );
  char* high= crStrdup( high_node );
  cr_tcscomm.low_node  = strtok( low, "'" );
  cr_tcscomm.high_node = strtok( high, "'" );
  crDebug( "crTcscommSetNodeRange:  low node=%s, high node=%s",
	   cr_tcscomm.low_node, cr_tcscomm.high_node );
}
