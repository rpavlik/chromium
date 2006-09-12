/* cpg - 5/20/02 - with parts borrowed from gm.c and tcpip.c */

/* i need to close connections properly, so the crserver can exit, etc. */
/*   -- broker disconnects through the mothership? */
/*   -- crTeacDoDisconnect is wrong, but it's not even getting called */
/*      at the moment; the client application just quits */

/******************************************************
 * Notes-
 * -Is CR_TEAC_BUFFER_PAD 16?
 ******************************************************/

#define CR_TEAC_COPY_MSGS_TO_MAIN_MEMORY 1
#define CR_TEAC_USE_RECV_THREAD 0

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
#include <string.h>
#if CR_TEAC_USE_RECV_THREAD
#include <pthread.h>
#endif

#include "teac.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_string.h"
#include "cr_net.h"
#include "cr_endian.h"
#include "net_internals.h"

#if ( CR_TEAC_USE_RECV_THREAD && ! CHROMIUM_THREADSAFE )
#error "Teac configuration error: USE_RECV_THREAD requires CHROMIUM_THREADSAFE"
#endif

#if ( CR_TEAC_USE_RECV_THREAD && ! CR_TEAC_COPY_MSGS_TO_MAIN_MEMORY )
#error "Teac configuration error: USE_RECV_THREAD requires CR_TEAC_COPY_MSGS_TO_MAIN_MEMORY!"
#endif

#ifdef never
#define CR_TEAC_SEND_CREDITS_THRESHOLD ( 1 << 18 )
#endif
#define CR_TEAC_SEND_CREDITS_THRESHOLD ( 1 << 16 )

/* some random bits.  Last must be 0 */
#define CR_TEAC_BUFFER_MAGIC 0x7b52e9e0

/* There must be 16 or fewer of these; they get packed into the
 * bottom 4 bits of a word containing CR_TEAC_BUFFER_MAGIC.
 */
typedef enum {
  CR_TEAC_KIND_SEND,
  CR_TEAC_KIND_FLOW_SEND,
  CR_TEAC_KIND_RECV,
  CR_TEAC_KIND_FLOW_RECV,
  CR_TEAC_KIND_RECV_LISTED
} CRTeacBufferKind;

typedef struct CRTeacConnection {
  CRConnection            *conn;
  struct CRTeacConnection *credit_prev;
  struct CRTeacConnection *credit_next;
  struct CRTeacBuffer     *incoming_head;
  struct CRTeacBuffer     *incoming_tail;
#ifdef CHROMIUM_THREADSAFE
  CRmutex                  msgListMutex;
#endif
} CRTeacConnection;

/* The union in the following struct gets forced to length 8, to
 * make sure there are no size ambiguities when passing between
 * architectures with 32bit and 64bit pointers.
 */
typedef struct CRTeacBuffer {
  unsigned int         magic;
  int                  len; /* used only with CR_TEAC_KIND_RECV_LISTED msgs */
  union {
    SBuffer*             sendBuffer;
    RBuffer*             rcvBuffer;
    struct CRTeacBuffer* incoming_next;
    char                 filler[8]; /* force size of union to 8 */
  } u;
} CRTeacBuffer;

#define CR_TEAC_BUFFER_PAD sizeof(CRTeacBuffer)

static struct {
  int                  initialized;
  int                  num_conns;
  CRNetReceiveFuncList *recv_list;
  CRNetCloseFuncList  *close_list;
  CRTeacConnection    *credit_head;
  CRTeacConnection    *credit_tail;
  unsigned int         inside_recv;
  void                *tcomm;
  char                 my_hostname[256];
  int                  my_rank;
  int                  low_context;
  int                  high_context;
  char                 *low_node;
  char                 *high_node;
  unsigned char        key[TEAC_KEY_SIZE];
#ifdef CHROMIUM_THREADSAFE
  CRmutex              teacAPIMutex;
  pthread_t            recvThread;
#endif
} cr_teac;

/* Forward declarations */
void *crTeacAlloc( CRConnection * );
void  crTeacFree( CRConnection *, void * );
int crTeacErrno( void );
char* crTeacErrorString( int err );
void crTeacInstantReclaim( CRConnection *conn, CRMessage *msg );
void crTeacSend( CRConnection *conn, void **bufp,
		 const void *start, unsigned int len );
