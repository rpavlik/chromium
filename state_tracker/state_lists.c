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

void crStateListsDestroy(CRContext *ctx)
{
	CRListsState *l = &ctx->lists;
	crFreeHashtable(l->hash, crFree); /* call crFree for each entry */
}

void crStateListsInit(CRContext *ctx)
{
	CRListsState *l = &ctx->lists;

	l->newEnd = GL_FALSE;
	l->mode = 0;
	l->currentIndex = 0;
	l->hash = crAllocHashtable();
}

void STATE_APIENTRY crStateNewList (GLuint list, GLenum mode) 
{
	CRContext *g = GetCurrentContext();
	CRListsState *l = &(g->lists);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glNewList called in Begin/End");
		return;
	}

	if (list == 0)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glNewList(list=0)");
		return;
	}

	if (l->currentIndex)
	{
		/* already building a list */
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glNewList called inside display list");
		return;
	}

	if (mode != GL_COMPILE && mode != GL_COMPILE_AND_EXECUTE)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glNewList invalid mode");
		return;
	}

	FLUSH();

	/* Need this???
	crStateCurrentRecover();
	*/

	l->currentIndex = list;
	l->mode = mode;
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

	if (!l->currentIndex)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glEndList called outside display list");
		return;
	}

	l->currentIndex = 0;
	l->mode = 0;
}

GLuint STATE_APIENTRY crStateGenLists(GLsizei range)
{
	CRContext *g = GetCurrentContext();
	CRListsState *l = &(g->lists);
	GLuint start;

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

	start = crHashtableAllocKeys(l->hash, range);
	CRASSERT(start > 0);
	return start;
}
	
void STATE_APIENTRY crStateDeleteLists (GLuint list, GLsizei range)
{
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

	crHashtableDeleteBlock(l->hash, list, range, crFree); /* call crFree to delete list data */
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

	if (list == 0)
		return GL_FALSE;

	return crHashtableIsKeyUsed(l->hash, list);
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
		CRContext *fromCtx, CRContext *toCtx )
{
	(void) b;
	(void) bitID;
	(void) fromCtx;
	(void) toCtx;
}

void crStateListsSwitch( CRListsBits *b, CRbitvalue *bitID, 
		CRContext *fromCtx, CRContext *toCtx )

{
	(void) b;
	(void) bitID;
	(void) fromCtx;
	(void) toCtx;
}
