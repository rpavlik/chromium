#include "packer.h"
#include "cr_opcodes.h"
#include "cr_glwrapper.h"

void PACK_APIENTRY crPackArrayElement (GLint index, CRClientState *c) 
{
	unsigned char *p;

#if 0
	if (index < 0)
		UNIMPLEMENTED();
#endif

	if (c->e.enabled) 
	{
		crPackEdgeFlagv(c->e.p + index*c->e.stride);
	}
	if (c->t.enabled) 
	{
		p = c->t.p + index*c->t.stride;
		switch (c->t.type) 
		{
			case GL_SHORT:
				switch (c->t.size) 
				{
					case 1: crPackTexCoord1sv((GLshort *)p); break;
					case 2: crPackTexCoord2sv((GLshort *)p); break;
					case 3: crPackTexCoord3sv((GLshort *)p); break;
					case 4: crPackTexCoord4sv((GLshort *)p); break;
				}
				break;
			case GL_INT:
				switch (c->t.size) 
				{
					case 1: crPackTexCoord1iv((GLint *)p); break;
					case 2: crPackTexCoord2iv((GLint *)p); break;
					case 3: crPackTexCoord3iv((GLint *)p); break;
					case 4: crPackTexCoord4iv((GLint *)p); break;
				}
				break;
			case GL_FLOAT:
				switch (c->t.size) 
				{
					case 1: crPackTexCoord1fv((GLfloat *)p); break;
					case 2: crPackTexCoord2fv((GLfloat *)p); break;
					case 3: crPackTexCoord3fv((GLfloat *)p); break;
					case 4: crPackTexCoord4fv((GLfloat *)p); break;
				}
				break;
			case GL_DOUBLE:
				switch (c->t.size) 
				{
					case 1: crPackTexCoord1dv((GLdouble *)p); break;
					case 2: crPackTexCoord2dv((GLdouble *)p); break;
					case 3: crPackTexCoord3dv((GLdouble *)p); break;
					case 4: crPackTexCoord4dv((GLdouble *)p); break;
				}
				break;
		}
	}
	if (c->i.enabled) 
	{
		p = c->i.p + index*c->i.stride;
		switch (c->i.type) 
		{
			case GL_SHORT: crPackIndexsv((GLshort *)p); break;
			case GL_INT: crPackIndexiv((GLint *)p); break;
			case GL_FLOAT: crPackIndexfv((GLfloat *)p); break;
			case GL_DOUBLE: crPackIndexdv((GLdouble *)p); break;
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
					case 3: crPackColor3bv((GLbyte *)p); break;
					case 4: crPackColor4bv((GLbyte *)p); break;
				}
				break;
			case GL_UNSIGNED_BYTE:
				switch (c->c.size) 
				{
					case 3: crPackColor3ubv((GLubyte *)p); break;
					case 4: crPackColor4ubv((GLubyte *)p); break;
				}
				break;
			case GL_SHORT:
				switch (c->c.size) 
				{
					case 3: crPackColor3sv((GLshort *)p); break;
					case 4: crPackColor4sv((GLshort *)p); break;
				}
				break;
			case GL_UNSIGNED_SHORT:
				switch (c->c.size) 
				{
					case 3: crPackColor3usv((GLushort *)p); break;
					case 4: crPackColor4usv((GLushort *)p); break;
				}
				break;
			case GL_INT:
				switch (c->c.size) 
				{
					case 3: crPackColor3iv((GLint *)p); break;
					case 4: crPackColor4iv((GLint *)p); break;
				}
				break;
			case GL_UNSIGNED_INT:
				switch (c->c.size) 
				{
					case 3: crPackColor3uiv((GLuint *)p); break;
					case 4: crPackColor4uiv((GLuint *)p); break;
				}
				break;
			case GL_FLOAT:
				switch (c->c.size) 
				{
					case 3: crPackColor3fv((GLfloat *)p); break;
					case 4: crPackColor4fv((GLfloat *)p); break;
				}
				break;
			case GL_DOUBLE:
				switch (c->c.size) 
				{
					case 3: crPackColor3dv((GLdouble *)p); break;
					case 4: crPackColor4dv((GLdouble *)p); break;
				}
				break;
		}
	}
	if (c->n.enabled) 
	{
		p = c->n.p + index*c->n.stride;
		switch (c->n.type) 
		{
			case GL_BYTE: crPackNormal3bv((GLbyte *)p); break;
			case GL_SHORT: crPackNormal3sv((GLshort *)p); break;
			case GL_INT: crPackNormal3iv((GLint *)p); break;
			case GL_FLOAT: crPackNormal3fv((GLfloat *)p); break;
			case GL_DOUBLE: crPackNormal3dv((GLdouble *)p); break;
		}
	}
	if (c->v.enabled) 
	{
		p = c->v.p + (index*c->v.stride);

		switch (c->v.type) 
		{
			case GL_SHORT:
				switch (c->v.size) 
				{
					case 2: crPackVertex2svBBOX((GLshort *)p); break;
					case 3: crPackVertex3svBBOX((GLshort *)p); break;
					case 4: crPackVertex4svBBOX((GLshort *)p); break;
				}
				break;
			case GL_INT:
				switch (c->v.size) 
				{
					case 2: crPackVertex2ivBBOX((GLint *)p); break;
					case 3: crPackVertex3ivBBOX((GLint *)p); break;
					case 4: crPackVertex4ivBBOX((GLint *)p); break;
				}
				break;
			case GL_FLOAT:
				switch (c->v.size) 
				{
					case 2: crPackVertex2fvBBOX((GLfloat *)p); break;
					case 3: crPackVertex3fvBBOX((GLfloat *)p); break;
					case 4: crPackVertex4fvBBOX((GLfloat *)p); break;
				}
				break;
			case GL_DOUBLE:
				switch (c->v.size) 
				{
					case 2: crPackVertex2dvBBOX((GLdouble *)p); break;
					case 3: crPackVertex3dvBBOX((GLdouble *)p); break;
					case 4: crPackVertex4dvBBOX((GLdouble *)p); break;
				}
				break;
		}
	}
}

