#include "cr_mothership.h"
#include "cr_error.h"

int main( int argc, char *argv[] )
{
	CRConnection *conn = crMothershipConnect( );
	if (conn)
	{
		crMothershipSendString( conn, NULL, "reset" );
		crMothershipDisconnect( conn );
	}
	else
	{
		crWarning( "\n\nNO MOTHERSHIP RUNNING?!\n\n");
	}
	return 0;
}
