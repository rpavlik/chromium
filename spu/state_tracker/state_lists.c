#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "cr_glstate.h"
#include "state/cr_statetypes.h"

void crStateListsInit(CRListsState *l) 
{
	l->maxListNesting = 10; // XXX
	l->newEnd = GL_FALSE;
	l->freeList = (CRListsFreeElem*) malloc (sizeof(*(l->freeList)));
	l->freeList->min = 1;
	l->freeList->max = GL_MAXUINT;
	l->freeList->next = NULL;
	l->freeList->prev = NULL;
}

void crStateListsBindName(CRListsState *l, GLuint name) 
{
	CRListsFreeElem *i;
	CRListsFreeElem *newelem;
	
	/* First find which region it fits in */
	for (i=l->freeList; i && !(i->min <= name && name <= i->max); i=i->next)
	{
		// EMPTY BODY
	}

	if (!i) 
	{
		crStateError( __LINE__, __FILE__, GL_INVALID_ENUM, "Couldn't bind list number %d", name );
	}

	/* (name, name) */
	if (i->max == name && i->min == name) 
	{
		/*Remove from freeList*/
		if (i==l->freeList) 
		{
			l->freeList = l->freeList->next;
			l->freeList->prev = NULL;
		} 
		else 
		{
			i->prev->next = i->next;
			i->next->prev = i->prev;
		}
		free (i);
		return;
	}
		
	/* (name, ~) */
	if (i->min == name) 
	{
		i->min++;
		return;
	}

	/* (~, name) */
	if (i->max == name) 
	{
		i->max--;
		return;
	}

	/* (<name, >name) change to        */
	/* (<name, name-1) (name+1, >name) */
	newelem = (CRListsFreeElem *) malloc (sizeof(CRListsFreeElem));
	newelem->min = name+1;
	newelem->max = i->max;
	i->max = name-1;

	newelem->next = i->next;
	newelem->prev = i;
	if (i->next)
	{
		i->next->prev = newelem;
	}
	i->next = newelem;
}


void crStateListsUnbindName(CRListsState *l, GLuint name) 
{
	CRListsFreeElem *i;
	CRListsFreeElem *newelem;

	/*********************************/
	/* Add the name to the freeList  */
	/* Find the bracketing sequences */

	for (i=l->freeList;
		i && i->next && i->next->min < name;
		i = i->next)
	{
		// EMPTY BODY
	}

	/* j will always be valid */
	if (!i) return;
	if (!i->next && i->max == name)  return;

	/* Case:  j:(~,name-1) */
	if (i->max+1 == name) 
	{
		i->max++;
		if (i->next && i->max+1 >= i->next->min) 
		{
			/* Collapse */
			i->next->min = i->min;
			i->next->prev = i->prev;
			if (i->prev)
			{
				i->prev->next = i->next;
			}
			if (i==l->freeList)
			{
				l->freeList = i->next;
			}
			free(i);
		}
		return;
	}

	/* Case: j->next: (name+1, ~)*/
	if (i->next && i->next->min-1 == name) 
	{
		i->next->min--;
		if (i->max+1 >= i->next->min) 
		{
			/* Collapse */
			i->next->min = i->min;
			i->next->prev = i->prev;
			if (i->prev)
			{
				i->prev->next = i->next;
			}
			if (i==l->freeList) 
			{
				l->freeList = i->next;
			}
			free(i);
		}
		return;
	}

	/* Case: j: (name+1, ~) j->next: null */
	if (!i->next && i->min-1 == name) 
	{
		i->min--;
		return;
	}

	newelem = (CRListsFreeElem *) malloc (sizeof (CRListsFreeElem));
	newelem->min = name;
	newelem->max = name;

	/* Case: j: (~,name-(2+))  j->next: (name+(2+), ~) or null */
	if (name > i->max) 
	{
		newelem->prev = i;
		newelem->next = i->next;
		if (i->next)
		{
			i->next->prev = newelem;
		}
		i->next = newelem;
		return;
	}

	/* Case: j: (name+(2+), ~) */
	/* Can only happen if j = t->freeList! */
	if (i == l->freeList && i->min > name) 
	{
		newelem->next = i;
		newelem->prev = i->prev;
		i->prev = newelem;
		l->freeList = newelem;
		return;
	}
}


GLuint STATE_APIENTRY crStateGenLists(GLsizei range)
{
	CRContext *g = GetCurrentContext();
	CRListsState *l = &(g->lists);
	CRListsFreeElem *f;
	GLuint ret;

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

	f = l->freeList;
	while (f)
	{
		GLuint temp = f->max - f->min;
		if (temp >= (GLuint) (range - 1))
		{
			ret = f->min;
			f->min += range-1;
			/*
			** To handle the list management, we call
			** crStatebindname on the last one
			** so that everything gets cleaned up
			*/
			crStateListsBindName(l, f->min);
			return ret;
		}
		f = f->next;
	}

	return 0;
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
		crStateListsUnbindName(l, list+i);
	}
}

GLboolean STATE_APIENTRY crStateIsList(GLuint list)
{
	CRContext *g = GetCurrentContext();
	CRListsState *l = &(g->lists);
	CRListsFreeElem *i;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
			"GenLists called in Begin/End");
		return GL_FALSE;
	}
	
	/* First find which region it fits in */
	for (i=l->freeList; i && !(i->min <= list && list <= i->max); i=i->next)
	{
		// EMPTY BODY
	}

	if (i != NULL) 
	{
		return GL_TRUE;
	}
	return GL_FALSE;
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
