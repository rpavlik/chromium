/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdlib.h>
#include <stdio.h>
#include "cr_mem.h"
#include "state.h"
#include "state/cr_statetypes.h"
#include "state_internals.h"

#define GLCLIENT_NUMPOINTERS 6
#define GLCLIENT_INDEX_ALLOC 1024
#define GLCLIENT_DATA_ALLOC 1024
#define GLCLIENT_CACHE_ALLOC 1024
#define GLCLIENT_LIST_ALLOC 1024
#define GLCLIENT_BIT_ALLOC 1024

void crStateClientInitBits (CRClientBits *c) 
{
	c->v = (GLbitvalue *) crCalloc(GLCLIENT_BIT_ALLOC*sizeof(GLbitvalue));
	c->n = (GLbitvalue *) crCalloc(GLCLIENT_BIT_ALLOC*sizeof(GLbitvalue));
	c->c = (GLbitvalue *) crCalloc(GLCLIENT_BIT_ALLOC*sizeof(GLbitvalue));
	c->s = (GLbitvalue *) crCalloc(GLCLIENT_BIT_ALLOC*sizeof(GLbitvalue));
	c->i = (GLbitvalue *) crCalloc(GLCLIENT_BIT_ALLOC*sizeof(GLbitvalue));
	c->t = (GLbitvalue *) crCalloc(GLCLIENT_BIT_ALLOC*sizeof(GLbitvalue));
	c->e = (GLbitvalue *) crCalloc(GLCLIENT_BIT_ALLOC*sizeof(GLbitvalue));
	c->valloc = GLCLIENT_BIT_ALLOC;
	c->nalloc = GLCLIENT_BIT_ALLOC;
	c->calloc = GLCLIENT_BIT_ALLOC;
	c->salloc = GLCLIENT_BIT_ALLOC;
	c->ialloc = GLCLIENT_BIT_ALLOC;
	c->talloc = GLCLIENT_BIT_ALLOC;
	c->ealloc = GLCLIENT_BIT_ALLOC;
}

void crStateClientInit(CRLimitsState *limits, CRClientState *c) 
{
  unsigned int i;

	/* pixel pack/unpack */
	c->unpack.rowLength   = 0;
	c->unpack.skipRows    = 0;
	c->unpack.skipPixels  = 0;
	c->unpack.skipImages  = 0;
	c->unpack.alignment   = 4;
	c->unpack.imageHeight = 0;
	c->unpack.swapBytes   = GL_FALSE;
	c->unpack.psLSBFirst  = GL_FALSE;
	c->pack.rowLength     = 0;
	c->pack.skipRows      = 0;
	c->pack.skipPixels    = 0;
	c->pack.skipImages    = 0;
	c->pack.alignment     = 4;
	c->pack.imageHeight   = 0;
	c->pack.swapBytes     = GL_FALSE;
	c->pack.psLSBFirst    = GL_FALSE;

	/* ARB multitexture */
	c->curClientTextureUnit = 0;

	c->list_alloc = GLCLIENT_LIST_ALLOC;
	c->list_size = 0;
	c->list = (int *) crCalloc(c->list_alloc * sizeof (int));

	/* vertex array */
	c->v.p = NULL;
	c->v.size = 0;
	c->v.type = GL_NONE;
	c->v.stride = 0;
	c->v.enabled = 0;
	c->c.p = NULL;
	c->c.size = 0;
	c->c.type = GL_NONE;
	c->c.stride = 0;
	c->c.enabled = 0;
	c->s.p = NULL;
	c->s.size = 0;
	c->s.type = GL_NONE;
	c->s.stride = 0;
	c->s.enabled = 0;
	c->e.p = NULL;
	c->e.size = 0;
	c->e.type = GL_NONE;
	c->e.stride = 0;
	c->e.enabled = 0;
	c->i.p = NULL;
	c->i.size = 0;
	c->i.type = GL_NONE;
	c->i.stride = 0;
	c->i.enabled = 0;
	c->n.p = NULL;
	c->n.size = 0;
	c->n.type = GL_NONE;
	c->n.stride = 0;
	c->n.enabled = 0;
	for (i = 0 ; i < limits->maxTextureUnits ; i++)
	{
		c->t[i].p = NULL;
		c->t[i].size = 0;
		c->t[i].type = GL_NONE;
		c->t[i].stride = 0;
		c->t[i].enabled = 0;
	}
}


