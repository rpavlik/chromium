/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_error.h"
#include "cr_string.h"
#include "cr_net.h"

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static char my_hostname[256];
static int my_pid = 0;
static int canada = 0;

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
		char *env = getenv( "CR_CANADA" );
		if (env)
			canada = 1;
		first = 0;
	}
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
	if (!my_hostname[0])
		__getHostInfo();
	#ifdef WINDOWS
	if ((err = GetLastError()) != 0)
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
	if (canada)
		crStrcat( txt, ", eh?" );
	fprintf( stderr, "%s%s\n", txt, canada ? ", eh?" : "" );
#ifdef WINDOWS
	if (getenv( "CR_GUI_ERROR" ) != NULL)
		MessageBox( NULL, txt, "Chromium Error", MB_OK );
	else
#endif
	va_end( args );
#ifdef WINDOWS
	DebugBreak();
#endif
	exit(1);
}

void crWarning( char *format, ... )
{
	va_list args;
	static char txt[8092];
	int offset;

	__crCheckCanada();
	if (!my_hostname[0])
		__getHostInfo();
	offset = sprintf( txt, "CR Warning(%s:%d): ", my_hostname, my_pid );
	va_start( args, format );
	vsprintf( txt + offset, format, args );
	fprintf( stderr, "%s%s\n", txt, canada ? ", eh?" : "" );
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

	__crCheckCanada();
	if (!my_hostname[0])
		__getHostInfo();
#ifdef WINDOWS
	if ((err = GetLastError()) != 0)
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
	fprintf( stderr, "%s%s\n", txt, canada ? ", eh?" : "" );
	va_end( args );
#else /* RELEASE */
	(void) format;
#endif
}
