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
#include "state_extensionfuncs.h"

void crStateBufferInit (CRBufferState *b) 
{
	GLcolorf zero_colorf = {0.0f, 0.0f, 0.0f, 0.0f};

	b->depthTest = GL_FALSE;
	b->blend     = GL_FALSE;
	b->alphaTest = GL_FALSE;
	b->logicOp   = GL_FALSE;
	b->dither    = GL_FALSE;
	b->depthMask = GL_TRUE;

	b->alphaTestFunc = GL_ALWAYS;
	b->alphaTestRef = 0;
	b->depthFunc = GL_LESS;
	b->blendSrc = GL_ONE;
	b->blendDst = GL_ZERO;
	b->blendColor = zero_colorf;
	b->logicOpMode = GL_COPY;
	b->drawBuffer = GL_BACK;
	b->readBuffer = GL_BACK;
	b->indexWriteMask = 0xffffffff;
	b->colorWriteMask.r = GL_TRUE;
	b->colorWriteMask.g = GL_TRUE;
	b->colorWriteMask.b = GL_TRUE;
	b->colorWriteMask.a = GL_TRUE;
	b->colorClearValue = zero_colorf;
	b->indexClearValue = 0;
	b->depthClearValue = (GLdefault) 1.0;
	b->accumClearValue = zero_colorf;

	crStateBufferInitExtensions( b );
}

void STATE_APIENTRY crStateAlphaFunc (GLenum func, GLclampf ref) 
{
	CRContext *g             = GetCurrentContext();
	CRBufferState *b         = &(g->buffer);
	CRStateBits *sb          = GetCurrentBits();
	CRBufferBits *bb = &(sb->buffer);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glAlphaFunc called in begin/end");
		return;
	}

	FLUSH();

	switch (func) 
	{
		case GL_NEVER:
		case GL_LESS:
		case GL_EQUAL:
		case GL_LEQUAL:
		case GL_GREATER:
		case GL_GEQUAL:
		case GL_NOTEQUAL:
		case GL_ALWAYS:
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glAlphaFunc:  Invalid func: %d", func);
			return;
	}

	if (ref < 0.0f) ref = 0.0f;
	if (ref > 1.0f) ref = 1.0f;

	b->alphaTestFunc = func;
	b->alphaTestRef = ref;
	bb->dirty = g->neg_bitid;
	bb->alphaFunc = g->neg_bitid;
}

void STATE_APIENTRY crStateDepthFunc (GLenum func) 
{
	CRContext *g = GetCurrentContext();
	CRBufferState *b = &(g->buffer);
	CRStateBits *sb = GetCurrentBits();
	CRBufferBits *bb = &(sb->buffer);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glDepthFunc called in begin/end");
		return;
	}

	FLUSH();

	switch (func) 
	{
		case GL_NEVER:
		case GL_LESS:
		case GL_EQUAL:
		case GL_LEQUAL:
		case GL_GREATER:
		case GL_NOTEQUAL:
		case GL_GEQUAL:
		case GL_ALWAYS:
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glDepthFunc:  Invalid func: %d", func);
			return;
	}


	b->depthFunc = func;
	bb->dirty = g->neg_bitid;
	bb->depthFunc = g->neg_bitid;
}

void STATE_APIENTRY crStateBlendFunc (GLenum sfactor, GLenum dfactor) 
{
	CRContext *g = GetCurrentContext();
	CRBufferState *b = &(g->buffer);
	CRStateBits *sb = GetCurrentBits();
	CRBufferBits *bb = &(sb->buffer);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glBlendFunc called in begin/end");
		return;
	}

	FLUSH();

	switch (sfactor) 
	{
		case GL_ZERO:
		case GL_ONE:
		case GL_DST_COLOR:
		case GL_ONE_MINUS_DST_COLOR:
		case GL_SRC_ALPHA:
		case GL_ONE_MINUS_SRC_ALPHA:
		case GL_DST_ALPHA:
		case GL_ONE_MINUS_DST_ALPHA:
		case GL_SRC_ALPHA_SATURATE:
			break;
		default:
			if (!crStateBlendFuncExtensionsCheckFactor( sfactor ))
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "Invalid sfactor passed to glBlendFunc: %d", sfactor);
				return;
			}
	}

	switch (dfactor) 
	{
		case GL_ZERO:
		case GL_ONE:
		case GL_SRC_COLOR:
		case GL_ONE_MINUS_SRC_COLOR:
		case GL_SRC_ALPHA:
		case GL_ONE_MINUS_SRC_ALPHA:
		case GL_DST_ALPHA:
		case GL_ONE_MINUS_DST_ALPHA:
			break;
		default:
			if (!crStateBlendFuncExtensionsCheckFactor( dfactor ))
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "Invalid dfactor passed to glBlendFunc: %d", dfactor);
				return;
			}
	}

	b->blendSrc = sfactor;
	b->blendDst = dfactor;
	bb->dirty = g->neg_bitid;
	bb->blendFunc = g->neg_bitid;
}

