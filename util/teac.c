/* cpg - 5/20/02 - with parts borrowed from gm.c and tcpip.c */

/* i need to close connections properly, so the crserver can exit, etc. */
/*   -- broker disconnects through the mothership? */
/*   -- crTeacDoDisconnect is wrong, but it's not even getting called */
/*      at the moment; the client application just quits */

#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "cr_error.h"
#include "cr_mem.h"
#include "cr_string.h"
#include "cr_net.h"
#include "cr_endian.h"
#include "net_internals.h"

#include "teac.h"

#define CR_TEAC_BUFFER_PAD 8 /* sizeof( SBuffer *) || sizeof( RBuffer * ) */
#define CR_TEAC_SEND_CREDITS_THRESHOLD ( 1 << 18 )

typedef enum {
  CRTeacMemory,
  CRTeacMemoryBig
} CRTeacBufferKind;

typedef struct CRTeacConnection {
  CRConnection            *conn;
  struct CRTeacConnection *credit_prev;
  struct CRTeacConnection *credit_next;
} CRTeacConnection;

typedef struct CRTeacBuffer {
  unsigned int         magic;
  CRTeacBufferKind     kind;
  unsigned int         len;
  unsigned int         allocated;
  unsigned int         pad;
} CRTeacBuffer;

static struct {
  int                  initialized;
  int                  num_conns;
  CRNetReceiveFunc     recv;
  CRNetCloseFunc       close;
  CRTeacConnection    *credit_head;
  CRTeacConnection    *credit_tail;
  unsigned int         inside_recv;
  void                *teac_conn;
  char                 my_hostname[256];
  int                  my_rank;
  int                  low_context;
  int                  high_context;
  char                 *low_node;
  char                 *high_node;
} cr_teac;

/* Forward declarations */
void *crTeacAlloc( CRConnection * );
void  crTeacFree( CRConnection *, void * );
int   crTeacRecv( void );

int
crTeacErrno( void )
{
  int err = errno;
  errno = 0;
  return err;
}

char *
crTeacErrorString( int err )
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

static void
crTeacCreditIncrease( CRTeacConnection *teac_conn )
{
  CRTeacConnection *parent;
  int               my_credits;

  /* move this node up the doubly-linked, if it isn't already in the
   * right position */

  parent = teac_conn->credit_prev;
  my_credits = teac_conn->conn->recv_credits;
  if ( parent && parent->conn->recv_credits < my_credits )
    {
      /* we can't be at the head of the list, because our parent is
       * non-NULL */

      /* are we the tail of the list? */
      if ( cr_teac.credit_tail == teac_conn )
	{
	  /* yes, pull up the tail pointer, and fix our parent to be tail */
	  cr_teac.credit_tail = parent;
	  parent->credit_next = NULL;
	}
      else
	{
	  /* no, just link around us */
	  parent->credit_next = teac_conn->credit_next;
	  teac_conn->credit_next->credit_prev = parent;
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
	  CRTeacConnection *child = parent->credit_next;
	  teac_conn->credit_next = child;
	  child->credit_prev = teac_conn;

	  parent->credit_next = teac_conn;
	  teac_conn->credit_prev = parent;
	}
      else
	{
	  CRTeacConnection *child = cr_teac.credit_head;
	  teac_conn->credit_next = child;
	  child->credit_prev = teac_conn;

	  cr_teac.credit_head = teac_conn;
	  teac_conn->credit_prev = NULL;
	}
    }
}

static void
crTeacCreditZero( CRTeacConnection *teac_conn )
{
  CRASSERT( cr_teac.credit_head == teac_conn );

  /* if we aren't already at the tail of the list, */
  if ( cr_teac.credit_tail != teac_conn )
    {
      /* then pull us off the head, */
      cr_teac.credit_head = teac_conn->credit_next;
      cr_teac.credit_head->credit_prev = NULL;

      /* and put us on the tail */
      cr_teac.credit_tail->credit_next = teac_conn;
      teac_conn->credit_prev = cr_teac.credit_tail;
      teac_conn->credit_next = NULL;

      cr_teac.credit_tail = teac_conn;
    }
}

