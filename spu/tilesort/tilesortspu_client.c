/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_packfunctions.h"
#include "cr_error.h"
#include "cr_net.h"
#include "tilesortspu.h"

void TILESORTSPU_APIENTRY tilesortspu_ArrayElement( GLint index )
{
	GET_CONTEXT(ctx);
	if (tilesort_spu.swap)
	{
		crPackArrayElementSWAP( index, &(ctx->client) );
	}
	else
	{
		crPackArrayElement( index, &(ctx->client) );
	}
}

void TILESORTSPU_APIENTRY tilesortspu_DrawArrays( GLenum mode, GLint first, GLsizei count )
{
	GET_CONTEXT(ctx);
	int i;

	if (count < 0)
	{
		crError("tilesortspu_DrawElements passed negative count: %d", count);
	}

	if (mode > GL_POLYGON)
	{
		crError("tilesortspu_DrawElements called with invalid mode: %d", mode);
	}

	if (ctx->current.inBeginEnd)
	{
		crError( "tilesortspu_DrawElements called in a Begin/End" );
	}

	tilesort_spu.self.Begin(mode);
	if (tilesort_spu.swap)
	{
		for (i=0; i<count; i++) 
		{
			crPackArrayElementSWAP(first++, &(ctx->client));
		}
	}
	else
	{
		for (i=0; i<count; i++) 
		{
			crPackArrayElement(first++, &(ctx->client));
		}
	}
	tilesort_spu.self.End();
}

void TILESORTSPU_APIENTRY tilesortspu_DrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
	GET_CONTEXT(ctx);
	int i;
	GLubyte *p = (GLubyte *)indices;

	if (count < 0)
	{
		crError("tilesortspu_DrawElements passed negative count: %d", count);
	}

	if (mode > GL_POLYGON)
	{
		crError("tilesortspu_DrawElements called with invalid mode: %d", mode);
	}

	if (type != GL_UNSIGNED_BYTE && type != GL_UNSIGNED_SHORT && type != GL_UNSIGNED_INT)
	{
		crError("tilesortspu_DrawElements called with invalid type: %d", type);
	}
	
	tilesort_spu.self.Begin( mode );
	switch (type) 
	{
	case GL_UNSIGNED_BYTE:
		if (tilesort_spu.swap)
		{
			for (i=0; i<count; i++)
			{
				crPackArrayElementSWAP((GLint) *p++, &(ctx->client));
			}
		}
		else
		{
			for (i=0; i<count; i++)
			{
				crPackArrayElement((GLint) *p++, &(ctx->client));
			}
		}
		break;
	case GL_UNSIGNED_SHORT:
		if (tilesort_spu.swap)
		{
			for (i=0; i<count; i++) 
			{
				crPackArrayElementSWAP((GLint) * (GLushort *) p, &(ctx->client));
				p+=sizeof (GLushort);
			}
		}
		else
		{
			for (i=0; i<count; i++) 
			{
				crPackArrayElement((GLint) * (GLushort *) p, &(ctx->client));
				p+=sizeof (GLushort);
			}
		}
		break;
	case GL_UNSIGNED_INT:
		if (tilesort_spu.swap)
		{
			for (i=0; i<count; i++) 
			{
				crPackArrayElementSWAP((GLint) * (GLuint *) p, &(ctx->client));
				p+=sizeof (GLuint);
			}
		}
		else
		{
			for (i=0; i<count; i++) 
			{
				crPackArrayElement((GLint) * (GLuint *) p, &(ctx->client));
				p+=sizeof (GLuint);
			}
		}
		break;
	default:
		crError( "this can't happen: crPackDrawElements" );
		break;
	}
	tilesort_spu.self.End();

	if(ctx->current.inBeginEnd)
	{
		crError( "tilesortspu_DrawElements called in a Begin/End" );
	}
}

void TILESORTSPU_APIENTRY tilesortspu_DrawRangeElements( GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)
{
	GET_CONTEXT(ctx);
	int i;
	GLubyte *p = (GLubyte *)indices;

	if (count < 0)
	{
		crError("tilesortspu_DrawRangeElements passed negative count: %d", count);
	}

	if (mode > GL_POLYGON)
	{
		crError("tilesortspu_DrawRangeElements called with invalid mode: %d", mode);
	}

	if (type != GL_UNSIGNED_BYTE && type != GL_UNSIGNED_SHORT && type != GL_UNSIGNED_INT)
	{
		crError("tilesortspu_DrawRangeElements called with invalid type: %d", type);
	}
	
	tilesort_spu.self.Begin( mode );
	switch (type) 
	{
	case GL_UNSIGNED_BYTE:
		if (tilesort_spu.swap)
		{
			for (i=start; i<count; i++)
			{
				crPackArrayElementSWAP((GLint) *p++, &(ctx->client));
			}
		}
		else
		{
			for (i=start; i<count; i++)
			{
				crPackArrayElement((GLint) *p++, &(ctx->client));
			}
		}
		break;
	case GL_UNSIGNED_SHORT:
		if (tilesort_spu.swap)
		{
			for (i=start; i<count; i++) 
			{
				crPackArrayElementSWAP((GLint) * (GLushort *) p, &(ctx->client));
				p+=sizeof (GLushort);
			}
		}
		else
		{
			for (i=start; i<count; i++) 
			{
				crPackArrayElement((GLint) * (GLushort *) p, &(ctx->client));
				p+=sizeof (GLushort);
			}
		}
		break;
	case GL_UNSIGNED_INT:
		if (tilesort_spu.swap)
		{
			for (i=start; i<count; i++) 
			{
				crPackArrayElementSWAP((GLint) * (GLuint *) p, &(ctx->client));
				p+=sizeof (GLuint);
			}
		}
		else
		{
			for (i=start; i<count; i++) 
			{
				crPackArrayElement((GLint) * (GLuint *) p, &(ctx->client));
				p+=sizeof (GLuint);
			}
		}
		break;
	default:
		crError( "this can't happen: crPackDrawRangeElements" );
		break;
	}
	tilesort_spu.self.End();

	if(ctx->current.inBeginEnd)
	{
		crError( "tilesortspu_DrawRangeElements called in a Begin/End" );
	}
}
