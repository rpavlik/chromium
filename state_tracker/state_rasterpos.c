/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdlib.h>
#include <stdio.h>
#include "state.h"
#include "state/cr_statetypes.h"
#include "state_internals.h"

void crStateRasterPosUpdate(CRContext *g,
							  GLfloat x, GLfloat y, GLfloat z, GLfloat w) 
{
	CRCurrentState *c = &(g->current);
	CRTransformState *t = &(g->transform);
	CRViewportState *v = &(g->viewport);
	GLvectorf p;

	p.x = x;
	p.y = y;
	p.z = z;
	p.w = w;

	crStateTransformXformPoint(t, &p);	

	if (p.x >  p.w || p.y >  p.w || p.z > p.w ||
		  p.x < -p.w || p.y < -p.w || p.z < -p.w) 
	{
		c->rasterValid = GL_FALSE;
		return;
	} 

	p.x /= p.w;
	p.y /= p.w;
	p.z /= p.w;
	p.w = 1.0f;

	crStateViewportApply(v, &p);

	c->rasterValid = GL_TRUE;
	c->rasterPos = p;
	c->rasterPosPre = p;

	c->rasterColor = c->color;
	c->rasterSecondaryColor = c->secondaryColor;
	c->rasterIndex = c->index;
	c->rasterTexture = c->texCoord[0];


	/*
	**  Need handle these for glGet...
	**  c->rasterdistance;
	**  c->rastercolor;
	**  c->rasterindex;
	**  c->rastertexture;
	*/
}


void STATE_APIENTRY crStateRasterPos2d(GLdouble x, GLdouble y)
{
  CRContext *g = GetCurrentContext();
  CRStateBits *sb = GetCurrentBits();
  CRCurrentBits *cb = &(sb->current);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "RasterPos called in Begin/End");
		return;
	}

	FLUSH();

	crStateRasterPosUpdate(g,(GLfloat) x, (GLfloat) y, 0.0f, 1.0f);

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->raster, g->neg_bitid);
}

void STATE_APIENTRY crStateRasterPos2f(GLfloat x, GLfloat y)
{
  CRContext *g = GetCurrentContext();
  CRStateBits *sb = GetCurrentBits();
  CRCurrentBits *cb = &(sb->current);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "RasterPos called in Begin/End");
		return;
	}

	FLUSH();

	crStateRasterPosUpdate(g,x, y, 0.0f, 1.0f);

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->raster, g->neg_bitid);
}

void STATE_APIENTRY crStateRasterPos2i(GLint x, GLint y)
{
  CRContext *g = GetCurrentContext();
  CRStateBits *sb = GetCurrentBits();
  CRCurrentBits *cb = &(sb->current);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "RasterPos called in Begin/End");
		return;
	}

	FLUSH();

	crStateRasterPosUpdate(g,(GLfloat) x, (GLfloat) y, 0.0f, 1.0f);

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->raster, g->neg_bitid);
}

void STATE_APIENTRY crStateRasterPos2s(GLshort x, GLshort y)
{
  CRContext *g = GetCurrentContext();
  CRStateBits *sb = GetCurrentBits();
  CRCurrentBits *cb = &(sb->current);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "RasterPos called in Begin/End");
		return;
	}

	FLUSH();

	crStateRasterPosUpdate(g,(GLfloat) x, (GLfloat) y, 0.0f, 1.0f);

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->raster, g->neg_bitid);
}

void STATE_APIENTRY crStateRasterPos3d(GLdouble x, GLdouble y, GLdouble z)
{
  CRContext *g = GetCurrentContext();
  CRStateBits *sb = GetCurrentBits();
  CRCurrentBits *cb = &(sb->current);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "RasterPos called in Begin/End");
		return;
	}

	FLUSH();

	crStateRasterPosUpdate(g,(GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0f);

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->raster, g->neg_bitid);
}

