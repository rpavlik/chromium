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
#include <winsock2.h>
#endif

#include <stdio.h>

#ifndef WINDOWS
#include <netinet/in.h>
#endif

#include "cr_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CRConnection CRConnection;

typedef enum {
	CR_NO_CONNECTION,
	CR_TCPIP,
	CR_UDPTCPIP,
	CR_FILE,
	CR_GM,
	CR_TEAC,
	CR_TCSCOMM,
	CR_DROP_PACKETS
} CRConnectionType;

#if defined(WINDOWS)
typedef SOCKET CRSocket;
#else
typedef int    CRSocket;
#endif

int crGetHostname( char *buf, unsigned int len );
int crGetPID(void);

#define CR_QUADRICS_DEFAULT_LOW_CONTEXT  32
#define CR_QUADRICS_DEFAULT_HIGH_CONTEXT 35

void crNetSetRank( int my_rank );
void crNetSetContextRange( int low_context, int high_context );
void crNetSetNodeRange( const char *low_node, const char *high_node );

void crNetDefaultRecv( CRConnection *conn, void *buf, unsigned int len );

typedef void (*CRVoidFunc)( void );
typedef int (*CRNetReceiveFunc)( CRConnection *conn, void *buf, unsigned int len );
typedef int (*CRNetConnectFunc)( CRConnection *conn );
typedef void (*CRNetCloseFunc)( unsigned int sender_id );

void crNetInit( CRNetReceiveFunc recvFunc, CRNetCloseFunc closeFunc );
void *crNetAlloc( CRConnection *conn );
void crNetSend( CRConnection *conn, void **bufp, void *start, unsigned int len );
void crNetBarf( CRConnection *conn, void **bufp, void *start, unsigned int len );
void crNetSendExact( CRConnection *conn, void *start, unsigned int len );
void crNetSingleRecv( CRConnection *conn, void *buf, unsigned int len );
unsigned int  crNetGetMessage( CRConnection *conn, CRMessage **message );
unsigned int  crNetPeekMessage( CRConnection *conn, CRMessage **message );
void crNetReadline( CRConnection *conn, void *buf );
int  crNetRecv( void );
void crNetFree( CRConnection *conn, void *buf );
void crNetAccept( CRConnection *conn, char *hostname, unsigned short port );
int crNetConnect( CRConnection *conn );
void crNetDisconnect( CRConnection *conn );
void crCloseSocket( CRSocket sock );

typedef struct __messageList {
	CRMessage *mesg;
	unsigned int len;
	struct __messageList *next;
} CRMessageList;

typedef struct CRMultiBuffer {
	unsigned int  len;
	unsigned int  max;
	void         *buf;
} CRMultiBuffer;

struct CRConnection {
	int ignore;
	CRConnectionType type;
	unsigned int id; /* from the mothership */

	CRMessageList *messageList, *messageTail;

	CRMultiBuffer multi;

	unsigned int mtu;
	unsigned int buffer_size;
	unsigned int krecv_buf_size;
	int broker;

	int endianness, swap;
	int actual_network;

 	unsigned char *userbuf;
 	int userbuf_len;

	char *hostname;
	int port;

	void *(*Alloc)( CRConnection *conn );
	void  (*Send)( CRConnection *conn, void **buf, void *start, unsigned int len );
	void  (*Barf)( CRConnection *conn, void **buf, void *start, unsigned int len );
	void  (*SendExact)( CRConnection *conn, void *start, unsigned int len );
	void  (*Recv)( CRConnection *conn, void *buf, unsigned int len );
	void  (*Free)( CRConnection *conn, void *buf );
	void  (*InstantReclaim)( CRConnection *conn, CRMessage *mess );
	void  (*HandleNewMessage)( CRConnection *conn, CRMessage *mess, unsigned int len );
	void  (*Accept)( CRConnection *conn, char *hostname, unsigned short port );
	int  (*Connect)( CRConnection *conn );
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

        /* Quadrics Elan3 (teac) */
        int teac_id;
        int teac_rank;

        /* Quadrics Elan3 (tcscomm) */
        int tcscomm_id;
        int tcscomm_rank;
};

CRConnection *crNetConnectToServer( char *server, unsigned short default_port, int mtu, int broker );
CRConnection *crNetAcceptClient( const char *protocol, char *hostname, unsigned short port, unsigned int mtu, int broker );

#ifdef __cplusplus
}
#endif

#endif /* CR_NET_H */