CRConnection *crTeacSelect( void );
void crTeacAccept( CRConnection *conn, const char *hostname, unsigned short port );
int crTeacDoConnect( CRConnection *conn );
void crTeacDoDisconnect( CRConnection *conn );
void crTeacHandleNewMessage( CRConnection *conn, CRMessage *msg,
			     unsigned int len );
void crTeacSingleRecv( CRConnection *conn, void *buf, unsigned int len );
void crTeacSendExact( CRConnection *conn, const void *buf, unsigned int len );
int crTeacRecv( void );
static CRTeacConnection *crTeacConnectionLookup( int teac_id );
void* crTeacRecvThread( void* args );

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

#if CR_TEAC_USE_RECV_THREAD
void* crTeacRecvThread( void* args )
{
  crDebug("crTeacRecvThread: starting listener thread.");
  while (1) {
    CRConnection* conn = crTeacSelect( );
    if ( conn ) {
      RBuffer* rbuf= NULL;
      CRTeacBuffer* teacMsg= NULL;
      CRTeacBuffer* cloneMsg= NULL;
      CRTeacConnection* teac_conn= crTeacConnectionLookup( conn->teac_id );
#ifdef never
      crDebug("crTeacRecvThread: beginning with allocation sizes %ld, %ld",
	      ((Tcomm*)cr_teac.tcomm)->totalSendBufferBytesAllocated,
	      ((Tcomm*)cr_teac.tcomm)->totalRecvBufferBytesAllocated);
      
      crDebug("crTeacRecvThread: entering teac_Recv on id %d",
	      conn->teac_id);
#endif
#ifdef CHROMIUM_THREADSAFE
      crLockMutex(&cr_teac.teacAPIMutex);
#endif
      rbuf = teac_Recv( cr_teac.tcomm, conn->teac_id );
#ifdef CHROMIUM_THREADSAFE
      crUnlockMutex(&cr_teac.teacAPIMutex);
#endif
#ifdef never
      crDebug("crTeacRecvThread: finished teac_Recv on id %d; got RBuffer at 0x%x",
	      conn->teac_id,(int)rbuf);
#endif
      if ( !rbuf ) {
	crError( "crTeacRecvThread:  teac_Recv( teac_id=%d ) failed",
		 conn->teac_id );
      }
      
      teacMsg= (CRTeacBuffer*)(rbuf->buf);
#ifdef never
      crDebug("crTeacRecvThread: new msg from teac_id %d is %ld bytes, at %lx",
	      conn->teac_id, rbuf->totSize,(long)(teacMsg+1));
#endif
      
      teacMsg->magic= (CR_TEAC_BUFFER_MAGIC | (int)CR_TEAC_KIND_RECV);
      teacMsg->u.rcvBuffer= rbuf;
      
      cloneMsg= (CRTeacBuffer*)crAlloc(rbuf->validSize);
      crMemcpy( cloneMsg, teacMsg, rbuf->validSize );
      cloneMsg->magic= (CR_TEAC_BUFFER_MAGIC | (int)CR_TEAC_KIND_RECV_LISTED);
      /* len field must be valid for LISTED messages */
      cloneMsg->len= rbuf->validSize - CR_TEAC_BUFFER_PAD; 
      cloneMsg->u.incoming_next= NULL;

      crLockMutex(&(teac_conn->msgListMutex));
      if (teac_conn->incoming_tail) {
	teac_conn->incoming_tail->u.incoming_next= cloneMsg;
	teac_conn->incoming_tail= cloneMsg;
      }
      else {
	/* This is the first in the list */
	teac_conn->incoming_head= teac_conn->incoming_tail= cloneMsg;
      }
      crUnlockMutex(&(teac_conn->msgListMutex));
      crTeacFree(conn,(CRMessage*)(teacMsg+1)); /* frees original message */
#ifdef never
      crDebug("crTeacRecvThread: appended new msg to incoming at %lx, len %d",
	      (long)(cloneMsg+1), cloneMsg->len);
#endif
    }
    else {
      /* Let's try just spinning */
#ifdef never
      sched_yield();
#endif
    }
  }
  (void)args; 
  return NULL;
}
#endif

