/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*
 * This file manages all the client-side state including:
 *  Pixel pack/unpack parameters
 *  Vertex arrays
 */


#include <stdio.h>
#include "cr_mem.h"
#include "state.h"
#include "state/cr_statetypes.h"
#include "state/cr_statefuncs.h"
#include "state_internals.h"

#define GLCLIENT_NUMPOINTERS 6
#define GLCLIENT_INDEX_ALLOC 1024
#define GLCLIENT_DATA_ALLOC 1024
#define GLCLIENT_CACHE_ALLOC 1024
#define GLCLIENT_LIST_ALLOC 1024
#define GLCLIENT_BIT_ALLOC 1024

void crStateClientInitBits (CRClientBits *c) 
{
	int i;

	c->v = (CRbitvalue *) crCalloc(GLCLIENT_BIT_ALLOC*sizeof(CRbitvalue));
	c->n = (CRbitvalue *) crCalloc(GLCLIENT_BIT_ALLOC*sizeof(CRbitvalue));
	c->c = (CRbitvalue *) crCalloc(GLCLIENT_BIT_ALLOC*sizeof(CRbitvalue));
	c->s = (CRbitvalue *) crCalloc(GLCLIENT_BIT_ALLOC*sizeof(CRbitvalue));
	c->i = (CRbitvalue *) crCalloc(GLCLIENT_BIT_ALLOC*sizeof(CRbitvalue));
	c->t = (CRbitvalue *) crCalloc(GLCLIENT_BIT_ALLOC*sizeof(CRbitvalue));
	c->e = (CRbitvalue *) crCalloc(GLCLIENT_BIT_ALLOC*sizeof(CRbitvalue));

#ifdef CR_NV_vertex_program
	for ( i = 0; i < CR_MAX_VERTEX_ATTRIBS; i++ )
		c->a[i] = (CRbitvalue *) crCalloc(GLCLIENT_BIT_ALLOC*sizeof(CRbitvalue));
#endif

	c->valloc = GLCLIENT_BIT_ALLOC;
	c->nalloc = GLCLIENT_BIT_ALLOC;
	c->calloc = GLCLIENT_BIT_ALLOC;
	c->salloc = GLCLIENT_BIT_ALLOC;
	c->ialloc = GLCLIENT_BIT_ALLOC;
	c->talloc = GLCLIENT_BIT_ALLOC;
	c->ealloc = GLCLIENT_BIT_ALLOC;
	c->aalloc = GLCLIENT_BIT_ALLOC;
}

void crStateClientDestroy(CRClientState *c)
{
	crFree( c->list );
}

void crStateClientInit(CRClientState *c) 
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
	c->array.v.p = NULL;
	c->array.v.size = 4;
	c->array.v.type = GL_FLOAT;
	c->array.v.stride = 0;
	c->array.v.enabled = 0;
	c->array.c.p = NULL;
	c->array.c.size = 4;
	c->array.c.type = GL_FLOAT;
	c->array.c.stride = 0;
	c->array.c.enabled = 0;
	c->array.f.p = NULL;
	c->array.f.size = 0;
	c->array.f.type = GL_FLOAT;
	c->array.f.stride = 0;
	c->array.f.enabled = 0;
	c->array.s.p = NULL;
	c->array.s.size = 3;
	c->array.s.type = GL_FLOAT;
	c->array.s.stride = 0;
	c->array.s.enabled = 0;
	c->array.e.p = NULL;
	c->array.e.size = 0;
	c->array.e.type = GL_FLOAT;
	c->array.e.stride = 0;
	c->array.e.enabled = 0;
	c->array.i.p = NULL;
	c->array.i.size = 0;
	c->array.i.type = GL_FLOAT;
	c->array.i.stride = 0;
	c->array.i.enabled = 0;
	c->array.n.p = NULL;
	c->array.n.size = 4;
	c->array.n.type = GL_FLOAT;
	c->array.n.stride = 0;
	c->array.n.enabled = 0;
	for (i = 0 ; i < CR_MAX_TEXTURE_UNITS ; i++)
	{
		c->array.t[i].p = NULL;
		c->array.t[i].size = 4;
		c->array.t[i].type = GL_FLOAT;
		c->array.t[i].stride = 0;
		c->array.t[i].enabled = 0;
	}
