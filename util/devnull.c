#include "cr_net.h"
#include "cr_mem.h"
#include "cr_error.h"

void crDevnullWriteExact( CRConnection *conn, void *buf, unsigned int len )
{
	crWarning( "Writing exact data on a devnull connection" );
	(void) conn;
	(void) buf;
	(void) len;
}

void *crDevnullAlloc( void )
{
	crWarning( "Allocating memory on a devnull connection" );
	return crAlloc( crNetMTU() );
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
	crWarning( "Freeing memory by a devnull connection" );
	crFree( buf );
	(void) conn;
}

void crDevnullSend( CRConnection *conn, void **bufp,
				 void *start, unsigned int len )
{
	crWarning( "\"Sending\" Data on a devnull connection" );
	
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

void crDevnullDoConnect( CRConnection *conn )
{
	crWarning( "Making a Dev/Null connection" );
	(void) conn;
}

void crDevnullDoDisconnect( CRConnection *conn )
{
	crWarning( "\"Disconnecting\" a Dev/Null connection" );
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
