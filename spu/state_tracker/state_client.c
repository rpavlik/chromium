#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "cr_glstate.h"
#include "state/cr_statetypes.h"

#define GLCLIENT_NUMPOINTERS 6
#define GLCLIENT_INDEX_ALLOC 1024
#define GLCLIENT_DATA_ALLOC 1024
#define GLCLIENT_CACHE_ALLOC 1024
#define GLCLIENT_LIST_ALLOC 1024
#define GLCLIENT_BIT_ALLOC 1024

void crStateClientInitBits (CRClientBits *c) 
{
	c->v = (GLbitvalue *) malloc (GLCLIENT_BIT_ALLOC*sizeof(GLbitvalue));
	c->n = (GLbitvalue *) malloc (GLCLIENT_BIT_ALLOC*sizeof(GLbitvalue));
	c->c = (GLbitvalue *) malloc (GLCLIENT_BIT_ALLOC*sizeof(GLbitvalue));
	c->i = (GLbitvalue *) malloc (GLCLIENT_BIT_ALLOC*sizeof(GLbitvalue));
	c->t = (GLbitvalue *) malloc (GLCLIENT_BIT_ALLOC*sizeof(GLbitvalue));
	c->e = (GLbitvalue *) malloc (GLCLIENT_BIT_ALLOC*sizeof(GLbitvalue));
	memset(c->v, 0, GLCLIENT_BIT_ALLOC*sizeof(GLbitvalue));
	memset(c->n, 0, GLCLIENT_BIT_ALLOC*sizeof(GLbitvalue));
	memset(c->c, 0, GLCLIENT_BIT_ALLOC*sizeof(GLbitvalue));
	memset(c->i, 0, GLCLIENT_BIT_ALLOC*sizeof(GLbitvalue));
	memset(c->t, 0, GLCLIENT_BIT_ALLOC*sizeof(GLbitvalue));
	memset(c->e, 0, GLCLIENT_BIT_ALLOC*sizeof(GLbitvalue));
	c->valloc = GLCLIENT_BIT_ALLOC;
	c->nalloc = GLCLIENT_BIT_ALLOC;
	c->calloc = GLCLIENT_BIT_ALLOC;
	c->ialloc = GLCLIENT_BIT_ALLOC;
	c->talloc = GLCLIENT_BIT_ALLOC;
	c->ealloc = GLCLIENT_BIT_ALLOC;
}

void crStateClientInit(CRClientState *c) 
{
	CRClientPointer *lookup[6];
	CRClientPointer *cp;
	int i;

	lookup[0] = &(c->v);
	lookup[1] = &(c->c);
	lookup[2] = &(c->e);
	lookup[3] = &(c->i);
	lookup[4] = &(c->n);
	lookup[5] = &(c->t);

	c->maxElementsIndices = 16384; // XXX
	c->maxElementsVertices = 16384; // XXX

	c->list_alloc = GLCLIENT_LIST_ALLOC;
	c->list_size = 0;
	c->list = (int *) malloc (c->list_alloc * sizeof (int));

	for (i=0; i<GLCLIENT_NUMPOINTERS; i++) 
	{
		cp = lookup[i];
		cp->p = NULL;
		cp->size = 0;
		cp->type = GL_NONE;
		cp->stride = 0;
		cp->enabled = 0;
	}
}

void crStateClientSetClientState(CRClientState *c, CRClientBits *cb, 
		GLbitvalue neg_bitid, GLenum array, GLboolean state) 
{
	switch (array) 
	{
		case GL_VERTEX_ARRAY:
			c->v.enabled = state;
			break;
		case GL_COLOR_ARRAY:
			c->c.enabled = state;
			break;
		case GL_NORMAL_ARRAY:
			c->n.enabled = state;
			break;
		case GL_INDEX_ARRAY:
			c->i.enabled = state;
			break;
		case GL_TEXTURE_COORD_ARRAY:
			c->t.enabled = state;
			break;
		case GL_EDGE_FLAG_ARRAY:
			c->e.enabled = state;
			break;	
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "Invalid Enum passed to Enable/Disable Client State");
			return;
	}
	cb->dirty = neg_bitid;
	cb->enableClientState = neg_bitid;
}

void STATE_APIENTRY crStateEnableClientState (GLenum array) 
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRClientBits *cb = &(sb->client);

	crStateClientSetClientState(c, cb, g->neg_bitid, array, GL_TRUE);
}

void STATE_APIENTRY crStateDisableClientState (GLenum array) 
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRClientBits *cb = &(sb->client);

	crStateClientSetClientState(c, cb, g->neg_bitid, array, GL_TRUE);
}

void crStateClientSetPointer (CRClientPointer *cp, GLint size, 
		GLenum type, GLsizei stride, const GLvoid *pointer) 
{
	cp->p = (unsigned char *) pointer;
	cp->size = size;
	cp->type = type;
	// Calculate the bytes per index for address calculation
	cp->bytesPerIndex = size;
	switch (type) 
	{
		case GL_BYTE:
		case GL_UNSIGNED_BYTE:
			break;
		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
			cp->bytesPerIndex *= sizeof(GLshort);
			break;
		case GL_INT:
		case GL_UNSIGNED_INT:
			cp->bytesPerIndex *= sizeof(GLint);
			break;
		case GL_FLOAT:
			cp->bytesPerIndex *= sizeof(GLfloat);
			break;
		case GL_DOUBLE:
			cp->bytesPerIndex *= sizeof(GLdouble);
			break;
		default:
			crStateError( __LINE__, __FILE__, GL_INVALID_VALUE, "Unknown type of vertex array: %d", type );
	}

	/* 
	 **  Note: If stide==0 then we set the 
	 **  stride equal address offset
	 **  therefore stride can never equal
	 **  zero.
	 */
	cp->stride = stride;
	if (!stride) cp->stride = cp->bytesPerIndex;
}

