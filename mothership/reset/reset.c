#include "cr_mothership.h"

int main( int argc, char *argv[] )
{
	CRConnection *conn = crMothershipConnect( );
	crMothershipSendString( conn, NULL, "reset" );
	crMothershipDisconnect( conn );
	return 0;
}