void STATE_APIENTRY crStateRasterPos3f(GLfloat x, GLfloat y, GLfloat z)
{
  CRContext *g = GetCurrentContext();
  CRStateBits *sb = GetCurrentBits();
  CRCurrentBits *cb = &(sb->current);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "RasterPos called in Begin/End");
		return;
	}

	FLUSH();

	crStateRasterPosUpdate(g,x, y, z, 1.0f);

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->raster, g->neg_bitid);
}

void STATE_APIENTRY crStateRasterPos3i(GLint x, GLint y, GLint z)
{
  CRContext *g = GetCurrentContext();
  CRStateBits *sb = GetCurrentBits();
  CRCurrentBits *cb = &(sb->current);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "RasterPos called in Begin/End");
		return;
	}

	FLUSH();

	crStateRasterPosUpdate(g,(GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0f);

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->raster, g->neg_bitid);
}

void STATE_APIENTRY crStateRasterPos3s(GLshort x, GLshort y, GLshort z)
{
	CRContext *g = GetCurrentContext();
  CRStateBits *sb = GetCurrentBits();
  CRCurrentBits *cb = &(sb->current);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "RasterPos called in Begin/End");
		return;
	}

	FLUSH();

	crStateRasterPosUpdate(g,(GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0f);

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->raster, g->neg_bitid);
}

void STATE_APIENTRY crStateRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	CRContext *g = GetCurrentContext();
  CRStateBits *sb = GetCurrentBits();
  CRCurrentBits *cb = &(sb->current);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "RasterPos called in Begin/End");
		return;
	}

	FLUSH();

	crStateRasterPosUpdate(g,(GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w);

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->raster, g->neg_bitid);
}

void STATE_APIENTRY crStateRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	CRContext *g = GetCurrentContext();
  CRStateBits *sb = GetCurrentBits();
  CRCurrentBits *cb = &(sb->current);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "RasterPos called in Begin/End");
		return;
	}

	FLUSH();

	crStateRasterPosUpdate(g,x, y, z, w);

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->raster, g->neg_bitid);
}

void STATE_APIENTRY crStateRasterPos4i(GLint x, GLint y, GLint z, GLint w)
{
	CRContext *g = GetCurrentContext();
  CRStateBits *sb = GetCurrentBits();
  CRCurrentBits *cb = &(sb->current);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "RasterPos called in Begin/End");
		return;
	}

	FLUSH();

	crStateRasterPosUpdate(g,(GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w);

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->raster, g->neg_bitid);
}

void STATE_APIENTRY crStateRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
	CRContext *g = GetCurrentContext();
  CRStateBits *sb = GetCurrentBits();
  CRCurrentBits *cb = &(sb->current);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "RasterPos called in Begin/End");
		return;
	}

	FLUSH();

	crStateRasterPosUpdate(g,(GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w);

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->raster, g->neg_bitid);
}

void STATE_APIENTRY crStateRasterPos2dv(const GLdouble *v)
{
	CRContext *g = GetCurrentContext();
  CRStateBits *sb = GetCurrentBits();
  CRCurrentBits *cb = &(sb->current);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "RasterPos called in Begin/End");
		return;
	}

	FLUSH();

	crStateRasterPosUpdate(g,(GLfloat) v[0], (GLfloat) v[1], 0.0f, 1.0f);

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->raster, g->neg_bitid);
}

void STATE_APIENTRY crStateRasterPos2fv(const GLfloat *v)
{
  CRContext *g = GetCurrentContext();
  CRStateBits *sb = GetCurrentBits();
  CRCurrentBits *cb = &(sb->current);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "RasterPos called in Begin/End");
		return;
	}

	FLUSH();

	crStateRasterPosUpdate(g,v[0], v[1], 0.0f, 1.0f);

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->raster, g->neg_bitid);
}

void STATE_APIENTRY crStateRasterPos2iv(const GLint *v)
{
	CRContext *g = GetCurrentContext();
  CRStateBits *sb = GetCurrentBits();
  CRCurrentBits *cb = &(sb->current);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "RasterPos called in Begin/End");
		return;
	}

	FLUSH();

	crStateRasterPosUpdate(g,(GLfloat) v[0], (GLfloat) v[1], 0.0f, 1.0f);

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->raster, g->neg_bitid);
}