static void
crTeacAddConnection( CRConnection *conn )
{
  CRTeacConnection *teac_conn = crAlloc( sizeof( CRTeacConnection ) );

  CRASSERT( teac_conn );

  teac_conn->conn = conn;
  teac_conn->credit_next = NULL;
  if ( cr_teac.credit_head ) {
    teac_conn->credit_prev = cr_teac.credit_tail;
    cr_teac.credit_tail->credit_next = teac_conn;
    cr_teac.credit_tail = teac_conn;
  }
  else {
    teac_conn->credit_prev = NULL;
    cr_teac.credit_head = teac_conn;
    cr_teac.credit_tail = teac_conn;
  }

  crTeacCreditIncrease( teac_conn );
}

static void
crTeacSendCredits( CRConnection *conn )
{
  char *payload = NULL;
  char *buf  = NULL;
  SBuffer *sbuf = NULL;
  CRMessageFlowControl *msg = NULL;

  CRASSERT( conn->recv_credits > 0 );
  
#ifdef never
  crDebug( "crTeacSendCredits:  sending %d credits to %s:%d",
	   conn->recv_credits, conn->hostname, conn->teac_rank );
#endif

  payload = (char *) crTeacAlloc( conn );
  buf  = (char *) payload - CR_TEAC_BUFFER_PAD; 
  sbuf = *((SBuffer **) buf );

  msg = (CRMessageFlowControl *)( payload );
  msg->header.type    = CR_MESSAGE_FLOW_CONTROL;
  msg->header.conn_id = conn->id;
  msg->credits        = conn->recv_credits;

  sbuf->validSize += sizeof( CRMessageFlowControl );
  conn->send_credits -= sbuf->validSize;

  teac_Send( cr_teac.teac_conn, &(conn->teac_id), 1, sbuf, buf );

  conn->recv_credits = 0;

#ifdef never
  crDebug( "crTeacSendCredits:  sent credits to %s:%d",
 	   conn->hostname, conn->teac_rank );
#endif
}

static void
crTeacMaybeSendCredits( void )
{
  if ( cr_teac.credit_head &&
       cr_teac.credit_head->conn->recv_credits >=
       CR_TEAC_SEND_CREDITS_THRESHOLD )
    {
      crTeacSendCredits( cr_teac.credit_head->conn );
      crTeacCreditZero( cr_teac.credit_head );
    }
}

static void
crTeacWaitForSendCredits( CRConnection *conn )
{
#ifdef never
  crDebug( "crTeacWaitForSendCredits:  waiting for credits "
	   "to talk to %s:%d (total=%d)",
	   conn->hostname, conn->teac_rank,
	   conn->send_credits );
#endif

  while ( conn->send_credits <= 0 ) {
    crTeacRecv( );
  }

#ifdef never
  crDebug( "crTeacWaitForSendCredits:  received credits "
	   "to talk to %s:%d (total=%d)",
	   conn->hostname, conn->teac_rank, conn->send_credits );
#endif
}

static CRTeacConnection *
crTeacConnectionLookup( int teac_id )
{
  CRTeacConnection *teac_conn = cr_teac.credit_head;
  CRConnection     *conn = NULL;

  while ( teac_conn ) {
    conn = teac_conn->conn;
    if ( conn->teac_id == teac_id )
      return teac_conn;

    teac_conn = teac_conn->credit_next;
  }

  return NULL;
}

void *
crTeacAlloc( CRConnection *conn )
{
  SBuffer *sbuf = teac_getSendBuffer( cr_teac.teac_conn,
				      CR_TEAC_BUFFER_PAD + conn->mtu );
  char *buf  = NULL;
  char *payload = NULL;

  buf = (char *) sbuf->buf;
  payload = buf + CR_TEAC_BUFFER_PAD; 
  *((SBuffer **) buf ) = sbuf;
  sbuf->validSize = CR_TEAC_BUFFER_PAD; 

  return (void *) payload; 
}

