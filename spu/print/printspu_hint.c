/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_glwrapper.h"
#include "cr_applications.h"
#include "printspu.h"

void PRINT_APIENTRY printHint( GLenum target, GLenum name )
{
	if (target == CR_PRINTSPU_STRING_HINT)
	{
		char *str = (char *) name;
		fprintf( print_spu.fp, "%s\n", str );
		fflush( print_spu.fp );
	}
	else
	{
		fprintf( print_spu.fp, "Hint( %d, %d )\n", target, name );
		fflush( print_spu.fp );
		print_spu.passthrough.Hint( target, name );
	}
}
