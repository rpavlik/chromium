#include "cr_environment.h"
#include <stdlib.h>

void crSetenv( const char *var, const char *value )
{
#ifdef LINUX
	setenv( var, value, 1 /* replace */ );
#else
	unsigned long len;
	char *buf;

	len = crStrlen(var) + 1 + crStrlen(value) + 1;
	buf = (char *) malloc( len );
	sprintf( buf, "%s=%s", var, value );
	putenv( buf );

	debug( "%s\n", buf );

	/* don't free the buf, the string is *part* of the environment,
	 * and can't be reclaimed */
#endif
}

char *crGetenv( const char *var )
{
	return getenv( var );
}
