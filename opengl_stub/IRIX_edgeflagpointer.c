/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "chromium.h"
#include "api_templates.h"

/* This is necessary because IRIX disagrees with Windows about arg 2. */

void glEdgeFlagPointer( GLsizei stride, const GLboolean *pointer )
{
	glim.EdgeFlagPointer( stride, pointer );
}
