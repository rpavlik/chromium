/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_NET_H
#define CR_NET_H

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#pragma warning( push, 3 ) /* shut up about warnings in YOUR OWN HEADER FILES!!! */
#include <winsock.h>
#endif

#include <stdio.h>

#ifndef WINDOWS
#include <sys/socket.h>
#ifndef DARWIN
#ifdef AF_INET6
/* getaddrinfo & co appeared with ipv6 */
#define ADDRINFO
#endif
#endif
#include <netinet/in.h>
#endif

#include "cr_protocol.h"
#include "cr_threads.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_SERVER_PORT 7000

/* If you change this, update DefaultMothershipPort in mothership.py */
#define DEFAULT_MOTHERSHIP_PORT 10000

typedef struct CRConnection CRConnection;

typedef enum {
	CR_NO_CONNECTION,
	CR_SDP,
	CR_TCPIP,
	CR_UDPTCPIP,
	CR_FILE,
	CR_GM,
	CR_IB,
	CR_TEAC,
	CR_TCSCOMM,
	CR_DROP_PACKETS
} CRConnectionType;

#if defined(WINDOWS)
typedef SOCKET CRSocket;
#else
typedef int    CRSocket;
#endif

typedef void (*CRVoidFunc)( void );
typedef int (*CRNetReceiveFunc)( CRConnection *conn, CRMessage *msg, unsigned int len );
typedef int (*CRNetConnectFunc)( CRConnection *conn );
typedef void (*CRNetCloseFunc)( unsigned int sender_id );

typedef struct __recvFuncList {
	CRNetReceiveFunc recv;
	struct __recvFuncList *next;
} CRNetReceiveFuncList;

typedef struct __closeFuncList {
	CRNetCloseFunc close;
	struct __closeFuncList *next;
} CRNetCloseFuncList;

typedef struct __messageListNode {
	CRMessage *mesg;    /* the actual message (header + payload) */
	unsigned int len;   /* length of message (header + payload) */
	CRConnection *conn; /* some messages are assoc. with specific connections*/
	struct __messageListNode *next;  /* next in list */
} CRMessageListNode;

typedef struct {
	CRMessageListNode *head, *tail;
	int numMessages;
	CRmutex lock;
	CRcondition nonEmpty;
} CRMessageList;


/**
 * Used to accumulate CR_MESSAGE_MULTI_BODY/TAIL chunks into one big buffer.
 */
typedef struct CRMultiBuffer {
	unsigned int  len;  /* current length (<= max) (with sizeof_buffer_header) */
	unsigned int  max;  /* size in bytes of data buffer */
	void         *buf;  /* data buffer */
} CRMultiBuffer;

/**
 * Chromium network connection (bidirectional).
 */
struct CRConnection {
	int ignore;
	CRConnectionType type;
	unsigned int id;         /* obtained from the mothership (if brokered) */

	/* List of messages that we've received on the network connection but
	 * nobody has yet consumed.
	 */
	CRMessageList messageList;

	CRMultiBuffer multi;

	unsigned int mtu;        /* max transmission unit size (in bytes) */
	unsigned int buffer_size;
	unsigned int krecv_buf_size;
	int broker;              /* is connection brokered through mothership? */
	int threaded;            /* is this a threaded connection? */
	int endianness, swap;
	int actual_network;      /* is this a real network? */

 	unsigned char *userbuf;
 	int userbuf_len;

	char *hostname;
	int port;

	/* To allocate a data buffer of size conn->buffer_size bytes */
	void *(*Alloc)( CRConnection *conn );
	/* To indicate the client's done with a data buffer */
	void  (*Free)( CRConnection *conn, void *buf );
	/* To send a data buffer.  If bufp is non-null, it must have been obtained
	 * from Alloc() and it'll be freed when Send() returns.
	 */
	void  (*Send)( CRConnection *conn, void **buf, const void *start, unsigned int len );
	/* To send a data buffer than can optionally be dropped on the floor */
	void  (*Barf)( CRConnection *conn, void **buf, const void *start, unsigned int len );
	/* To send 'len' bytes from buffer at 'start', no funny business */
	void  (*SendExact)( CRConnection *conn, const void *start, unsigned int len );
	/* To receive data.  'len' bytes will be placed into 'buf'. */
	void  (*Recv)( CRConnection *conn, void *buf, unsigned int len );
	/* To receive one message on the connection */
	void  (*RecvMsg)( CRConnection *conn );
	/* What's this??? */
	void  (*InstantReclaim)( CRConnection *conn, CRMessage *mess );
 /* Called when a full CR_MESSAGE_MULTI_HEAD/TAIL message has been received */
	void  (*HandleNewMessage)( CRConnection *conn, CRMessage *mess, unsigned int len );
	/* To accept a new connection from a client */
	void  (*Accept)( CRConnection *conn, const char *hostname, unsigned short port );
	/* To connect to a server (return 0 if error, 1 if success) */
	int  (*Connect)( CRConnection *conn );
	/* To disconnect from a server */
	void  (*Disconnect)( CRConnection *conn );

