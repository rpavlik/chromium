#ifndef CR_NET_H
#define CR_NET_H

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#pragma warning( push, 3 ) // shut up about warnings in YOUR OWN HEADER FILES!!!
#include <winsock2.h>
#endif

#include <stdio.h>

typedef struct CRConnection CRConnection;

typedef enum {
	CR_NO_CONNECTION,
	CR_TCPIP,
	CR_DROP_PACKETS,
	CR_FILE_TRACE,
	CR_GM,
	CR_MULTI_PACKET
} CRConnectionType;

#if defined(WINDOWS)
typedef SOCKET CRSocket;
#else
typedef int    CRSocket;
#endif

int crGetHostname( char *buf, unsigned int len );
int crGetPID(void);

typedef void (*CRVoidFunc)( void );
typedef void (*CRNetReceiveFunc)( CRConnection *conn, void *buf, unsigned int len );
typedef void (*CRNetConnectFunc)( CRConnection *conn );
typedef void (*CRNetCloseFunc)( unsigned int sender_id );

void crNetInit( CRNetReceiveFunc recvFunc, CRNetCloseFunc closeFunc );
void *crNetAlloc( CRConnection *conn );
void crNetSend( CRConnection *conn, void **bufp, void *start, unsigned int len );
void crNetSendExact( CRConnection *conn, void *start, unsigned int len );
void crNetSingleRecv( CRConnection *conn, void *buf, unsigned int len );
void crNetReadline( CRConnection *conn, void *buf );
int  crNetRecv( void );
void crNetFree( CRConnection *conn, void *buf );
void crNetAccept( CRConnection *conn, unsigned short port );
void crNetConnect( CRConnection *conn );
void crNetDisconnect( CRConnection *conn );
void crCloseSocket( CRSocket sock );
unsigned int crNetMTU( void );

struct CRConnection {
	CRConnectionType type;
	unsigned int sender_id;

	char *hostname;
	int port;

	void *(*Alloc)( void );
	void  (*Send)( CRConnection *conn, void **buf, void *start, unsigned int len );
	void  (*SendExact)( CRConnection *conn, void *start, unsigned int len );
	void  (*Recv)( CRConnection *conn, void *buf, unsigned int len );
	void  (*Free)( CRConnection *conn, void *buf );
	void  (*Accept)( CRConnection *conn, unsigned short port );
	void  (*Connect)( CRConnection *conn );
	void  (*Disconnect)( CRConnection *conn );

	/* logging */
	int total_bytes;

	/* credits for flow control */
	int send_credits;
	int recv_credits;

	/* TCP/IP */
	CRSocket tcp_socket;
	int index;

	/* FILE Tracing */
	char *filename;
	FILE *fp;

	/* Myrinet GM */
	unsigned int gm_node_id;
	unsigned int gm_port_num;
};

CRConnection *crNetConnectToServer( char *server, unsigned short default_port, int mtu );
CRConnection *crNetAcceptClient( char *protocol, unsigned short port );

#endif /* CR_NET_H */