static void
crTeacAddConnection( CRConnection *conn )
{
  CRTeacConnection *teac_conn = crAlloc( sizeof( CRTeacConnection ) );

  CRASSERT( teac_conn );

  teac_conn->conn = conn;
  teac_conn->credit_next = NULL;
#ifdef CHROMIUM_THREADSAFE
  crInitMutex(&(teac_conn->msgListMutex));
#endif
  teac_conn->incoming_head= teac_conn->incoming_tail= NULL;
  if ( cr_teac.credit_head ) {
    teac_conn->credit_prev = cr_teac.credit_tail;
    cr_teac.credit_tail->credit_next = teac_conn;
    cr_teac.credit_tail = teac_conn;
  }
  else {
    teac_conn->credit_prev = NULL;
    cr_teac.credit_head = teac_conn;
    cr_teac.credit_tail = teac_conn;
#if CR_TEAC_USE_RECV_THREAD
    /* First incoming teac connection, so start the receiver thread */
#ifdef never
    pthread_init();
#endif
    crDebug("crTeacAddConnection: about to start listener thread");
    pthread_create(&cr_teac.recvThread, NULL, crTeacRecvThread, NULL);
    pthread_detach(cr_teac.recvThread);
#endif
  }

  crTeacCreditIncrease( teac_conn );
}

