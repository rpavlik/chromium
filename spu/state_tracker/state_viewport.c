#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "cr_glstate.h"
#include "state/cr_statetypes.h"
#include "state_internals.h"

void crStateViewportInit(CRViewportState *v) 
{
	v->scissorTest = GL_FALSE;
	
	v->maxViewportDimsWidth = 16834; // XXX
	v->maxViewportDimsHeight = 16834; // XXX

	v->viewportValid = GL_TRUE;
	v->viewportX = 0;
	v->viewportY = 0;
	v->viewportW = 640;
	v->viewportH = 480;
	v->scissorValid = GL_TRUE;
	v->scissorX = 0;
	v->scissorY = 0;
	v->scissorW = 640;
	v->scissorH = 480;

	/*
	v->viewportValid = c->viewport_valid;
	v->v_x = c->viewport.x1;
	v->v_y = c->viewport.y1;
	v->v_w = c->viewport.x2 - c->viewport.x1;
	v->v_h = c->viewport.y2 - c->viewport.y1;

	v->s_valid = c->viewport_valid;
	v->s_x = c->viewport.x1;
	v->s_y = c->viewport.y1;
	v->s_w = c->viewport.x2 - c->viewport.x1;
	v->s_h = c->viewport.y2 - c->viewport.y1;
	*/

	/*
	v->getwindowsize = c->getwindowsize;
	*/

	v->farClip = 1.0;
	v->nearClip = 0.0;

}

void STATE_APIENTRY crStateViewport(GLint x, GLint y, GLsizei width, 
			GLsizei height) 
{
	CRContext *g = GetCurrentContext();
	CRViewportState *v = &(g->viewport);
	CRStateBits *sb = GetCurrentBits();
	CRViewportBits *vb = &(sb->viewport);
	CRTransformBits *tb = &(sb->transform);
	
	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glViewport called in Begin/End");
		return;
	}

	FLUSH();
	
	if (width < 0 || height < 0)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative viewport width or height: %dx%d", width, height);
		return;
	}

	// if (x > v->maxviewportdims_width)	x = v->maxviewportdims_width;
	// if (x < -v->maxviewportdims_width)	x = -v->maxviewportdims_width;
	// if (y > v->maxviewportdims_height)	y = v->maxviewportdims_height;
	// if (y < -v->maxviewportdims_height)	y = -v->maxviewportdims_height;
	// if (width > v->maxviewportdims_width)	width = v->maxviewportdims_width;
	//if (height > v->maxviewportdims_height)	height = v->maxviewportdims_height;

	v->viewportX = (GLint) (x);
	v->viewportY = (GLint) (y);
	v->viewportW = (GLint) (width);
	v->viewportH = (GLint) (height);

	v->viewportValid = GL_TRUE;

	vb->v_dims = g->neg_bitid;
	vb->dirty = g->neg_bitid;
	tb->base = g->neg_bitid;
	tb->dirty = g->neg_bitid;

}

void STATE_APIENTRY crStateDepthRange(GLclampd znear, GLclampd zfar) 
{
	CRContext *g = GetCurrentContext();
	CRViewportState *v = &(g->viewport);
	CRStateBits *sb = GetCurrentBits();
	CRViewportBits *vb = &(sb->viewport);
	CRTransformBits *tb = &(sb->transform);

	if (g->current.inBeginEnd)
	{	
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glDepthRange called in Begin/End");
		return;
	}

	FLUSH();

	v->nearClip = znear;
	v->farClip = zfar;
	if (v->nearClip < 0.0) v->nearClip = 0.0;
	if (v->nearClip > 1.0) v->nearClip = 1.0;
	if (v->farClip < 0.0) v->farClip = 0.0;
	if (v->farClip > 1.0) v->farClip = 1.0;

	vb->depth = g->neg_bitid;
	vb->dirty = g->neg_bitid;
	tb->dirty = g->neg_bitid;
}

void STATE_APIENTRY crStateScissor (GLint x, GLint y, 
					 GLsizei width, GLsizei height) 
{
	CRContext *g = GetCurrentContext();
	CRViewportState *v = &(g->viewport);
	CRStateBits *stateb = GetCurrentBits();
	CRViewportBits *vb = &(stateb->viewport);

	if (g->current.inBeginEnd) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
			"glScissor called in begin/end");
		return;
	}

	FLUSH();

	if (width < 0 || height < 0)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
			"glScissor called with negative width/height: %d,%d",
			width, height);
		return;
	}

	v->scissorX = (GLint) (x);
	v->scissorY = (GLint) (y);
	v->scissorW = (GLint) (width);
	v->scissorH = (GLint) (height);

	v->scissorValid = GL_TRUE;

	vb->s_dims = g->neg_bitid;
	vb->dirty = g->neg_bitid;
}
