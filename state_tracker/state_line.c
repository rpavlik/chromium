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

void crStateLineInit (CRLineState *l) {
	l->lineSmooth = GL_FALSE;
	l->lineStipple = GL_FALSE;
	l->width = 1.0f;
	l->pattern = 0xFFFF;
	l->repeat = 1;
	/*
	 *l->aliasedlinewidth_min = c->aliasedlinewidth_min; 
	 *l->aliasedlinewidth_max = c->aliasedlinewidth_max; 
	 *l->aliasedlinegranularity = c->aliasedlinegranularity; 
	 *l->smoothlinewidth_min = c->smoothlinewidth_min; 
	 *l->smoothlinewidth_max = c->smoothlinewidth_max; 
	 *l->smoothlinegranularity = c->smoothlinegranularity; */
}

void STATE_APIENTRY crStateLineWidth(GLfloat width) 
{
	CRContext *g = GetCurrentContext();
	CRStateBits *sb = GetCurrentBits();
	CRLineState *l = &(g->line);
	CRLineBits *lb = &(sb->line);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glLineWidth called in begin/end");
		return;
	}

	FLUSH();

	if (width <= 0.0f) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glLineWidth called with size <= 0.0: %f", width);
		return;
	}

	l->width = width;
	DIRTY(lb->width, g->neg_bitid);
	DIRTY(lb->dirty, g->neg_bitid);
}

void STATE_APIENTRY crStateLineStipple(GLint factor, GLushort pattern) 
{
	CRContext *g = GetCurrentContext();
	CRLineState *l = &(g->line);
	CRStateBits *sb = GetCurrentBits();
	CRLineBits *lb = &(sb->line);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
			"glLineStipple called in begin/end");
		return;
	}

	FLUSH();

	if (factor < 1) factor = 1;
	if (factor > 256) factor = 256;

	l->pattern = pattern;
	l->repeat = factor;
	DIRTY(lb->stipple, g->neg_bitid);
	DIRTY(lb->dirty, g->neg_bitid);
}

