/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */
	
#include "tilesortspu.h"

void TILESORTSPU_APIENTRY tilesortspu_GetDoublev( GLenum pname, GLdouble *params )
{
	switch (pname) {
	default:
		crStateGetDoublev( pname, params );
		break;
	}
}

void TILESORTSPU_APIENTRY tilesortspu_GetFloatv( GLenum pname, GLfloat *params )
{
	switch (pname) {
	default:
		crStateGetFloatv( pname, params );
		break;
	}
}

void TILESORTSPU_APIENTRY tilesortspu_GetIntegerv( GLenum pname, GLint *params )
{
	switch (pname) {
	default:
		crStateGetIntegerv( pname, params );
		break;
	}
}

void TILESORTSPU_APIENTRY tilesortspu_GetBooleanv( GLenum pname, GLboolean *params )
{
	switch (pname) {
	default:
		crStateGetBooleanv( pname, params );
		break;
	}
}