#ifdef CR_NV_vertex_program
	for (i = 0; i < CR_MAX_VERTEX_ATTRIBS; i++) {
		c->array.a[i].enabled = GL_FALSE;
		c->array.a[i].type = 0;
		c->array.a[i].size = 0;
		c->array.a[i].stride = 0;
	}
#endif
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
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
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
		CRbitvalue *neg_bitid, GLenum array, GLboolean state) 
{
	CRContext *g = GetCurrentContext();

	switch (array) 
	{
#ifdef CR_NV_vertex_program
		case GL_VERTEX_ATTRIB_ARRAY0_NV:
		case GL_VERTEX_ATTRIB_ARRAY1_NV:
		case GL_VERTEX_ATTRIB_ARRAY2_NV:
		case GL_VERTEX_ATTRIB_ARRAY3_NV:
		case GL_VERTEX_ATTRIB_ARRAY4_NV:
		case GL_VERTEX_ATTRIB_ARRAY5_NV:
		case GL_VERTEX_ATTRIB_ARRAY6_NV:
		case GL_VERTEX_ATTRIB_ARRAY7_NV:
		case GL_VERTEX_ATTRIB_ARRAY8_NV:
		case GL_VERTEX_ATTRIB_ARRAY9_NV:
		case GL_VERTEX_ATTRIB_ARRAY10_NV:
		case GL_VERTEX_ATTRIB_ARRAY11_NV:
		case GL_VERTEX_ATTRIB_ARRAY12_NV:
		case GL_VERTEX_ATTRIB_ARRAY13_NV:
		case GL_VERTEX_ATTRIB_ARRAY14_NV:
		case GL_VERTEX_ATTRIB_ARRAY15_NV:
			{
				const GLuint i = array - GL_VERTEX_ATTRIB_ARRAY0_NV;
				c->array.a[i].enabled = state;
			}
			break;
#endif
		case GL_VERTEX_ARRAY:
			c->array.v.enabled = state;
			break;
		case GL_COLOR_ARRAY:
			c->array.c.enabled = state;
			break;
		case GL_NORMAL_ARRAY:
			c->array.n.enabled = state;
			break;
		case GL_INDEX_ARRAY:
			c->array.i.enabled = state;
			break;
		case GL_TEXTURE_COORD_ARRAY:
			c->array.t[c->curClientTextureUnit].enabled = state;
			break;
		case GL_EDGE_FLAG_ARRAY:
			c->array.e.enabled = state;
			break;	
#ifdef CR_EXT_secondary_color
		case GL_SECONDARY_COLOR_ARRAY_EXT:
			if( g->extensions.EXT_secondary_color ){
				c->array.s.enabled = state;
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

static void crStateClientSetPointer (CRClientPointer *cp, GLint size, 
		GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer) 
{
	cp->p = (unsigned char *) pointer;
	cp->size = size;
	cp->type = type;
	cp->normalized = normalized;
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
	if (stride)
		cp->stride = stride;
	else
		cp->stride = cp->bytesPerIndex;
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
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glVertexPointer: invalid type: 0x%x", type);
		return;
	}
	if (stride < 0) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glVertexPointer: stride was negative: %d", stride);
		return;
	}

	crStateClientSetPointer(&(c->array.v), size, type, GL_FALSE, stride, p);
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
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glColorPointer: invalid type: 0x%x", type);
		return;
	}
	if (stride < 0) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glColorPointer: stride was negative: %d", stride);
		return;
	}

	crStateClientSetPointer(&(c->array.c), size, type, GL_TRUE, stride, p);
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
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glSecondaryColorPointerEXT: invalid type: 0x%x", type);
		return;
	}
	if (stride < 0) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glSecondaryColorPointerEXT: stride was negative: %d", stride);
		return;
	}

	crStateClientSetPointer(&(c->array.s), size, type, GL_TRUE, stride, p);
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
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glIndexPointer: invalid type: 0x%x", type);
		return;
	}
	if (stride < 0) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glIndexPointer: stride was negative: %d", stride);
		return;
	}

	crStateClientSetPointer(&(c->array.i), 1, type, GL_TRUE, stride, p);
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
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glNormalPointer: invalid type: 0x%x", type);
		return;
	}
	if (stride < 0) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glNormalPointer: stride was negative: %d", stride);
		return;
	}

	crStateClientSetPointer(&(c->array.n), 3, type, GL_TRUE, stride, p);
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
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glTexCoordPointer: invalid type: 0x%x", type);
		return;
	}
	if (stride < 0) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexCoordPointer: stride was negative: %d", stride);
		return;
	}

	crStateClientSetPointer(&(c->array.t[c->curClientTextureUnit]), size, type, GL_FALSE, stride, p);
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

	crStateClientSetPointer(&(c->array.e), 1, GL_UNSIGNED_BYTE, GL_FALSE, stride, p);
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
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glFogCoordPointerEXT: invalid type: 0x%x", type);
		return;
	}
	if (stride < 0) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glFogCoordPointerEXT: stride was negative: %d", stride);
		return;
	}

	crStateClientSetPointer(&(c->array.f), 1, type, GL_FALSE, stride, p);
	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->clientPointer, g->neg_bitid);
}


