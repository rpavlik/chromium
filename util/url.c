#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "cr_string.h"
#include "cr_url.h"
#include "cr_error.h"

static int is_digit_string( const char *s )
{
	if ( !isdigit( *s ) )
	{
		return 0;
	}

	while ( *s && isdigit( *s ) )
	{
		s++;
	}

	return ( *s == 0 );
}

int crParseURL( char *url, char *protocol, char *hostname,
				unsigned short *port, unsigned short default_port )
{
	char *temp, *temp2;

	/* pull off the protocol */
	temp = crStrstr( url, "://" );
	if ( temp == NULL )
	{
		crStrcpy( protocol, "tcpip" );
		temp = url;
	}
	else
	{
		crStrncpy( protocol, url, temp-url );
		temp += 3;
	}

	/* handle a trailing :<digits> to specify the port */

	/* there might be a filename here */
	temp2 = crStrrchr( temp, '/' );
	if ( temp2 == NULL )
	{
		temp2 = crStrrchr( temp, '\\' );
	}
	if ( temp2 == NULL )
	{
		temp2 = temp;
	}

	temp2 = crStrrchr( temp2, ':' );
	if ( temp2 )
	{
		crStrncpy( hostname, temp, temp2 - temp );
		temp2++;
		if ( !is_digit_string( temp2 ) )
			goto bad_url;

		*port = (unsigned short) atoi( temp2 );
	}
	else
	{
		crStrcpy( hostname, temp );
		*port = default_port;
	}

	return 1;

 bad_url:
	crWarning( "URL: expected <protocol>://"
				   "<destination>[:<port>], what is \"%s\"?", url );
	return 0;
}
