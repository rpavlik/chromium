/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "cr_glstate.h"
#include "state/cr_statetypes.h"
#include "state_internals.h"

void crStatePixelInit(CRPixelState *p) { 
	GLcolorf zero_color = {0.0f, 0.0f, 0.0f, 0.0f};
	GLcolorf one_color = {1.0f, 1.0f, 1.0f, 1.0f};

	p->unpack.rowLength   = 0;
	p->unpack.skipRows    = 0;
	p->unpack.skipPixels  = 0;
	p->unpack.skipImages  = 0;
	p->unpack.alignment   = 4;
	p->unpack.imageHeight = 0;
	p->unpack.swapBytes   = GL_FALSE;
	p->unpack.psLSBFirst    = GL_FALSE;
	p->pack.rowLength     = 0;
	p->pack.skipRows      = 0;
	p->pack.skipPixels    = 0;
	p->pack.skipImages    = 0;
	p->pack.alignment     = 4;
	p->pack.imageHeight   = 0;
	p->pack.swapBytes     = GL_FALSE;
	p->pack.psLSBFirst      = GL_FALSE;
	p->mapColor           = GL_FALSE;
	p->mapStencil         = GL_FALSE;
	p->indexShift         = 0;
	p->indexOffset        = 0;
	p->scale              = one_color;
	p->depthScale         = 1.0f;
	p->bias               = zero_color;
	p->depthBias          = 0.0f;
	p->xZoom              = 1.0f;
	p->yZoom              = 1.0f;
}

void STATE_APIENTRY crStatePixelStoref (GLenum pname, GLfloat param) {

	// The GL SPEC says I can do this on page 76.
	switch( pname )
	{
		case GL_PACK_SWAP_BYTES:
		case GL_PACK_LSB_FIRST:
		case GL_UNPACK_SWAP_BYTES:
		case GL_UNPACK_LSB_FIRST:
			crStatePixelStorei( pname, param == 0.0f ? 0: 1 );
			break;
		default:
			crStatePixelStorei( pname, (GLint) param );
			break;
	}
}

void STATE_APIENTRY crStatePixelStorei (GLenum pname, GLint param) {
	CRContext *g    = GetCurrentContext();
	CRStateBits *sb = GetCurrentBits();
	CRPixelState *p = &(g->pixel);
	CRPixelBits *pb = &(sb->pixel);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "PixelStore{if} called in Begin/End");
		return;
	}

	FLUSH();


	switch(pname) {
		case GL_PACK_SWAP_BYTES:
			p->pack.swapBytes = (GLboolean) param;
			pb->pack = g->neg_bitid;
			break;
		case GL_PACK_LSB_FIRST:
			p->pack.psLSBFirst = (GLboolean) param;
			pb->pack = g->neg_bitid;
			break;
		case GL_PACK_ROW_LENGTH:
			if (param < 0.0f) 
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative Row Length: %f", param);
				return;
			}
			p->pack.rowLength = param;
			pb->pack = g->neg_bitid;
			break;
		case GL_PACK_SKIP_PIXELS:
			if (param < 0.0f) 
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative Skip Pixels: %f", param);
				return;
			}
			p->pack.skipPixels = param;
			pb->pack = g->neg_bitid;
			break;
		case GL_PACK_SKIP_ROWS:
			if (param < 0.0f) 
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative Row Skip: %f", param);
				return;
			}
			p->pack.skipRows = param;
			pb->pack = g->neg_bitid;
			break;
		case GL_PACK_ALIGNMENT:
			if (((GLint) param) != 1 && 
					((GLint) param) != 2 &&
					((GLint) param) != 4 &&
					((GLint) param) != 8) 
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Invalid Alignment: %f", param);
				return;
			}
			p->pack.alignment = param;
			pb->pack = g->neg_bitid;
			break;

		case GL_UNPACK_SWAP_BYTES:
			p->unpack.swapBytes = (GLboolean) param;
			pb->unpack = g->neg_bitid;
			break;
		case GL_UNPACK_LSB_FIRST:
			p->unpack.psLSBFirst = (GLboolean) param;
			pb->unpack = g->neg_bitid;
			break;
		case GL_UNPACK_ROW_LENGTH:
			if (param < 0.0f) 
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative Row Length: %f", param);
				return;
			}
			p->unpack.rowLength = param;
			pb->unpack = g->neg_bitid;
			break;
		case GL_UNPACK_SKIP_PIXELS:
			if (param < 0.0f) 
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative Skip Pixels: %f", param);
				return;
			}
			p->unpack.skipPixels = param;
			pb->unpack = g->neg_bitid;
			break;
		case GL_UNPACK_SKIP_ROWS:
			if (param < 0.0f) 
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative Row Skip: %f", param);
				return;
			}
			p->unpack.skipRows = param;
			pb->unpack = g->neg_bitid;
			break;
		case GL_UNPACK_ALIGNMENT:
			if (((GLint) param) != 1 && 
					((GLint) param) != 2 &&
					((GLint) param) != 4 &&
					((GLint) param) != 8) 
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Invalid Alignment: %f", param);
				return;
			}
			p->unpack.alignment = param;
			pb->unpack = g->neg_bitid;
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Unknown glPixelStore Pname: %d", pname);
			return;
	}
	pb->dirty = g->neg_bitid;
}

void STATE_APIENTRY crStatePixelTransferi (GLenum pname, GLint param) {
	crStatePixelTransferf( pname, (GLfloat) param );
}

