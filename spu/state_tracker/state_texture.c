#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include "cr_glstate.h"
#include "state/cr_statetypes.h"
#include "cr_pixeldata.h"
#include "cr_mem.h"
#include "state_internals.h"
#include "state_extensionfuncs.h"

#define UNIMPLEMENTED() crStateError(__LINE__,__FILE__,GL_INVALID_OPERATION, "Unimplemented something or other" )
#define UNUSED(x) ((void) (x))

#define GET_TOBJ(tobj,id) 	for (tobj = t->mapping[id%CRTEXTURE_HASHSIZE]; tobj && tobj->name != id; tobj = tobj->next){}
#define GET_HWID(hwid,id) 	for (hwid = t->hwidhash[id%CRTEXTURE_HASHSIZE]; hwid && hwid->name != id; hwid = hwid->next){}

void crStateTextureInitTextureObj (CRTextureState *t, CRTextureObj *tobj, GLuint name);
CRTextureObj *crStateTextureAllocate_t(CRTextureState *t, GLuint name);
void crStateTextureDelete_t(CRTextureState *t, GLuint name);

CRTextureObj *tex_broadcast = NULL;

#if 0
void
__gltexture_broadcast (CRContext *g, CRTextureObj *tobj) {
	GLbitvalue update = g->update;
	g->update = GLUPDATE_TEXTURE;
	tex_broadcast = tobj;
	__glbucket_syncstate ();
	tex_broadcast = NULL;
	g->update = update;
}
#endif

void crStateTextureInit(CRTextureState *t) 
{
	int i;
	unsigned int a;
	GLvectorf zero_vector = {0.0f, 0.0f, 0.0f, 0.0f};
	GLcolorf zero_color = {0.0f, 0.0f, 0.0f, 0.0f};
	GLvectorf x_vector = {1.0f, 0.0f, 0.0f, 0.0f};
	GLvectorf y_vector = {0.0f, 1.0f, 0.0f, 0.0f};

#if 0
	t->maxTextureSize = c->maxTextureSize;
	t->max3Dtexturesize = c->max3Dtexturesize;
#endif
	t->maxTextureSize = 2048;
	t->max3DTextureSize = 2048;

	for (i=0, a=t->maxTextureSize; a; i++, a=a>>1);
	t->maxLevel = i;
	for (i=0, a=t->max3DTextureSize; a; i++, a=a>>1);
	t->max3DLevel = i;

	t->allocated = 1;

	t->textures = (CRTextureObj *) crAlloc (sizeof (CRTextureObj));
	crStateTextureInitTextureObj(t, t->textures, 0);
	t->firstFree = t->textures;
	t->firstFree->next = NULL;

	/* Initalize the hash table to empty */
	for (i=0; i<CRTEXTURE_HASHSIZE; i++)
		t->mapping[i] = NULL;

	for (i=0; i<CRTEXTURE_HASHSIZE; i++)
		t->hwidhash[i] = NULL;

	t->freeList = (CRTextureFreeElem *) crAlloc (sizeof(CRTextureFreeElem));
	t->freeList->min = 1;
	t->freeList->max = GL_MAXUINT;
	t->freeList->next = NULL;
	t->freeList->prev = NULL;

	t->currentTexture1D = &(t->base1D);
	t->currentTexture2D = &(t->base2D);
	t->currentTexture3D = &(t->base3D);

	t->currentTexture1DName = 0;
	t->currentTexture2DName = 0;
	t->currentTexture3DName = 0;

#if 0
	t->broadcast_textures = (GLboolean) c->broadcast_textures;
#endif

	t->enabled1D = GL_FALSE;
	t->enabled2D = GL_FALSE;
	t->enabled3D = GL_FALSE;
	t->textureGen.s = GL_FALSE;
	t->textureGen.t = GL_FALSE;
	t->textureGen.p = GL_FALSE;
	t->textureGen.q = GL_FALSE;

	t->envMode = GL_MODULATE;
	t->envColor = zero_color;

	crStateTextureInitTextureObj(t, &(t->base1D), 0);
	crStateTextureInitTextureObj(t, &(t->base2D), 0);
	crStateTextureInitTextureObj(t, &(t->base3D), 0);

	t->gen.s = GL_EYE_LINEAR;
	t->gen.t = GL_EYE_LINEAR;
	t->gen.p = GL_EYE_LINEAR;
	t->gen.q = GL_EYE_LINEAR;

	t->objSCoeff = x_vector;
	t->objTCoeff = y_vector;
	t->objRCoeff = zero_vector;
	t->objQCoeff = zero_vector;

	t->eyeSCoeff = x_vector;
	t->eyeTCoeff = y_vector;
	t->eyeRCoeff = zero_vector;
	t->eyeQCoeff = zero_vector;

	crStateTextureInitExtensions( t );
}

void crStateTextureInitTextureObj(CRTextureState *t, CRTextureObj *tobj, GLuint name)
{
	int i;
	CRTextureLevel *tl;

	tobj->borderColor.r = 0.0f;
	tobj->borderColor.g = 0.0f;
	tobj->borderColor.b = 0.0f;
	tobj->borderColor.a = 0.0f;
	tobj->minFilter     = GL_NEAREST_MIPMAP_LINEAR;
	tobj->magFilter     = GL_LINEAR;
	tobj->wrapS         = GL_REPEAT;
	tobj->wrapT         = GL_REPEAT;
	tobj->target        = GL_NONE;
	tobj->name          = name;

	tobj->level = (CRTextureLevel *) crAlloc (sizeof(CRTextureLevel) * t->maxLevel);
	for (i=0; i<t->maxLevel; i++) 
	{
		tl                = &(tobj->level[i]);
		tl->bytes         = 0;
		tl->img           = NULL;
		tl->width         = 0;
		tl->height        = 0;
		tl->depth         = 0;
		tl->border        = 0;
		tl->components    = 1;
		tl->bytesPerPixel = 0;
		tl->format        = GL_RGBA;
		tl->type          = GL_UNSIGNED_BYTE;
		tl->dirty         = 0;  // By default this level is ignored.
	}

	crStateTextureInitTextureObjExtensions( t, tobj );

	/* UGh. Should be neg_bitid */
	tobj->dirty = GLBITS_ONES;
	tobj->paramsBit = GLBITS_ONES;
	tobj->imageBit = GLBITS_ONES;
}

void crStateTextureInitTexture (GLuint name) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRTextureObj *tobj;

	GET_TOBJ(tobj, name);
	if (!tobj) return;

	crStateTextureInitTextureObj(t, tobj, name);
}

CRTextureObj * crStateTextureGet(GLuint name) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRTextureObj *tobj;

	GET_TOBJ(tobj, name);

	return tobj;
}

CRTextureObj * crStateTextureAllocate (GLuint name) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	return crStateTextureAllocate_t(t, name);
}

void __checkFreeList(CRTextureState *t) 
{
	CRTextureObj *tobj;
	CRTextureFreeElem *f;
	GLuint i;

	for (i=1; i < t->freeList->min; i++) 
	{
		GET_TOBJ(tobj, i);
		assert(tobj);
	}		

	for (f = t->freeList; f; f = f->next) 
	{
		if (f->max == GL_MAXUINT) 
		{
			return;
		}
		for (i = f->max+1; i < (f->next?f->next->min:GL_MAXUINT); i++) 
		{
			GET_TOBJ(tobj, i)
				if (t->mapping[i%CRTEXTURE_HASHSIZE]) 
					assert(tobj);
		}
	}
}


CRTextureObj * crStateTextureAllocate_t (CRTextureState *t, GLuint name) 
{
	GLuint i;
	CRTextureObj *tobj;
	CRTextureFreeElem *k;
	CRTextureFreeElem *newelem;

	if (!name) 
	{
		return NULL;
	}

	/* First we use the firstFree */
	if (t->firstFree != NULL) 
	{
		CRTextureObj *j;
		tobj = t->firstFree;

		i = name % CRTEXTURE_HASHSIZE;
		/* Check to see that it isn't already there */
		if (t->mapping[i] != NULL)  
		{
			for (j = t->mapping[i]; j && j->name != name; j=j->next)
			{
				/* EMPTY BODY */
			}
			if (j != NULL) 
			{
				return j; /* Return object that was already allocated. */
			}
		}
		/* Remove tobj from the free list */
		t->firstFree = tobj->next;

		/* Insert it in the hash table */
		tobj->next = t->mapping[i];
		t->mapping[i] = tobj;

		/* Initalize all of its members */
		crStateTextureInitTextureObj(t, tobj, name);

		/* Include name into the free list */
		/* First find which region it fits in */
		for (k=t->freeList; k && !(k->min <= name && name <= k->max) && !(k->min > name); k=k->next)
		{
			/* EMPTY BODY */
		}

		if (k == NULL) 
		{
			UNIMPLEMENTED();
		}

		if (k->min > name)
		{
			UNIMPLEMENTED();
		}

		/* (name, name) */
		if (k->max == name && k->min == name) 
		{
			/*Remove from freeList*/
			if (k==t->freeList) 
			{
				t->freeList = t->freeList->next;
				t->freeList->prev = NULL;
			} 
			else 
			{
				k->prev->next = k->next;
				k->next->prev = k->prev;
			}
			crFree (k);
			return tobj;
		}

		/* (name, ~) */
		if (k->min == name) 
		{
			k->min++;
			return tobj;
		}

		/* (~, name) */
		if (k->max == name) 
		{
			k->max--;
			return tobj;
		}

		/* (<name, >name) change to        */
		/* (<name, name-1) (name+1, >name) */
		newelem = (CRTextureFreeElem *) crAlloc (sizeof(CRTextureFreeElem));
		newelem->min = name+1;
		newelem->max = k->max;
		k->max = name-1;

		newelem->next = k->next;
		newelem->prev = k;
		if (k->next)
		{
			k->next->prev = newelem;
		}
		k->next = newelem;
		return tobj;
	}

	/* No firstFree available, lets allocate some more and try again. */
	tobj = t->textures;
	t->textures = (CRTextureObj *) realloc(tobj, t->allocated*2*sizeof (CRTextureObj));

	if (tobj != t->textures) 
	{
		/* Fix all the pointers */
		for (i=0; i<t->allocated; i++) 
		{
			if (t->textures[i].next)
			{
				t->textures[i].next = t->textures + (t->textures[i].next - tobj);
			}
		}
		for (i=0; i<CRTEXTURE_HASHSIZE; i++)
		{
			if (t->mapping[i])
			{
				t->mapping[i] = t->textures + (t->mapping[i] - tobj);
			}
		}

		if (t->currentTexture1D != &(t->base1D)) 
		{
			t->currentTexture1D = t->textures + (t->currentTexture1D - tobj);
		}
		if (t->currentTexture2D != &(t->base2D)) 
		{
			t->currentTexture2D = t->textures + (t->currentTexture2D - tobj);
		}
		if (t->currentTexture3D != &(t->base3D)) 
		{
			t->currentTexture3D = t->textures + (t->currentTexture3D - tobj);
		}
	}

	/* Update the free list */
	t->firstFree = t->textures+t->allocated;
	for (i=t->allocated; i < t->allocated*2; i++) 
	{
		crStateTextureInitTextureObj(t, t->textures+i, 0);		
		t->textures[i].next = t->textures+i+1;
	}

	t->textures[t->allocated*2-1].next = NULL;
	t->allocated*=2;

	/* Call function again. Gotta love that tail recursion! */
	return crStateTextureAllocate_t(t, name);
}