void STATE_APIENTRY crStateRasterPos2sv(const GLshort *v)
{
	CRContext *g = GetCurrentContext();
  CRStateBits *sb = GetCurrentBits();
  CRCurrentBits *cb = &(sb->current);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "RasterPos called in Begin/End");
		return;
	}

	FLUSH();

	crStateRasterPosUpdate(g,(GLfloat) v[0], (GLfloat) v[1], 0.0f, 1.0f);

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->raster, g->neg_bitid);
}

void STATE_APIENTRY crStateRasterPos3dv(const GLdouble *v)
{
	CRContext *g = GetCurrentContext();
  CRStateBits *sb = GetCurrentBits();
  CRCurrentBits *cb = &(sb->current);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "RasterPos called in Begin/End");
		return;
	}

	FLUSH();

	crStateRasterPosUpdate(g,(GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], 1.0f);

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->raster, g->neg_bitid);
}

void STATE_APIENTRY crStateRasterPos3fv(const GLfloat *v)
{
	CRContext *g = GetCurrentContext();
  CRStateBits *sb = GetCurrentBits();
  CRCurrentBits *cb = &(sb->current);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "RasterPos called in Begin/End");
		return;
	}

	FLUSH();

	crStateRasterPosUpdate(g,v[0], v[1], v[2], 1.0f);

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->raster, g->neg_bitid);
}

void STATE_APIENTRY crStateRasterPos3iv(const GLint *v)
{
	CRContext *g = GetCurrentContext();
  CRStateBits *sb = GetCurrentBits();
  CRCurrentBits *cb = &(sb->current);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "RasterPos called in Begin/End");
		return;
	}

	FLUSH();

	crStateRasterPosUpdate(g,(GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], 1.0f);

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->raster, g->neg_bitid);
}

void STATE_APIENTRY crStateRasterPos3sv(const GLshort *v)
{
	CRContext *g = GetCurrentContext();
  CRStateBits *sb = GetCurrentBits();
  CRCurrentBits *cb = &(sb->current);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "RasterPos called in Begin/End");
		return;
	}

	FLUSH();

	crStateRasterPosUpdate(g,(GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], 1.0f);

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->raster, g->neg_bitid);
}

void STATE_APIENTRY crStateRasterPos4dv(const GLdouble *v)
{
	CRContext *g = GetCurrentContext();
  CRStateBits *sb = GetCurrentBits();
  CRCurrentBits *cb = &(sb->current);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "RasterPos called in Begin/End");
		return;
	}

	FLUSH();

	crStateRasterPosUpdate(g,(GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) v[3]);

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->raster, g->neg_bitid);
}

void STATE_APIENTRY crStateRasterPos4fv(const GLfloat *v)
{
	CRContext *g = GetCurrentContext();
  CRStateBits *sb = GetCurrentBits();
  CRCurrentBits *cb = &(sb->current);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "RasterPos called in Begin/End");
		return;
	}

	FLUSH();

	crStateRasterPosUpdate(g,v[0], v[1], v[2], v[3]);

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->raster, g->neg_bitid);
}

void STATE_APIENTRY crStateRasterPos4iv(const GLint *v)
{
	CRContext *g = GetCurrentContext();
  CRStateBits *sb = GetCurrentBits();
  CRCurrentBits *cb = &(sb->current);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "RasterPos called in Begin/End");
		return;
	}

	FLUSH();

	crStateRasterPosUpdate(g,(GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) v[3]);

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->raster, g->neg_bitid);
}

void STATE_APIENTRY crStateRasterPos4sv(const GLshort *v)
{
	CRContext *g = GetCurrentContext();
  CRStateBits *sb = GetCurrentBits();
  CRCurrentBits *cb = &(sb->current);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "RasterPos called in Begin/End");
		return;
	}

	FLUSH();

	crStateRasterPosUpdate(g,(GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) v[3]);

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->raster, g->neg_bitid);
}
