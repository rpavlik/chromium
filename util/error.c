#include "cr_error.h"

#if defined (WINDOWS)
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#if defined(WINDOWS)
#include <windows.h>
#include <process.h>
#include <winsock.h>
#else
#include <unistd.h>
#endif

static char my_hostname[256];
static int my_pid = 0;

static void __getHostInfo( void )
{
	char *temp;
	if ( gethostname( my_hostname, sizeof( my_hostname ) ) )
	{
		strcpy( my_hostname, "Unable to get hostname" );
	}
	temp = my_hostname;
	while (*temp && *temp != '.')
		temp++;
	*temp = '\0';
	my_pid = (int) getpid();
}

void CRError( char *format, ... )
{
	va_list args;
	static char txt[8092];
	int offset;

	if (!my_hostname[0])
		__getHostInfo();
	offset = sprintf( txt, "Chromium Error(%s:%d): ", my_hostname, my_pid );
	va_start( args, format );
	vsprintf( txt + offset, format, args );
	fprintf( stderr, "%s\n", txt );
	va_end( args );
	exit(1);
}
