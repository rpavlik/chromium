#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "cr_glstate.h"
#include "state/cr_statetypes.h"
#include "state_internals.h"
#include "state_extensionfuncs.h"
#include "cr_error.h"

void crStateAttribInit (CRAttribState *a) 
{
	a->attribStackDepth = 0;
	a->maxAttribStackDepth = CR_MAX_ATTRIB_STACK_DEPTH; // XXX
}

void STATE_APIENTRY crStatePushAttrib(GLbitfield mask)
{
	CRContext *g = GetCurrentContext();
	CRAttribState *a = &(g->attrib);
	CRStateBits *sb = GetCurrentBits();
	CRAttribBits *ab = &(sb->attrib);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glPushAttrib called in Begin/End");
		return;
	}

	if (a->attribStackDepth == CR_MAX_ATTRIB_STACK_DEPTH - 1)
	{
		crStateError(__LINE__, __FILE__, GL_STACK_OVERFLOW, "glPushAttrib called with a full stack!" );
	}
	//crDebug( "pushAttrib called with value 0x%x", (unsigned int) mask );

	a->pushMaskStack[a->attribStackDepth++] = mask;

	ab->dirty = g->neg_bitid;
}

void STATE_APIENTRY crStatePopAttrib(void) 
{
	CRContext *g = GetCurrentContext();
	CRAttribState *a = &(g->attrib);
	CRStateBits *sb = GetCurrentBits();
	CRAttribBits *ab = &(sb->attrib);

	GLbitvalue mask;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glPopAttrib called in Begin/End");
		return;
	}

	if (a->attribStackDepth == 0)
	{
		crStateError(__LINE__, __FILE__, GL_STACK_UNDERFLOW, "glPopAttrib called with an empty stack!" );
	}

	mask = a->pushMaskStack[--a->attribStackDepth];

	//crDebug( "PopAttrib was called, the mask on the top of the stack was 0x%x", (unsigned int) mask );

	ab->dirty = g->neg_bitid;
}

void crStateAttribSwitch( CRAttribBits *bb, GLbitvalue bitID,
		CRAttribState *from, CRAttribState *to )
{
	(void) bb;
	(void) bitID;
	(void) from;
	(void) to;
}