/*
 * PixelStore functions are here, not in state_pixel.c because this
 * is client-side state, like vertex arrays.
 */

void STATE_APIENTRY crStatePixelStoref (GLenum pname, GLfloat param)
{

	/* The GL SPEC says I can do this on page 76. */
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

void STATE_APIENTRY crStatePixelStorei (GLenum pname, GLint param)
{
	CRContext *g    = GetCurrentContext();
	CRStateBits *sb = GetCurrentBits();
	CRClientState *c = &(g->client);
	CRClientBits *cb = &(sb->client);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "PixelStore{if} called in Begin/End");
		return;
	}

	FLUSH();


	switch(pname) {
		case GL_PACK_SWAP_BYTES:
			c->pack.swapBytes = (GLboolean) param;
			DIRTY(cb->pack, g->neg_bitid);
			break;
		case GL_PACK_LSB_FIRST:
			c->pack.psLSBFirst = (GLboolean) param;
			DIRTY(cb->pack, g->neg_bitid);
			break;
		case GL_PACK_ROW_LENGTH:
			if (param < 0.0f) 
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative Row Length: %f", param);
				return;
			}
			c->pack.rowLength = param;
			DIRTY(cb->pack, g->neg_bitid);
			break;
#ifdef CR_OPENGL_VERSION_1_2
		case GL_PACK_IMAGE_HEIGHT:
			if (param < 0.0f) 
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative Image Height: %f", param);
				return;
			}
			c->pack.imageHeight = param;
			DIRTY(cb->pack, g->neg_bitid);
			break;
#endif
		case GL_PACK_SKIP_PIXELS:
			if (param < 0.0f) 
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative Skip Pixels: %f", param);
				return;
			}
			c->pack.skipPixels = param;
			DIRTY(cb->pack, g->neg_bitid);
			break;
		case GL_PACK_SKIP_ROWS:
			if (param < 0.0f) 
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative Row Skip: %f", param);
				return;
			}
			c->pack.skipRows = param;
			DIRTY(cb->pack, g->neg_bitid);
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
			c->pack.alignment = param;
			DIRTY(cb->pack, g->neg_bitid);
			break;

		case GL_UNPACK_SWAP_BYTES:
			c->unpack.swapBytes = (GLboolean) param;
			DIRTY(cb->unpack, g->neg_bitid);
			break;
		case GL_UNPACK_LSB_FIRST:
			c->unpack.psLSBFirst = (GLboolean) param;
			DIRTY(cb->unpack, g->neg_bitid);
			break;
		case GL_UNPACK_ROW_LENGTH:
			if (param < 0.0f) 
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative Row Length: %f", param);
				return;
			}
			c->unpack.rowLength = param;
			DIRTY(cb->unpack, g->neg_bitid);
			break;
#ifdef CR_OPENGL_VERSION_1_2
		case GL_UNPACK_IMAGE_HEIGHT:
			if (param < 0.0f) 
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative Image Height: %f", param);
				return;
			}
			c->unpack.imageHeight = param;
			DIRTY(cb->unpack, g->neg_bitid);
			break;
#endif
		case GL_UNPACK_SKIP_PIXELS:
			if (param < 0.0f) 
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative Skip Pixels: %f", param);
				return;
			}
			c->unpack.skipPixels = param;
			DIRTY(cb->unpack, g->neg_bitid);
			break;
		case GL_UNPACK_SKIP_ROWS:
			if (param < 0.0f) 
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative Row Skip: %f", param);
				return;
			}
			c->unpack.skipRows = param;
			DIRTY(cb->unpack, g->neg_bitid);
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
			c->unpack.alignment = param;
			DIRTY(cb->unpack, g->neg_bitid);
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Unknown glPixelStore Pname: %d", pname);
			return;
	}
	DIRTY(cb->dirty, g->neg_bitid);
}


