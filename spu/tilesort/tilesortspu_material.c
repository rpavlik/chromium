/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_packfunctions.h"
#include "state/cr_statefuncs.h"
#include "tilesortspu.h"

void TILESORTSPU_APIENTRY tilesortspu_Materiali(GLenum face, GLenum mode, GLint param)
{
	crStateMateriali( face, mode, param );
	crPackMateriali( face, mode, param );
}

void TILESORTSPU_APIENTRY tilesortspu_Materialf(GLenum face, GLenum mode, GLfloat param)
{
	crStateMaterialf( face, mode, param );
	crPackMaterialf( face, mode, param );
}

void TILESORTSPU_APIENTRY tilesortspu_Materialiv(GLenum face, GLenum mode, const GLint *param)
{
	crStateMaterialiv( face, mode, param );
	crPackMaterialiv( face, mode, param );
}

void TILESORTSPU_APIENTRY tilesortspu_Materialfv(GLenum face, GLenum mode, const GLfloat *param)
{
	crStateMaterialfv( face, mode, param );
	crPackMaterialfv( face, mode, param );
}
