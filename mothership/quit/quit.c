/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mothership.h"
#include "cr_error.h"

int main( int argc, char *argv[] )
{
	CRConnection *conn = crMothershipConnect( );
	if (conn)
	{
		crMothershipSendString( conn, NULL, "exit" );
	}
	else
	{
		crWarning( "\n\nNO MOTHERSHIP RUNNING?!\n\n");
	}
	return 0;
}