void
crTeacFree( CRConnection *conn, void *buf )
{
  char *payload = (char *) buf;
  char *buffer = payload - CR_TEAC_BUFFER_PAD;
  RBuffer *rbuf = *((RBuffer **) buffer );

  conn->recv_credits += rbuf->validSize;
  teac_Dispose( cr_teac.teac_conn, rbuf );

  crTeacCreditIncrease( crTeacConnectionLookup( conn->teac_id ) );
}

void
crTeacInstantReclaim( CRConnection *conn, CRMessage *msg )
{
  crTeacFree( conn, msg );
}

void
crTeacSend( CRConnection *conn, void **bufp,
	    void *start, unsigned int len )
{
  char *payload = NULL;
  char *buf = NULL;
  SBuffer *sbuf;

  if ( cr_teac.inside_recv )
    crError( "crTeacSend:  cannot be called with crTeacRecv" );

  while ( crTeacRecv( ) )
    ;

  if ( conn->send_credits <= 0 )
    crTeacWaitForSendCredits( conn );

  if ( bufp == NULL ) {
    payload = (char *) crTeacAlloc( conn );
    buf  = payload - CR_TEAC_BUFFER_PAD; 
    sbuf = *((SBuffer **) buf );

    memcpy( payload, start, len );

    sbuf->validSize += len; 
    conn->send_credits -= sbuf->validSize;

    teac_Send( cr_teac.teac_conn, &(conn->teac_id), 1, sbuf, buf );

    return;
  }

  payload = (char *)( *bufp );
  buf = payload - CR_TEAC_BUFFER_PAD; 
  sbuf = *((SBuffer **) buf );

  sbuf->validSize += len;
  conn->send_credits -= sbuf->validSize;

  teac_Send( cr_teac.teac_conn, &(conn->teac_id), 1, sbuf,
	     (void *)((char *) start - CR_TEAC_BUFFER_PAD ) );
}

CRConnection *
crTeacSelect( void )
{
  int  count    = 0;
  int *teac_ids = malloc( sizeof( int ) * cr_teac.num_conns );
  int  ready_id = -1;
  CRTeacConnection *teac_conn = NULL;

  /* Build a list of currently valid teac_ids */
  teac_conn = cr_teac.credit_head;
  while ( teac_conn ) {
    teac_ids[count] = teac_conn->conn->teac_id;
    count++;

    teac_conn = teac_conn->credit_next;
  }

  /* Poll the connections */
  ready_id = teac_Poll( cr_teac.teac_conn, teac_ids, count );
  if ( ready_id == -1 )
    return NULL;

  /* Search for the connection with the proper teac_id */
  teac_conn = crTeacConnectionLookup( ready_id );
  if ( teac_conn ) {
#ifdef never
    crDebug( "crTeacSelect:  teac_id=%d has data available",
	     teac_conn->conn->teac_id );
#endif
    return teac_conn->conn;
  }

  return NULL;
}

int
crTeacRecv( void )
{
  CRConnection *conn = NULL;
  RBuffer *rbuf = NULL;
  char *buf = NULL;
  char *payload = NULL;
  int len = 0;

  cr_teac.inside_recv++;

  crTeacMaybeSendCredits( );

  conn = crTeacSelect( );
  if ( !conn ) {
    cr_teac.inside_recv--;
    return 0;
  }

  rbuf = teac_Recv( cr_teac.teac_conn, conn->teac_id );
  if ( !rbuf ) {
    crError( "crTeacRecv:  teac_Recv( teac_id=%d ) failed",
	     conn->teac_id );
  }

  buf = (char *) rbuf->buf;
  payload = buf + CR_TEAC_BUFFER_PAD;
  len = rbuf->validSize - CR_TEAC_BUFFER_PAD; 

  *((RBuffer **) buf ) = rbuf;

  if ( !cr_teac.recv( conn, payload, len ) ) {
    crNetDefaultRecv( conn, payload, len );
  }

  cr_teac.inside_recv--;

  return 1;
}