void STATE_APIENTRY crStateVertexPointer(GLint size, GLenum type, 
		GLsizei stride, const GLvoid *p) 
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRClientBits *cb = &(sb->client);

	if (size != 2 && size != 3 && size != 4)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glVertexPointer: invalid size: %d", size);
		return;
	}
	if (type != GL_SHORT && type != GL_INT &&
			type != GL_FLOAT && type != GL_DOUBLE)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glVertexPointer: invalid type: %d", type);
		return;
	}
	if (stride < 0) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glVertexPointer: stride was negative: %d", stride);
		return;
	}

	crStateClientSetPointer(&(c->v), size, type, stride, p);
	cb->dirty = g->neg_bitid;
	cb->clientPointer = g->neg_bitid;
}

void STATE_APIENTRY crStateColorPointer(GLint size, GLenum type, 
		GLsizei stride, const GLvoid *p) 
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRClientBits *cb = &(sb->client);

	if (size != 3 && size != 4)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glColorPointer: invalid size: %d", size);
		return;
	}
	if (type != GL_BYTE && type != GL_UNSIGNED_BYTE &&
			type != GL_SHORT && type != GL_UNSIGNED_SHORT &&
			type != GL_INT && type != GL_UNSIGNED_INT &&
			type != GL_FLOAT && type != GL_DOUBLE)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glColorPointer: invalid type: %d", type);
		return;
	}
	if (stride < 0) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glColorPointer: stride was negative: %d", stride);
		return;
	}

	crStateClientSetPointer(&(c->c), size, type, stride, p);
	cb->dirty = g->neg_bitid;
	cb->clientPointer = g->neg_bitid;
}

void STATE_APIENTRY crStateIndexPointer(GLenum type, GLsizei stride, 
		const GLvoid *p) 
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRClientBits *cb = &(sb->client);

	if (type != GL_SHORT && type != GL_INT &&
			type != GL_FLOAT && type != GL_DOUBLE)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glIndexPointer: invalid type: %d", type);
		return;
	}
	if (stride < 0) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glIndexPointer: stride was negative: %d", stride);
		return;
	}

	crStateClientSetPointer(&(c->i), 1, type, stride, p);
	cb->dirty = g->neg_bitid;
	cb->clientPointer = g->neg_bitid;
}

void STATE_APIENTRY crStateNormalPointer(GLenum type, GLsizei stride, 
		const GLvoid *p) 
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRClientBits *cb = &(sb->client);

	if (type != GL_BYTE && type != GL_SHORT &&
			type != GL_INT && type != GL_FLOAT &&
			type != GL_DOUBLE)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glNormalPointer: invalid type: %d", type);
		return;
	}
	if (stride < 0) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glNormalPointer: stride was negative: %d", stride);
		return;
	}

	crStateClientSetPointer(&(c->n), 3, type, stride, p);
	cb->dirty = g->neg_bitid;
	cb->clientPointer = g->neg_bitid;
}

void STATE_APIENTRY crStateTexCoordPointer(GLint size, GLenum type, 
		GLsizei stride, const GLvoid *p) 
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRClientBits *cb = &(sb->client);

	if (size != 1 && size != 2 && size != 3 && size != 4)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexCoordPointer: invalid size: %d", size);
		return;
	}
	if (type != GL_SHORT && type != GL_INT &&
			type != GL_FLOAT && type != GL_DOUBLE)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glTexCoordPointer: invalid type: %d", type);
		return;
	}
	if (stride < 0) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexCoordPointer: stride was negative: %d", stride);
		return;
	}

	crStateClientSetPointer(&(c->t), size, type, stride, p);
	cb->dirty = g->neg_bitid;
	cb->clientPointer = g->neg_bitid;
}

void STATE_APIENTRY crStateEdgeFlagPointer(GLsizei stride, const GLvoid *p) 
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRClientBits *cb = &(sb->client);

	if (stride < 0) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexCoordPointer: stride was negative: %d", stride);
		return;
	}

	crStateClientSetPointer(&(c->e), 1, GL_UNSIGNED_BYTE, stride, p);
	cb->dirty = g->neg_bitid;
	cb->clientPointer = g->neg_bitid;
}

void STATE_APIENTRY crStateGetPointerv(GLenum pname, GLvoid * * params) 
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
				"GetPointerv called in begin/end");
		return;
	}

	switch (pname) 
	{
		case GL_VERTEX_ARRAY:
			*params = (GLvoid *) c->v.p;
			break;
		case GL_COLOR_ARRAY:
			*params = (GLvoid *) c->c.p;
			break;
		case GL_NORMAL_ARRAY:
			*params = (GLvoid *) c->n.p;
			break;
		case GL_INDEX_ARRAY:
			*params = (GLvoid *) c->i.p;
			break;
		case GL_TEXTURE_COORD_ARRAY:
			*params = (GLvoid *) c->t.p;
			break;
		case GL_EDGE_FLAG_ARRAY:
			*params = (GLvoid *) c->e.p;
			break;
		default:
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
						"glGetPointerv: invalid pname: %d", pname);
				return;
			}
	}
}