void STATE_APIENTRY crStateVertexAttribPointerNV(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *p) 
{
	GLboolean normalized = GL_FALSE;
	/* Extra error checking for NV arrays */
	if (type != GL_UNSIGNED_BYTE && type != GL_SHORT &&
			type != GL_FLOAT && type != GL_DOUBLE) {
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								 "glVertexAttribPointerNV: invalid type: 0x%x", type);
		return;
	}
	crStateVertexAttribPointerARB(index, size, type, normalized, stride, p);
}


void STATE_APIENTRY crStateVertexAttribPointerARB(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *p) 
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRClientBits *cb = &(sb->client);

	FLUSH();

	if (index > CR_MAX_VERTEX_ATTRIBS)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glVertexAttribPointerARB: invalid index: %d", (int) index);
		return;
	}
	if (size != 1 && size != 2 && size != 3 && size != 4)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glVertexAttribPointerARB: invalid size: %d", size);
		return;
	}
	if (type != GL_BYTE && type != GL_UNSIGNED_BYTE &&
			type != GL_SHORT && type != GL_UNSIGNED_SHORT &&
			type != GL_INT && type != GL_UNSIGNED_INT &&
			type != GL_FLOAT && type != GL_DOUBLE)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glVertexAttribPointerARB: invalid type: 0x%x", type);
		return;
	}
	if (stride < 0) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glVertexAttribPointerARB: stride was negative: %d", stride);
		return;
	}

	crStateClientSetPointer(&(c->array.a[index]), size, type, normalized, stride, p);
	DIRTY(cb->dirty, g->neg_bitid);
	DIRTY(cb->clientPointer, g->neg_bitid);
}


void STATE_APIENTRY crStateGetVertexAttribPointervNV(GLuint index, GLenum pname, GLvoid **pointer)
{
	CRContext *g = GetCurrentContext();

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glGetVertexAttribPointervNV called in Begin/End");
		return;
	}

	if (index >= CR_MAX_VERTEX_ATTRIBS) {
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
								 "glGetVertexAttribPointervNV(index)");
		return;
	}

	if (pname != GL_ATTRIB_ARRAY_POINTER_NV) {
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								 "glGetVertexAttribPointervNV(pname)");
		return;
	}

	*pointer = g->client.array.a[index].p;
}