static void setClientState(CRClientState *c, CRClientBits *cb, 
		GLbitvalue *neg_bitid, GLenum array, GLboolean state) 
{
	CRContext *g = GetCurrentContext();

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
			c->t[c->curClientTextureUnit].enabled = state;
			break;
		case GL_EDGE_FLAG_ARRAY:
			c->e.enabled = state;
			break;	
#ifdef CR_EXT_secondary_color
		case GL_SECONDARY_COLOR_ARRAY_EXT:
			if( g->extensions.EXT_secondary_color ){
				c->s.enabled = state;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "Invalid Enum passed to Enable/Disable Client State: SECONDARY_COLOR_ARRAY_EXT - EXT_secondary_color is not enabled." );
				return;
			}
			break;
#endif
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "Invalid Enum passed to Enable/Disable Client State: 0x%x", array );
			return;
	}
	DIRTY(cb->dirty, neg_bitid);
	DIRTY(cb->enableClientState, neg_bitid);
}

void STATE_APIENTRY crStateEnableClientState (GLenum array) 
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRClientBits *cb = &(sb->client);

	FLUSH();

	setClientState(c, cb, g->neg_bitid, array, GL_TRUE);
}

void STATE_APIENTRY crStateDisableClientState (GLenum array) 
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRClientBits *cb = &(sb->client);

	FLUSH();

	setClientState(c, cb, g->neg_bitid, array, GL_FALSE);
}