void crStateTextureDelete(GLuint name) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	crStateTextureDelete_t(t, name);
}

void crStateTextureDelete_t(CRTextureState *t, GLuint name) 
{
	CRTextureObj *i;
	CRTextureObj **iprev;
	CRTextureFreeElem *j;
	CRTextureFreeElem *jnext;
	int k;

	if (!name) 
	{
		return;
	}

	/* Find it in the hash */
	for (iprev = &(t->mapping[name % CRTEXTURE_HASHSIZE]), i=t->mapping[name % CRTEXTURE_HASHSIZE]; 
			i && i->name != name; 
			iprev = &((*iprev)->next), i = i->next)
	{
		/* EMPTY BODY */
	}

	if (!i) 
	{
		return; /* Freeing a freed obj */
	}

	/* Clear the image ptr */
	for (k=0; k<t->maxLevel; k++) 
	{
		CRTextureLevel *tl = i->level+k;
		if (tl->img) 
		{
			crFree (tl->img);
			tl->img = NULL;
			tl->bytes = 0;
		}
	}
	crFree (i->level);
	i->level = NULL;

	/* Remove from hash table */
	*iprev = i->next;

	/* Add to firstFree */
	i->next = t->firstFree;
	t->firstFree = i;

	/*********************************/
	/* Add the name to the freeList  */
	/* Find the bracketing sequences */

	for (j=t->freeList;
			j && j->next && j->next->min < name;
			j = j->next)
	{
		/* EMPTY BODY */
	}

	/* j will always be valid */
	if (!j) 
	{
		UNIMPLEMENTED();
	}

	/* Case:  j:(~,name-1) */
	if (j->max+1 == name) 
	{
		j->max++;
		if (j->next && j->max+1 >= j->next->min) 
		{
			/* Collapse */
			j->next->min = j->min;
			j->next->prev = j->prev;
			if (j->prev)
			{
				j->prev->next = j->next;
			}
			if (j==t->freeList) 
			{
				t->freeList = j->next;
			}
			crFree(j);
		}
		return;
	}

	/* Case: j->next: (name+1, ~)*/
	if (j->next && j->next->min-1 == name) 
	{
		j->next->min--;
		if (j->max+1 >= j->next->min) 
		{
			/* Collapse */
			j->next->min = j->min;
			j->next->prev = j->prev;
			if (j->prev)
			{
				j->prev->next = j->next;
			}
			if (j==t->freeList) 
			{
				t->freeList = j->next;
			}
			crFree(j);
		}
		return;
	}

	/* Case: j: (name+1, ~) j->next: null */
	if (!j->next && j->min-1 == name) 
	{
		j->min--;
		return;
	}

	jnext = (CRTextureFreeElem *) crAlloc (sizeof (CRTextureFreeElem));
	jnext->min = name;
	jnext->max = name;

	/* Case: j: (~,name-(2+))  j->next: (name+(2+), ~) or null */
	if (name > j->max) 
	{
		jnext->prev = j;
		jnext->next = j->next;
		if (j->next)
		{
			j->next->prev = jnext;
		}
		j->next = jnext;
		return;
	}

	/* Case: j: (name+(2+), ~) */
	/* Can only happen if j = t->freeList! */
	if (j == t->freeList && j->min > name) 
	{
		jnext->next = j;
		jnext->prev = j->prev;
		j->prev = jnext;
		t->freeList = jnext;
		return;
	}

	UNIMPLEMENTED();
}


void crStateTextureDestroy(CRTextureState *t) 
{
	unsigned int i;
	int j;
	CRTextureFreeElem *f;

	for (i=0; i<t->allocated; i++) 
	{
		CRTextureLevel *tl = t->textures[i].level;
		for (j=0; j<t->maxLevel; j++)
		{
			if (tl && tl[j].img)
			{
				crFree(tl[j].img);
			}
		}
		crFree(t->textures[i].level);
	}

	f = t->freeList;
	while (f) 
	{
		CRTextureFreeElem *fnext = f->next;
		crFree (f);
		f = fnext;
	}

}


int crStateTextureGetSize(GLenum target, GLenum level) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	switch (target) 
	{
		case GL_TEXTURE_1D:
			return t->currentTexture1D->level[level].bytes;
		case GL_TEXTURE_2D:
			return t->currentTexture2D->level[level].bytes;
#if 0
		case GL_TEXTURE_3D:
			return t->currentTexture3D->level[level].bytes;
#endif
		default:
			crStateError( __LINE__, __FILE__, GL_INVALID_ENUM, "Bogus target in crStateTextureGetSize: %d", target );
	}
	return 0;
}

const GLvoid * crStateTextureGetData(GLenum target, GLenum level) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	switch (target) 
	{
		case GL_TEXTURE_1D:
			return t->currentTexture1D->level[level].img;
		case GL_TEXTURE_2D:
			return t->currentTexture2D->level[level].img;
#if 0
		case GL_TEXTURE_3D:
			return t->currentTexture3D->level[level].img;
#endif
		default:
			crStateError( __LINE__, __FILE__, GL_INVALID_ENUM, "Bogus target in crStateTextureGetData: %d", target );
	}
	return NULL;
}


void STATE_APIENTRY crStateGenTextures(GLsizei n, GLuint *textures) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	GLsizei i;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glGenTextures called in Begin/End");
		return;
	}

	FLUSH();

	if (n < 0)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative n passed to glGenTextures: %d", n);
		return;
	}

	i=0;
	/* Get a valid name */
	for (i=0; i<n; i++) 
	{
		if (!t->freeList)
		{
			UNIMPLEMENTED();
		}
		/* Grab the next free name*/
		textures[i] = t->freeList->min;
		assert( textures[i] );
		crStateTextureAllocate_t(t, textures[i]);
	}
}

void STATE_APIENTRY crStateDeleteTextures(GLsizei n, const GLuint *textures) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);
	int i;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glDeleteTextures called in Begin/End");
		return;
	}

	FLUSH();

	if (n < 0)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Negative n passed to glDeleteTextures: %d", n);
		return;
	}

	for (i=0; i<n; i++) 
	{
		GLuint name = textures[i];
		if (name) 
		{
			crStateTextureDelete_t(t, name);
			/* if the currentTexture is deleted, 
			 ** reset back to the base texture.
			 */
			if (name == t->currentTexture1DName) 
			{
				t->currentTexture1D = &(t->base1D);
				t->currentTexture1DName = 0;
			}
			if (name == t->currentTexture2DName) 
			{
				t->currentTexture2D = &(t->base2D);
				t->currentTexture2DName = 0;
			}
#if 0
			if (name == t->currentTexture3DName) 
			{
				t->currentTexture3D = &(t->base3D);
				t->currentTexture3DName = 0;
			}
#endif
		}
	}

	tb->dirty = g->neg_bitid;
	tb->current = g->neg_bitid;
}

void STATE_APIENTRY crStateBindTexture(GLenum target, GLuint texture) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRTextureObj *tobj;
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glBindTexture called in Begin/End");
		return;
	}

	FLUSH();

	/* Special Case name = 0 */
	if (!texture) 
	{
		switch (target) 
		{
			case GL_TEXTURE_1D:
				t->currentTexture1D = &(t->base1D);
				t->currentTexture1DName = 0;
				break;
			case GL_TEXTURE_2D:
				t->currentTexture2D = &(t->base2D);
				t->currentTexture2DName = 0;
				break;
#if 0
			case GL_TEXTURE_3D:
				t->currentTexture3D = &(t->base3D);
				t->currentTexture3DName = 0;
				break;
#endif
			default:
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "Invalid target passed to glBindTexture: %d", target);
				return;
		}

		tb->dirty = g->neg_bitid;
		tb->current = g->neg_bitid;
		return;
	}

	/* texture != 0 */
	/* Get the texture */
	GET_TOBJ(tobj, texture);
	if (!tobj)
	{
		tobj = crStateTextureAllocate_t(t, texture);
	}

	/* Check the targets */
	if (tobj->target == GL_NONE)
	{
		/* Target isn't set so set it now.*/
		tobj->target = target;
	}
	else
	{
		if (tobj->target != target)
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "Attempt to bind a texture of diffent dimenions");
			return;
		}
	}

	/* Set the current texture */
	switch (target) 
	{
		case GL_TEXTURE_1D:
			t->currentTexture1D = tobj;
			t->currentTexture1DName = texture;
			break;
		case GL_TEXTURE_2D:
			t->currentTexture2D = tobj;
			t->currentTexture2DName = texture;
			break;
#if 0
		case GL_TEXTURE_3D:
			t->currentTexture3D = tobj;
			t->currentTexture3DName = texture;
			break;
#endif
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "Invalid target passed to glBindTexture: %d", target);
			return;
	}

	tb->dirty = g->neg_bitid;
	tb->current = g->neg_bitid;
}



void STATE_APIENTRY crStateTexImage1D (GLenum target, GLint level, GLint components, 
		GLsizei width, GLint border, GLenum format,
		GLenum type, const GLvoid *pixels  ) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRPixelState *p = &(g->pixel);
	CRTextureObj *tobj;
	CRTextureLevel *tl;
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);
	int i;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glTexImage1D called in Begin/End");
		return;
	}

	FLUSH();

	if (target != GL_TEXTURE_1D)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glTexImage1D target != GL_TEXTURE_1D: %d", target);
		return;
	}

	if (level < 0 || level > t->maxLevel) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexImage1D level oob: %d", level);
		return;
	}

	/* Not needed in 1.1
		 if (componnents < 1 || components > 4)
		 {
		 crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexImage1D components oob: %d", components);
	 return;
	 }
	 */

	if (border != 0 && border != 1)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexImage1D border oob: %d", border);
		return;
	}

	/* check the bits in width */
	i=1;
	if (width > 0)
	{
		for (i=width-2*border; i>0 && !(i&0x1); i = i >> 1)
		{
			/* EMPTY BODY */
		}
	}
	if (width < 0 || i!=1) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexImage1D width is not valid: %d", width);
		return;
	}

	if (width > t->maxTextureSize)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
					"glTexImage1D width oob: %d", width);
		return;
	}

	/*
	 ** Only set these fields if 
	 ** defining the base texture.
	 */
	tobj = t->currentTexture1D;
	tl = tobj->level+level;
	tl->bytes = crPixelSize(format, type, width, 1);

	if (tl->img) 
	{
		crFree (tl->img);
	}
	tl->img = (GLubyte *) crAlloc (tl->bytes);

	if (pixels) 
	{
		crPixelCopy1D((GLvoid *) tl->img, pixels, format, type, width, &(p->unpack));
	}

	tl->width = width;
	tl->height = 1;
	tl->depth = 1;
	tl->format = format;
	tl->border = border;
	tl->components = components;
	tl->type = type;
	tl->bytesPerPixel = tl->bytes / width;

	tobj->target = target;

	tl->dirty = g->neg_bitid;
	tobj->dirty = g->neg_bitid;
	tobj->imageBit = g->neg_bitid;
	tb->dirty = g->neg_bitid;