	unsigned int sizeof_buffer_header;

	/* logging */
	int total_bytes_sent;
	int total_bytes_recv;

	/* credits for flow control */
	int send_credits;
	int recv_credits;

	/* TCP/IP */
	CRSocket tcp_socket;
	int index;

	CRSocket sdp_socket;

	/* UDP/IP */
	CRSocket udp_socket;
#ifndef ADDRINFO
	struct sockaddr_in remoteaddr;
#else
	struct sockaddr_storage remoteaddr;
#endif

	/* UDP/TCP/IP */
	unsigned int seq;
	unsigned int ack;
	void *udp_packet;
	int udp_packetlen;

	/* FILE Tracing */
	enum { CR_FILE_WRITE, CR_FILE_READ } file_direction;
	char *filename;
	int fd;

	/* Myrinet GM */
	unsigned int gm_node_id;
	unsigned int gm_port_num;

	/* Mellanox IB */
	unsigned int ib_node_id;
	unsigned int ib_port_num;

	/* Quadrics Elan3 (teac) */
	int teac_id;
	int teac_rank;

	/* Quadrics Elan3 (tcscomm) */
	int tcscomm_id;
	int tcscomm_rank;
};


/*
 * Network functions
 */
extern int crGetHostname( char *buf, unsigned int len );
extern int crGetPID( void );

extern void crNetInit( CRNetReceiveFunc recvFunc, CRNetCloseFunc closeFunc );

extern void *crNetAlloc( CRConnection *conn );
extern void crNetFree( CRConnection *conn, void *buf );

extern void crNetAccept( CRConnection *conn, const char *hostname, unsigned short port );
extern int crNetConnect( CRConnection *conn );
extern void crNetDisconnect( CRConnection *conn );
extern void crCloseSocket( CRSocket sock );

extern void crNetSend( CRConnection *conn, void **bufp, const void *start, unsigned int len );
extern void crNetBarf( CRConnection *conn, void **bufp, const void *start, unsigned int len );
extern void crNetSendExact( CRConnection *conn, const void *start, unsigned int len );
extern void crNetSingleRecv( CRConnection *conn, void *buf, unsigned int len );
extern unsigned int crNetGetMessage( CRConnection *conn, CRMessage **message );
extern unsigned int crNetPeekMessage( CRConnection *conn, CRMessage **message );
extern void crNetReadline( CRConnection *conn, void *buf );
extern int crNetRecv( void );
extern void crNetDefaultRecv( CRConnection *conn, CRMessage *msg, unsigned int len );
extern void crNetDispatchMessage( CRNetReceiveFuncList *rfl, CRConnection *conn, CRMessage *msg, unsigned int len );

extern CRConnection *crNetConnectToServer( const char *server, unsigned short default_port, int mtu, int broker );
extern CRConnection *crNetAcceptClient( const char *protocol, const char *hostname, unsigned short port, unsigned int mtu, int broker );


extern void crInitMessageList(CRMessageList *list);
extern void crEnqueueMessage(CRMessageList *list, CRMessage *msg, unsigned int len, CRConnection *conn);
extern void crDequeueMessage(CRMessageList *list, CRMessage **msg, unsigned int *len, CRConnection **conn);



/*
 * Quadrics stuff
 */
#define CR_QUADRICS_DEFAULT_LOW_CONTEXT  32
#define CR_QUADRICS_DEFAULT_HIGH_CONTEXT 35

void crNetSetRank( int my_rank );
void crNetSetContextRange( int low_context, int high_context );
void crNetSetNodeRange( const char *low_node, const char *high_node );
void crNetSetKey( const unsigned char* key, const int keyLength );


#ifdef __cplusplus
}
#endif

#endif /* CR_NET_H */