void crTeacInit( CRNetReceiveFunc recvFunc, CRNetCloseFunc closeFunc,
		 unsigned int mtu )
{
  (void) mtu;
  if ( cr_teac.initialized )
    {
      if ( cr_teac.recv == NULL && cr_teac.close == NULL )
	{
	  cr_teac.recv = recvFunc;
	  cr_teac.close = closeFunc;
	}
      else
	{
	  CRASSERT( cr_teac.recv == recvFunc );
	  CRASSERT( cr_teac.close == closeFunc );
	}
      return;
    }
  
  if ( crGetHostname( cr_teac.my_hostname, sizeof( cr_teac.my_hostname ) ) )
    {
      crError( "crTeacInit:  couldn't determine my own hostname!" );
    }
  strtok( cr_teac.my_hostname, "." );
  
  cr_teac.teac_conn = teac_Init( cr_teac.low_node, cr_teac.high_node,
				 cr_teac.low_context, cr_teac.high_context,
				 cr_teac.my_rank );
  if ( !cr_teac.teac_conn )
    crError( "crTeacInit:  teac_Init( %d, %d, %d ) failed",
	     cr_teac.low_context, cr_teac.high_context,
	     cr_teac.my_rank );

  cr_teac.num_conns = 0;

  cr_teac.recv = recvFunc;
  cr_teac.close = closeFunc;

  cr_teac.credit_head = NULL;
  cr_teac.credit_tail = NULL;

  cr_teac.inside_recv = 0;

  cr_teac.initialized = 1;
}

void
crTeacAccept( CRConnection *conn, unsigned short port )
{
  CRConnection *mother;
  char          response[8096];
  char          client_hostname[256];
  int           client_rank;
  int           client_endianness;

  crDebug( "crTeacAccept is being called -- "
	   "brokering the connection through the mothership!." );
  
  mother = __copy_of_crMothershipConnect( );
  
  /* Tell the mothership I'm willing to receive a client, and what my Teac info is */
  if (!__copy_of_crMothershipSendString( mother, response,
					 "acceptrequest quadrics %s %d %d",
					 cr_teac.my_hostname, cr_teac.my_rank,
					 conn->endianness ) ) {
    crError( "crTeacAccpet:  Mothership didn't like my accept request request" );
  }
  
  /* The response will contain the Teac information for the guy who accepted 
   * this connection.  The mothership will sit on the acceptrequest 
   * until someone connects. */
  sscanf( response, "%d %s %d %d",
	  &(conn->id), client_hostname, &(client_rank), &(client_endianness) );

  /* deal with endianness here */

  conn->hostname  = client_hostname;
  conn->teac_rank = client_rank;

  crDebug( "crTeacAccept:  opening connection to %s:%d",
	   conn->hostname, conn->teac_rank );  
  conn->teac_id = teac_getConnId( cr_teac.teac_conn,
				  conn->hostname,
				  conn->teac_rank );
  if ( conn->teac_id < 0 ) {
    crError( "crTeacAccept:  couldn't establish an Teac connection with %s:%d",
	     conn->hostname, conn->teac_rank );
  }
  crDebug( "crTeacAccept:  connection to %s:%d has teac_id=%d\n",
	   conn->hostname, conn->teac_rank, conn->teac_id );
  
  /* NOW, we can add the connection, since we have enough information 
   * to uniquely determine the sender when we get a packet! */
  crTeacAddConnection( conn );
  
  __copy_of_crMothershipDisconnect( mother );

  (void) port;
}

/* The function that actually connects.  This should only be called by clients 
 * Servers have another way to set up the connection. */
