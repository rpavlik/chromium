/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "packspu.h"
#include "cr_packfunctions.h"
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

void PACKSPU_APIENTRY packspu_InterleavedArrays( GLenum format, GLsizei stride, const GLvoid *pointer )
{
	crStateInterleavedArrays( format, stride, pointer );
}

void PACKSPU_APIENTRY packspu_ArrayElement( GLint index )
{
        GET_CONTEXT(ctx);
	CRClientState *clientState = &(ctx->clientState->client);
	if (pack_spu.swap)
	{
		crPackArrayElementSWAP( index, clientState );
	}
	else
	{
		crPackArrayElement( index, clientState );
	}
}

void PACKSPU_APIENTRY packspu_DrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices )
{
        GET_CONTEXT(ctx);
	CRClientState *clientState = &(ctx->clientState->client);
	if (pack_spu.swap)
	{
		crPackDrawElementsSWAP( mode, count, type, indices, clientState );
	}
	else
	{
		crPackDrawElements( mode, count, type, indices, clientState );
	}
}

void PACKSPU_APIENTRY packspu_DrawRangeElements( GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices )
{
        GET_CONTEXT(ctx);
	CRClientState *clientState = &(ctx->clientState->client);
	if (pack_spu.swap)
	{
		crPackDrawRangeElementsSWAP( mode, start, end, count, type, indices, clientState );
	}
	else
	{
		crPackDrawRangeElements( mode, start, end, count, type, indices, clientState );
	}
}

void PACKSPU_APIENTRY packspu_DrawArrays( GLenum mode, GLint first, GLsizei count )
{
        GET_CONTEXT(ctx);
	CRClientState *clientState = &(ctx->clientState->client);
	if (pack_spu.swap)
	{
		crPackDrawArraysSWAP( mode, first, count, clientState );
	}
	else
	{
		crPackDrawArrays( mode, first, count, clientState );
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

void PACKSPU_APIENTRY packspu_ClientActiveTextureARB( GLenum texUnit )
{
	crStateClientActiveTextureARB( texUnit );
}