void crStateClientSetPointer (CRClientPointer *cp, GLint size, 
		GLenum type, GLsizei stride, const GLvoid *pointer) 
{
	cp->p = (unsigned char *) pointer;
	cp->size = size;
	cp->type = type;
	/* Calculate the bytes per index for address calculation */
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
			return;
	}

	/* 
	 **  Note: If stride==0 then we set the 
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

	FLUSH();

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
	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->clientPointer, g->neg_bitid);
}

void STATE_APIENTRY crStateColorPointer(GLint size, GLenum type, 
		GLsizei stride, const GLvoid *p) 
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRClientBits *cb = &(sb->client);

	FLUSH();

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
	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->clientPointer, g->neg_bitid);
}

void STATE_APIENTRY crStateSecondaryColorPointerEXT(GLint size,
		GLenum type, GLsizei stride, const GLvoid *p)
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRClientBits *cb = &(sb->client);

	FLUSH();

	if ( !g->extensions.EXT_secondary_color )
	{
		crError( "glSecondaryColorPointerEXT called but EXT_secondary_color is disabled." );
		return;
	}

	if (size != 3)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glSecondaryColorPointerEXT: invalid size: %d", size);
		return;
	}
	if (type != GL_BYTE && type != GL_UNSIGNED_BYTE &&
			type != GL_SHORT && type != GL_UNSIGNED_SHORT &&
			type != GL_INT && type != GL_UNSIGNED_INT &&
			type != GL_FLOAT && type != GL_DOUBLE)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glSecondaryColorPointerEXT: invalid type: %d", type);
		return;
	}
	if (stride < 0) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glSecondaryColorPointerEXT: stride was negative: %d", stride);
		return;
	}

	crStateClientSetPointer(&(c->s), size, type, stride, p);
	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->clientPointer, g->neg_bitid);
}

void STATE_APIENTRY crStateIndexPointer(GLenum type, GLsizei stride, 
		const GLvoid *p) 
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRClientBits *cb = &(sb->client);

	FLUSH();

	if (type != GL_SHORT && type != GL_INT && type != GL_UNSIGNED_BYTE &&
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
	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->clientPointer, g->neg_bitid);
}

void STATE_APIENTRY crStateNormalPointer(GLenum type, GLsizei stride, 
		const GLvoid *p) 
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRClientBits *cb = &(sb->client);

	FLUSH();

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
	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->clientPointer, g->neg_bitid);
}

void STATE_APIENTRY crStateTexCoordPointer(GLint size, GLenum type, 
		GLsizei stride, const GLvoid *p) 
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRClientBits *cb = &(sb->client);

	FLUSH();

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

	crStateClientSetPointer(&(c->t[c->curClientTextureUnit]), size, type, stride, p);
	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->clientPointer, g->neg_bitid);
}

void STATE_APIENTRY crStateEdgeFlagPointer(GLsizei stride, const GLvoid *p) 
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRClientBits *cb = &(sb->client);

	FLUSH();

	if (stride < 0) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexCoordPointer: stride was negative: %d", stride);
		return;
	}

	crStateClientSetPointer(&(c->e), 1, GL_UNSIGNED_BYTE, stride, p);
	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->clientPointer, g->neg_bitid);
}

void STATE_APIENTRY crStateFogCoordPointerEXT(GLenum type, GLsizei stride, const GLvoid *p) 
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRClientBits *cb = &(sb->client);

	FLUSH();

	if (type != GL_BYTE && type != GL_UNSIGNED_BYTE &&
			type != GL_SHORT && type != GL_UNSIGNED_SHORT &&
			type != GL_INT && type != GL_UNSIGNED_INT &&
			type != GL_FLOAT && type != GL_DOUBLE)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glFogCoordPointerEXT: invalid type: %d", type);
		return;
	}
	if (stride < 0) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glFogCoordPointerEXT: stride was negative: %d", stride);
		return;
	}

	crStateClientSetPointer(&(c->f), 1, type, stride, p);
	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->clientPointer, g->neg_bitid);
}

void STATE_APIENTRY crStateVertexAttribPointerNV(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *p) 
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRClientBits *cb = &(sb->client);

	FLUSH();

	if (index > CR_MAX_VERTEX_ATTRIBS)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glVertexAttribPointerNV: invalid index: %d", (int) index);
		return;
	}
	if (size != 1 && size != 2 && size != 3 && size != 4)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glVertexAttribPointerNV: invalid size: %d", size);
		return;
	}
	if (type != GL_SHORT && type != GL_UNSIGNED_BYTE &&
			type != GL_FLOAT && type != GL_DOUBLE)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glVertexAttribPointerNV: invalid type: %d", type);
		return;
	}
	if (stride < 0) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glVertexAttribPointerNV: stride was negative: %d", stride);
		return;
	}

	crStateClientSetPointer(&(c->a[index]), size, type, stride, p);
	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->clientPointer, g->neg_bitid);
}



/* 
** Currently I treat Interleaved Arrays as if the 
** user uses them as separate arrays.
** Certainly not the most efficient method but it 
** lets me use the same glDrawArrays method.
*/
void STATE_APIENTRY crStateInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *p) 
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRClientBits *cb = &(sb->client);
	CRClientPointer *cp;
	unsigned char *base = (unsigned char *) p;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glInterleavedArrays called in begin/end");
		return;
	}

	FLUSH();

	if (stride < 0)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glInterleavedArrays: stride < 0: %d", stride);
		return;
	}

	switch (format) 
	{
		case GL_T4F_C4F_N3F_V4F:
		case GL_T2F_C4F_N3F_V3F:
		case GL_C4F_N3F_V3F:
		case GL_T4F_V4F:
		case GL_T2F_C3F_V3F:
		case GL_T2F_N3F_V3F:
		case GL_C3F_V3F:
		case GL_N3F_V3F:
		case GL_T2F_C4UB_V3F:
		case GL_T2F_V3F:
		case GL_C4UB_V3F:
		case GL_V3F:
		case GL_C4UB_V2F:
		case GL_V2F:
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glInterleavedArrays: Unrecognized format: %d", format);
			return;
	}

	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->clientPointer, g->neg_bitid);

