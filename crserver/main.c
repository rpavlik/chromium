#include "server.h"
#include "cr_net.h"
#include "cr_unpack.h"
#include "cr_protocol.h"
#include "cr_error.h"

CRServer cr_server;

void crServerGo( void )
{
	for(;;)
		crNetRecv();
}

void crServerRecv( CRConnection *conn, void *buf, unsigned int len )
{
	CRMessage *msg = (CRMessage *) buf;
	CRMessageOpcodes *ops;
	char *data_ptr;
	switch(msg->type)
	{
		case CR_MESSAGE_OPCODES:
			ops = (CRMessageOpcodes *) msg;
			data_ptr = (char *) buf + sizeof(*ops) + (( ops->numOpcodes +3 ) & ~0x03);

			crUnpack( data_ptr,data_ptr-1,ops->numOpcodes,&(cr_server.dispatch) );
			crNetFree( conn, msg );
			break;
		default:
			crError( "Bad message type in crServerRecv: %d\n", msg->type );
			break;
	}
}

void crServerClose( unsigned int id )
{
	crError( "Client disconnected!" );
}

int main( int argc, char *argv[] )
{
	crNetInit(crServerRecv, crServerClose);
	crServerGatherConfiguration();
	crServerInitDispatch();
	crUnpackSetReturnPointer( &(cr_server.return_ptr) );
	crUnpackSetWritebackPointer( &(cr_server.writeback_ptr) );
	crServerGo();
	return 0;
}