#if 0
	if (t->broadcast_textures)
		__gltexture_broadcast (g, tobj);
#endif
}

void STATE_APIENTRY crStateTexImage2D (GLenum target, GLint level, GLint components, 
		GLsizei width, GLsizei height, GLint border,
		GLenum format, GLenum type, const GLvoid *pixels  ) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRPixelState *p = &(g->pixel);
	CRTextureObj *tobj;
	CRTextureLevel *tl;
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);
	int i;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glTexImage2D called in Begin/End");
		return;
	}

	FLUSH();

	if (target != GL_TEXTURE_2D)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glTexImage2D target != GL_TEXTURE_2D: %d", target);
		return;
	}

	if (level < 0 || level > t->maxLevel) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexImage2D level oob: %d", level);
		return;
	}

	if (border != 0 && border != 1)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexImage2D border oob: %d", border);
		return;
	}

	/* check the bits in width */
	i = 1;
	if (width > 0)
	{
		for (i=width-2*border; i>0 && !(i&0x1); i = i >> 1)
		{
			/* EMPTY BODY */
		}
	}
	if (width < 0 || i!=1) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexImage2D width is not valid: %d", width);
		return;
	}

	/* check the bits in height */
	i=1;
	if (height > 0)
	{
		for (i=height-2*border; i>0 && !(i&0x1); i = i >> 1)
		{
			/* EMPTY BODY */
		}
	}
	if (height < 0 || i!=1) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexImage2D height is not valid: %d", height);
		return;
	}

	if (width > t->maxTextureSize)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
					"glTexImage2D width oob: %d", width);
		return;
	}

	if (height > t->maxTextureSize)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
					"glTexImage2D height oob: %d", height);
		return;
	}

	/*
	 ** Only set these fields if 
	 ** defining the base texture.
	 */
	tobj = t->currentTexture2D;
	tl = tobj->level+level;
	tl->bytes= crPixelSize(format, type, width, height);

	if (tl->img)
	{
		crFree (tl->img);
	}
	tl->img= (GLubyte *) crAlloc (tl->bytes);

	if (pixels) 
	{
		crPixelCopy2D((GLvoid *) (tl->img), pixels, format, type, width, height, &(p->unpack) );
	}

	tl->width = width;
	tl->height = height;
	tl->depth = 1;
	tl->format = format;
	tl->components = components;
	tl->border = border;
	tl->type = type;
	tl->bytesPerPixel = tl->bytes/ (width*height);

	tobj->target = target;

	tl->dirty = g->neg_bitid;
	tobj->dirty = g->neg_bitid;
	tobj->imageBit = g->neg_bitid;
	tb->dirty = g->neg_bitid;

#if 0
	if (t->broadcast_textures)
		__gltexture_broadcast (g, tobj);
#endif
}

void STATE_APIENTRY crStateTexSubImage1D (GLenum target, GLint level, GLint xoffset, 
		GLsizei width, GLenum format,
		GLenum type, const GLvoid *pixels  ) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRPixelState *p = &(g->pixel);
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);
	CRTextureObj *tobj = t->currentTexture1D;
	CRTextureLevel *tl = tobj->level + level;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glSubTexImage1D called in Begin/End");
		return;
	}

	FLUSH();

	if (target != GL_TEXTURE_1D)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glSubTexImage1D target != GL_TEXTURE_1D: %d", target);
		return;
	}

	if (level < 0 || level > t->maxLevel) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glSubTexImage1D level oob: %d", level);
		return;
	}

	if (width > t->maxTextureSize)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
					"glSubTexImage1D width oob: %d", width);
		return;
	}

	/* Not needed in 1.1
		 if (componnents < 1 || components > 4)
		 {
		 crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexImage1D components oob: %d", components);
	 return;
	 }
	 */

	/* XXX We need to handle conversion! */	
	if (format != tl->format) 
	{
		UNIMPLEMENTED();
		exit (1);
	}
	if (type != tl->type) 
	{
		UNIMPLEMENTED();
		exit (1);
	}
	if (width + xoffset > tl->width) 
	{
		UNIMPLEMENTED();
		exit (1);
	}

	crPixelCopy1D(tl->img + xoffset*tl->bytesPerPixel, pixels, format, type, width, &(p->unpack) );

	tl->dirty = g->neg_bitid;
	tobj->dirty = g->neg_bitid;
	tobj->imageBit = g->neg_bitid;
	tb->dirty = g->neg_bitid;

#if 0
	if (t->broadcast_textures)
		__gltexture_broadcast (g, tobj);
#endif
}

void STATE_APIENTRY crStateTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, 
		GLsizei width, GLsizei height,
		GLenum format, GLenum type, const GLvoid *pixels  ) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRPixelState *p = &(g->pixel);
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);
	CRTextureObj *tobj = t->currentTexture2D;
	CRTextureLevel *tl = tobj->level + level;

	GLubyte *subimg = NULL;
	GLubyte *img = NULL;
	GLubyte *src;

	int i;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glTexSubImage2D called in Begin/End");
		return;
	}

	FLUSH();

	if (target != GL_TEXTURE_2D)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glTexSubImage2D target != GL_TEXTURE_2D: %d", target);
		return;
	}

	if (level < 0 || level > t->maxLevel) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexSubImage2D level oob: %d", level);
		return;
	}

	if (width < 0 || width > t->maxTextureSize)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
					"glSubTexImage2D width oob: %d", width);
		return;
	}

	if (height < 0 || height > t->maxTextureSize)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
					"glSubTexImage2D height oob: %d", height);
		return;
	}


	/* XXX We need to handle conversion! */	
	if (format != tl->format) 
	{
		UNIMPLEMENTED();
		exit (1);
	}
	if (type != tl->type) 
	{
		UNIMPLEMENTED();
		exit (1);
	}
	if (width + xoffset > tl->width) 
	{
		UNIMPLEMENTED();
		exit (1);
	}
	if (height + yoffset > tl->height) 
	{
		UNIMPLEMENTED();
		exit (1);
	}

	subimg = (GLubyte *) crAlloc (crPixelSize(format, type, width, height));

	crPixelCopy2D( subimg,	pixels, format, type, width, height, &(p->unpack) );

	img =   tl->img +
		xoffset*tl->bytesPerPixel +
		yoffset*tl->width*tl->bytesPerPixel;
	src = subimg;

	/* Copy the data into the texture */
	for (i=0; i<height; i++) 
	{
		memcpy (img, src, tl->bytesPerPixel * width);
		img += tl->width * tl->bytesPerPixel;
		src += width * tl->bytesPerPixel;
	}

	crFree (subimg);

	tobj->dirty = g->neg_bitid;
	tobj->imageBit = g->neg_bitid;
	tl->dirty = g->neg_bitid;
	tb->dirty = g->neg_bitid;

#if 0
	if (t->broadcast_textures)
		__gltexture_broadcast (g, tobj);
#endif
}

#if 0
void STATE_APIENTRY crStateTexImage3D (GLenum target, GLint level, GLint components, 
		GLsizei width, GLsizei height, GLsizei depth,
		GLint border, GLenum format, GLenum type, const GLvoid *pixels  ) {
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRPixelState *p = &(g->pixel);
	CRTextureObj *tobj;
	CRTextureLevel *tl;
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);
	int i;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glTexImage3D called in Begin/End");
		return;
	}


	if (target != GL_TEXTURE_3D)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glTexImage3D target != GL_TEXTURE_3D: %d", target);
		return;
	}

	if (level < 0 || level > t->max3Dlevel) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexImage3D level oob: %d", level);
		return;
	}

	if (border != 0 && border != 1)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexImage3D border oob: %d", border);
		return;
	}

	/* check the bits in width */
	i=1;
	if (width > 0)
		for (i=width-2*border; i>0 && !(i&0x1); i = i >> 1);
	if (width < 0 || i!=1) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexImage3D width is not valid: %d", width);
		return;
	}

	/* check the bits in height */
	i=1;
	if (height > 0)
		for (i=height-2*border; i>0 && !(i&0x1); i = i >> 1);
	if (height < 0 || i!=1) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexImage3D height is not valid: %d", height);
		return;
	}

	/* check the bits in height */
	i=1;
	if (depth > 0)
		for (i=depth-2*border; i>0 && !(i&0x1); i = i >> 1);
	if (depth < 0 || i!=1) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexImage3D depth is not valid: %d", depth);
		return;
	}

	if (width > t->max3Dtexturesize)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
					"glTexImage3D width oob: %d", width);
		return;
	}

	if (height > t->max3Dtexturesize)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
					"glTexImage3D height oob: %d", height);
		return;
	}

	/*
	 ** Only set these fields if 
	 ** defining the base texture.
	 */
	tobj = t->currentTexture1D;
	tl = tobj->level+level;

	tobj->target = target;

	tl->components = components;
	tl->border = border;
	tl->width = width;
	tl->height = height;
	tl->depth = depth;
	tl->format = format;
	tl->type = type;
	tl->bytes = __glpixel_getdata3Dsize(width, height, depth, format, type);
	tl->img = (GLubyte *) crAlloc (tl->bytes);

	__glpixel_getdata3D_p(p, (GLvoid *) tl->img,
			width, height, depth, format, type, pixels);

	tl->dirty = g->neg_bitid;
	tobj->imageBit = g->neg_bitid;
	tobj->dirty = g->neg_bitid;
	tb->dirty = g->neg_bitid;

	if (t->broadcast_textures)
		__gltexture_broadcast (g, tobj);
}
#endif