/* p, size, type, stride, enabled, bytesPerIndex */
/*
**  VertexPointer 
*/
	
	cp = &(c->v);
	cp->type = GL_FLOAT;
	cp->enabled = GL_TRUE;

	switch (format) 
	{
		case GL_T4F_C4F_N3F_V4F:
			cp->p = base+4*sizeof(GLfloat)+4*sizeof(GLfloat)+3*sizeof(GLfloat);
			cp->size = 4;
			break;
		case GL_T2F_C4F_N3F_V3F:
			cp->p = base+2*sizeof(GLfloat)+4*sizeof(GLfloat)+3*sizeof(GLfloat);
			cp->size = 3;
			break;
		case GL_C4F_N3F_V3F:
			cp->p = base+4*sizeof(GLfloat)+3*sizeof(GLfloat);
			cp->size = 3;
			break;
		case GL_T4F_V4F:
			cp->p = base+4*sizeof(GLfloat);
			cp->size = 4;
			break;
		case GL_T2F_C3F_V3F:
			cp->p = base+2*sizeof(GLfloat)+3*sizeof(GLfloat);
			cp->size = 3;
			break;
		case GL_T2F_N3F_V3F:
			cp->p = base+2*sizeof(GLfloat)+3*sizeof(GLfloat);
			cp->size = 3;
			break;
		case GL_C3F_V3F:
			cp->p = base+3*sizeof(GLfloat);
			cp->size = 3;
			break;
		case GL_N3F_V3F:
			cp->p = base+3*sizeof(GLfloat);
			cp->size = 3;
			break;
		case GL_T2F_C4UB_V3F:
			cp->p = base+2*sizeof(GLfloat)+4*sizeof(GLubyte);
			cp->size = 3;
			break;
		case GL_T2F_V3F:
			cp->p = base+2*sizeof(GLfloat);
			cp->size = 3;
			break;
		case GL_C4UB_V3F:
			cp->p = base+4*sizeof(GLubyte);
			cp->size = 3;
			break;
		case GL_V3F:
			cp->p = base;
			cp->size = 3;
			break;
		case GL_C4UB_V2F:
			cp->p = base+4*sizeof(GLubyte);
			cp->size = 2;
			break;
		case GL_V2F:
			cp->p = base;
			cp->size = 2;
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glInterleavedArrays: Unrecognized format: %d", format);
			return;
	}

	cp->bytesPerIndex = cp->size * sizeof (GLfloat);

	if (stride)
		cp->stride = stride + (cp->p - base);
	else
		cp->stride = cp->bytesPerIndex + (cp->p - base);

/*
**  NormalPointer
*/

	cp = &(c->n);
	cp->enabled = GL_TRUE;
	switch (format) 
	{
		case GL_T4F_C4F_N3F_V4F:
			cp->p = base+4*sizeof(GLfloat)+4*sizeof(GLfloat);
			cp->stride = 4*sizeof(GLfloat)+4*sizeof(GLfloat)+stride;
			if (!stride) cp->stride += 3*sizeof(GLfloat)+4*sizeof(GLfloat);
			break;
		case GL_T2F_C4F_N3F_V3F:
			cp->p = base+2*sizeof(GLfloat)+4*sizeof(GLfloat);
			cp->stride = 2*sizeof(GLfloat)+4*sizeof(GLfloat)+stride;
			if (!stride) cp->stride += 3*sizeof(GLfloat)+3*sizeof(GLfloat);
			break;
		case GL_C4F_N3F_V3F:
			cp->p = base+4*sizeof(GLfloat);
			cp->stride = 4*sizeof(GLfloat)+stride;
			if (!stride) cp->stride += 3*sizeof(GLfloat)+3*sizeof(GLfloat);
			break;
		case GL_T2F_N3F_V3F:
			cp->p = base+2*sizeof(GLfloat);
			cp->stride = 2*sizeof(GLfloat)+stride;
			if (!stride) cp->stride += 3*sizeof(GLfloat)+3*sizeof(GLfloat);
			break;
		case GL_N3F_V3F:
			cp->p = base;
			cp->stride = stride;
			if (!stride) cp->stride += 3*sizeof(GLfloat)+3*sizeof(GLfloat);
			break;
		case GL_T4F_V4F:
		case GL_T2F_C3F_V3F:
		case GL_C3F_V3F:
		case GL_T2F_C4UB_V3F:
		case GL_T2F_V3F:
		case GL_C4UB_V3F:
		case GL_V3F:
		case GL_C4UB_V2F:
		case GL_V2F:
			cp->enabled = GL_FALSE;
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glInterleavedArrays: Unrecognized format: %d", format);
			return;
	}

	if (cp->enabled) 
	{
		cp->type = GL_FLOAT;
		cp->size = 3;
		cp->bytesPerIndex = cp->size * sizeof (GLfloat);
	}
	
