#ifndef NET_INTERNALS_H
#define NET_INTERNALS_H

#include "cr_bufpool.h"
#include "cr_threads.h"

CRConnection *__copy_of_crMothershipConnect( void );
int __copy_of_crMothershipReadResponse( CRConnection *conn, void *buf );
int __copy_of_crMothershipSendString( CRConnection *conn, char *response_buf, char *str, ... );
void __copy_of_crMothershipDisconnect( CRConnection *conn );
extern int __crSelect( int n, fd_set *readfds, int sec, int usec );

extern void crDevnullInit( CRNetReceiveFuncList *rfl, CRNetCloseFuncList *cfl, unsigned int mtu );
extern void crDevnullConnection( CRConnection *conn );
extern int crDevnullRecv( void );
extern CRConnection** crDevnullDump( int * num );

extern void crFileInit( CRNetReceiveFuncList *rfl, CRNetCloseFuncList *cfl, unsigned int mtu );
extern void crFileConnection( CRConnection *conn );
extern int crFileRecv( void );
extern CRConnection** crFileDump( int* num );

typedef enum {
        CRTCPIPMemory,
        CRTCPIPMemoryBig
} CRTCPIPBufferKind;

#define CR_TCPIP_BUFFER_MAGIC 0x89134532

typedef struct CRTCPIPBuffer {
	unsigned int          magic;
	CRTCPIPBufferKind     kind;
	unsigned int          len;
	unsigned int          allocated;
	unsigned int          pad;
} CRTCPIPBuffer;

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
} cr_tcpip_data;

extern cr_tcpip_data cr_tcpip;

extern void crTCPIPInit( CRNetReceiveFuncList *rfl, CRNetCloseFuncList *cfl, unsigned int mtu );
extern void crTCPIPConnection( CRConnection *conn );
extern int crTCPIPRecv( void );
extern CRConnection** crTCPIPDump( int *num );
extern int crTCPIPDoConnect( CRConnection *conn );
extern void crTCPIPDoDisconnect( CRConnection *conn );
extern int crTCPIPErrno( void );
extern char *crTCPIPErrorString( int err );
extern void crTCPIPAccept( CRConnection *conn, char *hostname, unsigned short port );
extern void crTCPIPWriteExact( CRConnection *conn, void *buf, unsigned int len );
extern void crTCPIPFree( CRConnection *conn, void *buf );
extern void *crTCPIPAlloc( CRConnection *conn );
extern void crTCPIPReadExact( CRConnection *conn, void *buf, unsigned int len );
extern int __tcpip_write_exact( CRSocket sock, void *buf, unsigned int len );
extern int __tcpip_read_exact( CRSocket sock, void *buf, unsigned int len );
extern void __tcpip_dead_connection( CRConnection *conn );

extern void crUDPTCPIPInit( CRNetReceiveFuncList *rfl, CRNetCloseFuncList *cfl, unsigned int mtu );
extern void crUDPTCPIPConnection( CRConnection *conn );
extern int crUDPTCPIPRecv( void );

extern void crGmInit( CRNetReceiveFuncList *rfl, CRNetCloseFuncList *cfl, unsigned int mtu );
extern void crGmConnection( CRConnection *conn );
extern int crGmRecv( void );
extern CRConnection** crGmDump( int *num );
extern int crGmDoConnect( CRConnection *conn );
extern void crGmDoDisconnect( CRConnection *conn );
extern int crGmErrno( void );
extern char *crGmErrorString( int err );
extern void crGmAccept( CRConnection *conn, char *hostname, unsigned short port );
extern void crGmSendExact( CRConnection *conn, void *buf, unsigned int len );
extern void crGmFree( CRConnection *conn, void *buf );
extern void *crGmAlloc( CRConnection *conn );
extern void crGmReadExact( CRConnection *conn, void *buf, unsigned int len );
extern void crGmBogusRecv( CRConnection *conn, void *buf, unsigned int len );
extern void crGmHandleNewMessage( CRConnection *conn, CRMessage *msg, unsigned int len );
extern void crGmInstantReclaim( CRConnection *conn, CRMessage *msg );
extern unsigned int crGmNodeId( void );
extern unsigned int crGmPortNum( void );

#ifdef TEAC_SUPPORT
extern void crTeacInit( CRNetReceiveFuncList *rfl, CRNetCloseFuncList *cfl,
			unsigned int mtu );
extern void crTeacConnection( CRConnection *conn );
extern int  crTeacRecv( void );
#endif

#ifdef TCSCOMM_SUPPORT
extern void crTcscommInit( CRNetReceiveFuncList *rfl, CRNetCloseFuncList *cfl,
		    unsigned int mtu );
extern void crTcscommConnection( CRConnection *conn );
extern int  crTcscommRecv( void );


#endif

extern CRConnection** crNetDump( int* num );

#endif /* NET_INTERNALS_H */
