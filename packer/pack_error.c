/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */


#include "cr_error.h"
#include "cr_glwrapper.h"
#include <stdarg.h>
#include <stdio.h>


/*
 * This is called by the packer functions when an OpenGL error is
 * detected.
 */
void __PackError( int line, const char *file, GLenum error, char *format, ... )
{
	char errstr[8096];
	va_list args;

	va_start( args, format );
	vsprintf( errstr, format, args );
	va_end( args );

	/* temporary */
	crWarning("GL Error in packer: %s line %d: code=0x%x %s\n",
						file, line, (int) error, errstr);
}


/*
 * XXX eventually we should re-implement __PackError() above so that
 * errors can be routed back to the pack SPU (for example).
 * We can do this by offering a callback mechanism for the error
 * handler here, which the pack SPU would hook into.
 */
void __PackErrorHandler( void *function_pointer )
{
	/* not implemented yet */
	(void) function_pointer;
}
