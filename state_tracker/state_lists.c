/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_mem.h"
#include "state.h"
#include "state/cr_statetypes.h"
#include "state_internals.h"

void crStateListsInit(CRContext *ctx)
{
	CRListsState *l = &ctx->lists;

	l->newEnd = GL_FALSE;
	l->mode = 0;
	l->idPool = crAllocIdPool();
	l->hash = crAllocHashtable();
}

void STATE_APIENTRY crStateNewList (GLuint list, GLenum mode) 
{
	CRContext *g = GetCurrentContext();
	CRListsState *l = &(g->lists);
	CRListEffect *effect;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glNewList called in Begin/End");
		return;
	}

	if (l->newEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glNewList called inside display list");
		return;
	}

	if (mode != GL_COMPILE && mode != GL_COMPILE_AND_EXECUTE)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glNewList invalid mode");
		return;
	}

	FLUSH();

	crIdPoolAllocId(l->idPool, list);

	l->currentIndex = list;
	l->newEnd = GL_TRUE;
	l->mode = mode;
	effect = crCalloc(sizeof(CRListEffect));
	crHashtableAdd(l->hash, list, effect);
}

void STATE_APIENTRY crStateEndList (void) 
{
	CRContext *g = GetCurrentContext();
	CRListsState *l = &(g->lists);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glEndList called in Begin/End");
		return;
	}

	if (!l->newEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glEndList called outside display list");
		return;
	}

	l->currentIndex = 0;
	l->newEnd = GL_FALSE;
	l->mode = 0;
}

GLuint STATE_APIENTRY crStateGenLists(GLsizei range)
{
	CRContext *g = GetCurrentContext();
	CRListsState *l = &(g->lists);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glGenLists called in Begin/End");
		return 0;
	}

	if (range < 0)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative range passed to glGenLists: %d", range);
		return 0;
	}

	return crIdPoolAllocBlock(l->idPool, range);
}
	
void STATE_APIENTRY crStateDeleteLists (GLuint list, GLsizei range)
{
	int i;
	CRContext *g = GetCurrentContext();
	CRListsState *l = &(g->lists);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glDeleteLists called in Begin/End");
		return;
	}

	if (range < 0)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative range passed to glDeleteLists: %d", range);
		return;
	}

	for (i=0; i<range; i++)
	{
		crHashtableDelete(l->hash, list + i, GL_TRUE);
		crIdPoolFreeBlock(l->idPool, list, range);
	}
}

GLboolean STATE_APIENTRY crStateIsList(GLuint list)
{
	CRContext *g = GetCurrentContext();
	CRListsState *l = &(g->lists);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
			"GenLists called in Begin/End");
		return GL_FALSE;
	}

	return crIdPoolIsIdUsed(l->idPool, list);
}
	
void STATE_APIENTRY crStateListBase (GLuint base)
{
	CRContext *g = GetCurrentContext();
	CRListsState *l = &(g->lists);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
			"ListBase called in Begin/End");
		return;
	}

	l->base = base;
}

void crStateListsDiff( CRListsBits *b, CRbitvalue *bitID, 
		CRListsState *from, CRListsState *to )
{
	(void) b;
	(void) bitID;
	(void) from;
	(void) to;
}

void crStateListsSwitch( CRListsBits *b, CRbitvalue *bitID, 
		CRListsState *from, CRListsState *to )
{
	(void) b;
	(void) bitID;
	(void) from;
	(void) to;
}