void STATE_APIENTRY crStateTexParameterfv (GLenum target, GLenum pname, const GLfloat *param) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRTextureObj *tobj = NULL;
	GLenum e = (GLenum) *param;
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					"TexParameterfv called in Begin/End");
		return;
	}

	FLUSH();

	switch (target) 
	{
		case GL_TEXTURE_1D:
			tobj = t->currentTexture1D;
			break;
		case GL_TEXTURE_2D:
			tobj = t->currentTexture2D;
			break;
#if 0
		case GL_TEXTURE_3D:
			tobj = t->currentTexture3D;
			break;
#endif
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"TexParamterfv: target is invalid: %d", target);
			return;
	}

	switch (pname) 
	{
		case GL_TEXTURE_MIN_FILTER:
			if (e != GL_NEAREST &&
					e != GL_LINEAR &&
					e != GL_NEAREST_MIPMAP_NEAREST &&
					e != GL_LINEAR_MIPMAP_NEAREST &&
					e != GL_NEAREST_MIPMAP_LINEAR &&
					e != GL_LINEAR_MIPMAP_LINEAR)
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
							"TexParamterfv: GL_TEXTURE_MIN_FILTER invalid param: %d", e);
				return;
			}
			tobj->minFilter = e;
			break;
		case GL_TEXTURE_MAG_FILTER:
			if (e != GL_NEAREST &&
					e != GL_LINEAR)
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
							"TexParamterfv: GL_TEXTURE_MAG_FILTER invalid param: %d", e);
				return;
			}
			tobj->magFilter = e;
			break;
		case GL_TEXTURE_WRAP_S:
			if (e != GL_CLAMP &&
					e != GL_REPEAT)
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
							"TexParameterfv: GL_TEXTURE_WRAP_S invalid param: %d", e);
				return;
			}
			tobj->wrapS = e;
			break;
		case GL_TEXTURE_WRAP_T:
			if (e != GL_CLAMP &&
					e != GL_REPEAT)
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
							"TexParameterfv: GL_TEXTURE_WRAP_T invalid param: %d", e);
				return;
			}
			tobj->wrapT = e;
			break;
		case GL_TEXTURE_BORDER_COLOR:
			tobj->borderColor.r = param[0];
			tobj->borderColor.g = param[1];
			tobj->borderColor.b = param[2];
			tobj->borderColor.a = param[3];
			break;
		default:
			if (!crStateTexParameterfvExtensions( t, tobj, pname, param ))
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
							"TexParamterfv: Invalid pname: %d", pname);
			}
			return;
	}

	tobj->dirty = g->neg_bitid;
	tobj->paramsBit = g->neg_bitid;
	tb->dirty = g->neg_bitid;

#if 0
	if (t->broadcast_textures)
		__gltexture_broadcast (g, tobj);
#endif
}

void STATE_APIENTRY crStateTexParameteriv (GLenum target, GLenum pname, const GLint *param) 
{
	GLfloat f_param;
	GLcolor f_color;
	switch (pname) 
	{
		case GL_TEXTURE_MIN_FILTER:
		case GL_TEXTURE_MAG_FILTER:
		case GL_TEXTURE_WRAP_S:
		case GL_TEXTURE_WRAP_T:
			f_param = (GLfloat) (*param);
			crStateTexParameterfv( target, pname, &(f_param) );
			break;
		case GL_TEXTURE_BORDER_COLOR:
			f_color.r = ((GLfloat) param[0])/GL_MAXINT;
			f_color.g = ((GLfloat) param[1])/GL_MAXINT;
			f_color.b = ((GLfloat) param[2])/GL_MAXINT;
			f_color.a = ((GLfloat) param[3])/GL_MAXINT;
			crStateTexParameterfv( target, pname, (const GLfloat *) &(f_color) );
			break;
		default:
			if (!crStateTexParameterivExtensions( target, pname, param ))
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
							"TexParamteriv: Invalid pname: %d", pname);
			}
			return;
	}
}

void STATE_APIENTRY crStateTexParameterf (GLenum target, GLenum pname, GLfloat param) 
{
	crStateTexParameterfv( target, pname, &param );
}


void STATE_APIENTRY crStateTexParameteri (GLenum target, GLenum pname, GLint param) {
	GLfloat f_param = (GLfloat) param;
	crStateTexParameterfv( target, pname, &f_param );
}


void STATE_APIENTRY crStateTexEnvfv (GLenum target, GLenum pname, const GLfloat *param) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);
	GLenum e;
	GLcolorf c;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					"glTexEnvfv called in begin/end");
		return;
	}

	FLUSH();

	if (target != GL_TEXTURE_ENV)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					"glTexEnvfv: target != GL_TEXTURE_ENV: %d", target);
		return;
	}

	switch (pname) 
	{
		case GL_TEXTURE_ENV_MODE:
			e = (GLenum) *param;
			if (e != GL_MODULATE &&
					e != GL_DECAL &&
					e != GL_BLEND &&
					e != GL_REPLACE)
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
							"glTexEnvfv: invalid param: %f", *param);
				return;
			}
			t->envMode = e;
			break;
		case GL_TEXTURE_ENV_COLOR:
			c.r = param[0];
			c.g = param[1];
			c.b = param[2];
			c.a = param[3];
			if (c.r > 1.0f) c.r = 1.0f;
			if (c.g > 1.0f) c.g = 1.0f;
			if (c.b > 1.0f) c.b = 1.0f;
			if (c.a > 1.0f) c.a = 1.0f;
			if (c.r < 0.0f) c.r = 0.0f;
			if (c.g < 0.0f) c.g = 0.0f;
			if (c.b < 0.0f) c.b = 0.0f;
			if (c.a < 0.0f) c.a = 0.0f;
			t->envColor = c;
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"glTexEnvfv: invalid pname: %d", pname);
			return;
	}

	tb->envBit = g->neg_bitid;
	tb->dirty = g->neg_bitid;
}

void STATE_APIENTRY crStateTexEnviv (GLenum target, GLenum pname, const GLint *param) 
{
	GLfloat f_param;
	GLcolor f_color;

	switch (pname) {
		case GL_TEXTURE_ENV_MODE:
			f_param = (GLfloat) (*param);
			crStateTexEnvfv( target, pname, &f_param );
			break;
		case GL_TEXTURE_ENV_COLOR:
			f_color.r = ((GLfloat) param[0]) / GL_MAXINT;
			f_color.g = ((GLfloat) param[1]) / GL_MAXINT;
			f_color.b = ((GLfloat) param[2]) / GL_MAXINT;
			f_color.a = ((GLfloat) param[3]) / GL_MAXINT;
			crStateTexEnvfv( target, pname, (const GLfloat *) &f_color );
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"glTexEnvfv: invalid pname: %d", pname);
			return;
	}
}

void STATE_APIENTRY crStateTexEnvf (GLenum target, GLenum pname, GLfloat param) 
{
	crStateTexEnvfv( target, pname, &param );
}

void STATE_APIENTRY crStateTexEnvi (GLenum target, GLenum pname, GLint param) 
{
	GLfloat f_param = (GLfloat) param;
	crStateTexEnvfv( target, pname, &f_param );
}

void STATE_APIENTRY crStateGetTexEnvfv (GLenum target, GLenum pname, GLfloat *param) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__,GL_INVALID_OPERATION,
					"glGetTexEnvfv called in begin/end");
		return;
	}

	if (target != GL_TEXTURE_ENV)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					"glGetTexEnvfv: target != GL_TEXTURE_ENV: %d", target);
		return;
	}

	switch (pname) {
		case GL_TEXTURE_ENV_MODE:
			*param = (GLfloat) t->envMode;
			break;
		case GL_TEXTURE_ENV_COLOR:
			param[0] = t->envColor.r;
			param[1] = t->envColor.g;
			param[2] = t->envColor.b;
			param[3] = t->envColor.a;
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"glGetTexEnvfv: invalid pname: %d", pname);
			return;
	}
}

void STATE_APIENTRY crStateGetTexEnviv (GLenum target, GLenum pname, GLint *param) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__,GL_INVALID_OPERATION,
					"glGetTexEnviv called in begin/end");
		return;
	}

	if (target != GL_TEXTURE_ENV)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					"glGetTexEnviv: target != GL_TEXTURE_ENV: %d", target);
		return;
	}

	switch (pname) {
		case GL_TEXTURE_ENV_MODE:
			*param = (GLint) t->envMode;
			break;
		case GL_TEXTURE_ENV_COLOR:
			param[0] = (GLint) (t->envColor.r * GL_MAXINT);
			param[0] = (GLint) (t->envColor.g * GL_MAXINT);
			param[0] = (GLint) (t->envColor.b * GL_MAXINT);
			param[0] = (GLint) (t->envColor.a * GL_MAXINT);
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"glGetTexEnviv: invalid pname: %d", pname);
			return;
	}
}

void STATE_APIENTRY crStateTexGendv (GLenum coord, GLenum pname, const GLdouble *param) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRTransformState *trans = &(g->transform);
	GLvectorf v;
	GLenum e;
	GLmatrix inv;
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					"glTexGen called in begin/end");
		return;
	}

	FLUSH();

	switch (coord) 
	{
		case GL_S:
			switch (pname) 
			{
				case GL_TEXTURE_GEN_MODE:
					e = (GLenum) *param;
					if (e != GL_OBJECT_LINEAR &&
							e != GL_EYE_LINEAR &&
							e != GL_SPHERE_MAP)
					{
						crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
									"glTexGen called with bad param: %lf", *param);
						return;
					}
					t->gen.s = e;
					tb->gen = g->neg_bitid;
					tb->dirty = g->neg_bitid;
					break;
				case GL_OBJECT_PLANE:
					v.x = (GLfloat) param[0];
					v.y = (GLfloat) param[1];
					v.z = (GLfloat) param[2];
					v.w = (GLfloat) param[3];
					t->objSCoeff = v;
					tb->objGen = g->neg_bitid;
					tb->dirty = g->neg_bitid;
					break;
				case GL_EYE_PLANE:
					v.x = (GLfloat) param[0];
					v.y = (GLfloat) param[1];
					v.z = (GLfloat) param[2];
					v.w = (GLfloat) param[3];
					crStateTransformInvertTransposeMatrix(&inv, trans->modelView+trans->modelViewDepth);
					crStateTransformXformPointMatrixf(&inv, &v);
					t->eyeSCoeff = v;
					tb->eyeGen = g->neg_bitid;
					tb->dirty = g->neg_bitid;
					break;
				default:
					crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								"glTexGen called with bogus pname: %d", pname);
					return;
			}
			break;
		case GL_T:
			switch (pname) {
				case GL_TEXTURE_GEN_MODE:
					e = (GLenum) *param;
					if (e != GL_OBJECT_LINEAR &&
							e != GL_EYE_LINEAR &&
							e != GL_SPHERE_MAP)
					{
						crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
									"glTexGen called with bad param: %lf", *param);
						return;
					}
					t->gen.t = e;
					tb->gen = g->neg_bitid;
					tb->dirty = g->neg_bitid;
					break;
				case GL_OBJECT_PLANE:
					v.x = (GLfloat) param[0];
					v.y = (GLfloat) param[1];
					v.z = (GLfloat) param[2];
					v.w = (GLfloat) param[3];
					t->objTCoeff = v;
					tb->objGen = g->neg_bitid;
					tb->dirty = g->neg_bitid;
					break;
				case GL_EYE_PLANE:
					v.x = (GLfloat) param[0];
					v.y = (GLfloat) param[1];
					v.z = (GLfloat) param[2];
					v.w = (GLfloat) param[3];
					crStateTransformInvertTransposeMatrix(&inv, trans->modelView+trans->modelViewDepth);
					crStateTransformXformPointMatrixf(&inv, &v);
					t->eyeTCoeff = v;
					tb->eyeGen = g->neg_bitid;
					tb->dirty = g->neg_bitid;
					break;
				default:
					crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								"glTexGen called with bogus pname: %d", pname);
					return;
			}
			break;
		case GL_R:
			switch (pname) {
				case GL_TEXTURE_GEN_MODE:
					e = (GLenum) *param;
					if (e != GL_OBJECT_LINEAR &&
							e != GL_EYE_LINEAR)
					{
						crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
									"glTexGen called with bad param: %lf", *param);
						return;
					}
					t->gen.p = e;
					tb->gen = g->neg_bitid;
					tb->dirty = g->neg_bitid;
					break;
				case GL_OBJECT_PLANE:
					v.x = (GLfloat) param[0];
					v.y = (GLfloat) param[1];
					v.z = (GLfloat) param[2];
					v.w = (GLfloat) param[3];
					t->objRCoeff = v;
					tb->objGen = g->neg_bitid;
					tb->dirty = g->neg_bitid;
					break;
				case GL_EYE_PLANE:
					v.x = (GLfloat) param[0];
					v.y = (GLfloat) param[1];
					v.z = (GLfloat) param[2];
					v.w = (GLfloat) param[3];
					crStateTransformInvertTransposeMatrix(&inv, trans->modelView+trans->modelViewDepth);
					crStateTransformXformPointMatrixf(&inv, &v);
					t->eyeRCoeff = v;
					tb->eyeGen = g->neg_bitid;
					tb->dirty = g->neg_bitid;
					break;
				default:
					crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								"glTexGen called with bogus pname: %d", pname);
					return;
			}
			break;
		case GL_Q:
			switch (pname) {
				case GL_TEXTURE_GEN_MODE:
					e = (GLenum) *param;
					if (e != GL_OBJECT_LINEAR &&
							e != GL_EYE_LINEAR)
					{
						crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
									"glTexGen called with bad param: %lf", *param);
						return;
					}
					t->gen.q = e;
					tb->gen = g->neg_bitid;
					tb->dirty = g->neg_bitid;
					break;
				case GL_OBJECT_PLANE:
					v.x = (GLfloat) param[0];
					v.y = (GLfloat) param[1];
					v.z = (GLfloat) param[2];
					v.w = (GLfloat) param[3];
					t->objQCoeff = v;
					tb->objGen = g->neg_bitid;
					tb->dirty = g->neg_bitid;
					break;
				case GL_EYE_PLANE:
					v.x = (GLfloat) param[0];
					v.y = (GLfloat) param[1];
					v.z = (GLfloat) param[2];
					v.w = (GLfloat) param[3];
					crStateTransformInvertTransposeMatrix(&inv, trans->modelView+trans->modelViewDepth);
					crStateTransformXformPointMatrixf(&inv, &v);
					t->eyeQCoeff = v;
					tb->eyeGen = g->neg_bitid;
					tb->dirty = g->neg_bitid;
					break;
				default:
					crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								"glTexGen called with bogus pname: %d", pname);
					return;
			}
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"glTexGen called with bogus coord: %d", coord);
			return;
	}
}