int
crTeacDoConnect( CRConnection *conn )
{
  CRConnection *mother;
  char          response[8096];
  int           server_endianness;
	
  crDebug( "crTeacDoConnect is being called -- "
	   "brokering the connection through the mothership!" );

  mother = __copy_of_crMothershipConnect( );

  /* Tell the mothership who I want to connect to, and what my Teac info is */
  if (!__copy_of_crMothershipSendString( mother, response,
					 "connectrequest quadrics %s %d %s %d %d",
					 conn->hostname, conn->teac_rank,
					 cr_teac.my_hostname, cr_teac.my_rank,
					 conn->endianness ) )
    {
      crError( "crTeacDoConnect:  Mothership didn't like my connect request request" );
    }
  
  /* The response will contain the Teac information for the guy who accepted 
   * this connection.  The mothership will sit on the connectrequest 
   * until someone accepts. */
  
  sscanf( response, "%d %d", &(conn->id), &(server_endianness) );

  /* deal with endianness here */

  crDebug( "crTeacDoConnect:  opening connection to %s:%d",
	   conn->hostname, conn->teac_rank );
  conn->teac_id = teac_getConnId( cr_teac.teac_conn,
				  conn->hostname,
				  conn->teac_rank );
  if ( conn->teac_id < 0 ) {
    crError( "crTeacDoConnect:  couldn't establish an Teac connection with %s:%d",
	     conn->hostname, conn->teac_rank );
  }
  crDebug( "crTeacDoConnect:  connection to %s:%d has teac_id=%d\n",
	   conn->hostname, conn->teac_rank, conn->teac_id );

  /* NOW, we can add the connection, since we have enough information 
   * to uniquely determine the sender when we get a packet! */
  crTeacAddConnection( conn );
  
  __copy_of_crMothershipDisconnect( mother );

  return 1;
}

void crTeacDoDisconnect( CRConnection *conn )
{
  crDebug( "crTeacCloseConnection:  shutting down Teac" );
  teac_Close( cr_teac.teac_conn );
}

void
crTeacHandleNewMessage( CRConnection *conn, CRMessage *msg,
			unsigned int len )
{
  crError( "crTeacHandleNewMessage should not get called." );

  (void) conn;
  (void) msg;
  (void) len;
}

void
crTeacSingleRecv( CRConnection *conn, void *buf, unsigned int len )
{
  crError( "crTeacSingleRecv should not get called." );
  (void) conn;
  (void) buf;
  (void) len;
}

void
crTeacSendExact( CRConnection *conn, void *buf, unsigned int len ) {
  crError( "crTeacSendExact should not get called." );
  (void) conn;
  (void) buf;
  (void) len;
}

void crTeacConnection( CRConnection *conn )
{
  CRASSERT( cr_teac.initialized );

  conn->type  = CR_TEAC;
  conn->Alloc = crTeacAlloc;
  conn->Send  = crTeacSend;
  conn->SendExact  = crTeacSendExact;
  conn->Recv  = crTeacSingleRecv;
  conn->Free  = crTeacFree;
  conn->Accept = crTeacAccept;
  conn->Connect = crTeacDoConnect;
  conn->Disconnect = crTeacDoDisconnect;
  conn->InstantReclaim = crTeacInstantReclaim;
  conn->HandleNewMessage = crTeacHandleNewMessage;
  conn->index = cr_teac.num_conns;
  conn->sizeof_buffer_header = sizeof( CRTeacBuffer );
  conn->actual_network = 1; 

  cr_teac.num_conns++;
}

void
crTeacSetRank( int rank )
{
  cr_teac.my_rank = rank;
}

void
crTeacSetContextRange( int low_context, int high_context )
{
  cr_teac.low_context  = low_context;
  cr_teac.high_context = high_context;
  crDebug( "crTeacSetContextRange:  using contexts %d-%d",
	   cr_teac.low_context, cr_teac.high_context );
}

void
crTeacSetNodeRange( const char *low_node, const char *high_node )
{
  cr_teac.low_node  = strtok( low_node, "'" );
  cr_teac.high_node = strtok( high_node, "'" );
  crDebug( "crTeacSetNodeRange:  low node=%s, high node=%s",
	   cr_teac.low_node, cr_teac.high_node );
}