void STATE_APIENTRY crStateGetVertexAttribPointervARB(GLuint index, GLenum pname, GLvoid **pointer)
{
		crStateGetVertexAttribPointervNV(index, pname, pointer);
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
	
	cp = &(c->array.v);
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

	cp = &(c->array.n);
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

	cp = &(c->array.c);
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

	cp = &(c->array.t[c->curClientTextureUnit]);
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
			*params = (GLvoid *) c->array.v.p;
			break;
		case GL_COLOR_ARRAY_POINTER:
			*params = (GLvoid *) c->array.c.p;
			break;
		case GL_NORMAL_ARRAY_POINTER:
			*params = (GLvoid *) c->array.n.p;
			break;
		case GL_INDEX_ARRAY_POINTER:
			*params = (GLvoid *) c->array.i.p;
			break;
		case GL_TEXTURE_COORD_ARRAY_POINTER:
			*params = (GLvoid *) c->array.t[c->curClientTextureUnit].p;
			break;
		case GL_EDGE_FLAG_ARRAY_POINTER:
			*params = (GLvoid *) c->array.e.p;
			break;
#ifdef CR_EXT_fog_coord
		case GL_FOG_COORDINATE_ARRAY_POINTER_EXT:
			*params = (GLvoid *) c->array.f.p;
			break;
#endif
#ifdef CR_EXT_secondary_color
		case GL_SECONDARY_COLOR_ARRAY_POINTER_EXT:
			if( g->extensions.EXT_secondary_color ){
				*params = (GLvoid *) c->array.s.p;
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


void STATE_APIENTRY crStatePushClientAttrib( GLbitfield mask )
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glPushClientAttrib called in Begin/End");
		return;
	}

	if (c->attribStackDepth == CR_MAX_CLIENT_ATTRIB_STACK_DEPTH - 1) {
		crStateError(__LINE__, __FILE__, GL_STACK_OVERFLOW,
								 "glPushClientAttrib called with a full stack!" );
		return;
	}

	FLUSH();

	c->pushMaskStack[c->attribStackDepth++] = mask;

	if (mask & GL_CLIENT_PIXEL_STORE_BIT) {
		c->pixelPackStoreStack[c->pixelStoreStackDepth] = c->pack;
		c->pixelUnpackStoreStack[c->pixelStoreStackDepth] = c->unpack;
		c->pixelStoreStackDepth++;
	}
	if (mask & GL_CLIENT_VERTEX_ARRAY_BIT) {
		c->vertexArrayStack[c->vertexArrayStackDepth] = c->array;
		c->vertexArrayStackDepth++;
	}

	/* dirty? - no, because we haven't really changed any state */
}


void STATE_APIENTRY crStatePopClientAttrib( void )
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRClientBits *cb = &(sb->client);
	CRbitvalue mask;

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glPopClientAttrib called in Begin/End");
		return;
	}

	if (c->attribStackDepth == 0) {
		crStateError(__LINE__, __FILE__, GL_STACK_UNDERFLOW,
								 "glPopClientAttrib called with an empty stack!" );
		return;
	}

	FLUSH();

	mask = c->pushMaskStack[--c->attribStackDepth];

	if (mask & GL_CLIENT_PIXEL_STORE_BIT) {
		if (c->pixelStoreStackDepth == 0) {
			crError("bug in glPopClientAttrib (pixel store) ");
			return;
		}
		c->pixelStoreStackDepth--;
		c->pack = c->pixelPackStoreStack[c->pixelStoreStackDepth];
		c->unpack = c->pixelUnpackStoreStack[c->pixelStoreStackDepth];
		DIRTY(cb->pack, g->neg_bitid);
	}

	if (mask & GL_CLIENT_VERTEX_ARRAY_BIT) {
		if (c->vertexArrayStackDepth == 0) {
			crError("bug in glPopClientAttrib (vertex array) ");
			return;
		}
		c->vertexArrayStackDepth--;
		c->array = c->vertexArrayStack[c->vertexArrayStackDepth];
		DIRTY(cb->clientPointer, g->neg_bitid);
	}

	DIRTY(cb->dirty, g->neg_bitid);
}