void STATE_APIENTRY crStateLogicOp (GLenum opcode) 
{
	CRContext *g = GetCurrentContext();
	CRBufferState *b = &(g->buffer);
	CRStateBits *sb = GetCurrentBits();
	CRBufferBits *bb = &(sb->buffer);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glLogicOp called in begin/end");
		return;
	}

	FLUSH();

	switch (opcode) 
	{
		case GL_CLEAR:
		case GL_SET:
		case GL_COPY:
		case GL_COPY_INVERTED:
		case GL_NOOP:
		case GL_INVERT:
		case GL_AND:
		case GL_NAND:
		case GL_OR:
		case GL_NOR:
		case GL_XOR:
		case GL_EQUIV:
		case GL_AND_REVERSE:
		case GL_AND_INVERTED:
		case GL_OR_REVERSE:
		case GL_OR_INVERTED:
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glLogicOp called with bogus opcode: %d", opcode);
			return;
	}

	b->logicOpMode = opcode;
	bb->dirty = g->neg_bitid;
	bb->logicOp = g->neg_bitid;
}

void STATE_APIENTRY crStateDrawBuffer (GLenum mode) 
{
	CRContext *g = GetCurrentContext();
	CRBufferState *b = &(g->buffer);
	CRStateBits *sb = GetCurrentBits();
	CRBufferBits *bb = &(sb->buffer);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glDrawBuffer called in begin/end");
		return;
	}

	FLUSH();

	switch (mode) 
	{
		case GL_NONE:
		case GL_FRONT_LEFT:
		case GL_FRONT_RIGHT:
		case GL_BACK_LEFT:
		case GL_BACK_RIGHT:
		case GL_FRONT:
		case GL_BACK:
		case GL_LEFT:
		case GL_RIGHT:
		case GL_FRONT_AND_BACK:
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glDrawBuffer called with bogus mode: %d", mode);
			return;
	}

	b->drawBuffer = mode;
	bb->dirty = g->neg_bitid;
	bb->drawBuffer = g->neg_bitid;
}

void STATE_APIENTRY crStateReadBuffer (GLenum mode) 
{
	CRContext *g = GetCurrentContext();
	CRBufferState *b = &(g->buffer);
	CRStateBits *sb = GetCurrentBits();
	CRBufferBits *bb = &(sb->buffer);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glReadBuffer called in begin/end");
		return;
	}

	FLUSH();

	switch (mode) 
	{
		case GL_NONE:
		case GL_FRONT_LEFT:
		case GL_FRONT_RIGHT:
		case GL_BACK_LEFT:
		case GL_BACK_RIGHT:
		case GL_FRONT:
		case GL_BACK:
		case GL_LEFT:
		case GL_RIGHT:
		case GL_FRONT_AND_BACK:
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glReadBuffer called with bogus mode: %d", mode);
			return;
	}

	b->readBuffer = mode;
	bb->dirty = g->neg_bitid;
	bb->readBuffer = g->neg_bitid;
}

void STATE_APIENTRY crStateIndexMask (GLuint mask) 
{
	CRContext *g = GetCurrentContext();
	CRBufferState *b = &(g->buffer);
	CRStateBits *sp = GetCurrentBits();
	CRBufferBits *bb = &(sp->buffer);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glReadBuffer called in begin/end");
		return;
	}

	FLUSH();

	b->indexWriteMask = mask;
	bb->dirty = g->neg_bitid;
	bb->indexMask = g->neg_bitid;
}

