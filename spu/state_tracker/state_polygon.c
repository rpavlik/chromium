/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdlib.h>
#include <stdio.h>
#include "cr_glstate.h"
#include "state/cr_statetypes.h"
#include "cr_pixeldata.h"
#include "state_internals.h"

void crStatePolygonInit(CRPolygonState *p) 
{
	int i;
	p->polygonSmooth = GL_FALSE;
	p->polygonOffsetFill = GL_FALSE;
	p->polygonOffsetLine = GL_FALSE;
	p->polygonOffsetPoint = GL_FALSE;
	p->polygonStipple = GL_FALSE;
	p->cullFace = GL_FALSE;

	p->offsetFactor = 0;
	p->offsetUnits = 0;
	p->cullFaceMode = GL_BACK;
	p->frontFace = GL_CCW;
	p->frontMode = GL_FILL;
	p->backMode = GL_FILL;
	for (i=0; i<32; i++)
		p->stipple[i] = 0xFFFFFFFF;
}

void STATE_APIENTRY crStateCullFace(GLenum mode) 
{
	CRContext *g = GetCurrentContext();
	CRPolygonState *p = &(g->polygon);
	CRStateBits *sb = GetCurrentBits();
	CRPolygonBits *pb = &(sb->polygon);

	if (g->current.inBeginEnd) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
				"glCullFace called in begin/end");
		return;
	}

	FLUSH();

	if (mode != GL_FRONT && mode != GL_BACK)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
				"glCullFace called with bogus mode: %d", mode);
		return;
	}

	p->cullFaceMode = mode;
	pb->mode = g->neg_bitid;
	pb->dirty = g->neg_bitid;
}

void STATE_APIENTRY crStateFrontFace (GLenum mode) 
{
	CRContext *g = GetCurrentContext();
	CRPolygonState *p = &(g->polygon);
	CRStateBits *sb = GetCurrentBits();
	CRPolygonBits *pb = &(sb->polygon);

	if (g->current.inBeginEnd) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
				"glFrontFace called in begin/end");
		return;
	}

	FLUSH();

	if (mode != GL_CW && mode != GL_CCW)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
				"glFrontFace called with bogus mode: %d", mode);
		return;
	}

	p->frontFace = mode;
	pb->mode = g->neg_bitid;
	pb->dirty = g->neg_bitid;
}

void  STATE_APIENTRY crStatePolygonMode (GLenum face, GLenum mode) 
{
	CRContext *g = GetCurrentContext();
	CRPolygonState *p = &(g->polygon);
	CRStateBits *sb = GetCurrentBits();
	CRPolygonBits *pb = &(sb->polygon);

	if (g->current.inBeginEnd) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
				"glPolygonMode called in begin/end");
		return;
	}

	FLUSH();

	if (mode != GL_POINT && mode != GL_LINE && mode != GL_FILL)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
				"glPolygonMode called with bogus mode: %d", mode);
		return;
	}

	switch (face) {
		case GL_FRONT:
			p->frontMode = mode;
			break;
		case GL_FRONT_AND_BACK:
			p->frontMode = mode;
		case GL_BACK:
			p->backMode = mode;
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					"glPolygonMode called with bogus face: %d", face);
			return;
	}
	pb->mode = g->neg_bitid;
	pb->dirty = g->neg_bitid;
}

void STATE_APIENTRY crStatePolygonOffset (GLfloat factor, GLfloat units) 
{
	CRContext *g = GetCurrentContext();
	CRPolygonState *p = &(g->polygon);
	CRStateBits *sb = GetCurrentBits();
	CRPolygonBits *pb = &(sb->polygon);

	if (g->current.inBeginEnd) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
				"glPolygonOffset called in begin/end");
		return;
	}

	FLUSH();

	p->offsetFactor = factor;
	p->offsetUnits = units;

	pb->offset = g->neg_bitid;
	pb->dirty = g->neg_bitid;
}

void STATE_APIENTRY crStatePolygonStipple (const GLubyte *p) 
{
	CRContext *g = GetCurrentContext();
	CRPolygonState *poly = &(g->polygon);
	CRPixelState *pix = &(g->pixel);
	CRStateBits *sb = GetCurrentBits();
	CRPolygonBits *pb = &(sb->polygon);

	if (g->current.inBeginEnd) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
				"glPolygonStipple called in begin/end");
		return;
	}

	FLUSH();

	if (!p)
	{
		crStateError(__LINE__, __FILE__, GL_NO_ERROR,
				"Void pointer passed to PolygonStipple");
		return;
	}

	crPixelCopy2D ( (void *) poly->stipple, p,
			32, 32, GL_COLOR_INDEX, GL_BITMAP, 
			&(pix->pack));

	pb->dirty = g->neg_bitid;
	pb->stipple = g->neg_bitid;
}

void STATE_APIENTRY crStateGetPolygonStipple( GLubyte *b )
{
	crStateError( __LINE__, __FILE__, GL_NO_ERROR,
			"GetPolygonStipple is unimplemented." );
	(void)b;
}
