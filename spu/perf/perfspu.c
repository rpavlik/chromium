/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_spu.h"
#include "perfspu.h"
#include "cr_error.h"

void PERFSPU_APIENTRY perfspuBegin( GLenum mode )
{
	perf_spu.super.Begin( mode );
}

void PERFSPU_APIENTRY perfspuEnd( )
{
	perf_spu.super.End( );
}

void PERFSPU_APIENTRY perfspuDrawPixels( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels )
{
	perf_spu.super.DrawPixels( width, height, format, type, pixels );
}

void PERFSPU_APIENTRY perfspuReadPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels )
{
	perf_spu.super.ReadPixels( x, y, width, height, format, type, pixels );
}

void PERFSPU_APIENTRY perfspuVertex2d( GLdouble v0, GLdouble v1 )
{
	perf_spu.super.Vertex2d( v0, v1 );
}

void PERFSPU_APIENTRY perfspuVertex2dv( GLdouble *v0 )
{
	perf_spu.super.Vertex2dv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex2f( GLfloat v0, GLfloat v1 )
{
	perf_spu.super.Vertex2f( v0, v1 );
}

void PERFSPU_APIENTRY perfspuVertex2fv( GLfloat *v0 )
{
	perf_spu.super.Vertex2fv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex2i( GLint v0, GLint v1 )
{
	perf_spu.super.Vertex2i( v0, v1 );
}

void PERFSPU_APIENTRY perfspuVertex2iv( GLint *v0 )
{
	perf_spu.super.Vertex2iv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex2s( GLshort v0, GLshort v1 )
{
	perf_spu.super.Vertex2s( v0, v1 );
}

void PERFSPU_APIENTRY perfspuVertex2sv( GLshort *v0 )
{
	perf_spu.super.Vertex2sv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex3d( GLdouble v0, GLdouble v1, GLdouble v2)
{
	perf_spu.super.Vertex3d( v0, v1, v2 );
}

void PERFSPU_APIENTRY perfspuVertex3dv( GLdouble *v0 )
{
	perf_spu.super.Vertex3dv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex3f( GLfloat v0, GLfloat v1, GLfloat v2 )
{
	perf_spu.super.Vertex3f( v0, v1, v2 );
}

void PERFSPU_APIENTRY perfspuVertex3fv( GLfloat *v0)
{
	perf_spu.super.Vertex3fv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex3i( GLint v0, GLint v1, GLint v2 )
{
	perf_spu.super.Vertex3i( v0, v1, v2 );
}

void PERFSPU_APIENTRY perfspuVertex3iv( GLint *v0 )
{
	perf_spu.super.Vertex3iv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex3s( GLshort v0, GLshort v1, GLshort v2 )
{
	perf_spu.super.Vertex3s( v0, v1, v2 );
}

void PERFSPU_APIENTRY perfspuVertex3sv( GLshort *v0 )
{
	perf_spu.super.Vertex3sv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex4d( GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3 )
{
	perf_spu.super.Vertex4d( v0, v1, v2, v3 );
}

void PERFSPU_APIENTRY perfspuVertex4dv( GLdouble *v0 )
{
	perf_spu.super.Vertex4dv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex4f( GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 )
{
	perf_spu.super.Vertex4f( v0, v1, v2, v3 );
}

void PERFSPU_APIENTRY perfspuVertex4fv( GLfloat *v0 )
{
	perf_spu.super.Vertex4fv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex4i( GLint v0, GLint v1, GLint v2, GLint v3 )
{
	perf_spu.super.Vertex4i( v0, v1, v2, v3 );
}

void PERFSPU_APIENTRY perfspuVertex4iv( GLint *v0 )
{
	perf_spu.super.Vertex4iv( v0 );
}

void PERFSPU_APIENTRY perfspuVertex4s( GLshort v0, GLshort v1, GLshort v2, GLshort v3 )
{
	perf_spu.super.Vertex4s( v0, v1, v2, v3 );
}

void PERFSPU_APIENTRY perfspuVertex4sv( GLshort *v0 )
{
	perf_spu.super.Vertex4sv( v0 );
}

void PERFSPU_APIENTRY perfspuSwapBuffers( GLint window, GLint flags )
{
	perf_spu.super.SwapBuffers( window, flags );
}

SPUNamedFunctionTable perf_table[] = {
	{ "Begin", (SPUGenericFunction) perfspuBegin },
	{ "End", (SPUGenericFunction) perfspuEnd },
	{ "DrawPixels", (SPUGenericFunction) perfspuDrawPixels },
	{ "ReadPixels", (SPUGenericFunction) perfspuReadPixels },
	{ "Vertex2d", (SPUGenericFunction) perfspuVertex2d },
	{ "Vertex2dv", (SPUGenericFunction) perfspuVertex2dv },
	{ "Vertex2f", (SPUGenericFunction) perfspuVertex2f },
	{ "Vertex2fv", (SPUGenericFunction) perfspuVertex2fv },
	{ "Vertex2i", (SPUGenericFunction) perfspuVertex2i },
	{ "Vertex2iv", (SPUGenericFunction) perfspuVertex2iv },
	{ "Vertex2s", (SPUGenericFunction) perfspuVertex2s },
	{ "Vertex2sv", (SPUGenericFunction) perfspuVertex2sv },
	{ "Vertex3d", (SPUGenericFunction) perfspuVertex3d },
	{ "Vertex3dv", (SPUGenericFunction) perfspuVertex3dv },
	{ "Vertex3f", (SPUGenericFunction) perfspuVertex3f },
	{ "Vertex3fv", (SPUGenericFunction) perfspuVertex3fv },
	{ "Vertex3i", (SPUGenericFunction) perfspuVertex3i },
	{ "Vertex3iv", (SPUGenericFunction) perfspuVertex3iv },
	{ "Vertex3s", (SPUGenericFunction) perfspuVertex3s },
	{ "Vertex3sv", (SPUGenericFunction) perfspuVertex3sv },
	{ "Vertex4d", (SPUGenericFunction) perfspuVertex4d },
	{ "Vertex4dv", (SPUGenericFunction) perfspuVertex4dv },
	{ "Vertex4f", (SPUGenericFunction) perfspuVertex4f },
	{ "Vertex4fv", (SPUGenericFunction) perfspuVertex4fv },
	{ "Vertex4i", (SPUGenericFunction) perfspuVertex4i },
	{ "Vertex4iv", (SPUGenericFunction) perfspuVertex4iv },
	{ "Vertex4s", (SPUGenericFunction) perfspuVertex4s },
	{ "Vertex4sv", (SPUGenericFunction) perfspuVertex4sv },
	{ "SwapBuffers", (SPUGenericFunction) perfspuSwapBuffers },
	{ NULL, NULL }
};
