#include <iostream.h>

#include <qapp.h>
#include <qlabel.h>

#include "cr_mem.h"
#include "cr_net.h"
#include "cr_url.h"
#include "cr_pack.h"
#include "../../spu/injector/injectorspu.h"

// Bad Chromium. BAD!
#ifdef Bool
#undef Bool
#endif

#include "cr-recorder.h"
#include "recorderdialogimpl.h"

CRPackContext* packCon ;
CRConnection* conn ; /* = crAlloc( sizeof(CRConnection*) ) ; */
CRPackBuffer packBuf ;
void* buf ;
int packSwap = 0 ;
crRecorderDialogImpl* dialog = 0 ;

int recorderReceiveData( CRConnection* conn, void* buf, unsigned int len )
{
	CRMessage* m = (CRMessage*) buf ;
	switch (m->header.type) {
		case CR_MESSAGE_OOB:
			if (dialog) {
				QString s ;
				s.sprintf( "%04d", ((CRMessageOobFrameInfo*)m)->frameNum ) ;
				dialog->currentFrameLabel->setText( s ) ;
				// cout << "Frame " << ((CRMessageOobFrameInfo*)m)->frameNum << endl ;
			}
			crNetFree( conn, buf ) ;
			return 1 ;
			break ;
		default:
			crError( "Unexpected message type %d!", m->header.type ) ;
	}
	return 0 ;
}

QNetRecv::QNetRecv( QObject* parent, const char* name )
	: QObject( parent, name )
{
}

void QNetRecv::timerEvent( QTimerEvent* )
{
	int i = 0 ;
	int c = 0 ;
	int l ;
	/* crNetRecv() will return 0 as soon as all incoming data has been queued.
	 * We stop after 5 loops because we need to handle GUI events, too. */
	while ( (l=crNetRecv()) && (i<5) ) {
		i++ ;
		c += l ;
	}
	//cout << "Timer. Recvd " << c << " over " << i << " loops" << endl ;
}

/* Stolen shamelessly from spu/pack/packspu_net.c */
static CRMessageOpcodes *
__prependHeader( CRPackBuffer *buf, unsigned int *len, unsigned int senderID )
{
	int num_opcodes;
	CRMessageOpcodes *hdr;

	CRASSERT( buf );
	CRASSERT( buf->opcode_current < buf->opcode_start );
	CRASSERT( buf->opcode_current >= buf->opcode_end );
	CRASSERT( buf->data_current > buf->data_start );
	CRASSERT( buf->data_current <= buf->data_end );

	num_opcodes = buf->opcode_start - buf->opcode_current;
	hdr = (CRMessageOpcodes *)
		( buf->data_start - ( ( num_opcodes + 3 ) & ~0x3 ) - sizeof(*hdr) );

	CRASSERT( (void *) hdr >= buf->pack );

	if (packSwap)
	{
		hdr->header.type = (CRMessageType) SWAP32(CR_MESSAGE_OPCODES);
		hdr->numOpcodes  = SWAP32(num_opcodes);
	}
	else
	{
		hdr->header.type = CR_MESSAGE_OPCODES;
		hdr->numOpcodes  = num_opcodes;
	}

	*len = buf->data_current - (unsigned char *) hdr;

	return hdr;
}


/* Actually send the packed-up OpenGL commands in the buffer */
void packerFlushTheToiletFirstThingInTheMorning( void* arg )
{
	CRMessageOpcodes* hdr ;
	CRPackBuffer* pBuf ;
	unsigned int len ;

	crPackGetBuffer( packCon, &packBuf ) ; 
	pBuf = &packBuf ;
	if ( pBuf->opcode_current == pBuf->opcode_start ) {
		crPackSetBuffer( packCon, pBuf ) ;
		crPackResetPointers( packCon ) ;
		return ;
	}

	hdr = __prependHeader( pBuf, &len, 0 ) ;
	if ( pBuf->holds_BeginEnd )
		crNetBarf( conn, &(pBuf->pack), hdr, len );
	else
		crNetSend( conn, &(pBuf->pack), hdr, len );

	pBuf->pack = crNetAlloc( conn );

	/* the network may have found a new mtu */
	pBuf->mtu = conn->mtu;

	crPackSetBuffer( packCon, pBuf );

	crPackResetPointers( packCon );
}

int main( int argc, char** argv )
{
	QApplication a( argc, argv ) ;

	char hostname[4096], protocol[4096] ;
	unsigned short port ;

	crNetInit( (CRNetReceiveFunc) recorderReceiveData, NULL ) ;
	if ( ! crParseURL( "tcpip://127.0.0.1:8194", protocol, hostname, &port, (unsigned short) INJECTORSPU_OOB_PORT) )
		crError( "Malformed URL: \"%s\"", "tcpip://127.0.0.1" ) ;
	conn = crNetConnectToServer( hostname, (short) port, 32768 /*mtu*/, 0 /*broker*/ ) ;

	buf = crNetAlloc( conn ) ;

	packCon = crPackNewContext( packSwap ) ;

	crPackFlushFunc( packCon, (CRPackFlushFunc) packerFlushTheToiletFirstThingInTheMorning ) ;
	crPackFlushArg( packCon, conn ) ;

	crPackInitBuffer( &packBuf, buf, conn->buffer_size /*server buffer size*/, conn->mtu ) ;
	CRASSERT( packBuf.data_end ) ;
	CRASSERT( packBuf.data_start < packBuf.data_end ) ;
	CRASSERT( packBuf.opcode_end ) ;
	CRASSERT( packBuf.opcode_end < packBuf.opcode_start ) ;
	crPackSetBuffer( packCon, &packBuf ) ;
	CRASSERT( packBuf.data_start == packCon->buffer.data_start ) ;
	CRASSERT( packBuf.data_end == packCon->buffer.data_end ) ;
	CRASSERT( packBuf.opcode_current == packCon->buffer.opcode_current ) ;
	CRASSERT( packBuf.opcode_start == packCon->buffer.opcode_start ) ;
	CRASSERT( packBuf.opcode_end == packCon->buffer.opcode_end ) ;
	crPackSetContext( packCon ) ;

	crRecorderDialogImpl* d = new crRecorderDialogImpl() ;
	d->setPackContext( packCon ) ;
	dialog = d ;
	QNetRecv* nr = new QNetRecv( 0, "CrNetRecv" ) ;
	nr->startTimer( 50 /* ms */ ) ;

	a.setMainWidget( d ) ;
	d->show() ;
	return a.exec() ;
}
