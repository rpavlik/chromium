#include <stdio.h>
#include <stdlib.h>
#include "cr_glstate.h"
#include "state/cr_statetypes.h"
#include "state_internals.h"

void crStateStencilInit(CRStencilState *s) 
{
	s->stencilTest = GL_FALSE;
	s->func = GL_ALWAYS;
	s->mask = 0xFFFFFFFF;
	s->ref = 0;
	s->fail = GL_KEEP;
	s->passDepthFail = GL_KEEP;
	s->passDepthPass = GL_KEEP;
	s->clearValue = 0;
	s->writeMask = 0xFFFFFFFF;
}

void STATE_APIENTRY crStateStencilFunc(GLenum func, GLint ref, GLuint mask) 
{
	CRContext *g = GetCurrentContext();
	CRStencilState *s = &(g->stencil);
	CRStateBits *stateb = GetCurrentBits();
	CRStencilBits *sb = &(stateb->stencil);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, 
			"glStencilFunc called in begin/end");
		return;
	}

	FLUSH();

	if (func != GL_NEVER &&
		func != GL_LESS &&
		func != GL_LEQUAL &&
		func != GL_GREATER &&
		func != GL_GEQUAL &&
		func != GL_EQUAL &&
		func != GL_NOTEQUAL &&
		func != GL_ALWAYS)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, 
			"glStencilFunc called with bogu func: %d", func);
		return;
	}

	s->func = func;
	s->ref = ref;
	s->mask = mask;

	sb->func = g->neg_bitid;
	sb->dirty = g->neg_bitid;
}

void STATE_APIENTRY crStateStencilOp (GLenum fail, GLenum zfail, GLenum zpass) 
{
	CRContext *g = GetCurrentContext();
	CRStencilState *s = &(g->stencil);
	CRStateBits *stateb = GetCurrentBits();
	CRStencilBits *sb = &(stateb->stencil);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, 
			"glStencilOp called in begin/end");
		return;
	}

	FLUSH();

	if (fail != GL_KEEP &&
		fail != GL_ZERO &&
		fail != GL_REPLACE &&
		fail != GL_INCR &&
		fail != GL_DECR &&
		fail != GL_INVERT) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, 
			"glStencilOp called with bogus fail: %d", fail);
		return;
	}

	if (zfail != GL_KEEP &&
		zfail != GL_ZERO &&
		zfail != GL_REPLACE &&
		zfail != GL_INCR &&
		zfail != GL_DECR &&
		zfail != GL_INVERT) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, 
			"glStencilOp called with bogus zfail: %d", zfail);
		return;
	}

	if (zpass != GL_KEEP &&
		zpass != GL_ZERO &&
		zpass != GL_REPLACE &&
		zpass != GL_INCR &&
		zpass != GL_DECR &&
		zpass != GL_INVERT) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, 
			"glStencilOp called with bogus zpass: %d", zpass);
		return;
	}

	s->fail = fail;
	s->passDepthFail = zfail;
	s->passDepthPass = zpass;

	sb->op = g->neg_bitid;
	sb->dirty = g->neg_bitid;
}


void STATE_APIENTRY crStateClearStencil (GLint c) 
{
	CRContext *g = GetCurrentContext();
	CRStencilState *s = &(g->stencil);
	CRStateBits *stateb = GetCurrentBits();
	CRStencilBits *sb = &(stateb->stencil);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, 
			"glClearStencil called in begin/end");
		return;
	}

	FLUSH();


	s->clearValue = c;
	
	sb->clearValue = g->neg_bitid;
	sb->dirty = g->neg_bitid;
}

void STATE_APIENTRY crStateStencilMask (GLuint mask) 
{
	CRContext *g = GetCurrentContext();
	CRStencilState *s = &(g->stencil);
	CRStateBits *stateb = GetCurrentBits();
	CRStencilBits *sb = &(stateb->stencil);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, 
			"glStencilMask called in begin/end");
		return;
	}

	FLUSH();

	s->mask = mask;

	sb->writeMask = g->neg_bitid;
	sb->dirty = g->neg_bitid;
}