void STATE_APIENTRY crStatePixelTransferf (GLenum pname, GLfloat param) {
	CRContext *g    = GetCurrentContext();
	CRStateBits *sb = GetCurrentBits();
	CRPixelState *p = &(g->pixel);
	CRPixelBits *pb = &(sb->pixel);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "PixelTransfer{if} called in Begin/End");
		return;
	}

	FLUSH();

	switch( pname )
	{
		case GL_MAP_COLOR:
			p->mapColor = (GLboolean) ((param == 0.0f) ? GL_FALSE : GL_TRUE);
			break;
		case GL_MAP_STENCIL:
			p->mapStencil = (GLboolean) ((param == 0.0f) ? GL_FALSE : GL_TRUE);
			break;
		case GL_INDEX_SHIFT:
			p->indexShift = (GLint) param;
			break;
		case GL_INDEX_OFFSET:
			p->indexOffset = (GLint) param;
			break;
		case GL_RED_SCALE:
			p->scale.r = param;
			break;
		case GL_GREEN_SCALE:
			p->scale.g = param;
			break;
		case GL_BLUE_SCALE:
			p->scale.b = param;
			break;
		case GL_ALPHA_SCALE:
			p->scale.a = param;
			break;
		case GL_DEPTH_SCALE:
			p->depthScale = param;
			break;
		case GL_RED_BIAS:
			p->bias.r = param;
			break;
		case GL_GREEN_BIAS:
			p->bias.g = param;
			break;
		case GL_BLUE_BIAS:
			p->bias.b = param;
			break;
		case GL_ALPHA_BIAS:
			p->bias.a = param;
			break;
		case GL_DEPTH_BIAS:
			p->depthBias = param;
			break;
		default:
			crStateError( __LINE__, __FILE__, GL_INVALID_VALUE, "Unknown glPixelTransfer pname: %d", pname );
			return;
	}
	pb->transfer = g->neg_bitid;
	pb->dirty = g->neg_bitid;
}

void STATE_APIENTRY crStatePixelZoom (GLfloat xfactor, GLfloat yfactor) 
{
	CRContext *g    = GetCurrentContext();
	CRStateBits *sb = GetCurrentBits();
	CRPixelState *p = &(g->pixel);
	CRPixelBits *pb = &(sb->pixel);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "PixelZoom called in Begin/End");
		return;
	}

	FLUSH();

	p->xZoom = xfactor;
	p->yZoom = yfactor;
	pb->zoom = g->neg_bitid;
	pb->dirty = g->neg_bitid;
}

#define UNUSED(x) ((void)(x))

void STATE_APIENTRY crStatePixelMapfv (GLenum map, GLint mapsize, const GLfloat * values) {
	UNUSED(map);
	UNUSED(mapsize);
	UNUSED(values);
	crStateError( __LINE__, __FILE__, GL_NO_ERROR, "Unimplemented Call: PixelMapfv" );
}

void STATE_APIENTRY crStateBitmap( GLsizei width, GLsizei height, 
		GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, 
		const GLubyte *bitmap)
{
	CRContext *g = GetCurrentContext();
	CRCurrentState *c = &(g->current);
	CRStateBits *s = GetCurrentBits();
	CRCurrentBits *cb = &(s->current);

	(void) xorig;
	(void) yorig;
	(void) bitmap;
	
	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, 
			"Bitmap called in begin/end");
		return;
	}

	if (width < 0 || height < 0)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
			"Bitmap called with neg dims: %dx%d", width, height);
		return;
	}

	if (!c->rasterValid)
	{
		return;
	}

	c->rasterPos.x += xmove;
	c->rasterPos.y += ymove;
	cb->raster = g->neg_bitid;
	cb->dirty = g->neg_bitid;

	c->rasterPosPre.x += xmove;
	c->rasterPosPre.y += ymove;
}
 
void STATE_APIENTRY crStatePixelMapuiv (GLenum map, GLint mapsize, const GLuint * values) {
	UNUSED(map);
	UNUSED(mapsize);
	UNUSED(values);
	crStateError( __LINE__, __FILE__, GL_NO_ERROR, "Unimplemented Call: PixelMapuiv" );
}
 
void STATE_APIENTRY crStatePixelMapusv (GLenum map, GLint mapsize, const GLushort * values) {
	UNUSED(map);
	UNUSED(mapsize);
	UNUSED(values);
	crStateError( __LINE__, __FILE__, GL_NO_ERROR, "Unimplemented Call: PixelMapusv" );
}
 
void STATE_APIENTRY crStateGetPixelMapfv (GLenum map, GLfloat * values) {
	UNUSED(map);
	UNUSED(values);
	crStateError( __LINE__, __FILE__, GL_NO_ERROR, "Unimplemented Call: GetPixelMapfv" );
}
 
void STATE_APIENTRY crStateGetPixelMapuiv (GLenum map, GLuint * values) {
	UNUSED(map);
	UNUSED(values);
	crStateError( __LINE__, __FILE__, GL_NO_ERROR, "Unimplemented Call: GetPixelMapuiv" );
}
 
void STATE_APIENTRY crStateGetPixelMapusv (GLenum map, GLushort * values) {
	UNUSED(map);
	UNUSED(values);
	crStateError( __LINE__, __FILE__, GL_NO_ERROR, "Unimplemented Call: GetPixelMapusv" );
}
