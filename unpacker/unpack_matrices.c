/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "unpacker.h"
#include "cr_glwrapper.h"
#include <memory.h>

void crUnpackMultMatrixd( void  )
{
#ifdef WIREGL_UNALIGNED_ACCESS_OKAY
	GLdouble *m = DATA_POINTER( 0, GLdouble );
#else
	GLdouble m[16];
	memcpy( m, DATA_POINTER( 0, GLdouble ), sizeof(m) );
#endif

	cr_unpackDispatch.MultMatrixd( m );
	INCR_DATA_PTR( 16*sizeof( GLdouble ) );
}

void crUnpackMultMatrixf( void  )
{
	GLfloat *m = DATA_POINTER( 0, GLfloat );

	cr_unpackDispatch.MultMatrixf( m );
	INCR_DATA_PTR( 16*sizeof( GLfloat ) );
}

void crUnpackLoadMatrixd( void  )
{
#ifdef WIREGL_UNALIGNED_ACCESS_OKAY
	GLdouble *m = DATA_POINTER( 0, GLdouble );
#else
	GLdouble m[16];
	memcpy( m, DATA_POINTER( 0, GLdouble ), sizeof(m) );
#endif

	cr_unpackDispatch.LoadMatrixd( m );
	INCR_DATA_PTR( 16*sizeof( GLdouble ) );
}

void crUnpackLoadMatrixf( void  )
{
	GLfloat *m = DATA_POINTER( 0, GLfloat );

	cr_unpackDispatch.LoadMatrixf( m );
	INCR_DATA_PTR( 16*sizeof( GLfloat ) );
}
