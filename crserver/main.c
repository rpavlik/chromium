#include "server.h"

CRServer cr_server;

void crServerGo( void )
{
}

int main( int argc, char *argv[] )
{
	crServerGatherConfiguration();
	crServerGo();
}