static void
crTeacSendCredits( CRConnection *conn )
{
  CRTeacBuffer *buf  = NULL;
  SBuffer *sbuf = NULL;
  CRMessageFlowControl *msg = NULL;

  CRASSERT( conn->recv_credits > 0 );
  
  if (teac_sendBufferAvailable(cr_teac.tcomm)) {
#ifdef never
    crDebug( "crTeacSendCredits:  sending %d credits to %s:%d, id %d",
	     conn->recv_credits, conn->hostname, conn->teac_rank, 
	     conn->teac_id);
#endif

#ifdef CHROMIUM_THREADSAFE
    crLockMutex(&cr_teac.teacAPIMutex);
#endif
    sbuf= teac_getSendBuffer( cr_teac.tcomm, 
			      CR_TEAC_BUFFER_PAD + conn->mtu );
#ifdef CHROMIUM_THREADSAFE
    crUnlockMutex(&cr_teac.teacAPIMutex);
#endif

    buf = (CRTeacBuffer*) sbuf->buf;
    buf->magic= (CR_TEAC_BUFFER_MAGIC | (int)CR_TEAC_KIND_FLOW_SEND);
    buf->len= 0; /* not used for this msg type */
    buf->u.sendBuffer= sbuf;
    sbuf->validSize = CR_TEAC_BUFFER_PAD; 
    *((SBuffer **) buf ) = sbuf;

    msg = (CRMessageFlowControl *)( buf+1 );
    msg->header.type    = CR_MESSAGE_FLOW_CONTROL;
    msg->header.conn_id = conn->id;
    msg->credits        = conn->recv_credits;
    
    sbuf->validSize += sizeof( CRMessageFlowControl );
    conn->send_credits -= sbuf->validSize;
    
#ifdef CHROMIUM_THREADSAFE
    crLockMutex(&cr_teac.teacAPIMutex);
#endif
    teac_Send( cr_teac.tcomm, &(conn->teac_id), 1, sbuf, buf );
#ifdef CHROMIUM_THREADSAFE
    crUnlockMutex(&cr_teac.teacAPIMutex);
#endif
    
    conn->recv_credits = 0;
    
#ifdef never
    crDebug( "crTeacSendCredits:  sent credits to %s:%d",
	     conn->hostname, conn->teac_rank );
#endif
  }
  else {
#ifdef never
    crDebug( "crTeacSendCredits: no buffer; deferring sending to %s:%d, id %d",
	     conn->hostname, conn->teac_rank, conn->teac_id);
#endif
  }

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
  crDebug( "crTeacWaitForSendCredits:  found credits "
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
  SBuffer *sbuf = NULL;
  CRTeacBuffer *buf  = NULL;
  CRMessage* msg= NULL;

#ifdef CHROMIUM_THREADSAFE
    crLockMutex(&cr_teac.teacAPIMutex);
#endif
  sbuf= teac_getUnreadySendBuffer( cr_teac.tcomm,
				   CR_TEAC_BUFFER_PAD + conn->mtu );
#ifdef CHROMIUM_THREADSAFE
    crUnlockMutex(&cr_teac.teacAPIMutex);
#endif
  buf = (CRTeacBuffer*) sbuf->buf;
  buf->magic= (CR_TEAC_BUFFER_MAGIC | CR_TEAC_KIND_SEND);
  buf->len= 0; /* not used for this msg type */
  buf->u.sendBuffer= sbuf;
  sbuf->validSize = CR_TEAC_BUFFER_PAD; 

  msg = (CRMessage *)( buf+1 );
  msg->header.conn_id = conn->id;
    
#ifdef never
  crDebug("crTeacAlloc: sizes %ld, %ld; alloc %d at %lx",
	  ((Tcomm*)cr_teac.tcomm)->totalSendBufferBytesAllocated,
	  ((Tcomm*)cr_teac.tcomm)->totalRecvBufferBytesAllocated,
	  CR_TEAC_BUFFER_PAD + conn->mtu,(long)msg);
#endif
  return (void *)msg; 
}

void
crTeacFree( CRConnection *conn, void *buf )
{
  CRMessage* msg= (CRMessage*)buf;
  CRTeacBuffer* teacBuf= ((CRTeacBuffer*)msg) - 1;
  CRTeacBufferKind kind= (CRTeacBufferKind)(teacBuf->magic & 0xF);

  switch (kind) {
  case CR_TEAC_KIND_RECV: 
    {
      RBuffer* rbuf= teacBuf->u.rcvBuffer;
      conn->recv_credits += rbuf->validSize;
#ifdef never
      crDebug("crTeacFree: sizes %ld, %ld; freeing %ld at %lx",
	      ((Tcomm*)cr_teac.tcomm)->totalSendBufferBytesAllocated,
	      ((Tcomm*)cr_teac.tcomm)->totalRecvBufferBytesAllocated,
	      (rbuf->totSize),(long)msg);
      
      crDebug( "crTeacFree: freeing RBuffer at 0x%x from buffer 0x%x",
	       (int)rbuf,(int)buf);
#endif
#ifdef CHROMIUM_THREADSAFE
    crLockMutex(&cr_teac.teacAPIMutex);
#endif
      teac_Dispose( cr_teac.tcomm, rbuf );
#ifdef CHROMIUM_THREADSAFE
    crUnlockMutex(&cr_teac.teacAPIMutex);
#endif
      crTeacCreditIncrease( crTeacConnectionLookup( conn->teac_id ) );
    }
    break;
  case CR_TEAC_KIND_RECV_LISTED:
    {
      crFree(teacBuf);
    }
    break;
  case CR_TEAC_KIND_FLOW_RECV:
    {
      RBuffer* rbuf= teacBuf->u.rcvBuffer;
      conn->recv_credits += rbuf->validSize;
      crWarning("crTeacFree: misplaced CR_TEAC_KIND_FLOW_RECV message; freeing");
#ifdef CHROMIUM_THREADSAFE
    crLockMutex(&cr_teac.teacAPIMutex);
#endif
      teac_Dispose( cr_teac.tcomm, rbuf );
#ifdef CHROMIUM_THREADSAFE
    crUnlockMutex(&cr_teac.teacAPIMutex);
#endif
    }
    break;
  case CR_TEAC_KIND_SEND:
    {
      crWarning("crTeacFree: misplaced CR_TEAC_KIND_SEND message; losing it!");
    }
    break;
  case CR_TEAC_KIND_FLOW_SEND:
    {
      crWarning("crTeacFree: misplaced CR_TEAC_KIND_FLOW_SEND message; losing it!");
    }
    break;
  }
}

void
crTeacInstantReclaim( CRConnection *conn, CRMessage *msg )
{
  /* Do nothing- message will be freed after dispatching in crTeacRecv() */
#ifdef never
  crDebug("crTeacInstantReclaim: got %d credits from %s:%d",
	  ((CRMessageFlowControl*)msg)->credits,
	  conn->hostname,conn->teac_rank);
#endif
}

void
crTeacSend( CRConnection *conn, void **bufp,
	    const void *start, unsigned int len )
{
  CRMessage *buf = NULL;
  CRTeacBuffer* teacBuf= NULL;
  SBuffer *sbuf;
  CRTeacBufferKind kind;

#ifdef never
  crDebug("crTeacSend: beginning with allocation sizes %ld, %ld",
	  ((Tcomm*)cr_teac.tcomm)->totalSendBufferBytesAllocated,
	  ((Tcomm*)cr_teac.tcomm)->totalRecvBufferBytesAllocated);
#endif

  if ( cr_teac.inside_recv )
    crError( "crTeacSend:  cannot be called with crTeacRecv" );

  while ( crTeacRecv( ) )
    ;

  if ( conn->send_credits <= 0 )
    crTeacWaitForSendCredits( conn );

  if ( bufp == NULL ) {
    buf = (CRMessage*) crTeacAlloc( conn );
    crMemcpy( (void*)buf, start, len );
  }
  else {
    buf = (CRMessage *)( *bufp );
  }

  teacBuf= ((CRTeacBuffer*)buf)-1;
  kind= (CRTeacBufferKind)(teacBuf->magic & 0xF);
  if (kind != CR_TEAC_KIND_SEND)
    crError("crTeacSend: someone passed me a buffer of type %d!",kind);
  sbuf = teacBuf->u.sendBuffer;

  sbuf->validSize += len;
  conn->send_credits -= sbuf->validSize;

  crDebug("crTeacSend: entering send block to %d, msg type %d",
	  conn->teac_id,(((CRMessage*)start)->header.type-CR_MESSAGE_OPCODES));
#if CR_TEAC_USE_RECV_THREAD
  while (!teac_sendBufferAvailable(cr_teac.tcomm)) {
    /* Let's try just spinning */
#ifdef never
    sched_yield(); /* let recieve thread run while we wait */
#endif
  }
#endif
#ifdef never
  crDebug("crTeacSend: entering makeSendBufferReady to %d, msg type %d",
	  conn->teac_id,(((CRMessage*)start)->header.type-CR_MESSAGE_OPCODES));
#endif
#ifdef CHROMIUM_THREADSAFE
  crLockMutex(&cr_teac.teacAPIMutex);
#endif
  sbuf= teac_makeSendBufferReady( cr_teac.tcomm, sbuf );
#ifdef never
  crDebug("crTeacSend: Sending to id %d",conn->teac_id );
#endif
  if (bufp==NULL) 
    teac_Send( cr_teac.tcomm, &(conn->teac_id), 1, sbuf, teacBuf );
  else
    teac_Send( cr_teac.tcomm, &(conn->teac_id), 1, sbuf,
	       (void *)((char *) start - CR_TEAC_BUFFER_PAD ) );
#ifdef CHROMIUM_THREADSAFE
  crUnlockMutex(&cr_teac.teacAPIMutex);
#endif
#ifdef never
  crDebug("crTeacSend: finished Send of msg type %d to %d; loc %lx",
	  (((CRMessage*)start)->header.type-CR_MESSAGE_OPCODES),
	  conn->teac_id, (long)buf);
#endif
  /* I have the sense that we're supposed to set *bufp to NULL here,
   * but doing so breaks things.
   */
}

CRConnection *
crTeacSelect( void )
{
  int  count    = 0;
  int *teac_ids = crAlloc( sizeof( int ) * cr_teac.num_conns );
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
#ifdef CHROMIUM_THREADSAFE
  crLockMutex(&cr_teac.teacAPIMutex);
#endif
  ready_id = teac_Poll( cr_teac.tcomm, teac_ids, count );
#ifdef CHROMIUM_THREADSAFE
  crUnlockMutex(&cr_teac.teacAPIMutex);
#endif
  crFree(teac_ids);
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
  CRMessage* msg= NULL;
  CRTeacConnection *teac_conn= NULL;
  CRTeacBuffer* teacMsg= NULL;
  CRTeacBuffer* cloneMsg= NULL;
  int len = 0;

  cr_teac.inside_recv++;

  crTeacMaybeSendCredits( );

#if CR_TEAC_USE_RECV_THREAD

  teac_conn= cr_teac.credit_head;
  teacMsg= NULL;
  while (teac_conn ) {
    if (teac_conn->incoming_head) {
      /* This connection has at least one incoming message */
      crLockMutex(&(teac_conn->msgListMutex));
      teacMsg= teac_conn->incoming_head;
      teac_conn->incoming_head= teacMsg->u.incoming_next;
      if (teac_conn->incoming_tail==teacMsg)
	teac_conn->incoming_tail= NULL;
      crUnlockMutex(&(teac_conn->msgListMutex));
      msg= (CRMessage*)(teacMsg+1);
      len= teacMsg->len;
      conn= teac_conn->conn;
      crDebug("crTeacRecv: popped msg from incoming at %lx, length %d",
	      (long)msg, len);
      break;
    }
    teac_conn= teac_conn->credit_next;
  }
  if (!teacMsg) {
    cr_teac.inside_recv--;
    return 0;
  }
  
#else

  conn = crTeacSelect( );
  if ( !conn ) {
    cr_teac.inside_recv--;
    return 0;
  }

#ifdef never
  crDebug("crTeacRecv: beginning with allocation sizes %ld, %ld",
	  ((Tcomm*)cr_teac.tcomm)->totalSendBufferBytesAllocated,
	  ((Tcomm*)cr_teac.tcomm)->totalRecvBufferBytesAllocated);

  crDebug("crTeacRecv: entering teac_Recv on id %d",
	  conn->teac_id);
#endif
#ifdef CHROMIUM_THREADSAFE
  crLockMutex(&cr_teac.teacAPIMutex);
#endif
  rbuf = teac_Recv( cr_teac.tcomm, conn->teac_id );
#ifdef CHROMIUM_THREADSAFE
  crUnlockMutex(&cr_teac.teacAPIMutex);
#endif
#ifdef never
  crDebug("crTeacRecv: finished teac_Recv on id %d; got RBuffer at 0x%x",
	  conn->teac_id,(int)rbuf);
#endif
  if ( !rbuf ) {
    crError( "crTeacRecv:  teac_Recv( teac_id=%d ) failed",
	     conn->teac_id );
  }

  teacMsg= (CRTeacBuffer*)(rbuf->buf);
  msg= (CRMessage*)(teacMsg+1);
  len = rbuf->validSize - CR_TEAC_BUFFER_PAD; 
#ifdef never
  crDebug("crTeacRecv: new msg from teac_id %d is %ld bytes, type %d, at %lx",
	  conn->teac_id, rbuf->totSize,
	  (int)(msg->header.type-CR_MESSAGE_OPCODES),(long)msg);
#endif

  teacMsg->magic= (CR_TEAC_BUFFER_MAGIC | (int)CR_TEAC_KIND_RECV);
  teacMsg->u.rcvBuffer= rbuf;

#if CR_TEAC_COPY_MSGS_TO_MAIN_MEMORY
  cloneMsg= (CRTeacBuffer*)crAlloc(rbuf->validSize);
  crMemcpy( cloneMsg, teacMsg, rbuf->validSize );
  cloneMsg->magic= (CR_TEAC_BUFFER_MAGIC | (int)CR_TEAC_KIND_RECV_LISTED);
  cloneMsg->len= len; /* valid for LISTED messages */
  cloneMsg->u.rcvBuffer= NULL;
  crTeacFree(conn,msg); /* frees original message */
  msg= (CRMessage*)(cloneMsg+1); /* Leave the clone in its place */
#endif

#endif

  crNetDispatchMessage( cr_teac.recv_list, conn, msg, len );

  /* CR_MESSAGE_OPCODES is freed in
   * crserverlib/server_stream.c 
   *
   * OOB messages are the programmer's problem.  -- Humper 12/17/01 */
  if (msg->header.type != CR_MESSAGE_OPCODES
      && msg->header.type != CR_MESSAGE_OOB) {
    crTeacFree( conn, msg );
  }

  cr_teac.inside_recv--;

  return 1;
}

void crTeacInit( CRNetReceiveFuncList *rfl, CRNetCloseFuncList *cfl,
		 unsigned int mtu )
{
  (void) mtu;
  cr_teac.recv_list = rfl;
  cr_teac.close_list = cfl;
  int i;
  long key_sum= 0;

  if ( cr_teac.initialized )
    {
      return;
    }
  
  if ( crGetHostname( cr_teac.my_hostname, sizeof( cr_teac.my_hostname ) ) )
    {
      crError( "crTeacInit:  couldn't determine my own hostname!" );
    }
  strtok( cr_teac.my_hostname, "." );
  
  if ((cr_teac.low_node==NULL) || (cr_teac.high_node==NULL)) 
    {
      crError( "crTeacInit: node range is not set!" );
    }    

#ifdef CHROMIUM_THREADSAFE
  crInitMutex(&cr_teac.teacAPIMutex);
  cr_teac.recvThread= (pthread_t)0;
#endif

  key_sum= 0;
  for (i=0; i<(int)sizeof(cr_teac.key); i++) key_sum += cr_teac.key[i];
  if (key_sum==0) /* key not initialized */
    crStrncpy((char*)&(cr_teac.key), "This is pretty random!", 
	      sizeof(cr_teac.key));
#ifdef CHROMIUM_THREADSAFE
  crLockMutex(&cr_teac.teacAPIMutex);
#endif
  cr_teac.tcomm = teac_Init( cr_teac.low_node, cr_teac.high_node,
				 cr_teac.low_context, cr_teac.high_context,
				 cr_teac.my_rank, cr_teac.key );
#ifdef CHROMIUM_THREADSAFE
  crUnlockMutex(&cr_teac.teacAPIMutex);
#endif
  if ( !cr_teac.tcomm )
    crError( "crTeacInit:  teac_Init( %d, %d, %d ) failed",
	     cr_teac.low_context, cr_teac.high_context,
	     cr_teac.my_rank );

  cr_teac.num_conns = 0;

  cr_teac.credit_head = NULL;
  cr_teac.credit_tail = NULL;
  cr_teac.inside_recv = 0;

  cr_teac.initialized = 1;
}

void
crTeacAccept( CRConnection *conn, const char *hostname, unsigned short port )
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
    crError( "crTeacAccept:  Mothership didn't like my accept request" );
  }
  
  /* The response will contain the Teac information for the guy who accepted 
   * this connection.  The mothership will sit on the acceptrequest 
   * until someone connects. */
  sscanf( response, "%d %s %d %d",
	  &(conn->id), client_hostname, &(client_rank), &(client_endianness) );

  /* deal with endianness here */

  conn->hostname  = crStrdup(client_hostname);
  conn->teac_rank = client_rank;

  crDebug( "crTeacAccept:  opening connection to %s:%d",
	   conn->hostname, conn->teac_rank );  