/*
**  ColorPointer
*/

	cp = &(c->c);
	cp->enabled = GL_TRUE;
	switch (format) 
	{
		case GL_T4F_C4F_N3F_V4F:
			cp->size = 4;
			cp->type = GL_FLOAT;
			cp->bytesPerIndex = cp->size * sizeof(GLfloat);
			cp->p = base+4*sizeof(GLfloat);
			cp->stride = 4*sizeof(GLfloat)+stride;
			if (!stride) cp->stride += 4*sizeof(GLfloat)+3*sizeof(GLfloat)+4*sizeof(GLfloat);
			break;
		case GL_T2F_C4F_N3F_V3F:
			cp->size = 4;
			cp->type = GL_FLOAT;
			cp->bytesPerIndex = cp->size * sizeof(GLfloat);
			cp->p = base+2*sizeof(GLfloat);
			cp->stride = 2*sizeof(GLfloat)+stride;
			if (!stride) cp->stride += 4*sizeof(GLfloat)+3*sizeof(GLfloat)+3*sizeof(GLfloat);
			break;
		case GL_C4F_N3F_V3F:
			cp->size = 4;
			cp->type = GL_FLOAT;
			cp->bytesPerIndex = cp->size * sizeof(GLfloat);
			cp->p = base;
			cp->stride = stride;
			if (!stride) cp->stride += 4*sizeof(GLfloat)+3*sizeof(GLfloat)+3*sizeof(GLfloat);
			break;
		case GL_T2F_C3F_V3F:
			cp->size = 3;
			cp->type = GL_FLOAT;
			cp->bytesPerIndex = cp->size * sizeof(GLfloat);
			cp->p = base+2*sizeof(GLfloat);
			cp->stride = 2*sizeof(GLfloat)+stride;
			if (!stride) cp->stride += 3*sizeof(GLfloat)+3*sizeof(GLfloat);
			break;
		case GL_C3F_V3F:
			cp->size = 3;
			cp->type = GL_FLOAT;
			cp->bytesPerIndex = cp->size * sizeof(GLfloat);
			cp->p = base;
			cp->stride = stride;
			if (!stride) cp->stride += 3*sizeof(GLfloat) + 3*sizeof(GLfloat);
			break;
		case GL_T2F_C4UB_V3F:
			cp->size = 4;
			cp->type = GL_UNSIGNED_BYTE;
			cp->bytesPerIndex = cp->size * sizeof(GLubyte);
			cp->p = base+2*sizeof(GLfloat);
			cp->stride = 2*sizeof(GLfloat)+stride;
			if (!stride) cp->stride += 4*sizeof(GLubyte)+2*sizeof(GLfloat);
			break;
		case GL_C4UB_V3F:
			cp->size = 4;
			cp->type = GL_UNSIGNED_BYTE;
			cp->bytesPerIndex = cp->size * sizeof(GLubyte);
			cp->p = base;
			cp->stride = stride;
			if (!stride) cp->stride += 4*sizeof(GLubyte)+3*sizeof(GLfloat);
			break;
		case GL_C4UB_V2F:
			cp->size = 4;
			cp->type = GL_UNSIGNED_BYTE;
			cp->bytesPerIndex = cp->size * sizeof(GLubyte);
			cp->p = base;
			cp->stride = stride;
			if (!stride) cp->stride += 4*sizeof(GLubyte)+2*sizeof(GLfloat);
			break;
		case GL_T2F_N3F_V3F:
		case GL_N3F_V3F:
		case GL_T4F_V4F:
		case GL_T2F_V3F:
		case GL_V3F:
		case GL_V2F:
			cp->enabled = GL_FALSE;
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glInterleavedArrays: Unrecognized format: %d", format);
			return;
	}

