#include "cr_net.h"
#include "cr_mem.h"
#include "cr_error.h"

void crDevnullWriteExact( CRConnection *conn, void *buf, unsigned int len )
{
	(void) conn;
	(void) buf;
	(void) len;
}

void *crDevnullAlloc( CRConnection *conn )
{
	return crAlloc( conn->mtu );
}

void crDevnullSingleRecv( CRConnection *conn, void *buf, unsigned int len )
{
	crError( "You can't receive data on a devnull connection!" );
	(void) conn;
	(void) buf;
	(void) len;
}

void crDevnullFree( CRConnection *conn, void *buf )
{
	crFree( buf );
	(void) conn;
}

void crDevnullSend( CRConnection *conn, void **bufp,
				 void *start, unsigned int len )
{
	
	if (bufp)
	{
		// We're sending somethnig we've allocated.  It's now ours.
		// If the callers wants to send something else, he'll allocate
		// something else.
		//
		// ENFORCE IT!

		crDevnullFree( conn, *bufp );
	}
	(void) conn;
	(void) bufp;
	(void) start;
	(void) len;
}

int crDevnullRecv( void )
{
	crError( "You can't receive data on a DevNull network, stupid." );
	return 0;
}

void crDevnullInit( CRNetReceiveFunc recvFunc, CRNetCloseFunc closeFunc )
{
	(void) recvFunc;
	(void) closeFunc;
}

void crDevnullAccept( CRConnection *conn, unsigned short port )
{
	crError( "Well, you *could* accept a devnull client, but you'd be disappointed. ");
	(void) conn;
	(void) port;
}

int crDevnullDoConnect( CRConnection *conn )
{
	(void) conn; 
	return 1;
}

void crDevnullDoDisconnect( CRConnection *conn )
{
	(void) conn;
}

void crDevnullConnection( CRConnection *conn )
{
	conn->type  = CR_DROP_PACKETS;
	conn->Alloc = crDevnullAlloc;
	conn->Send  = crDevnullSend;
	conn->SendExact  = crDevnullWriteExact;
	conn->Recv  = crDevnullSingleRecv;
	conn->Free  = crDevnullFree;
	conn->Accept = crDevnullAccept;
	conn->Connect = crDevnullDoConnect;
	conn->Disconnect = crDevnullDoDisconnect;
}