#ifdef CHROMIUM_THREADSAFE
  crLockMutex(&cr_teac.teacAPIMutex);
#endif
  conn->teac_id = teac_getConnId( cr_teac.tcomm,
				  conn->hostname,
				  conn->teac_rank );
#ifdef CHROMIUM_THREADSAFE
  crUnlockMutex(&cr_teac.teacAPIMutex);
#endif
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
      crError( "crTeacDoConnect:  Mothership didn't like my connect request" );
    }
  
  /* The response will contain the Teac information for the guy who accepted 
   * this connection.  The mothership will sit on the connectrequest 
   * until someone accepts. */
  
  sscanf( response, "%d %d", &(conn->id), &(server_endianness) );

  /* deal with endianness here */

  crDebug( "crTeacDoConnect:  opening connection to %s:%d",
	   conn->hostname, conn->teac_rank );
#ifdef CHROMIUM_THREADSAFE
  crLockMutex(&cr_teac.teacAPIMutex);
#endif
  conn->teac_id = teac_getConnId( cr_teac.tcomm,
				  conn->hostname,
				  conn->teac_rank );
#ifdef CHROMIUM_THREADSAFE
  crUnlockMutex(&cr_teac.teacAPIMutex);
#endif
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
#ifdef CHROMIUM_THREADSAFE
  crLockMutex(&cr_teac.teacAPIMutex);
