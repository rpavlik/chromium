/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "packspu.h"
#include "cr_packfunctions.h"
#include "cr_glwrapper.h"
#include "cr_glstate.h"

void PACKSPU_APIENTRY packspu_ColorPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	crStateColorPointer( size, type, stride, pointer );
}

void PACKSPU_APIENTRY packspu_SecondaryColorPointerEXT( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	crStateSecondaryColorPointerEXT( size, type, stride, pointer );
}

void PACKSPU_APIENTRY packspu_VertexPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	crStateVertexPointer( size, type, stride, pointer );
}

void PACKSPU_APIENTRY packspu_TexCoordPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	crStateTexCoordPointer( size, type, stride, pointer );
}

void PACKSPU_APIENTRY packspu_NormalPointer( GLenum type, GLsizei stride, const GLvoid *pointer )
{
	crStateNormalPointer( type, stride, pointer );
}

void PACKSPU_APIENTRY packspu_EdgeFlagPointer( GLsizei stride, const GLvoid *pointer )
{
	crStateEdgeFlagPointer( stride, pointer );
}

void PACKSPU_APIENTRY packspu_GetPointerv( GLenum pname, GLvoid **params )
{
	crStateGetPointerv( pname, params );
}

void PACKSPU_APIENTRY packspu_ArrayElement( GLint index )
{
	if (pack_spu.swap)
	{
		crPackArrayElementSWAP( index, &(pack_spu.ctx->client) );
	}
	else
	{
		crPackArrayElement( index, &(pack_spu.ctx->client) );
	}
}

void PACKSPU_APIENTRY packspu_DrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices )
{
	if (pack_spu.swap)
	{
		crPackDrawElementsSWAP( mode, count, type, indices, &(pack_spu.ctx->client) );
	}
	else
	{
		crPackDrawElements( mode, count, type, indices, &(pack_spu.ctx->client) );
	}
}

void PACKSPU_APIENTRY packspu_DrawArrays( GLenum mode, GLint first, GLsizei count )
{
	if (pack_spu.swap)
	{
		crPackDrawArraysSWAP( mode, first, count, &(pack_spu.ctx->client) );
	}
	else
	{
		crPackDrawArrays( mode, first, count, &(pack_spu.ctx->client) );
	}
}

void PACKSPU_APIENTRY packspu_EnableClientState( GLenum array )
{
	crStateEnableClientState( array );
}

void PACKSPU_APIENTRY packspu_DisableClientState( GLenum array )
{
	crStateDisableClientState( array );
}