/*
**  TexturePointer
*/

	cp = &(c->t[c->curClientTextureUnit]);
	cp->enabled = GL_TRUE;
	switch (format) 
	{
		case GL_T4F_C4F_N3F_V4F:
			cp->size = 4;
			cp->p = base;
			cp->stride = stride;
			if (!stride) cp->stride = 4*sizeof(GLfloat)+4*sizeof(GLfloat)+3*sizeof(GLfloat)+4*sizeof(GLfloat);
			break;
		case GL_T2F_C4F_N3F_V3F:
			cp->size = 3;
			cp->p = base;
			cp->stride = stride;
			if (!stride) cp->stride = 2*sizeof(GLfloat)+4*sizeof(GLfloat)+3*sizeof(GLfloat)+3*sizeof(GLfloat);
			break;
		case GL_T2F_C3F_V3F:
		case GL_T2F_N3F_V3F:
			cp->size = 3;
			cp->p = base;
			cp->stride = stride;
			if (!stride) cp->stride = 2*sizeof(GLfloat)+3*sizeof(GLfloat)+3*sizeof(GLfloat);
			break;
		case GL_T2F_C4UB_V3F:
			cp->size = 3;
			cp->p = base;
			cp->stride = stride;
			if (!stride) cp->stride = 2*sizeof(GLfloat)+4*sizeof(GLubyte)+3*sizeof(GLfloat);
			break;
		case GL_T4F_V4F:
			cp->size = 4;
			cp->p = base;
			cp->stride = stride;
			if (!stride) cp->stride = 4*sizeof(GLfloat)+4*sizeof(GLfloat);
			break;
		case GL_T2F_V3F:
			cp->size = 3;
			cp->p = base;
			cp->stride = stride;
			if (!stride) cp->stride = 2*sizeof(GLfloat)+3*sizeof(GLfloat);
			break;
		case GL_C4UB_V3F:
		case GL_C4UB_V2F:
		case GL_C3F_V3F:
		case GL_C4F_N3F_V3F:
		case GL_N3F_V3F:
		case GL_V3F:
		case GL_V2F:
			cp->enabled = GL_FALSE;
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glInterleavedArrays: Unrecognized format: %d", format);
			return;
	}

	if (cp->enabled) 
	{
		cp->type = GL_FLOAT;
		cp->bytesPerIndex = cp->size * sizeof (GLfloat);
	}	
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
		case GL_VERTEX_ARRAY_POINTER:
			*params = (GLvoid *) c->v.p;
			break;
		case GL_COLOR_ARRAY_POINTER:
			*params = (GLvoid *) c->c.p;
			break;
		case GL_NORMAL_ARRAY_POINTER:
			*params = (GLvoid *) c->n.p;
			break;
		case GL_INDEX_ARRAY_POINTER:
			*params = (GLvoid *) c->i.p;
			break;
		case GL_TEXTURE_COORD_ARRAY_POINTER:
			*params = (GLvoid *) c->t[c->curClientTextureUnit].p;
			break;
		case GL_EDGE_FLAG_ARRAY_POINTER:
			*params = (GLvoid *) c->e.p;
			break;
#ifdef CR_EXT_fog_coord
		case GL_FOG_COORDINATE_ARRAY_POINTER_EXT:
			*params = (GLvoid *) c->f.p;
			break;
#endif
#ifdef CR_EXT_secondary_color
		case GL_SECONDARY_COLOR_ARRAY_POINTER_EXT:
			if( g->extensions.EXT_secondary_color ){
				*params = (GLvoid *) c->s.p;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "Invalid Enum passed to glGetPointerv: SECONDARY_COLOR_ARRAY_EXT - EXT_secondary_color is not enabled." );
				return;
			}
			break;
#endif
		case GL_FEEDBACK_BUFFER_POINTER:
		case GL_SELECTION_BUFFER_POINTER:
			/* do nothing - API switching should pick this up */
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					"glGetPointerv: invalid pname: %d", pname);
			return;
	}
}
