/* Copyright (c) 2001, Stanford University
* All rights reserved
*
* See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_spu.h"
#include "cr_error.h"
#include "state/cr_limits.h"
#include "arrayspu.h"

ArraySPU array_spu;

static void ARRAYSPU_APIENTRY arrayspu_ArrayElement( GLint index )
{
	unsigned char *p;
	CRClientState *c = &(array_spu.ctx->client);
  unsigned int unit;

	if (c->e.enabled)
	{
		array_spu.self.EdgeFlagv(c->e.p + index*c->e.stride);
	}
	for (unit = 0 ; unit < array_spu.ctx->limits.maxTextureUnits ; unit++)
	{
		if (c->t[unit].enabled)
		{
			p = c->t[unit].p + index*c->t[unit].stride;
			switch (c->t[unit].type)
			{
				case GL_SHORT:
					switch (c->t[unit].size)
					{
						case 1: array_spu.self.MultiTexCoord1svARB(GL_TEXTURE0_ARB + unit, (GLshort *)p); break;
						case 2: array_spu.self.MultiTexCoord2svARB(GL_TEXTURE0_ARB + unit, (GLshort *)p); break;
						case 3: array_spu.self.MultiTexCoord3svARB(GL_TEXTURE0_ARB + unit, (GLshort *)p); break;
						case 4: array_spu.self.MultiTexCoord4svARB(GL_TEXTURE0_ARB + unit, (GLshort *)p); break;
					}
					break;
				case GL_INT:
					switch (c->t[unit].size)
					{
						case 1: array_spu.self.MultiTexCoord1ivARB(GL_TEXTURE0_ARB + unit, (GLint *)p); break;
						case 2: array_spu.self.MultiTexCoord2ivARB(GL_TEXTURE0_ARB + unit, (GLint *)p); break;
						case 3: array_spu.self.MultiTexCoord3ivARB(GL_TEXTURE0_ARB + unit, (GLint *)p); break;
						case 4: array_spu.self.MultiTexCoord4ivARB(GL_TEXTURE0_ARB + unit, (GLint *)p); break;
					}
					break;
				case GL_FLOAT:
					switch (c->t[unit].size)
					{
						case 1: array_spu.self.MultiTexCoord1fvARB(GL_TEXTURE0_ARB + unit, (GLfloat *)p); break;
						case 2: array_spu.self.MultiTexCoord2fvARB(GL_TEXTURE0_ARB + unit, (GLfloat *)p); break;
						case 3: array_spu.self.MultiTexCoord3fvARB(GL_TEXTURE0_ARB + unit, (GLfloat *)p); break;
						case 4: array_spu.self.MultiTexCoord4fvARB(GL_TEXTURE0_ARB + unit, (GLfloat *)p); break;
					}
					break;
				case GL_DOUBLE:
					switch (c->t[unit].size)
					{
						case 1: array_spu.self.MultiTexCoord1dvARB(GL_TEXTURE0_ARB + unit, (GLdouble *)p); break;
						case 2: array_spu.self.MultiTexCoord2dvARB(GL_TEXTURE0_ARB + unit, (GLdouble *)p); break;
						case 3: array_spu.self.MultiTexCoord3dvARB(GL_TEXTURE0_ARB + unit, (GLdouble *)p); break;
						case 4: array_spu.self.MultiTexCoord4dvARB(GL_TEXTURE0_ARB + unit, (GLdouble *)p); break;
					}
					break;
			}
		}
	}
	if (c->i.enabled)
	{
		p = c->i.p + index*c->i.stride;
		switch (c->i.type)
		{
			case GL_SHORT: array_spu.self.Indexsv((GLshort *)p); break;
			case GL_INT: array_spu.self.Indexiv((GLint *)p); break;
			case GL_FLOAT: array_spu.self.Indexfv((GLfloat *)p); break;
			case GL_DOUBLE: array_spu.self.Indexdv((GLdouble *)p); break;
		}
	}
	if (c->c.enabled)
	{
		p = c->c.p + index*c->c.stride;
		switch (c->c.type)
		{
			case GL_BYTE:
				switch (c->c.size)
				{
					case 3: array_spu.self.Color3bv((GLbyte *)p); break;
					case 4: array_spu.self.Color4bv((GLbyte *)p); break;
				}
				break;
			case GL_UNSIGNED_BYTE:
				switch (c->c.size)
				{
					case 3: array_spu.self.Color3ubv((GLubyte *)p); break;
					case 4: array_spu.self.Color4ubv((GLubyte *)p); break;
				}
				break;
			case GL_SHORT:
				switch (c->c.size)
				{
					case 3: array_spu.self.Color3sv((GLshort *)p); break;
					case 4: array_spu.self.Color4sv((GLshort *)p); break;
				}
				break;
			case GL_UNSIGNED_SHORT:
				switch (c->c.size)
				{
					case 3: array_spu.self.Color3usv((GLushort *)p); break;
					case 4: array_spu.self.Color4usv((GLushort *)p); break;
				}
				break;
			case GL_INT:
				switch (c->c.size)
				{
					case 3: array_spu.self.Color3iv((GLint *)p); break;
					case 4: array_spu.self.Color4iv((GLint *)p); break;
				}
				break;
			case GL_UNSIGNED_INT:
				switch (c->c.size)
				{
					case 3: array_spu.self.Color3uiv((GLuint *)p); break;
					case 4: array_spu.self.Color4uiv((GLuint *)p); break;
				}
				break;
			case GL_FLOAT:
				switch (c->c.size)
				{
					case 3: array_spu.self.Color3fv((GLfloat *)p); break;
					case 4: array_spu.self.Color4fv((GLfloat *)p); break;
				}
				break;
			case GL_DOUBLE:
				switch (c->c.size)
				{
					case 3: array_spu.self.Color3dv((GLdouble *)p); break;
					case 4: array_spu.self.Color4dv((GLdouble *)p); break;
				}
				break;
		}
	}
	if (c->n.enabled)
	{
		p = c->n.p + index*c->n.stride;
		switch (c->n.type)
		{
			case GL_BYTE: array_spu.self.Normal3bv((GLbyte *)p); break;
			case GL_SHORT: array_spu.self.Normal3sv((GLshort *)p); break;
			case GL_INT: array_spu.self.Normal3iv((GLint *)p); break;
			case GL_FLOAT: array_spu.self.Normal3fv((GLfloat *)p); break;
			case GL_DOUBLE: array_spu.self.Normal3dv((GLdouble *)p); break;
		}
	}
#ifdef CR_EXT_secondary_color
	if (c->s.enabled)
	{
		p = c->s.p + index*c->s.stride;
		switch (c->s.type)
		{
			case GL_BYTE:
				array_spu.self.SecondaryColor3bvEXT((GLbyte *)p); break;
			case GL_UNSIGNED_BYTE:
				array_spu.self.SecondaryColor3ubvEXT((GLubyte *)p); break;
			case GL_SHORT:
				array_spu.self.SecondaryColor3svEXT((GLshort *)p); break;
			case GL_UNSIGNED_SHORT:
				array_spu.self.SecondaryColor3usvEXT((GLushort *)p); break;
			case GL_INT:
				array_spu.self.SecondaryColor3ivEXT((GLint *)p); break;
			case GL_UNSIGNED_INT:
				array_spu.self.SecondaryColor3uivEXT((GLuint *)p); break;
			case GL_FLOAT:
				array_spu.self.SecondaryColor3fvEXT((GLfloat *)p); break;
			case GL_DOUBLE:
				array_spu.self.SecondaryColor3dvEXT((GLdouble *)p); break;
		}
	}
#endif
	if (c->v.enabled)
	{
		p = c->v.p + (index*c->v.stride);

		switch (c->v.type)
		{
			case GL_SHORT:
				switch (c->v.size)
				{
					case 2: array_spu.self.Vertex2sv((GLshort *)p); break;
					case 3: array_spu.self.Vertex3sv((GLshort *)p); break;
					case 4: array_spu.self.Vertex4sv((GLshort *)p); break;
				}
				break;
			case GL_INT:
				switch (c->v.size)
				{
					case 2: array_spu.self.Vertex2iv((GLint *)p); break;
					case 3: array_spu.self.Vertex3iv((GLint *)p); break;
					case 4: array_spu.self.Vertex4iv((GLint *)p); break;
				}
				break;
			case GL_FLOAT:
				switch (c->v.size)
				{
					case 2: array_spu.self.Vertex2fv((GLfloat *)p); break;
					case 3: array_spu.self.Vertex3fv((GLfloat *)p); break;
					case 4: array_spu.self.Vertex4fv((GLfloat *)p); break;
				}
				break;
			case GL_DOUBLE:
				switch (c->v.size)
				{
					case 2: array_spu.self.Vertex2dv((GLdouble *)p); break;
					case 3: array_spu.self.Vertex3dv((GLdouble *)p); break;
					case 4: array_spu.self.Vertex4dv((GLdouble *)p); break;
				}
				break;
		}
	}
}

static void ARRAYSPU_APIENTRY arrayspu_DrawArrays(GLenum mode, GLint first, GLsizei count, CRClientState *c)
{
	int i;

	if (count < 0)
	{
		crError("array_spu.self.DrawArrays passed negative count: %d", count);
	}

	if (mode > GL_POLYGON)
	{
		crError("array_spu.self.DrawArrays called with invalid mode: %d", mode);
	}

	array_spu.self.Begin(mode);
	for (i=0; i<count; i++)
	{
		array_spu.self.ArrayElement(first++);
	}
	array_spu.self.End();
}

static void ARRAYSPU_APIENTRY arrayspu_DrawElements(GLenum mode, GLsizei count,
		GLenum type, const GLvoid *indices, CRClientState *c)
{
	int i;
	GLubyte *p = (GLubyte *)indices;

	if (count < 0)
	{
		crError("array_spu.self.DrawElements passed negative count: %d", count);
	}

	if (mode > GL_POLYGON)
	{
		crError("array_spu.self.DrawElements called with invalid mode: %d", mode);
	}

	if (type != GL_UNSIGNED_BYTE && type != GL_UNSIGNED_SHORT && type != GL_UNSIGNED_INT)
	{
		crError("array_spu.self.DrawElements called with invalid type: %d", type);
	}

	array_spu.self.Begin(mode);
	switch (type)
	{
		case GL_UNSIGNED_BYTE:
			for (i=0; i<count; i++)
			{
				array_spu.self.ArrayElement((GLint) *p++);
			}
			break;
		case GL_UNSIGNED_SHORT:
			for (i=0; i<count; i++)
			{
				array_spu.self.ArrayElement((GLint) * (GLushort *) p);
				p+=sizeof (GLushort);
			}
			break;
		case GL_UNSIGNED_INT:
			for (i=0; i<count; i++)
			{
				array_spu.self.ArrayElement((GLint) * (GLuint *) p);
				p+=sizeof (GLuint);
			}
			break;
		default:
			crError( "this can't happen: array_spu.self.DrawElements" );
			break;
	}
	array_spu.self.End();
}

static void ARRAYSPU_APIENTRY arrayspu_ColorPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	crStateColorPointer( size, type, stride, pointer );
}

static void ARRAYSPU_APIENTRY arrayspu_SecondaryColorPointerEXT( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	crStateSecondaryColorPointerEXT( size, type, stride, pointer );
}

static void ARRAYSPU_APIENTRY arrayspu_VertexPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	crStateVertexPointer( size, type, stride, pointer );
}

static void ARRAYSPU_APIENTRY arrayspu_TexCoordPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	crStateTexCoordPointer( size, type, stride, pointer );
}

static void ARRAYSPU_APIENTRY arrayspu_NormalPointer( GLenum type, GLsizei stride, const GLvoid *pointer )
{
	crStateNormalPointer( type, stride, pointer );
}

static void ARRAYSPU_APIENTRY arrayspu_EdgeFlagPointer( GLsizei stride, const GLvoid *pointer )
{
	crStateEdgeFlagPointer( stride, pointer );
}

static void ARRAYSPU_APIENTRY arrayspu_VertexAttribPointerNV( GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid * pointer )
{
	crStateVertexAttribPointerNV( index, size, type, stride, pointer );
}

static void ARRAYSPU_APIENTRY arrayspu_FogCoordPointerEXT( GLenum type, GLsizei stride, const GLvoid *pointer )
{
	crStateFogCoordPointerEXT( type, stride, pointer );
}

static void ARRAYSPU_APIENTRY arrayspu_GetPointerv( GLenum pname, GLvoid **params )
{
	crStateGetPointerv( pname, params );
}

static void ARRAYSPU_APIENTRY arrayspu_EnableClientState( GLenum array )
{
	crStateEnableClientState( array );
}

static void ARRAYSPU_APIENTRY arrayspu_DisableClientState( GLenum array )
{
	crStateDisableClientState( array );
}

static void ARRAYSPU_APIENTRY arrayspu_ClientActiveTextureARB( GLenum texture )
{
	crStateDisableClientState( texture );
}

static void ARRAYSPU_APIENTRY arrayspu_MultiDrawArraysEXT(GLenum mode, GLint *first, GLsizei *count, GLsizei primcount, CRClientState *c)
{
	int i;

	if (primcount < 0)
	{
		crError("array_spu.self.MultiDrawArraysEXT passed negative count: %d", primcount);
	}

	if (mode > GL_POLYGON)
	{
		crError("array_spu.self.MultiDrawArraysEXT called with invalid mode: %d", mode);
	}

	for (i = 0; i < primcount; i++)
	{
		array_spu.self.DrawArrays(mode, first[i], count[i]);
	}
}

static void ARRAYSPU_APIENTRY arrayspu_MultiDrawElementsEXT(GLenum mode, GLsizei *count, GLenum type, const GLvoid **indices, GLsizei primcount, CRClientState *c)
{
	int i;

	if (primcount < 0)
	{
		crError("array_spu.self.MultiDrawElementsEXT passed negative count: %d", primcount);
	}

	if (mode > GL_POLYGON)
	{
		crError("array_spu.self.MultiDrawElementsEXT called with invalid mode: %d", mode);
	}

	if (type != GL_UNSIGNED_BYTE && type != GL_UNSIGNED_SHORT && type != GL_UNSIGNED_INT)
	{
		crError("array_spu.self.MultiDrawElementsEXT called with invalid type: %d", type);
	}

	for (i = 0; i < primcount; i++)
	{
		array_spu.self.DrawElements(mode, count[i], type, indices[i]);
	}
}

SPUNamedFunctionTable _cr_array_table[] = {
	{ "ArrayElement", (SPUGenericFunction) arrayspu_ArrayElement },
	{ "DrawArrays", (SPUGenericFunction) arrayspu_DrawArrays},
	{ "DrawElements", (SPUGenericFunction)  arrayspu_DrawElements},
	{ "ColorPointer", (SPUGenericFunction) arrayspu_ColorPointer},
	{ "SecondaryColorPointerEXT", (SPUGenericFunction) arrayspu_SecondaryColorPointerEXT},
	{ "VertexPointer", (SPUGenericFunction) arrayspu_VertexPointer},
	{ "TexCoordPointer", (SPUGenericFunction) arrayspu_TexCoordPointer},
	{ "NormalPointer", (SPUGenericFunction) arrayspu_NormalPointer},
	{ "EdgeFlagPointer", (SPUGenericFunction) arrayspu_EdgeFlagPointer},
	{ "VertexAttribPointerNV", (SPUGenericFunction) arrayspu_VertexAttribPointerNV},
	{ "FogCoordPointerEXT", (SPUGenericFunction) arrayspu_FogCoordPointerEXT},
	{ "GetPointerv", (SPUGenericFunction) arrayspu_GetPointerv},
	{ "EnableClientState", (SPUGenericFunction) arrayspu_EnableClientState},
	{ "DisableClientState", (SPUGenericFunction) arrayspu_DisableClientState},
	{ "ClientActiveTextureARB", (SPUGenericFunction) arrayspu_ClientActiveTextureARB },
	{ "MultiDrawArraysEXT", (SPUGenericFunction) arrayspu_MultiDrawArraysEXT },
	{ "MultiDrawElementsEXT", (SPUGenericFunction) arrayspu_MultiDrawElementsEXT },
	{ NULL, NULL }
};
