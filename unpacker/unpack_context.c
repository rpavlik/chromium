/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "unpacker.h"
#include "cr_glwrapper.h"

#include <stdio.h>

void crUnpackExtendCreateContext( void )
{
	GLint visual = READ_DATA( 8, GLint );
	GLint retVal;
	SET_RETURN_PTR( 12 );
	SET_WRITEBACK_PTR( 20 );
	retVal = cr_unpackDispatch.CreateContext( NULL, visual );
}

void crUnpackDestroyContext( void )
{
	GLint visual = READ_DATA( 8, GLint );
	cr_unpackDispatch.DestroyContext( NULL, visual );
	INCR_DATA_PTR( 12 );
}

void crUnpackMakeCurrent( void )
{
	GLint drawable = READ_DATA( 8, GLint );
	GLint ctx = READ_DATA( 12, GLint );
	cr_unpackDispatch.MakeCurrent( NULL, drawable, ctx );
	INCR_DATA_PTR( 16 );
}