void STATE_APIENTRY crStateTexGenfv (GLenum coord, GLenum pname, const GLfloat *param) 
{
	GLdouble d_param;
	GLvectord d_vector;
	switch (pname) 
	{
		case GL_TEXTURE_GEN_MODE:
			d_param = (GLdouble) *param;
			crStateTexGendv( coord, pname, &d_param );
			break;
		case GL_OBJECT_PLANE:
		case GL_EYE_PLANE:
			d_vector.x = (GLdouble) param[0];
			d_vector.y = (GLdouble) param[1];
			d_vector.z = (GLdouble) param[2];
			d_vector.w = (GLdouble) param[3];
			crStateTexGendv( coord, pname, (const double *) &d_vector );
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"glTexGen called with bogus pname: %d", pname);
			return;
	}
}

void STATE_APIENTRY crStateTexGeniv (GLenum coord, GLenum pname, const GLint *param) 
{
	GLdouble d_param;
	GLvectord d_vector;
	switch (pname) 
	{
		case GL_TEXTURE_GEN_MODE:
			d_param = (GLdouble) *param;
			crStateTexGendv( coord, pname, &d_param );
			break;
		case GL_OBJECT_PLANE:
		case GL_EYE_PLANE:
			d_vector.x = (GLdouble) param[0];
			d_vector.y = (GLdouble) param[1];
			d_vector.z = (GLdouble) param[2];
			d_vector.w = (GLdouble) param[3];
			crStateTexGendv( coord, pname, (const double *) &d_vector );
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"glTexGen called with bogus pname: %d", pname);
			return;
	}
}

void STATE_APIENTRY crStateTexGend (GLenum coord, GLenum pname, GLdouble param) 
{
	crStateTexGendv( coord, pname, &param );
}

void STATE_APIENTRY crStateTexGenf (GLenum coord, GLenum pname, GLfloat param) 
{
	GLdouble d_param = (GLdouble) param;
	crStateTexGendv( coord, pname, &d_param );
}

void STATE_APIENTRY crStateTexGeni (GLenum coord, GLenum pname, GLint param) 
{
	GLdouble d_param = (GLdouble) param;
	crStateTexGendv( coord, pname, &d_param );
}

void STATE_APIENTRY crStateGetTexGendv (GLenum coord, GLenum pname, GLdouble *param) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					"glGetTexGen called in begin/end");
		return;
	}

	switch (pname) {
		case GL_TEXTURE_GEN_MODE:
			switch (coord) {
				case GL_S:
					*param = (GLdouble) t->gen.s;
					break;
				case GL_T:
					*param = (GLdouble) t->gen.t;
					break;
				case GL_R:
					*param = (GLdouble) t->gen.p;
					break;
				case GL_Q:
					*param = (GLdouble) t->gen.q;
					break;
				default:
					crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								"glGetTexGen called with bogus coord: %d", coord);
					return;
			}
			break;
		case GL_OBJECT_PLANE:
			switch (coord) {
				case GL_S:
					param[0] = (GLdouble) t->objSCoeff.x;
					param[1] = (GLdouble) t->objSCoeff.y;
					param[2] = (GLdouble) t->objSCoeff.z;
					param[3] = (GLdouble) t->objSCoeff.w;
					break;
				case GL_T:
					param[0] = (GLdouble) t->objTCoeff.x;
					param[1] = (GLdouble) t->objTCoeff.y;
					param[2] = (GLdouble) t->objTCoeff.z;
					param[3] = (GLdouble) t->objTCoeff.w;
					break;
				case GL_R:
					param[0] = (GLdouble) t->objRCoeff.x;
					param[1] = (GLdouble) t->objRCoeff.y;
					param[2] = (GLdouble) t->objRCoeff.z;
					param[3] = (GLdouble) t->objRCoeff.w;
					break;
				case GL_Q:
					param[0] = (GLdouble) t->objQCoeff.x;
					param[1] = (GLdouble) t->objQCoeff.y;
					param[2] = (GLdouble) t->objQCoeff.z;
					param[3] = (GLdouble) t->objQCoeff.w;
					break;
				default:
					crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								"glGetTexGen called with bogus coord: %d", coord);
					return;
			}
			break;
		case GL_EYE_PLANE:
			switch (coord) {
				case GL_S:
					param[0] = (GLdouble) t->eyeSCoeff.x;
					param[1] = (GLdouble) t->eyeSCoeff.y;
					param[2] = (GLdouble) t->eyeSCoeff.z;
					param[3] = (GLdouble) t->eyeSCoeff.w;
					break;
				case GL_T:
					param[0] = (GLdouble) t->eyeTCoeff.x;
					param[1] = (GLdouble) t->eyeTCoeff.y;
					param[2] = (GLdouble) t->eyeTCoeff.z;
					param[3] = (GLdouble) t->eyeTCoeff.w;
					break;
				case GL_R:
					param[0] = (GLdouble) t->eyeRCoeff.x;
					param[1] = (GLdouble) t->eyeRCoeff.y;
					param[2] = (GLdouble) t->eyeRCoeff.z;
					param[3] = (GLdouble) t->eyeRCoeff.w;
					break;
				case GL_Q:
					param[0] = (GLdouble) t->eyeQCoeff.x;
					param[1] = (GLdouble) t->eyeQCoeff.y;
					param[2] = (GLdouble) t->eyeQCoeff.z;
					param[3] = (GLdouble) t->eyeQCoeff.w;
					break;
				default:
					crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								"glGetTexGen called with bogus coord: %d", coord);
					return;
			}
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"glGetTexGen called with bogus pname: %d", pname);
			return;
	}
}

void STATE_APIENTRY crStateGetTexGenfv (GLenum coord, GLenum pname, GLfloat *param) {
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					"glGetTexGen called in begin/end");
		return;
	}

	switch (pname) {
		case GL_TEXTURE_GEN_MODE:
			switch (coord) {
				case GL_S:
					*param = (GLfloat) t->gen.s;
					break;
				case GL_T:
					*param = (GLfloat) t->gen.t;
					break;
				case GL_R:
					*param = (GLfloat) t->gen.p;
					break;
				case GL_Q:
					*param = (GLfloat) t->gen.q;
					break;
				default:
					crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								"glGetTexGen called with bogus coord: %d", coord);
					return;
			}
			break;
		case GL_OBJECT_PLANE:
			switch (coord) {
				case GL_S:
					param[0] = t->objSCoeff.x;
					param[1] = t->objSCoeff.y;
					param[2] = t->objSCoeff.z;
					param[3] = t->objSCoeff.w;
					break;
				case GL_T:
					param[0] =  t->objTCoeff.x;
					param[1] =  t->objTCoeff.y;
					param[2] =  t->objTCoeff.z;
					param[3] =  t->objTCoeff.w;
					break;
				case GL_R:
					param[0] =  t->objRCoeff.x;
					param[1] =  t->objRCoeff.y;
					param[2] =  t->objRCoeff.z;
					param[3] =  t->objRCoeff.w;
					break;
				case GL_Q:
					param[0] =  t->objQCoeff.x;
					param[1] =  t->objQCoeff.y;
					param[2] =  t->objQCoeff.z;
					param[3] =  t->objQCoeff.w;
					break;
				default:
					crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								"glGetTexGen called with bogus coord: %d", coord);
					return;
			}
			break;
		case GL_EYE_PLANE:
			switch (coord) {
				case GL_S:
					param[0] =  t->eyeSCoeff.x;
					param[1] =  t->eyeSCoeff.y;
					param[2] =  t->eyeSCoeff.z;
					param[3] =  t->eyeSCoeff.w;
					break;
				case GL_T:
					param[0] =  t->eyeTCoeff.x;
					param[1] =  t->eyeTCoeff.y;
					param[2] =  t->eyeTCoeff.z;
					param[3] =  t->eyeTCoeff.w;
					break;
				case GL_R:
					param[0] =  t->eyeRCoeff.x;
					param[1] =  t->eyeRCoeff.y;
					param[2] =  t->eyeRCoeff.z;
					param[3] =  t->eyeRCoeff.w;
					break;
				case GL_Q:
					param[0] =  t->eyeQCoeff.x;
					param[1] =  t->eyeQCoeff.y;
					param[2] =  t->eyeQCoeff.z;
					param[3] =  t->eyeQCoeff.w;
					break;
				default:
					crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								"glGetTexGen called with bogus coord: %d", coord);
					return;
			}
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"glGetTexGen called with bogus pname: %d", pname);
			return;
	}
}

