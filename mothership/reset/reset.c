#include "cr_mothership.h"

int main( int argc, char *argv[] )
{
	CRConnection *conn = crMothershipConnect( );
	crMothershipReset( conn );
	return 0;
}
