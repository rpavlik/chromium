/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "replicatespu.h"
#include "cr_packfunctions.h"
#include "cr_glstate.h"
#include "replicatespu_proto.h"

void REPLICATESPU_APIENTRY replicatespu_FogCoordPointerEXT( GLenum type, GLsizei stride, const GLvoid *pointer )
{
	crStateFogCoordPointerEXT( type, stride, pointer );
}

void REPLICATESPU_APIENTRY replicatespu_ColorPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	crStateColorPointer( size, type, stride, pointer );
}

void REPLICATESPU_APIENTRY replicatespu_SecondaryColorPointerEXT( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	crStateSecondaryColorPointerEXT( size, type, stride, pointer );
}

void REPLICATESPU_APIENTRY replicatespu_VertexPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	crStateVertexPointer( size, type, stride, pointer );
}

void REPLICATESPU_APIENTRY replicatespu_TexCoordPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	crStateTexCoordPointer( size, type, stride, pointer );
}

void REPLICATESPU_APIENTRY replicatespu_NormalPointer( GLenum type, GLsizei stride, const GLvoid *pointer )
{
	crStateNormalPointer( type, stride, pointer );
}

void REPLICATESPU_APIENTRY replicatespu_EdgeFlagPointer( GLsizei stride, const GLvoid *pointer )
{
	crStateEdgeFlagPointer( stride, pointer );
}

void REPLICATESPU_APIENTRY replicatespu_GetPointerv( GLenum pname, GLvoid **params )
{
	crStateGetPointerv( pname, params );
}

void REPLICATESPU_APIENTRY replicatespu_InterleavedArrays( GLenum format, GLsizei stride, const GLvoid *pointer )
{
	crStateInterleavedArrays( format, stride, pointer );
}

void REPLICATESPU_APIENTRY replicatespu_ArrayElement( GLint index )
{
        GET_CONTEXT(ctx);
	CRClientState *clientState = &(ctx->State->client);
	if (replicate_spu.swap)
	{
		crPackArrayElementSWAP( index, clientState );
	}
	else
	{
		crPackArrayElement( index, clientState );
	}
}

void REPLICATESPU_APIENTRY replicatespu_DrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices )
{
        GET_CONTEXT(ctx);
	CRClientState *clientState = &(ctx->State->client);
	if (replicate_spu.swap)
	{
		crPackDrawElementsSWAP( mode, count, type, indices, clientState );
	}
	else
	{
		crPackDrawElements( mode, count, type, indices, clientState );
	}
}

void REPLICATESPU_APIENTRY replicatespu_DrawRangeElements( GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices )
{
        GET_CONTEXT(ctx);
	CRClientState *clientState = &(ctx->State->client);
	if (replicate_spu.swap)
	{
		crPackDrawRangeElementsSWAP( mode, start, end, count, type, indices, clientState );
	}
	else
	{
		crPackDrawRangeElements( mode, start, end, count, type, indices, clientState );
	}
}

void REPLICATESPU_APIENTRY replicatespu_DrawArrays( GLenum mode, GLint first, GLsizei count )
{
        GET_CONTEXT(ctx);
	CRClientState *clientState = &(ctx->State->client);
	if (replicate_spu.swap)
	{
		crPackDrawArraysSWAP( mode, first, count, clientState );
	}
	else
	{
		crPackDrawArrays( mode, first, count, clientState );
	}
}

void REPLICATESPU_APIENTRY replicatespu_EnableClientState( GLenum array )
{
	crStateEnableClientState( array );
}

void REPLICATESPU_APIENTRY replicatespu_DisableClientState( GLenum array )
{
	crStateDisableClientState( array );
}

void REPLICATESPU_APIENTRY replicatespu_ClientActiveTextureARB( GLenum texUnit )
{
	crStateClientActiveTextureARB( texUnit );
}

void REPLICATESPU_APIENTRY replicatespu_MultiDrawArraysEXT( GLenum mode, GLint *first, GLsizei *count, GLsizei primcount )
{
        GET_CONTEXT(ctx);
	CRClientState *clientState = &(ctx->State->client);
	if (replicate_spu.swap)
	{
		crPackMultiDrawArraysEXTSWAP( mode, first, count, primcount, clientState );
	}
	else
	{
		crPackMultiDrawArraysEXT( mode, first, count, primcount, clientState );
	}
}

void REPLICATESPU_APIENTRY replicatespu_MultiDrawElementsEXT( GLenum mode, const GLsizei *count, GLenum type, const GLvoid **indices, GLsizei primcount )
{
        GET_CONTEXT(ctx);
	CRClientState *clientState = &(ctx->State->client);
	if (replicate_spu.swap)
	{
		crPackMultiDrawElementsEXTSWAP( mode, count, type, indices, primcount, clientState );
	}
	else
	{
		crPackMultiDrawElementsEXT( mode, count, type, indices, primcount, clientState );
	}
}