#endif
  teac_Close( cr_teac.tcomm );
#ifdef CHROMIUM_THREADSAFE
  crUnlockMutex(&cr_teac.teacAPIMutex);
#endif
  crNetCallCloseCallbacks(conn);
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
crTeacSendExact( CRConnection *conn, const void *buf, unsigned int len ) 
{
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

void crTeacSetRank( int rank )
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
  char* low= crStrdup( low_node );
  char* high= crStrdup( high_node );
  cr_teac.low_node  = strtok( low, "'" );
  cr_teac.high_node = strtok( high, "'" );  
  crDebug( "crTeacSetNodeRange:  low node=%s, high node=%s",
	   cr_teac.low_node, cr_teac.high_node );
}

void 
crTeacSetKey( const unsigned char* key, const int keyLength )
{
  int i;
  char msgbuf[3*TEAC_KEY_SIZE+1];
  char* runner= msgbuf;
  int bytesToCopy= (keyLength<TEAC_KEY_SIZE)?keyLength:TEAC_KEY_SIZE;
  crMemcpy(&(cr_teac.key),key,bytesToCopy);
  for (i=0; i<TEAC_KEY_SIZE; i++) { 
    sprintf(runner,"%02x,",cr_teac.key[i]);
    runner += 3;
  }
  *runner= '\0';
  crDebug( "crTeacSetKey: Teac key is [%s]", msgbuf );
}
