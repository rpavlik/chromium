/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_glwrapper.h"
#include "api_templates.h"

// This is necessary because IRIX disagrees with Windows about arg 2.

void glEdgeFlagPointer( GLsizei stride, const GLboolean *pointer )
{
	glim.dgeFlagPointer( stride, pointer );
}
