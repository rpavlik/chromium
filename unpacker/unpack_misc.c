/*
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "unpacker.h"

void crUnpackExtendChromiumParametervCR( void  )
{
	GLenum target = READ_DATA( 8, GLenum );
	GLenum type = READ_DATA( 12, GLenum );
	GLsizei count = READ_DATA( 16, GLsizei );
	GLvoid *values = DATA_POINTER( 20, GLvoid );

	cr_unpackDispatch.ChromiumParametervCR(target, type, count, values);


	/*
	INCR_VAR_PTR();
	*/
}


/**
 * This is just an accessor wrapping the cr_unpackData variable.
 * Fixes a crserver linker/loader problem involving shared library global vars.
 * The unpacker lib and crserver lib seemed to find the variable at different
 * addresses (!?!)
 */
const unsigned char *crUnpackGetDataPointer(void)
{
	return cr_unpackData;
}