void PACK_APIENTRY crPackDrawArrays(GLenum mode, GLint first, GLsizei count, CRClientState *c) 
{
	int i;

	if (count < 0)
	{
		crError("crPackDrawArrays passed negative count: %d", count);
	}

	if (mode > GL_POLYGON)
	{
		crError("crPackDrawArrays called with invalid mode: %d", mode);
	}

	crPackBegin (mode);
	for (i=0; i<count; i++) 
	{
		crPackArrayElement(first++, c);
	}
	crPackEnd();
}

void PACK_APIENTRY crPackDrawElements(GLenum mode, GLsizei count, 
																			GLenum type, const GLvoid *indices, CRClientState *c) 
{
	int i;
	GLubyte *p = (GLubyte *)indices;

	if (count < 0)
	{
		crError("crPackDrawElements passed negative count: %d", count);
	}

	if (mode > GL_POLYGON)
	{
		crError("crPackDrawElements called with invalid mode: %d", mode);
	}

	if (type != GL_UNSIGNED_BYTE && type != GL_UNSIGNED_SHORT && type != GL_UNSIGNED_INT)
	{
		crError("crPackDrawElements called with invalid type: %d", type);
	}
	
	crPackBegin (mode);
	switch (type) 
	{
	case GL_UNSIGNED_BYTE:
		for (i=0; i<count; i++)
		{
			crPackArrayElement((GLint) *p++, c);
		}
		break;
	case GL_UNSIGNED_SHORT:
		for (i=0; i<count; i++) 
		{
			crPackArrayElement((GLint) * (GLushort *) p, c);
			p+=sizeof (GLushort);
		}
		break;
	case GL_UNSIGNED_INT:
		for (i=0; i<count; i++) 
		{
			crPackArrayElement((GLint) * (GLuint *) p, c);
			p+=sizeof (GLuint);
		}
		break;
	default:
		crError( "this can't happen: crPackDrawElements" );
		break;
	}
	crPackEnd();
}