void STATE_APIENTRY crStateGetTexGeniv (GLenum coord, GLenum pname, GLint *param) {
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					"glGetTexGen called in begin/end");
		return;
	}

	switch (pname) {
		case GL_TEXTURE_GEN_MODE:
			switch (coord) {
				case GL_S:
					*param = (GLint) t->gen.s;
					break;
				case GL_T:
					*param = (GLint) t->gen.t;
					break;
				case GL_R:
					*param = (GLint) t->gen.p;
					break;
				case GL_Q:
					*param = (GLint) t->gen.q;
					break;
				default:
					crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								"glGetTexGen called with bogus coord: %d", coord);
					return;
			}
			break;
		case GL_OBJECT_PLANE:
			switch (coord) {
				case GL_S:
					param[0] = (GLint) t->objSCoeff.x;
					param[1] = (GLint) t->objSCoeff.y;
					param[2] = (GLint) t->objSCoeff.z;
					param[3] = (GLint) t->objSCoeff.w;
					break;
				case GL_T:
					param[0] = (GLint) t->objTCoeff.x;
					param[1] = (GLint) t->objTCoeff.y;
					param[2] = (GLint) t->objTCoeff.z;
					param[3] = (GLint) t->objTCoeff.w;
					break;
				case GL_R:
					param[0] = (GLint) t->objRCoeff.x;
					param[1] = (GLint) t->objRCoeff.y;
					param[2] = (GLint) t->objRCoeff.z;
					param[3] = (GLint) t->objRCoeff.w;
					break;
				case GL_Q:
					param[0] = (GLint) t->objQCoeff.x;
					param[1] = (GLint) t->objQCoeff.y;
					param[2] = (GLint) t->objQCoeff.z;
					param[3] = (GLint) t->objQCoeff.w;
					break;
				default:
					crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								"glGetTexGen called with bogus coord: %d", coord);
					return;
			}
			break;
		case GL_EYE_PLANE:
			switch (coord) {
				case GL_S:
					param[0] = (GLint) t->eyeSCoeff.x;
					param[1] = (GLint) t->eyeSCoeff.y;
					param[2] = (GLint) t->eyeSCoeff.z;
					param[3] = (GLint) t->eyeSCoeff.w;
					break;
				case GL_T:
					param[0] = (GLint) t->eyeTCoeff.x;
					param[1] = (GLint) t->eyeTCoeff.y;
					param[2] = (GLint) t->eyeTCoeff.z;
					param[3] = (GLint) t->eyeTCoeff.w;
					break;
				case GL_R:
					param[0] = (GLint) t->eyeRCoeff.x;
					param[1] = (GLint) t->eyeRCoeff.y;
					param[2] = (GLint) t->eyeRCoeff.z;
					param[3] = (GLint) t->eyeRCoeff.w;
					break;
				case GL_Q:
					param[0] = (GLint) t->eyeQCoeff.x;
					param[1] = (GLint) t->eyeQCoeff.y;
					param[2] = (GLint) t->eyeQCoeff.z;
					param[3] = (GLint) t->eyeQCoeff.w;
					break;
				default:
					crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								"glGetTexGen called with bogus coord: %d", coord);
					return;
			}
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"glGetTexGen called with bogus pname: %d", pname);
			return;
	}
}

void STATE_APIENTRY crStateGetTexImage (GLenum target, GLint level, GLenum format,
		GLenum type, GLvoid * pixels)
{
	UNUSED(target);
	UNUSED(level);
	UNUSED(format);
	UNUSED(type);
	UNUSED(pixels);
	UNIMPLEMENTED();
}


void STATE_APIENTRY crStateGetTexLevelParameterfv (GLenum target, GLint level, 
		GLenum pname, GLfloat *params) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRTextureObj *tobj = NULL;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					"glGetTexLevelParameter called in begin/end");
		return;
	}

	if (level < 0 && level > t->maxLevel)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
					"glGetTexLevelParameter: level oob: %d", level);
		return;
	}

	switch (target) 
	{
		case GL_TEXTURE_1D:
			tobj = t->currentTexture1D;
			break;
		case GL_TEXTURE_2D:
			tobj = t->currentTexture2D;
			break;
#if 0
		case GL_TEXTURE_3D:
			tobj = t->currentTexture3D;
			break;
#endif
		default: 
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"GetTexLevelParameter: invalid target: %d", target);
			return;
	}

	switch (pname) 
	{
		case GL_TEXTURE_WIDTH:
			*params = (GLfloat) tobj->level[level].width;
			break;
		case GL_TEXTURE_HEIGHT:
			*params = (GLfloat) tobj->level[level].height;
			break;
		case GL_TEXTURE_COMPONENTS:
			*params = (GLfloat) tobj->level[level].components;
			break;
		case GL_TEXTURE_BORDER:
			*params = (GLfloat) tobj->level[level].border;
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"GetTexLevelParameter: invalid pname: %d", pname);
			return;
	}
}

void STATE_APIENTRY crStateGetTexLevelParameteriv (GLenum target, GLint level, 
		GLenum pname, GLint *params) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRTextureObj *tobj = NULL;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					"glGetTexLevelParameter called in begin/end");
		return;
	}

	if (level < 0 && level > t->maxLevel)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
					"glGetTexLevelParameter: level oob: %d", level);
		return;
	}

	switch (target) 
	{
		case GL_TEXTURE_1D:
			tobj = t->currentTexture1D;
			break;
		case GL_TEXTURE_2D:
			tobj = t->currentTexture2D;
			break;
#if 0
		case GL_TEXTURE_3D:
			tobj = t->currentTexture3D;
			break;
#endif
		default: 
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"GetTexLevelParameter: invalid target: %d", target);
			return;
	}

	switch (pname) 
	{
		case GL_TEXTURE_WIDTH:
			*params = (GLint) tobj->level[level].width;
			break;
		case GL_TEXTURE_HEIGHT:
			*params = (GLint) tobj->level[level].height;
			break;
		case GL_TEXTURE_COMPONENTS:
			*params = (GLint) tobj->level[level].components;
			break;
		case GL_TEXTURE_BORDER:
			*params = (GLint) tobj->level[level].border;
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"GetTexLevelParameter: invalid pname: %d", pname);
			return;
	}
}

void STATE_APIENTRY crStateGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRTextureObj *tobj = NULL;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					"glGetTexParameter called in begin/end");
		return;
	}

	switch (target) 
	{
		case GL_TEXTURE_1D:
			tobj = t->currentTexture1D;
			break;
		case GL_TEXTURE_2D:
			tobj = t->currentTexture2D;
			break;
#if 0
		case GL_TEXTURE_3D:
			tobj = t->currentTexture3D;
			break;
#endif
		default: 
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"glGetTexParameter: invalid target: %d", target);
			return;
	}

	switch (pname) 
	{
		case GL_TEXTURE_MAG_FILTER:
			*params = (GLfloat) tobj->magFilter;
			break;
		case GL_TEXTURE_MIN_FILTER:
			*params = (GLfloat) tobj->minFilter;
			break;
		case GL_TEXTURE_WRAP_S:
			*params = (GLfloat) tobj->wrapS;
			break;
		case GL_TEXTURE_WRAP_T:
			*params = (GLfloat) tobj->wrapT;
			break;
		case GL_TEXTURE_BORDER_COLOR:
			params[0] = tobj->borderColor.r;
			params[1] = tobj->borderColor.g;
			params[2] = tobj->borderColor.b;
			params[3] = tobj->borderColor.a;
			break;
		default:
			if (!crStateGetTexParameterfvExtensions( tobj, pname, params ))
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexParameter: invalid pname: %d", pname);
			}
			return;
	}
}

void STATE_APIENTRY crStateGetTexParameteriv (GLenum target, GLenum pname, GLint *params) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRTextureObj *tobj = NULL;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					"glGetTexParameter called in begin/end");
		return;
	}

	switch (target) 
	{
		case GL_TEXTURE_1D:
			tobj = t->currentTexture1D;
			break;
		case GL_TEXTURE_2D:
			tobj = t->currentTexture2D;
			break;
#if 0
		case GL_TEXTURE_3D:
			tobj = t->currentTexture3D;
			break;
#endif
		default: 
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"glGetTexParameter: invalid target: %d", target);
			return;
	}

	switch (pname) 
	{
		case GL_TEXTURE_MAG_FILTER:
			*params = (GLint) tobj->magFilter;
			break;
		case GL_TEXTURE_MIN_FILTER:
			*params = (GLint) tobj->minFilter;
			break;
		case GL_TEXTURE_WRAP_S:
			*params = (GLint) tobj->wrapS;
			break;
		case GL_TEXTURE_WRAP_T:
			*params = (GLint) tobj->wrapT;
			break;
		case GL_TEXTURE_BORDER_COLOR:
			params[0] = (GLint) (tobj->borderColor.r * GL_MAXINT);
			params[1] = (GLint) (tobj->borderColor.g * GL_MAXINT);
			params[2] = (GLint) (tobj->borderColor.b * GL_MAXINT);
			params[3] = (GLint) (tobj->borderColor.a * GL_MAXINT);
			break;
		default:
			if (!crStateGetTexParameterivExtensions( tobj, pname, params ))
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, 
							"glGetTexParameter: invalid pname: %d", pname);
			}
			return;
	}
}

// TODO:
void STATE_APIENTRY crStatePrioritizeTextures (GLsizei n, const GLuint * textures, const GLclampf * priorities) 
{
	UNUSED(n);
	UNUSED(textures);
	UNUSED(priorities);
	return;
}

GLboolean STATE_APIENTRY crStateAreTexturesResident(GLsizei n, const GLuint * textures, GLboolean * residences) 
{
	UNUSED(n);
	UNUSED(textures);
	UNUSED(residences);
	return GL_TRUE;
}

