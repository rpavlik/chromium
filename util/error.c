/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_error.h"
#include "cr_environment.h"
#include "cr_string.h"
#include "cr_net.h"

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>

static char my_hostname[256];
static int my_pid = 0;
static int canada = 0;
static int swedish_chef = 0;
static int australia = 0;

static void __getHostInfo( void )
{
	char *temp;
	if ( crGetHostname( my_hostname, sizeof( my_hostname ) ) )
	{
		crStrcpy( my_hostname, "????" );
	}
	temp = crStrchr( my_hostname, '.' );
	if (temp)
	{
		*temp = '\0';
	}
	my_pid = crGetPID();
}

static void __crCheckCanada(void)
{
	static int first = 1;
	if (first)
	{
		const char *env = crGetenv( "CR_CANADA" );
		if (env)
			canada = 1;
		first = 0;
	}
}

static void __crCheckSwedishChef(void)
{
	static int first = 1;
	if (first)
	{
		const char *env = crGetenv( "CR_SWEDEN" );
		if (env)
			swedish_chef = 1;
		first = 0;
	}
}

static void __crCheckAustralia(void)
{
	static int first = 1;
	if (first)
	{
		const char *env = crGetenv( "CR_AUSTRALIA" );
		const char *env2 = crGetenv( "CR_AUSSIE" );
		if (env || env2)
			australia = 1;
		first = 0;
	}
}

static void outputChromiumMessage( FILE *output, char *str )
{
	fprintf( output, "%s%s%s%s\n", str, 
			swedish_chef ? " BORK BORK BORK!" : "",
			canada ? ", eh?" : "",
			australia ? ", mate!" : ""
			);
	fflush( output );
}

void crError( char *format, ... )
{
	va_list args;
	static char txt[8092];
	int offset;
#ifdef WINDOWS
	DWORD err;
#endif

	__crCheckCanada();
	__crCheckSwedishChef();
	__crCheckAustralia();
	if (!my_hostname[0])
		__getHostInfo();
#ifdef WINDOWS
	if ((err = GetLastError()) != 0 && crGetenv( "CR_WINDOWS_ERRORS" ) != NULL )
	{
		static char buf[8092], *temp;

		SetLastError(0);
		sprintf( buf, "err=%d", err );

		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_MAX_WIDTH_MASK, NULL, err,
				MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
				(LPTSTR) &temp, 0, NULL );
		if ( temp )
		{
			crStrncpy( buf, temp, sizeof(buf)-1 );
			buf[sizeof(buf)-1] = 0;
		}

		temp = buf + crStrlen(buf) - 1;
		while ( temp > buf && isspace( *temp ) )
		{
			*temp = '\0';
			temp--;
		}

		offset = sprintf( txt, "\t-----------------------\n\tWindows ERROR: %s\n\t----------------------\nCR Error(%s:%d): ", buf, my_hostname, my_pid );
	}
	else
	{
		offset = sprintf( txt, "CR Error(%s:%d): ", my_hostname, my_pid );
	}
#else
	offset = sprintf( txt, "CR Error(%s:%d): ", my_hostname, my_pid );
#endif
	va_start( args, format );
	vsprintf( txt + offset, format, args );
	outputChromiumMessage( stderr, txt );
#ifdef WINDOWS
	if (crGetenv( "CR_GUI_ERROR" ) != NULL)
	{
		MessageBox( NULL, txt, "Chromium Error", MB_OK );
	}
	else
	{	
#endif
		va_end( args );
#ifdef WINDOWS
	}
	if (crGetenv( "CR_DEBUG_ON_ERROR" ) != NULL)
	{
		DebugBreak();
	}
#endif
	/* Give chance for things to close down */
	raise( SIGTERM );
 
	exit(1);
}

void crWarning( char *format, ... )
{
	va_list args;
	static char txt[8092];
	int offset;

	__crCheckCanada();
	__crCheckSwedishChef();
	__crCheckAustralia();
	if (!my_hostname[0])
		__getHostInfo();
	offset = sprintf( txt, "CR Warning(%s:%d): ", my_hostname, my_pid );
	va_start( args, format );
	vsprintf( txt + offset, format, args );
	outputChromiumMessage( stderr, txt );
	va_end( args );
}

void crDebug( char *format, ... )
{
#ifndef NDEBUG
	va_list args;
	static char txt[8092];
	int offset;
#ifdef WINDOWS
	DWORD err;
#endif
	static FILE *output;
	static int first_time = 1;

	if (first_time)
	{
		const char *fname = crGetenv( "CR_DEBUG_FILE" );
		first_time = 0;
		if (fname)
		{
			output = fopen( fname, "w" );
			if (!output)
			{
				crError( "Couldn't open debug log %s", fname ); 
			}
		}
		else
		{
			output = stderr;
		}
	}

	__crCheckCanada();
	__crCheckSwedishChef();
	__crCheckAustralia();
	if (!my_hostname[0])
		__getHostInfo();
#ifdef WINDOWS
	if ((err = GetLastError()) != 0 && crGetenv( "CR_WINDOWS_ERRORS" ) != NULL )
	{
		static char buf[8092], *temp;

		SetLastError(0);
		sprintf( buf, "err=%d", err );

		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_MAX_WIDTH_MASK, NULL, err,
				MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
				(LPTSTR) &temp, 0, NULL );
		if ( temp )
		{
			crStrncpy( buf, temp, sizeof(buf)-1 );
			buf[sizeof(buf)-1] = 0;
		}

		temp = buf + crStrlen(buf) - 1;
		while ( temp > buf && isspace( *temp ) )
		{
			*temp = '\0';
			temp--;
		}

		offset = sprintf( txt, "\t-----------------------\n\tWindows ERROR: %s\n\t-----------------\nCR Debug(%s:%d): ", buf, my_hostname, my_pid );
	}
	else
	{
		offset = sprintf( txt, "CR Debug(%s:%d): ", my_hostname, my_pid );
	}
#else
	offset = sprintf( txt, "CR Debug(%s:%d): ", my_hostname, my_pid );
#endif
	va_start( args, format );
	vsprintf( txt + offset, format, args );
	outputChromiumMessage( output, txt );
	va_end( args );
#else /* RELEASE */
	(void) format;
#endif
}
