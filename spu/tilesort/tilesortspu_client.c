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
	if (tilesort_spu.swap)
	{
		crPackArrayElementSWAP( index, &(tilesort_spu.ctx->client) );
	}
	else
	{
		crPackArrayElement( index, &(tilesort_spu.ctx->client) );
	}
}

void TILESORTSPU_APIENTRY tilesortspu_DrawArrays( GLenum mode, GLint first, GLsizei count )
{
	int i;

	if (count < 0)
	{
		crError("tilesortspu_DrawElements passed negative count: %d", count);
	}

	if (mode > GL_POLYGON)
	{
		crError("tilesortspu_DrawElements called with invalid mode: %d", mode);
	}

	if (tilesort_spu.ctx->current.inBeginEnd)
	{
		crError( "tilesortspu_DrawElements called in a Begin/End" );
	}

	tilesort_spu.self.Begin(mode);
	if (tilesort_spu.swap)
	{
		for (i=0; i<count; i++) 
		{
			crPackArrayElementSWAP(first++, &(tilesort_spu.ctx->client));
		}
	}
	else
	{
		for (i=0; i<count; i++) 
		{
			crPackArrayElement(first++, &(tilesort_spu.ctx->client));
		}
	}
	tilesort_spu.self.End();
}

void TILESORTSPU_APIENTRY tilesortspu_DrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
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
				crPackArrayElementSWAP((GLint) *p++, &(tilesort_spu.ctx->client));
			}
		}
		else
		{
			for (i=0; i<count; i++)
			{
				crPackArrayElement((GLint) *p++, &(tilesort_spu.ctx->client));
			}
		}
		break;
	case GL_UNSIGNED_SHORT:
		if (tilesort_spu.swap)
		{
			for (i=0; i<count; i++) 
			{
				crPackArrayElementSWAP((GLint) * (GLushort *) p, &(tilesort_spu.ctx->client));
				p+=sizeof (GLushort);
			}
		}
		else
		{
			for (i=0; i<count; i++) 
			{
				crPackArrayElement((GLint) * (GLushort *) p, &(tilesort_spu.ctx->client));
				p+=sizeof (GLushort);
			}
		}
		break;
	case GL_UNSIGNED_INT:
		if (tilesort_spu.swap)
		{
			for (i=0; i<count; i++) 
			{
				crPackArrayElementSWAP((GLint) * (GLuint *) p, &(tilesort_spu.ctx->client));
				p+=sizeof (GLuint);
			}
		}
		else
		{
			for (i=0; i<count; i++) 
			{
				crPackArrayElement((GLint) * (GLuint *) p, &(tilesort_spu.ctx->client));
				p+=sizeof (GLuint);
			}
		}
		break;
	default:
		crError( "this can't happen: crPackDrawElements" );
		break;
	}
	tilesort_spu.self.End();

	if(tilesort_spu.ctx->current.inBeginEnd)
	{
		crError( "tilesortspu_DrawArrays called in a Begin/End" );
	}
}