void crStateTextureSwitch(CRTextureBits *t, GLbitvalue bitID, 
						  CRTextureState *from, CRTextureState *to) 
{
	GLbitvalue nbitID = ~bitID;

	if (t->enable & bitID) {
		glAble able[2];
		able[0] = diff_api.Disable;
		able[1] = diff_api.Enable;
		if (from->enabled1D != to->enabled1D) {
			able[to->enabled1D](GL_TEXTURE_1D);
			t->enable = GLBITS_ONES;
			t->dirty = GLBITS_ONES;
		}
		if (from->enabled2D != to->enabled2D) {
			able[to->enabled2D](GL_TEXTURE_2D);
			t->enable = GLBITS_ONES;
			t->dirty = GLBITS_ONES;
		}
#if 0
		if (from->enabled3D != to->enabled3D) {
			able[to->enabled3D](GL_TEXTURE_3D);
			t->enable = GLBITS_ONES;
			t->dirty = GLBITS_ONES;
		}
#endif
		if (from->textureGen.s != to->textureGen.s ||
			from->textureGen.t != to->textureGen.t ||
			from->textureGen.p != to->textureGen.p ||
			from->textureGen.q != to->textureGen.q) {
			able[to->textureGen.s](GL_TEXTURE_GEN_S);
			able[to->textureGen.t](GL_TEXTURE_GEN_T);
			able[to->textureGen.p](GL_TEXTURE_GEN_R);
			able[to->textureGen.q](GL_TEXTURE_GEN_Q);
			t->enable = GLBITS_ONES;
			t->dirty = GLBITS_ONES;
		}
		t->enable &= nbitID;
	}

	/* 
	**  A thought on switching with textures:
	**  Since we are only performing a switch
	**  and not a sync, we won't need to 
	**  update individual textures, just
	**  the bindings....
	*/

	if (t->current & bitID) {
		if (from->currentTexture1DName != to->currentTexture1DName) {
			diff_api.BindTexture(GL_TEXTURE_1D, to->currentTexture1DName);
			t->current = GLBITS_ONES;
			t->dirty = GLBITS_ONES;
		}
		if (from->currentTexture2DName != to->currentTexture2DName) {
			diff_api.BindTexture(GL_TEXTURE_2D, to->currentTexture2DName);
			t->current = GLBITS_ONES;
			t->dirty = GLBITS_ONES;
		}
#if 0
		if (from->currenttexture3dname != to->currenttexture3dname) {
			diff_api.BindTexture(GL_TEXTURE_3D, to->currenttexture3dname);
			t->current = GLBITS_ONES;
			t->dirty = GLBITS_ONES;
		}
#endif
	}

	/* Texture enviroment */
	if (t->envBit & bitID) {
		if (from->envMode != to->envMode) {
			diff_api.TexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, to->envMode);
			t->envBit = GLBITS_ONES;
			t->dirty = GLBITS_ONES;
		}
		if (from->envColor.r != to->envColor.r &&
			from->envColor.g != to->envColor.g &&
			from->envColor.b != to->envColor.b &&
			from->envColor.a != to->envColor.a) {
			GLfloat f[4];
			f[0] = to->envColor.r;
			f[1] = to->envColor.g;
			f[2] = to->envColor.b;
			f[3] = to->envColor.a;
			diff_api.TexEnvfv (GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, (const GLfloat *) f);
			t->envBit = GLBITS_ONES;
			t->dirty = GLBITS_ONES;
		}
		t->envBit &= nbitID;
	}

	if (t->objGen & bitID) {
		if (from->objSCoeff.x != to->objSCoeff.x ||
			from->objSCoeff.y != to->objSCoeff.y ||
			from->objSCoeff.z != to->objSCoeff.z ||
			from->objSCoeff.w != to->objSCoeff.w) {
			GLfloat f[4];
			f[0] = to->objSCoeff.x;
			f[1] = to->objSCoeff.y;
			f[2] = to->objSCoeff.z;
			f[3] = to->objSCoeff.w;
			diff_api.TexGenfv (GL_S, GL_OBJECT_PLANE, (const GLfloat *) f);
			t->objGen = GLBITS_ONES;
			t->dirty = GLBITS_ONES;
		}
		if (from->objTCoeff.x != to->objTCoeff.x ||
			from->objTCoeff.y != to->objTCoeff.y ||
			from->objTCoeff.z != to->objTCoeff.z ||
			from->objTCoeff.w != to->objTCoeff.w) {
			GLfloat f[4];
			f[0] = to->objTCoeff.x;
			f[1] = to->objTCoeff.y;
			f[2] = to->objTCoeff.z;
			f[3] = to->objTCoeff.w;
			diff_api.TexGenfv (GL_T, GL_OBJECT_PLANE, (const GLfloat *) f);
			t->objGen = GLBITS_ONES;
			t->dirty = GLBITS_ONES;
		}
		if (from->objRCoeff.x != to->objRCoeff.x ||
			from->objRCoeff.y != to->objRCoeff.y ||
			from->objRCoeff.z != to->objRCoeff.z ||
			from->objRCoeff.w != to->objRCoeff.w) {
			GLfloat f[4];
			f[0] = to->objRCoeff.x;
			f[1] = to->objRCoeff.y;
			f[2] = to->objRCoeff.z;
			f[3] = to->objRCoeff.w;
			diff_api.TexGenfv (GL_R, GL_OBJECT_PLANE, (const GLfloat *) f);
			t->objGen = GLBITS_ONES;
			t->dirty = GLBITS_ONES;
		}
		if (from->objQCoeff.x != to->objQCoeff.x ||
			from->objQCoeff.y != to->objQCoeff.y ||
			from->objQCoeff.z != to->objQCoeff.z ||
			from->objQCoeff.w != to->objQCoeff.w) {
			GLfloat f[4];
			f[0] = to->objQCoeff.x;
			f[1] = to->objQCoeff.y;
			f[2] = to->objQCoeff.z;
			f[3] = to->objQCoeff.w;
			diff_api.TexGenfv (GL_Q, GL_OBJECT_PLANE, (const GLfloat *) f);
			t->objGen = GLBITS_ONES;
			t->dirty = GLBITS_ONES;
		}
		t->objGen &= nbitID;
	}
	if (t->eyeGen & bitID) {
		diff_api.MatrixMode(GL_MODELVIEW);
		diff_api.PushMatrix();
		diff_api.LoadIdentity();
		if (from->eyeSCoeff.x != to->eyeSCoeff.x ||
			from->eyeSCoeff.y != to->eyeSCoeff.y ||
			from->eyeSCoeff.z != to->eyeSCoeff.z ||
			from->eyeSCoeff.w != to->eyeSCoeff.w) {
			GLfloat f[4];
			f[0] = to->eyeSCoeff.x;
			f[1] = to->eyeSCoeff.y;
			f[2] = to->eyeSCoeff.z;
			f[3] = to->eyeSCoeff.w;
			diff_api.TexGenfv (GL_S, GL_EYE_PLANE, (const GLfloat *) f);
			t->eyeGen = GLBITS_ONES;
			t->dirty = GLBITS_ONES;
		}
		if (from->eyeTCoeff.x != to->eyeTCoeff.x ||
			from->eyeTCoeff.y != to->eyeTCoeff.y ||
			from->eyeTCoeff.z != to->eyeTCoeff.z ||
			from->eyeTCoeff.w != to->eyeTCoeff.w) {
			GLfloat f[4];
			f[0] = to->eyeTCoeff.x;
			f[1] = to->eyeTCoeff.y;
			f[2] = to->eyeTCoeff.z;
			f[3] = to->eyeTCoeff.w;
			diff_api.TexGenfv (GL_T, GL_EYE_PLANE, (const GLfloat *) f);
			t->eyeGen = GLBITS_ONES;
			t->dirty = GLBITS_ONES;
		}
		if (from->eyeRCoeff.x != to->eyeRCoeff.x ||
			from->eyeRCoeff.y != to->eyeRCoeff.y ||
			from->eyeRCoeff.z != to->eyeRCoeff.z ||
			from->eyeRCoeff.w != to->eyeRCoeff.w) {
			GLfloat f[4];
			f[0] = to->eyeRCoeff.x;
			f[1] = to->eyeRCoeff.y;
			f[2] = to->eyeRCoeff.z;
			f[3] = to->eyeRCoeff.w;
			diff_api.TexGenfv (GL_R, GL_EYE_PLANE, (const GLfloat *) f);
			t->eyeGen = GLBITS_ONES;
			t->dirty = GLBITS_ONES;
		}
		if (from->eyeQCoeff.x != to->eyeQCoeff.x ||
			from->eyeQCoeff.y != to->eyeQCoeff.y ||
			from->eyeQCoeff.z != to->eyeQCoeff.z ||
			from->eyeQCoeff.w != to->eyeQCoeff.w) {
			GLfloat f[4];
			f[0] = to->eyeQCoeff.x;
			f[1] = to->eyeQCoeff.y;
			f[2] = to->eyeQCoeff.z;
			f[3] = to->eyeQCoeff.w;
			diff_api.TexGenfv (GL_Q, GL_EYE_PLANE, (const GLfloat *) f);
			t->eyeGen = GLBITS_ONES;
			t->dirty = GLBITS_ONES;
		}
		diff_api.PopMatrix();
		t->eyeGen &= nbitID;
	}
	if (t->gen & bitID) {
		if (from->gen.s != to->gen.s ||
			from->gen.t != to->gen.t ||
			from->gen.p != to->gen.p ||
			from->gen.q != to->gen.q) {
			diff_api.TexGeni (GL_S, GL_TEXTURE_GEN_MODE, to->gen.s);
			diff_api.TexGeni (GL_T, GL_TEXTURE_GEN_MODE, to->gen.t);
			diff_api.TexGeni (GL_R, GL_TEXTURE_GEN_MODE, to->gen.p);
			diff_api.TexGeni (GL_Q, GL_TEXTURE_GEN_MODE, to->gen.q);	
			t->gen = GLBITS_ONES;
			t->dirty = GLBITS_ONES;
		}
		t->gen &= nbitID;
	}
	t->dirty &= nbitID;
}

