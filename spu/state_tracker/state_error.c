/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "state/cr_stateerror.h"
#include "state/cr_statetypes.h"
#include "cr_glstate.h"
#include "cr_error.h"
#include <stdarg.h>
#include <stdio.h>

void crStateError( int line, char *file, GLenum err, char *format, ... )
{
	char errstr[8096];
	va_list args;

	(void) err;

	va_start( args, format );
	vsprintf( errstr, format, args );
	va_end( args );
	crError( "Error in the CR State Manager in %s on line %d:\n%s", file, line, errstr );
}

GLenum STATE_APIENTRY crStateGetError(void)
{
	/* GL errors are fatal in Chromium, 
	 * so we're not getting here any other way. */

	return GL_NO_ERROR;
}