void STATE_APIENTRY crStateColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) 
{
	CRContext *g = GetCurrentContext();
	CRBufferState *b = &(g->buffer);
	CRStateBits *sp = GetCurrentBits();
	CRBufferBits *bb = &(sp->buffer);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glReadBuffer called in begin/end");
		return;
	}

	FLUSH();

	b->colorWriteMask.r = red;
	b->colorWriteMask.g = green;
	b->colorWriteMask.b = blue;
	b->colorWriteMask.a = alpha;
	bb->dirty = g->neg_bitid;
	bb->colorWriteMask = g->neg_bitid;
}

void STATE_APIENTRY crStateClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) 
{
	CRContext *g = GetCurrentContext();
	CRBufferState *b = &(g->buffer);
	CRStateBits *sp = GetCurrentBits();
	CRBufferBits *bb = &(sp->buffer);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glClearColor called in begin/end");
		return;
	}

	FLUSH();

	if (red < 0.0f) red = 0.0f;
	if (red > 1.0f) red = 1.0f;
	if (green < 0.0f) green = 0.0f;
	if (green > 1.0f) green = 1.0f;
	if (blue < 0.0f) blue = 0.0f;
	if (blue > 1.0f) blue = 1.0f;
	if (alpha < 0.0f) alpha = 0.0f;
	if (alpha > 1.0f) alpha = 1.0f;

	b->colorClearValue.r = red;
	b->colorClearValue.g = green;
	b->colorClearValue.b = blue;
	b->colorClearValue.a = alpha;
	bb->dirty = g->neg_bitid;
	bb->clearColor = g->neg_bitid;
}

void STATE_APIENTRY crStateClearIndex (GLfloat c) 
{
	CRContext *g = GetCurrentContext();
	CRBufferState *b = &(g->buffer);
	CRStateBits *sp = GetCurrentBits();
	CRBufferBits *bb = &(sp->buffer);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glClearIndex called in begin/end");
		return;
	}

	b->indexClearValue = c;
	bb->dirty = g->neg_bitid;
	bb->clearIndex = g->neg_bitid;
}

void STATE_APIENTRY crStateClearDepth (GLclampd depth) 
{
	CRContext *g = GetCurrentContext();
	CRBufferState *b = &(g->buffer);
	CRStateBits *sp = GetCurrentBits();
	CRBufferBits *bb = &(sp->buffer);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glClearDepth called in begin/end");
		return;
	}

	FLUSH();

	if (depth < 0.0) depth = 0.0;
	if (depth > 1.0) depth = 1.0;

	b->depthClearValue = (GLdefault) depth;
	bb->dirty = g->neg_bitid;
	bb->clearDepth = g->neg_bitid;
}

void STATE_APIENTRY crStateClearAccum (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) 
{
	CRContext *g = GetCurrentContext();
	CRBufferState *b = &(g->buffer);
	CRStateBits *sp = GetCurrentBits();
	CRBufferBits *bb = &(sp->buffer);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glClearAccum called in begin/end");
		return;
	}
	
	FLUSH();

	if (red < -1.0f) red = 0.0f;
	if (red > 1.0f) red = 1.0f;
	if (green < -1.0f) green = 0.0f;
	if (green > 1.0f) green = 1.0f;
	if (blue < -1.0f) blue = 0.0f;
	if (blue > 1.0f) blue = 1.0f;
	if (alpha < -1.0f) alpha = 0.0f;
	if (alpha > 1.0f) alpha = 1.0f;

	b->accumClearValue.r = red;
	b->accumClearValue.g = green;
	b->accumClearValue.b = blue;
	b->accumClearValue.a = alpha;
	bb->dirty = g->neg_bitid;
	bb->clearAccum = g->neg_bitid;
}

void STATE_APIENTRY crStateDepthMask (GLboolean b) 
{
	CRContext *g = GetCurrentContext();
	CRBufferState *bs = &(g->buffer);
	CRStateBits *sp = GetCurrentBits();
	CRBufferBits *bb = &(sp->buffer);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "DepthMask called in begin/end");
		return;
	}

	FLUSH();

	bs->depthMask = b;
	bb->dirty = g->neg_bitid;
	bb->depthMask = g->neg_bitid;
}