void crStateTextureDiff(CRTextureBits *t, GLbitvalue bitID, 
						 CRTextureState *from, CRTextureState *to) 
{
	GLbitvalue nbitID = ~bitID;
	CRTextureObj *tobj = NULL;
	GLuint name=0;
	GLuint *cname=NULL;

	if (t->enable & bitID) {
		glAble able[2];
		able[0] = diff_api.Disable;
		able[1] = diff_api.Enable;
		if (from->enabled1D != to->enabled1D) {
			able[to->enabled1D](GL_TEXTURE_1D);
			from->enabled1D = to->enabled1D;
		}
		if (from->enabled2D != to->enabled2D) {
			able[to->enabled2D](GL_TEXTURE_2D);
			from->enabled2D = to->enabled2D;
		}
#if 0
		if (from->enabled3D != to->enabled3D) {
			able[to->enabled3D](GL_TEXTURE_3D);
			from->enabled3D = to->enabled3D;
		}
#endif
		if (from->textureGen.s != to->textureGen.s ||
			from->textureGen.t != to->textureGen.t ||
			from->textureGen.p != to->textureGen.p ||
			from->textureGen.q != to->textureGen.q) {
			able[to->textureGen.s](GL_TEXTURE_GEN_S);
			able[to->textureGen.t](GL_TEXTURE_GEN_T);
			able[to->textureGen.p](GL_TEXTURE_GEN_R);
			able[to->textureGen.q](GL_TEXTURE_GEN_Q);
			from->textureGen = to->textureGen;
		}
		t->enable &= nbitID;
	}

	/* Get the active texture */
	if (to->enabled1D == GL_TRUE) {
		tobj = to->currentTexture1D;
		name = to->currentTexture1DName;
		cname = &(from->currentTexture1DName);
	}

	if (to->enabled2D == GL_TRUE) {
		tobj = to->currentTexture2D;
		name = to->currentTexture2DName;
		cname = &(from->currentTexture2DName);
	}

#if 0
	if (to->enabled3d == GL_TRUE) {
		tobj = to->currenttexture3d;
		name = to->currenttexture3dname;
		cname = &(from->currenttexture3dname);
	}
#endif

	/* Handle texture broadcasting */
	if (tex_broadcast != NULL) {
		tobj = tex_broadcast;
		switch (tex_broadcast->target) {
		case GL_TEXTURE_1D:
			name = to->currentTexture1DName;
			cname = &(from->currentTexture1DName);
			break;
		case GL_TEXTURE_2D:
			name = to->currentTexture2DName;
			cname = &(from->currentTexture2DName);
			break;
#if 0
		case GL_TEXTURE_3D:
			name = to->currenttexture3dname;
			cname = &(from->currenttexture3dname);
			break;
#endif
		default:
			crStateError( __LINE__, __FILE__, GL_INVALID_ENUM, "Bogus target in crStateTextureDiff: %d", tex_broadcast->target );
		}
	}


	/* if texturing is not enabled, return 
	** note that we leave the dirty bits set
	*/
	if (!tobj) {
		t->dirty &= nbitID;
		return;
	}

	if (t->current & bitID) {
		if (*cname != name) {
			diff_api.BindTexture(tobj->target, name);
			*cname = name;
		}
		t->current &= nbitID;
	}

	/*
	** Texture Restore 
	** Since textures are allocated objects
	** it seems wastefull to allocate each one
	** on all the pipes.
	** So instead, we'll skip the value compare
	** and just use the bit test.
	*/

	if (tobj->dirty & bitID) {
			
		if (tobj->paramsBit & bitID) {
			GLfloat f[4];
			f[0] = tobj->borderColor.r;
			f[1] = tobj->borderColor.g;
			f[2] = tobj->borderColor.b;
			f[3] = tobj->borderColor.a;
			diff_api.TexParameteri(tobj->target, GL_TEXTURE_MIN_FILTER, tobj->minFilter);
			diff_api.TexParameteri(tobj->target, GL_TEXTURE_MAG_FILTER, tobj->magFilter);
			diff_api.TexParameteri(tobj->target, GL_TEXTURE_WRAP_S, tobj->wrapS);
			diff_api.TexParameteri(tobj->target, GL_TEXTURE_WRAP_T, tobj->wrapT);
			diff_api.TexParameterfv(tobj->target, GL_TEXTURE_BORDER_COLOR, (const GLfloat *) f);
			crStateTextureDiffParameterExtensions( tobj );
			tobj->paramsBit &= nbitID;
		}

		if (tobj->imageBit & bitID) {
			int i;
			for (i=0; i<to->maxLevel; i++) {
				CRTextureLevel *tl = &(tobj->level[i]);
				if (tl->dirty & bitID) {
					switch (tobj->target) {
					case GL_TEXTURE_1D:
						diff_api.TexImage1D(tobj->target, i, tl->components,
											tl->width, tl->border, tl->format,
											tl->type, tl->img);
						break;
					case GL_TEXTURE_2D:
						diff_api.TexImage2D(tobj->target, i, tl->components,
											tl->width, tl->height, tl->border,
											tl->format, tl->type, tl->img);
						break;
#if 0
					case GL_TEXTURE_3D:
						diff_api.TexImage3D(tobj->target, i, tl->components,
											tl->width, tl->height, tl->depth,
											tl->border, tl->format, tl->type,
											tl->img);

						break;
#endif
					default:
						UNIMPLEMENTED();
					}	
					tl->dirty &= nbitID;
				}
			}
			tobj->imageBit &= nbitID;
		}
		tobj->dirty &= nbitID;
	}

	/* Texture enviroment */
	if (t->envBit & bitID) {
		if (from->envMode != to->envMode) {
			diff_api.TexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, to->envMode);
			from->envMode = to->envMode;
		}
		if (from->envColor.r != to->envColor.r &&
			from->envColor.g != to->envColor.g &&
			from->envColor.b != to->envColor.b &&
			from->envColor.a != to->envColor.a) {
			GLfloat f[4];
			f[0] = to->envColor.r;
			f[1] = to->envColor.g;
			f[2] = to->envColor.b;
			f[3] = to->envColor.a;
			diff_api.TexEnvfv (GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, (const GLfloat *) f);
			from->envColor = to->envColor;
		}
		t->envBit &= nbitID;
	}

	if (t->objGen & bitID) {
		if (from->objSCoeff.x != to->objSCoeff.x ||
			from->objSCoeff.y != to->objSCoeff.y ||
			from->objSCoeff.z != to->objSCoeff.z ||
			from->objSCoeff.w != to->objSCoeff.w) {
			GLfloat f[4];
			f[0] = to->objSCoeff.x;
			f[1] = to->objSCoeff.y;
			f[2] = to->objSCoeff.z;
			f[3] = to->objSCoeff.w;
			diff_api.TexGenfv (GL_S, GL_OBJECT_PLANE, (const GLfloat *) f);
			from->objSCoeff = to->objSCoeff;
		}
		if (from->objTCoeff.x != to->objTCoeff.x ||
			from->objTCoeff.y != to->objTCoeff.y ||
			from->objTCoeff.z != to->objTCoeff.z ||
			from->objTCoeff.w != to->objTCoeff.w) {
			GLfloat f[4];
			f[0] = to->objTCoeff.x;
			f[1] = to->objTCoeff.y;
			f[2] = to->objTCoeff.z;
			f[3] = to->objTCoeff.w;
			diff_api.TexGenfv (GL_T, GL_OBJECT_PLANE, (const GLfloat *) f);
			from->objTCoeff = to->objTCoeff;
		}
		if (from->objRCoeff.x != to->objRCoeff.x ||
			from->objRCoeff.y != to->objRCoeff.y ||
			from->objRCoeff.z != to->objRCoeff.z ||
			from->objRCoeff.w != to->objRCoeff.w) {
			GLfloat f[4];
			f[0] = to->objRCoeff.x;
			f[1] = to->objRCoeff.y;
			f[2] = to->objRCoeff.z;
			f[3] = to->objRCoeff.w;
			diff_api.TexGenfv (GL_R, GL_OBJECT_PLANE, (const GLfloat *) f);
			from->objRCoeff = to->objRCoeff;
		}
		if (from->objQCoeff.x != to->objQCoeff.x ||
			from->objQCoeff.y != to->objQCoeff.y ||
			from->objQCoeff.z != to->objQCoeff.z ||
			from->objQCoeff.w != to->objQCoeff.w) {
			GLfloat f[4];
			f[0] = to->objQCoeff.x;
			f[1] = to->objQCoeff.y;
			f[2] = to->objQCoeff.z;
			f[3] = to->objQCoeff.w;
			diff_api.TexGenfv (GL_Q, GL_OBJECT_PLANE, (const GLfloat *) f);
			from->objQCoeff = to->objQCoeff;
		}
		t->objGen &= nbitID;
	}
	if (t->eyeGen & bitID) {
		diff_api.MatrixMode(GL_MODELVIEW);
		diff_api.PushMatrix();
		diff_api.LoadIdentity();
		if (from->eyeSCoeff.x != to->eyeSCoeff.x ||
			from->eyeSCoeff.y != to->eyeSCoeff.y ||
			from->eyeSCoeff.z != to->eyeSCoeff.z ||
			from->eyeSCoeff.w != to->eyeSCoeff.w) {
			GLfloat f[4];
			f[0] = to->eyeSCoeff.x;
			f[1] = to->eyeSCoeff.y;
			f[2] = to->eyeSCoeff.z;
			f[3] = to->eyeSCoeff.w;
			diff_api.TexGenfv (GL_S, GL_EYE_PLANE, (const GLfloat *) f);
			from->eyeSCoeff = to->eyeSCoeff;
		}
		if (from->eyeTCoeff.x != to->eyeTCoeff.x ||
			from->eyeTCoeff.y != to->eyeTCoeff.y ||
			from->eyeTCoeff.z != to->eyeTCoeff.z ||
			from->eyeTCoeff.w != to->eyeTCoeff.w) {
			GLfloat f[4];
			f[0] = to->eyeTCoeff.x;
			f[1] = to->eyeTCoeff.y;
			f[2] = to->eyeTCoeff.z;
			f[3] = to->eyeTCoeff.w;
			diff_api.TexGenfv (GL_T, GL_EYE_PLANE, (const GLfloat *) f);
			from->eyeTCoeff = to->eyeTCoeff;
		}
		if (from->eyeRCoeff.x != to->eyeRCoeff.x ||
			from->eyeRCoeff.y != to->eyeRCoeff.y ||
			from->eyeRCoeff.z != to->eyeRCoeff.z ||
			from->eyeRCoeff.w != to->eyeRCoeff.w) {
			GLfloat f[4];
			f[0] = to->eyeRCoeff.x;
			f[1] = to->eyeRCoeff.y;
			f[2] = to->eyeRCoeff.z;
			f[3] = to->eyeRCoeff.w;
			diff_api.TexGenfv (GL_R, GL_EYE_PLANE, (const GLfloat *) f);
			from->eyeRCoeff = to->eyeRCoeff;
		}
		if (from->eyeQCoeff.x != to->eyeQCoeff.x ||
			from->eyeQCoeff.y != to->eyeQCoeff.y ||
			from->eyeQCoeff.z != to->eyeQCoeff.z ||
			from->eyeQCoeff.w != to->eyeQCoeff.w) {
			GLfloat f[4];
			f[0] = to->eyeQCoeff.x;
			f[1] = to->eyeQCoeff.y;
			f[2] = to->eyeQCoeff.z;
			f[3] = to->eyeQCoeff.w;
			diff_api.TexGenfv (GL_Q, GL_EYE_PLANE, (const GLfloat *) f);
			from->eyeQCoeff = to->eyeQCoeff;
		}
		diff_api.PopMatrix();
		t->eyeGen &= nbitID;
	}
	if (t->gen & bitID) {
		if (from->gen.s != to->gen.s ||
			from->gen.t != to->gen.t ||
			from->gen.p != to->gen.p ||
			from->gen.q != to->gen.q) {
			diff_api.TexGeni (GL_S, GL_TEXTURE_GEN_MODE, to->gen.s);
			diff_api.TexGeni (GL_T, GL_TEXTURE_GEN_MODE, to->gen.t);
			diff_api.TexGeni (GL_R, GL_TEXTURE_GEN_MODE, to->gen.p);
			diff_api.TexGeni (GL_Q, GL_TEXTURE_GEN_MODE, to->gen.q);	
			from->gen = to->gen;
		}
		t->gen &= nbitID;
	}
	t->dirty &= nbitID;
}
