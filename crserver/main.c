#include "server.h"
#include "cr_net.h"
#include "cr_unpack.h"
#include "cr_protocol.h"
#include "cr_error.h"
#include "cr_glstate.h"

CRServer cr_server;



void crServerClose( unsigned int id )
{
	crError( "Client disconnected!" );
	(void) id;
}

int main( int argc, char *argv[] )
{
	crNetInit(crServerRecv, crServerClose);
	crStateInit();
	crServerGatherConfiguration();
	crServerInitDispatch();
	crUnpackSetReturnPointer( &(cr_server.return_ptr) );
	crUnpackSetWritebackPointer( &(cr_server.writeback_ptr) );
	crServerSerializeRemoteStreams();
	return 0;
}
