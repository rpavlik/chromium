/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include "cr_glstate.h"
#include "state/cr_statetypes.h"
#include "cr_pixeldata.h"
#include "cr_mem.h"
#include "state_internals.h"

#define UNIMPLEMENTED() crStateError(__LINE__,__FILE__,GL_INVALID_OPERATION, "Unimplemented something or other" )
#define UNUSED(x) ((void) (x))

#define GET_TOBJ(tobj,state,id) 	for (tobj = state->mapping[id%CRTEXTURE_HASHSIZE]; tobj && tobj->name != id; tobj = tobj->next){}

void crStateTextureInitTextureObj (CRTextureState *t, CRTextureObj *tobj, GLuint name, GLenum target);

CRTextureObj *crStateTextureAllocate_t(CRTextureState *t, GLuint name);

void crStateTextureDelete_t(CRTextureState *t, GLuint name);

void crStateTextureInit(const CRLimitsState *limits, CRTextureState *t) 
{
	int i, h;
	unsigned int a;
	GLvectorf zero_vector = {0.0f, 0.0f, 0.0f, 0.0f};
	GLcolorf zero_color = {0.0f, 0.0f, 0.0f, 0.0f};
	GLvectorf x_vector = {1.0f, 0.0f, 0.0f, 0.0f};
	GLvectorf y_vector = {0.0f, 1.0f, 0.0f, 0.0f};

	for (i=0, a=limits->maxTextureSize; a; i++, a=a>>1);
	t->maxLevel = i;
	for (i=0, a=limits->max3DTextureSize; a; i++, a=a>>1);
	t->max3DLevel = i;
#ifdef CR_ARB_texture_cube_map
	for (i=0, a=limits->maxCubeMapTextureSize; a; i++, a=a>>1);
	t->maxCubeMapLevel = i;
#endif

	t->allocated = 1;

	t->textures = (CRTextureObj *) crAlloc (sizeof (CRTextureObj));
	crStateTextureInitTextureObj(t, t->textures, 0, GL_NONE);
	t->firstFree = t->textures;
	t->firstFree->next = NULL;

	/* Initalize the hash table to empty */
	for (h=0; h<CRTEXTURE_HASHSIZE; h++)
	{
		t->mapping[h] = NULL;
	}

	t->freeList = (CRTextureFreeElem *) crAlloc (sizeof(CRTextureFreeElem));
	t->freeList->min = 1;
	t->freeList->max = GL_MAXUINT;
	t->freeList->next = NULL;
	t->freeList->prev = NULL;
	t->currentTexture1D = &(t->base1D);
	t->currentTexture2D = &(t->base2D);
	t->currentTexture3D = &(t->base3D);
	t->currentTextureCubeMap = &(t->baseCubeMap);

	crStateTextureInitTextureObj(t, &(t->base1D), 0, GL_TEXTURE_1D);
	crStateTextureInitTextureObj(t, &(t->base2D), 0, GL_TEXTURE_2D);
#ifdef CR_OPENGL_VERSION_1_2
	crStateTextureInitTextureObj(t, &(t->base3D), 0, GL_TEXTURE_3D);
#endif
#ifdef CR_ARB_texture_cube_map
	crStateTextureInitTextureObj(t, &(t->baseCubeMap), 0,
		GL_TEXTURE_CUBE_MAP_ARB);
#endif

	crStateTextureInitTextureObj(t, &(t->proxy1D), 0, GL_TEXTURE_1D);
	crStateTextureInitTextureObj(t, &(t->proxy2D), 0, GL_TEXTURE_2D);
#ifdef CR_OPENGL_VERSION_1_2
	crStateTextureInitTextureObj(t, &(t->proxy3D), 0, GL_TEXTURE_3D);
#endif
#ifdef CR_ARB_texture_cube_map
	crStateTextureInitTextureObj(t, &(t->proxyCubeMap), 0,
		GL_TEXTURE_CUBE_MAP_ARB);
#endif

	t->curTextureUnit = 0;

	/* Per-unit initialization */
	for ( i = 0 ; i < CR_MAX_TEXTURE_UNITS ; i++)
	{
		t->unit[i].currentTexture1DName = 0;
		t->unit[i].currentTexture2DName = 0;
		t->unit[i].currentTexture3DName = 0;
		t->unit[i].currentTextureCubeMapName = 0;

		t->unit[i].enabled1D = GL_FALSE;
		t->unit[i].enabled2D = GL_FALSE;
		t->unit[i].enabled3D = GL_FALSE;
		t->unit[i].enabledCubeMap = GL_FALSE;
		t->unit[i].textureGen.s = GL_FALSE;
		t->unit[i].textureGen.t = GL_FALSE;
		t->unit[i].textureGen.r = GL_FALSE;
		t->unit[i].textureGen.q = GL_FALSE;

		t->unit[i].gen.s = GL_EYE_LINEAR;
		t->unit[i].gen.t = GL_EYE_LINEAR;
		t->unit[i].gen.r = GL_EYE_LINEAR;
		t->unit[i].gen.q = GL_EYE_LINEAR;

		t->unit[i].objSCoeff = x_vector;
		t->unit[i].objTCoeff = y_vector;
		t->unit[i].objRCoeff = zero_vector;
		t->unit[i].objQCoeff = zero_vector;

		t->unit[i].eyeSCoeff = x_vector;
		t->unit[i].eyeTCoeff = y_vector;
		t->unit[i].eyeRCoeff = zero_vector;
		t->unit[i].eyeQCoeff = zero_vector;
		t->unit[i].envMode = GL_MODULATE;
		t->unit[i].envColor = zero_color;
	}
}

void crStateTextureInitTextureObj(CRTextureState *t, CRTextureObj *tobj, GLuint name, GLenum target)
{
	int i;
	int j;
	CRTextureLevel *tl;

	tobj->borderColor.r = 0.0f;
	tobj->borderColor.g = 0.0f;
	tobj->borderColor.b = 0.0f;
	tobj->borderColor.a = 0.0f;
	tobj->minFilter     = GL_NEAREST_MIPMAP_LINEAR;
	tobj->magFilter     = GL_LINEAR;
	tobj->wrapS         = GL_REPEAT;
	tobj->wrapT         = GL_REPEAT;
#ifdef CR_OPENGL_VERSION_1_2
	tobj->wrapR         = GL_REPEAT;
#endif
	tobj->target        = target;
	tobj->name          = name;

#define INIT_LEVELS(ARRAY)											\
	tobj->ARRAY = (CRTextureLevel *) crAlloc(sizeof(CRTextureLevel)	\
												 * t->maxLevel);	\
	if (!tobj->ARRAY)												\
		return; /* out of memory */									\
	for (i=0; i<t->maxLevel; i++)									\
	{																\
		tl                = &(tobj->ARRAY[i]);						\
		tl->bytes         = 0;										\
		tl->img           = NULL;									\
		tl->width         = 0;										\
		tl->height        = 0;										\
		tl->depth         = 0;										\
		tl->border        = 0;										\
		tl->components    = 1;										\
		tl->bytesPerPixel = 0;										\
		tl->format        = GL_RGBA;								\
		tl->type          = GL_UNSIGNED_BYTE;						\
		for (j = 0 ; j < CR_MAX_TEXTURE_UNITS; j++)					\
		{															\
			tl->dirty[j]     = 0;  /* By default this level is ignored.*/	\
		}																	\
	}

	INIT_LEVELS(level);

#ifdef CR_ARB_texture_cube_map
	INIT_LEVELS(negativeXlevel);
	INIT_LEVELS(positiveYlevel);
	INIT_LEVELS(negativeYlevel);
	INIT_LEVELS(positiveZlevel);
	INIT_LEVELS(negativeZlevel);
#endif

#ifdef CR_EXT_texture_filter_anisotropic
	tobj->maxAnisotropy = 1.0f;
#endif

	/* UGh. Should be neg_bitid */
	tobj->dirty = GLBITS_ONES;
	for (i = 0 ; i < CR_MAX_TEXTURE_UNITS; i++)
	{
		tobj->paramsBit[i] = GLBITS_ONES;
		tobj->imageBit[i] = GLBITS_ONES;
	}
}

#if 0
void crStateTextureInitTexture (GLuint name) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRTextureObj *tobj;

	GET_TOBJ(tobj, name);
	if (!tobj) return;

	crStateTextureInitTextureObj(t, tobj, name, GL_NONE);
}

CRTextureObj * crStateTextureGet(GLuint name) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRTextureObj *tobj;

	GET_TOBJ(tobj, name);

	return tobj;
}
#endif

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
		crStateTextureInitTextureObj(t, tobj, name, GL_NONE);

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
#ifdef CR_OPENGL_VERSION_1_2
		if (t->currentTexture3D != &(t->base3D)) 
		{
			t->currentTexture3D = t->textures + (t->currentTexture3D - tobj);
		}
#endif
#ifdef CR_ARB_texture_cube_map
		if (t->currentTextureCubeMap != &(t->baseCubeMap))
		{
			t->currentTextureCubeMap = t->textures + (t->currentTextureCubeMap - tobj);
		}
#endif
	}

	/* Update the free list */
	t->firstFree = t->textures + t->allocated;
	for (i=t->allocated; i < t->allocated*2; i++) 
	{
		crStateTextureInitTextureObj(t, t->textures+i, 0, GL_NONE);
		t->textures[i].next = t->textures+i+1;
	}

	t->textures[t->allocated*2-1].next = NULL;
	t->allocated*=2;

	/* Call function again. Gotta love that tail recursion! */
	return crStateTextureAllocate_t(t, name);
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
		CRASSERT( textures[i] );
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
			if (name == t->unit[t->curTextureUnit].currentTexture1DName) 
			{
				t->currentTexture1D = &(t->base1D);
				t->unit[t->curTextureUnit].currentTexture1DName = 0;
			}
			if (name == t->unit[t->curTextureUnit].currentTexture2DName) 
			{
				t->currentTexture2D = &(t->base2D);
				t->unit[t->curTextureUnit].currentTexture2DName = 0;
			}
#ifdef CR_OPENGL_VERSION_1_2
			if (name == t->currentTexture3DName[t->curTextureUnit]) 
			{
				t->currentTexture3D = &(t->base3D);
				t->unit[t->curTextureUnit].currentTexture3DName = 0;
			}
#endif
#ifdef CR_ARB_texture_cube_map
			if (name == t->unit[t->curTextureUnit].currentTextureCubeMapName)
			{
				t->currentTextureCubeMap = &(t->baseCubeMap);
				t->unit[t->curTextureUnit].currentTextureCubeMapName = 0;
			}
#endif
		}
	}

	tb->dirty = g->neg_bitid;
	tb->current[t->curTextureUnit] = g->neg_bitid;
}



void STATE_APIENTRY crStateClientActiveTextureARB( GLenum texture )
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);

	if (!g->extensions.ARB_multitexture) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glClientActiveTextureARB not available");
		return;
	}

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glClientActiveTextureARB called in Begin/End");
		return;
	}

	if ( texture < GL_TEXTURE0_ARB || texture >= GL_TEXTURE0_ARB + CR_MAX_TEXTURE_UNITS)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "Bad texture unit passed to crStateClientActiveTexture: %d (max is %d)", texture, CR_MAX_TEXTURE_UNITS );
		return;
	}

	FLUSH();

	c->curClientTextureUnit = texture - GL_TEXTURE0_ARB;
}

void STATE_APIENTRY crStateActiveTextureARB( GLenum texture )
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);

	if (!g->extensions.ARB_multitexture) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glActiveTextureARB not available");
		return;
	}

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glActiveTextureARB called in Begin/End");
		return;
	}

	if ( texture < GL_TEXTURE0_ARB || texture >= GL_TEXTURE0_ARB + CR_MAX_TEXTURE_UNITS)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "Bad texture unit passed to crStateActiveTexture: %d (max is %d)", texture, CR_MAX_TEXTURE_UNITS );
		return;
	}

	FLUSH();

	t->curTextureUnit = texture - GL_TEXTURE0_ARB;
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
				t->unit[t->curTextureUnit].currentTexture1DName = 0;
				break;
			case GL_TEXTURE_2D:
				t->currentTexture2D = &(t->base2D);
				t->unit[t->curTextureUnit].currentTexture2DName = 0;
				break;
#ifdef CR_OPENGL_VERSION_1_2
			case GL_TEXTURE_3D:
				t->currentTexture3D[t->curTextureUnit] = &(t->base3D);
				t->unit[t->curTextureUnit].currentTexture3DName = 0;
				break;
#endif
#ifdef CR_ARB_texture_cube_map
			case GL_TEXTURE_CUBE_MAP_ARB:
				if (!g->extensions.ARB_texture_cube_map) {
					crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"Invalid target passed to glBindTexture: %d", target);
					return;
				}
				t->currentTextureCubeMap = &(t->baseCubeMap);
				t->unit[t->curTextureUnit].currentTextureCubeMapName = 0;
				break;
#endif
			default:
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "Invalid target passed to glBindTexture: %d", target);
				return;
		}

		tb->dirty = g->neg_bitid;
		tb->current[t->curTextureUnit] = g->neg_bitid;
		return;
	}

	/* texture != 0 */
	/* Get the texture */
	GET_TOBJ(tobj, t, texture);
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
	if (tobj->target != target)
	{
		crWarning( "You called glBindTexture with a target of 0x%x, but the texture you wanted was target 0x%x", target, tobj->target );
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "Attempt to bind a texture of diffent dimenions");
		return;
	}

	/* Set the current texture */
	switch (target) 
	{
		case GL_TEXTURE_1D:
			t->currentTexture1D = tobj;
			t->unit[t->curTextureUnit].currentTexture1DName = texture;
			break;
		case GL_TEXTURE_2D:
			t->currentTexture2D = tobj;
			t->unit[t->curTextureUnit].currentTexture2DName = texture;
			break;
#ifdef CR_OPENGL_VERSION_1_2
		case GL_TEXTURE_3D:
			t->currentTexture3D[t->curTextureUnit] = tobj;
			t->unit[t->curTextureUnit].currentTexture3DName = texture;
			break;
#endif
#ifdef CR_ARB_texture_cube_map
		case GL_TEXTURE_CUBE_MAP_ARB:
			t->currentTextureCubeMap = tobj;
			t->unit[t->curTextureUnit].currentTextureCubeMapName = texture;
			break;
#endif
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "Invalid target passed to glBindTexture: %d", target);
			return;
	}

	tb->dirty = g->neg_bitid;
	tb->current[t->curTextureUnit] = g->neg_bitid;
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

	if (target != GL_TEXTURE_1D && target != GL_PROXY_TEXTURE_1D)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glTexImage1D target != (GL_TEXTURE_1D | GL_PROXY_TEXTURE_1D): 0x%x", target);
		return;
	}

	if (level < 0 || level > t->maxLevel) 
	{
		if (target == GL_PROXY_TEXTURE_1D)
		{
			/* clear all the texture object state */
			crStateTextureInitTextureObj(t, &(t->proxy1D), 0, GL_TEXTURE_1D);
		}
		else
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
				"glTexImage1D level oob: %d", level);
		}
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
		if (target == GL_PROXY_TEXTURE_1D)
		{
			/* clear all the texture object state */
			crStateTextureInitTextureObj(t, &(t->proxy1D), 0, GL_TEXTURE_1D);
		}
		else
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
				"glTexImage1D border oob: %d", border);
		}
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
		if (target == GL_PROXY_TEXTURE_1D)
		{
			/* clear all the texture object state */
			crStateTextureInitTextureObj(t, &(t->proxy1D), 0, GL_TEXTURE_1D);
		}
		else
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
				"glTexImage1D width is not valid: %d", width);
		}
		return;
	}

	if (width > (int) g->limits.maxTextureSize)
	{
		if (target == GL_PROXY_TEXTURE_1D)
		{
			/* clear all the texture object state */
			crStateTextureInitTextureObj(t, &(t->proxy1D), 0, GL_TEXTURE_1D);
		}
		else
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
				"glTexImage1D width oob: %d", width);
		}
		return;
	}

	if (target == GL_PROXY_TEXTURE_1D)
	{
		tobj = t->currentTexture1D;
		tl = tobj->level+level;
		tl->bytes = 0;
	}
	else
	{
		assert(target == GL_TEXTURE_1D);
		tobj = t->currentTexture1D;
		tobj->target = GL_TEXTURE_1D;
		tl = tobj->level+level;
		tl->bytes = crPixelSize(format, type, width, 1);
	}

	if (tl->bytes)
	{
		/* this is not a proxy texture target so alloc storage */
		if (tl->img) 
			crFree (tl->img);
		tl->img = (GLubyte *) crAlloc (tl->bytes);
		if (!tl->img)
		{
			crStateError(__LINE__,__FILE__, GL_OUT_OF_MEMORY,
				     "glTexImage1D out of memory");
			return;
		}
		if (pixels) 
			crPixelCopy1D((GLvoid *) tl->img, pixels, format, type,
				width, &(p->unpack));
	}

	tl->width = width;
	tl->height = 1;
	tl->depth = 1;
	tl->format = format;
	tl->border = border;
	tl->components = components;
	tl->type = type;
	tl->bytesPerPixel = tl->bytes / width;

	/* XXX may need to do some fine-tuning here for proxy textures */
	tobj->dirty = g->neg_bitid;
	for (i = 0; i < CR_MAX_TEXTURE_UNITS; i++)
	{
		tl->dirty[i] = g->neg_bitid;
		tobj->imageBit[i] = g->neg_bitid;
	}
	tb->dirty = g->neg_bitid;
}

void STATE_APIENTRY crStateTexImage2D (GLenum target, GLint level, GLint components, 
		GLsizei width, GLsizei height, GLint border,
		GLenum format, GLenum type, const GLvoid *pixels  ) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRPixelState *p = &(g->pixel);
	CRTextureObj *tobj = NULL;
	CRTextureLevel *tl = NULL;
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);
	int i;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glTexImage2D called in Begin/End");
		return;
	}

	FLUSH();

	switch (target) {
		case GL_TEXTURE_2D:
		case GL_PROXY_TEXTURE_2D:
			break;  /* legal */
#ifdef CR_ARB_texture_cube_map
		case GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB:
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB:
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB:
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB:
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB:
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB:
		case GL_PROXY_TEXTURE_CUBE_MAP_ARB:
			if (g->extensions.ARB_texture_cube_map)
				break;  /* legal */
#endif
		default:
			/* error */
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
				"glTexImage2D invalid target: 0x%x", target);
			return;
	}

	if (level < 0 || (target == GL_TEXTURE_2D && level > t->maxLevel) || (target != GL_TEXTURE_2D && level > t->maxCubeMapLevel))
	{
		if (target == GL_PROXY_TEXTURE_2D)
		{
			/* clear all the texture object state */
			crStateTextureInitTextureObj(t, &(t->proxy2D), 0, GL_TEXTURE_2D);
		}
#ifdef CR_ARB_texture_cube_map
		else if (target == GL_PROXY_TEXTURE_CUBE_MAP_ARB)
		{
			/* clear all the texture object state */
			crStateTextureInitTextureObj(t, &(t->proxyCubeMap), 0,
				GL_TEXTURE_CUBE_MAP_ARB);
		}
#endif
		else
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
				"glTexImage2D level oob: %d", level);
		}
		return;
	}

	if (border != 0 && border != 1)
	{
		if (target == GL_PROXY_TEXTURE_2D)
		{
			/* clear all the texture object state */
			crStateTextureInitTextureObj(t, &(t->proxy2D), 0, GL_TEXTURE_2D);
		}
#ifdef CR_ARB_texture_cube_map
		else if (target == GL_PROXY_TEXTURE_CUBE_MAP_ARB)
		{
			/* clear all the texture object state */
			crStateTextureInitTextureObj(t, &(t->proxyCubeMap), 0,
				GL_TEXTURE_CUBE_MAP_ARB);
		}
#endif
		else
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
				"glTexImage2D border oob: %d", border);
		}
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
		if (target == GL_PROXY_TEXTURE_2D)
		{
			/* clear all the texture object state */
			crStateTextureInitTextureObj(t, &(t->proxy2D), 0, GL_TEXTURE_2D);
		}
#ifdef CR_ARB_texture_cube_map
		else if (target == GL_PROXY_TEXTURE_CUBE_MAP_ARB)
		{
			/* clear all the texture object state */
			crStateTextureInitTextureObj(t, &(t->proxyCubeMap), 0,
				GL_TEXTURE_CUBE_MAP_ARB);
		}
#endif
		else
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
				"glTexImage2D width is not valid: %d", width);
		}
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

	if (target == GL_TEXTURE_2D)
	{
		if (width > (int) g->limits.maxTextureSize)
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
				     "glTexImage2D width oob: %d", width);
			return;
		}
		if (height > (int) g->limits.maxTextureSize)
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
				     "glTexImage2D height oob: %d", height);
			return;
		}
	}
	else if (target == GL_PROXY_TEXTURE_2D)
	{
		if (width > (int) g->limits.maxTextureSize || height > (int) g->limits.maxTextureSize)
		{
			/* clear all the texture object state */
			crStateTextureInitTextureObj(t, &(t->proxy2D), 0, GL_TEXTURE_2D);
			return;
		}
	}
#ifdef CR_ARB_texture_cube_map
	else if (target == GL_PROXY_TEXTURE_CUBE_MAP_ARB)
	{
		if (width > (int) g->limits.maxCubeMapTextureSize ||
			height > (int) g->limits.maxCubeMapTextureSize ||
			width != height)
		{
			/* clear all the texture object state */
			crStateTextureInitTextureObj(t, &(t->proxyCubeMap), 0,
				GL_TEXTURE_CUBE_MAP_ARB);
			return;
		}
	}
	else /* Cube map */
	{
		if (width > (int) g->limits.maxCubeMapTextureSize)
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
				     "glTexImage2D width oob: %d", width);
			return;
		}
		if (height > (int) g->limits.maxCubeMapTextureSize)
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
				     "glTexImage2D height oob: %d", height);
			return;
		}
		/* Check that cube map width and height are equal. */
		if (width != height)
		{
			crStateError(__LINE__,__FILE__, GL_INVALID_VALUE,
				     "glTexImage2D cube map width and height are unequal: %dx%d", width, height);
			return;
		}
#endif
	}

	/*
	 ** Only set these fields if 
	 ** defining the base texture.
	 */
	if (target == GL_TEXTURE_2D )
	{
		tobj = t->currentTexture2D;
		tl = tobj->level+level;
		tl->bytes = crPixelSize(format, type, width, height);
	}
	else if (target == GL_PROXY_TEXTURE_2D) {
		tobj = &(t->proxy2D);
		tl = tobj->level+level;
		tl->bytes = 0;
	}
#ifdef CR_ARB_texture_cube_map
	else if (target == GL_PROXY_TEXTURE_CUBE_MAP_ARB) {
		tobj = &(t->proxyCubeMap);
		tl = tobj->level+level;
		tl->bytes = 0;
	}
	else if (target == GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB)
	{
		tobj = t->currentTextureCubeMap;
		tl = tobj->level + level;
		tl->bytes = crPixelSize(format, type, width, height);
	}
	else if (target == GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB)
	{
		tobj = t->currentTextureCubeMap;
		tl = tobj->negativeXlevel + level;
		tl->bytes = crPixelSize(format, type, width, height);
	}
	else if (target == GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB)
	{
		tobj = t->currentTextureCubeMap;
		tl = tobj->positiveYlevel + level;
		tl->bytes = crPixelSize(format, type, width, height);
	}
	else if (target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB)
	{
		tobj = t->currentTextureCubeMap;
		tl = tobj->negativeYlevel + level;
		tl->bytes = crPixelSize(format, type, width, height);
	}
	else if (target == GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB)
	{
		tobj = t->currentTextureCubeMap;
		tl = tobj->positiveZlevel + level;
		tl->bytes = crPixelSize(format, type, width, height);
	}
	else if (target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB)
	{
		tobj = t->currentTextureCubeMap;
		tl = tobj->negativeZlevel + level;
		tl->bytes = crPixelSize(format, type, width, height);
	}
#endif

	if (tl->bytes)
	{
		/* this is not a proxy texture target so alloc storage */
		if (tl->img)
			crFree(tl->img);
		tl->img = (GLubyte *) crAlloc(tl->bytes);
		if (!tl->img)
		{
			crStateError(__LINE__,__FILE__, GL_OUT_OF_MEMORY,
				     "glTexImage2D out of memory");
			return;
		}
		if (pixels)
			crPixelCopy2D((GLvoid *) (tl->img), pixels, format,
				type, width, height, &(p->unpack) );
	}

	tl->width = width;
	tl->height = height;
	tl->depth = 1;
	tl->format = format;
	tl->components = components;
	tl->border = border;
	tl->type = type;
	tl->bytesPerPixel = tl->bytes/ (width*height);

	/* XXX may need to do some fine-tuning here for proxy textures */
	tobj->dirty = g->neg_bitid;
	for (i = 0 ; i < CR_MAX_TEXTURE_UNITS; i++)
	{
		tl->dirty[i] = g->neg_bitid;
		tobj->imageBit[i] = g->neg_bitid;
	}
	tb->dirty = g->neg_bitid;
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
	int i;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glSubTexImage1D called in Begin/End");
		return;
	}

	FLUSH();

	if (target != GL_TEXTURE_1D && target != GL_PROXY_TEXTURE_1D)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glSubTexImage1D target != GL_TEXTURE_1D: %d", target);
		return;
	}

	if (level < 0 || level > t->maxLevel) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
			"glSubTexImage1D level oob: %d", level);
		return;
	}

	if (width > (int) g->limits.maxTextureSize)
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

	tobj->dirty = g->neg_bitid;
	for (i = 0 ; i < CR_MAX_TEXTURE_UNITS; i++)
	{
		tl->dirty[i] = g->neg_bitid;
		tobj->imageBit[i] = g->neg_bitid;
	}
	tb->dirty = g->neg_bitid;
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
	int i;

	GLubyte *subimg = NULL;
	GLubyte *img = NULL;
	GLubyte *src;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glTexSubImage2D called in Begin/End");
		return;
	}

	FLUSH();

	if (target == GL_TEXTURE_2D)
	{
		if (level < 0 || level > t->maxLevel)
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexSubImage2D level oob: %d", level);
			return;
		}
		if (width < 0 || width > (int) g->limits.maxTextureSize)
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
				     "glSubTexImage2D width oob: %d", width);
			return;
		}
		if (height < 0 || height > (int) g->limits.maxTextureSize)
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
				     "glSubTexImage2D height oob: %d", height);
			return;
		}
	}
#ifdef CR_ARB_texture_cube_map
	else if (target >= GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB && target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB && g->extensions.ARB_texture_cube_map)
	{
		if (level < 0 || level > t->maxCubeMapLevel)
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexSubImage2D level oob: %d", level);
			return;
		}
		if (width < 0 || width > (int) g->limits.maxCubeMapTextureSize)
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
				     "glTexSubImage2D width oob: %d", width);
			return;
		}
		if (height < 0 || height > (int) g->limits.maxCubeMapTextureSize)
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
				     "glSubTexImage2D height oob: %d", height);
			return;
		}
	}
#endif /* CR_ARB_texture_cube_map */
	else
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glTexSubImage2D target != GL_TEXTURE_2D: %d", target);
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

	crPixelCopy2D( subimg, pixels, format, type, width, height, &(p->unpack) );

	img = tl->img +
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
	for (i = 0 ; i < CR_MAX_TEXTURE_UNITS; i++)
	{
		tobj->imageBit[i] = g->neg_bitid;
		tl->dirty[i] = g->neg_bitid;
	}
	tb->dirty = g->neg_bitid;
}

#ifdef CR_OPENGL_VERSION_1_2
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


	if (target != GL_TEXTURE_3D && target != GL_PROXY_TEXTURE_3D)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glTexImage3D invalid target: 0x%x", target);
		return;
	}

	if (level < 0 || level > t->max3Dlevel) 
	{
		if (target == GL_PROXY_TEXTURE_3D)
		{
			/* clear all the texture object state */
			crStateTextureInitTextureObj(t, &(t->proxy3D), 0, GL_TEXTURE_3D);
		}
		else
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
				"glTexImage3D level oob: %d", level);
		}
		return;
	}

	if (border != 0 && border != 1)
	{
		if (target == GL_PROXY_TEXTURE_3D)
		{
			/* clear all the texture object state */
			crStateTextureInitTextureObj(t, &(t->proxy3D), 0, GL_TEXTURE_3D);
		}
		else
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
				"glTexImage3D border oob: %d", border);
		}
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
		if (target == GL_PROXY_TEXTURE_3D)
		{
			/* clear all the texture object state */
			crStateTextureInitTextureObj(t, &(t->proxy3D), 0, GL_TEXTURE_3D);
		}
		else
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
			    "glTexImage3D height is not valid: %d", height);
		}
		return;
	}

	/* check the bits in height */
	i=1;
	if (depth > 0)
		for (i=depth-2*border; i>0 && !(i&0x1); i = i >> 1);
	if (depth < 0 || i!=1) 
	{
		if (target == GL_PROXY_TEXTURE_3D)
		{
			/* clear all the texture object state */
			crStateTextureInitTextureObj(t, &(t->proxy3D), 0, GL_TEXTURE_3D);
		}
		else
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
				"glTexImage3D depth is not valid: %d", depth);
		}
		return;
	}

	if (width > g->limits.max3Dtexturesize)
	{
		if (target == GL_PROXY_TEXTURE_3D)
		{
			/* clear all the texture object state */
			crStateTextureInitTextureObj(t, &(t->proxy3D), 0, GL_TEXTURE_3D);
		}
		else
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
				"glTexImage3D width oob: %d", width);
		return;
	}

	if (height > g->limits.max3Dtexturesize)
	{
		if (target == GL_PROXY_TEXTURE_3D)
		{
			/* clear all the texture object state */
			crStateTextureInitTextureObj(t, &(t->proxy3D), 0), GL_TEXTURE_3D);
		}
		else
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
				"glTexImage3D height oob: %d", height);
		}
		return;
	}

	if (target == GL_TEXTURE_3D)
	{
		tobj = t->currentTexture3D;  /* FIXME: per unit! */
		tl = tobj->level + level;
#if 1
		/* XXX which of these is right? (BrianP) */
		tl->bytes = crPixelSize(format, type, width, height);
#else
		tl->bytes = __glpixel_getdata3Dsize(width, height, depth,
			format, type);
#endif
	}
	else if (target == GL_PROXY_TEXTURE_3D)
	{
		tobj = t->proxyTexture3D;  /* FIXME: per unit! */
		tl = tobj->level + level;
		tl->bytes = 0;
	}

	if (tl->bytes)
	{
		/* this is not a proxy texture target so alloc storage */
		if (tl->img)
			crFree(tl->img);
		tl->img = (GLubyte *) crAlloc(tl->bytes);
		if (!tl->img)
		{
			crStateError(__LINE__,__FILE__, GL_OUT_OF_MEMORY,
				     "glTexImage3D out of memory");
			return;
		}
		if (pixels)
#if 0
			/* XXX not implemented? */
			crPixelCopy3D((GLvoid *) (tl->img), pixels, format,
				type, width, height, &(p->unpack) );
#else
			__glpixel_getdata3D_p(p, (GLvoid *) tl->img,
				width, height, depth, format, type, pixels);
#endif
	}

	tl->components = components;
	tl->border = border;
	tl->width = width;
	tl->height = height;
	tl->depth = depth;
	tl->format = format;
	tl->type = type;

	/* XXX may need to do some fine-tuning here for proxy textures */
	tl->dirty = g->neg_bitid;
	for (i = 0 ; i < CR_MAX_TEXTURE_UNITS; i++)
	{
		tobj->imageBit[i] = g->neg_bitid;
	}
	tobj->dirty = g->neg_bitid;
	tb->dirty = g->neg_bitid;
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
	int i;

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
#ifdef CR_OPENGL_VERSION_1_2
		case GL_TEXTURE_3D:
			tobj = t->currentTexture3D;
			break;
#endif
#ifdef CR_ARB_texture_cube_map
		case GL_TEXTURE_CUBE_MAP_ARB:
			if (!g->extensions.ARB_texture_cube_map) {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					"TexParamterfv: target is invalid: %d", target);
				return;
			}
			tobj = t->currentTextureCubeMap;
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
			if (e != GL_NEAREST && e != GL_LINEAR)
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
							"TexParamterfv: GL_TEXTURE_MAG_FILTER invalid param: %d", e);
				return;
			}
			tobj->magFilter = e;
			break;
		case GL_TEXTURE_WRAP_S:
			if (e == GL_CLAMP || e == GL_REPEAT) {
				tobj->wrapS = e;
			}
			else if (e == GL_CLAMP_TO_EDGE_EXT && g->extensions.EXT_texture_edge_clamp) {
				tobj->wrapS = e;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					"TexParameterfv: GL_TEXTURE_WRAP_S invalid param: 0x%x", e);
				return;
			}
			break;
		case GL_TEXTURE_WRAP_T:
			if (e == GL_CLAMP || e == GL_REPEAT) {
				tobj->wrapT = e;
			}
			else if (e == GL_CLAMP_TO_EDGE_EXT && g->extensions.EXT_texture_edge_clamp) {
				tobj->wrapT = e;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					"TexParameterfv: GL_TEXTURE_WRAP_T invalid param: 0x%x", e);
				return;
			}
			break;
#ifdef CR_OPENGL_VERSION_1_2
		case GL_TEXTURE_WRAP_R:
			if (e == GL_CLAMP || e == GL_REPEAT) {
				tobj->wrapR = e;
			}
			else if (e == GL_CLAMP_TO_EDGE_EXT && g->extensions.EXT_texture_edge_clamp) {
				tobj->wrapR = e;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					"TexParameterfv: GL_TEXTURE_WRAP_R invalid param: 0x%x", e);
				return;
			}
			break;
#endif
		case GL_TEXTURE_BORDER_COLOR:
			tobj->borderColor.r = param[0];
			tobj->borderColor.g = param[1];
			tobj->borderColor.b = param[2];
			tobj->borderColor.a = param[3];
			break;
#ifdef CR_EXT_texture_filter_anisotropic
		case GL_TEXTURE_MAX_ANISOTROPY_EXT:
			if (g->extensions.EXT_texture_filter_anisotropic) {
				if (param[0] < 1.0f)
				{
					crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
						"TexParameterfv: GL_TEXTURE_MAX_ANISOTROPY_EXT called with parameter less than 1: %f", param[0]);
					return;
				}
				tobj->maxAnisotropy = param[0];
				if (tobj->maxAnisotropy > g->limits.maxTextureAnisotropy)
				{
					tobj->maxAnisotropy = g->limits.maxTextureAnisotropy;
				}
			}
			break;
#endif
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
				"TexParamterfv: Invalid pname: %d", pname);
			return;
	}

	tobj->dirty = g->neg_bitid;
	for (i = 0 ; i < CR_MAX_TEXTURE_UNITS; i++)
	{
		tobj->paramsBit[i] = g->neg_bitid;
	}
	tb->dirty = g->neg_bitid;
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
		case GL_TEXTURE_WRAP_R:
#ifdef CR_EXT_texture_filter_anisotropic
		case GL_TEXTURE_MAX_ANISOTROPY_EXT:
#endif
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
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
				"TexParamteriv: Invalid pname: %d", pname);
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
					e != GL_ADD &&
					e != GL_REPLACE)
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
							"glTexEnvfv: invalid param: %f", *param);
				return;
			}
			t->unit[t->curTextureUnit].envMode = e;
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
			t->unit[t->curTextureUnit].envColor = c;
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"glTexEnvfv: invalid pname: %d", pname);
			return;
	}

	tb->envBit[t->curTextureUnit] = g->neg_bitid;
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
			*param = (GLfloat) t->unit[t->curTextureUnit].envMode;
			break;
		case GL_TEXTURE_ENV_COLOR:
			param[0] = t->unit[t->curTextureUnit].envColor.r;
			param[1] = t->unit[t->curTextureUnit].envColor.g;
			param[2] = t->unit[t->curTextureUnit].envColor.b;
			param[3] = t->unit[t->curTextureUnit].envColor.a;
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
			*param = (GLint) t->unit[t->curTextureUnit].envMode;
			break;
		case GL_TEXTURE_ENV_COLOR:
			param[0] = (GLint) (t->unit[t->curTextureUnit].envColor.r * GL_MAXINT);
			param[0] = (GLint) (t->unit[t->curTextureUnit].envColor.g * GL_MAXINT);
			param[0] = (GLint) (t->unit[t->curTextureUnit].envColor.b * GL_MAXINT);
			param[0] = (GLint) (t->unit[t->curTextureUnit].envColor.a * GL_MAXINT);
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
					if (e == GL_OBJECT_LINEAR ||
						e == GL_EYE_LINEAR ||
						e == GL_SPHERE_MAP
#if defined(GL_ARB_texture_cube_map) || defined(GL_EXT_texture_cube_map) || defined(GL_NV_texgen_reflection)
						|| ((e == GL_REFLECTION_MAP_ARB || e == GL_NORMAL_MAP_ARB)
							&& g->extensions.ARB_texture_cube_map)
#endif
					) {
						t->unit[t->curTextureUnit].gen.s = e;
						tb->gen[t->curTextureUnit] = g->neg_bitid;
						tb->dirty = g->neg_bitid;
					}
					else {
						crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
							"glTexGen called with bad param: %lf", *param);
						return;
					}
					break;
				case GL_OBJECT_PLANE:
					v.x = (GLfloat) param[0];
					v.y = (GLfloat) param[1];
					v.z = (GLfloat) param[2];
					v.w = (GLfloat) param[3];
					t->unit[t->curTextureUnit].objSCoeff = v;
					tb->objGen[t->curTextureUnit] = g->neg_bitid;
					tb->dirty = g->neg_bitid;
					break;
				case GL_EYE_PLANE:
					v.x = (GLfloat) param[0];
					v.y = (GLfloat) param[1];
					v.z = (GLfloat) param[2];
					v.w = (GLfloat) param[3];
					crStateTransformInvertTransposeMatrix(&inv, trans->modelView+trans->modelViewDepth);
					crStateTransformXformPointMatrixf(&inv, &v);
					t->unit[t->curTextureUnit].eyeSCoeff = v;
					tb->eyeGen[t->curTextureUnit] = g->neg_bitid;
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
					if (e == GL_OBJECT_LINEAR ||
						e == GL_EYE_LINEAR ||
						e == GL_SPHERE_MAP
#if defined(GL_ARB_texture_cube_map) || defined(GL_EXT_texture_cube_map) || defined(GL_NV_texgen_reflection)
						|| ((e == GL_REFLECTION_MAP_ARB || e == GL_NORMAL_MAP_ARB)
							&& g->extensions.ARB_texture_cube_map)
#endif
					) {
						t->unit[t->curTextureUnit].gen.t = e;
						tb->gen[t->curTextureUnit] = g->neg_bitid;
						tb->dirty = g->neg_bitid;
					}
					else {
						crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
							"glTexGen called with bad param: %lf", *param);
						return;
					}
					break;
				case GL_OBJECT_PLANE:
					v.x = (GLfloat) param[0];
					v.y = (GLfloat) param[1];
					v.z = (GLfloat) param[2];
					v.w = (GLfloat) param[3];
					t->unit[t->curTextureUnit].objTCoeff = v;
					tb->objGen[t->curTextureUnit] = g->neg_bitid;
					tb->dirty = g->neg_bitid;
					break;
				case GL_EYE_PLANE:
					v.x = (GLfloat) param[0];
					v.y = (GLfloat) param[1];
					v.z = (GLfloat) param[2];
					v.w = (GLfloat) param[3];
					crStateTransformInvertTransposeMatrix(&inv, trans->modelView+trans->modelViewDepth);
					crStateTransformXformPointMatrixf(&inv, &v);
					t->unit[t->curTextureUnit].eyeTCoeff = v;
					tb->eyeGen[t->curTextureUnit] = g->neg_bitid;
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
					if (e == GL_OBJECT_LINEAR ||
						e == GL_EYE_LINEAR ||
						e == GL_SPHERE_MAP
#if defined(GL_ARB_texture_cube_map) || defined(GL_EXT_texture_cube_map) || defined(GL_NV_texgen_reflection)
						|| ((e == GL_REFLECTION_MAP_ARB || e == GL_NORMAL_MAP_ARB)
							&& g->extensions.ARB_texture_cube_map)
#endif
					) {
						t->unit[t->curTextureUnit].gen.r = e;
						tb->gen[t->curTextureUnit] = g->neg_bitid;
						tb->dirty = g->neg_bitid;
					}
					else {
						crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
							"glTexGen called with bad param: %lf", *param);
						return;
					}
					break;
				case GL_OBJECT_PLANE:
					v.x = (GLfloat) param[0];
					v.y = (GLfloat) param[1];
					v.z = (GLfloat) param[2];
					v.w = (GLfloat) param[3];
					t->unit[t->curTextureUnit].objRCoeff = v;
					tb->objGen[t->curTextureUnit] = g->neg_bitid;
					tb->dirty = g->neg_bitid;
					break;
				case GL_EYE_PLANE:
					v.x = (GLfloat) param[0];
					v.y = (GLfloat) param[1];
					v.z = (GLfloat) param[2];
					v.w = (GLfloat) param[3];
					crStateTransformInvertTransposeMatrix(&inv, trans->modelView+trans->modelViewDepth);
					crStateTransformXformPointMatrixf(&inv, &v);
					t->unit[t->curTextureUnit].eyeRCoeff = v;
					tb->eyeGen[t->curTextureUnit] = g->neg_bitid;
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
					if (e == GL_OBJECT_LINEAR ||
						e == GL_EYE_LINEAR ||
						e == GL_SPHERE_MAP)
					{
						t->unit[t->curTextureUnit].gen.q = e;
						tb->gen[t->curTextureUnit] = g->neg_bitid;
						tb->dirty = g->neg_bitid;
					}
					else {
						crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
							"glTexGen called with bad param: %lf", *param);
						return;
					}
					break;
				case GL_OBJECT_PLANE:
					v.x = (GLfloat) param[0];
					v.y = (GLfloat) param[1];
					v.z = (GLfloat) param[2];
					v.w = (GLfloat) param[3];
					t->unit[t->curTextureUnit].objQCoeff = v;
					tb->objGen[t->curTextureUnit] = g->neg_bitid;
					tb->dirty = g->neg_bitid;
					break;
				case GL_EYE_PLANE:
					v.x = (GLfloat) param[0];
					v.y = (GLfloat) param[1];
					v.z = (GLfloat) param[2];
					v.w = (GLfloat) param[3];
					crStateTransformInvertTransposeMatrix(&inv, trans->modelView+trans->modelViewDepth);
					crStateTransformXformPointMatrixf(&inv, &v);
					t->unit[t->curTextureUnit].eyeQCoeff = v;
					tb->eyeGen[t->curTextureUnit] = g->neg_bitid;
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
					*param = (GLdouble) t->unit[t->curTextureUnit].gen.s;
					break;
				case GL_T:
					*param = (GLdouble) t->unit[t->curTextureUnit].gen.t;
					break;
				case GL_R:
					*param = (GLdouble) t->unit[t->curTextureUnit].gen.r;
					break;
				case GL_Q:
					*param = (GLdouble) t->unit[t->curTextureUnit].gen.q;
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
					param[0] = (GLdouble) t->unit[t->curTextureUnit].objSCoeff.x;
					param[1] = (GLdouble) t->unit[t->curTextureUnit].objSCoeff.y;
					param[2] = (GLdouble) t->unit[t->curTextureUnit].objSCoeff.z;
					param[3] = (GLdouble) t->unit[t->curTextureUnit].objSCoeff.w;
					break;
				case GL_T:
					param[0] = (GLdouble) t->unit[t->curTextureUnit].objTCoeff.x;
					param[1] = (GLdouble) t->unit[t->curTextureUnit].objTCoeff.y;
					param[2] = (GLdouble) t->unit[t->curTextureUnit].objTCoeff.z;
					param[3] = (GLdouble) t->unit[t->curTextureUnit].objTCoeff.w;
					break;
				case GL_R:
					param[0] = (GLdouble) t->unit[t->curTextureUnit].objRCoeff.x;
					param[1] = (GLdouble) t->unit[t->curTextureUnit].objRCoeff.y;
					param[2] = (GLdouble) t->unit[t->curTextureUnit].objRCoeff.z;
					param[3] = (GLdouble) t->unit[t->curTextureUnit].objRCoeff.w;
					break;
				case GL_Q:
					param[0] = (GLdouble) t->unit[t->curTextureUnit].objQCoeff.x;
					param[1] = (GLdouble) t->unit[t->curTextureUnit].objQCoeff.y;
					param[2] = (GLdouble) t->unit[t->curTextureUnit].objQCoeff.z;
					param[3] = (GLdouble) t->unit[t->curTextureUnit].objQCoeff.w;
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
					param[0] = (GLdouble) t->unit[t->curTextureUnit].eyeSCoeff.x;
					param[1] = (GLdouble) t->unit[t->curTextureUnit].eyeSCoeff.y;
					param[2] = (GLdouble) t->unit[t->curTextureUnit].eyeSCoeff.z;
					param[3] = (GLdouble) t->unit[t->curTextureUnit].eyeSCoeff.w;
					break;
				case GL_T:
					param[0] = (GLdouble) t->unit[t->curTextureUnit].eyeTCoeff.x;
					param[1] = (GLdouble) t->unit[t->curTextureUnit].eyeTCoeff.y;
					param[2] = (GLdouble) t->unit[t->curTextureUnit].eyeTCoeff.z;
					param[3] = (GLdouble) t->unit[t->curTextureUnit].eyeTCoeff.w;
					break;
				case GL_R:
					param[0] = (GLdouble) t->unit[t->curTextureUnit].eyeRCoeff.x;
					param[1] = (GLdouble) t->unit[t->curTextureUnit].eyeRCoeff.y;
					param[2] = (GLdouble) t->unit[t->curTextureUnit].eyeRCoeff.z;
					param[3] = (GLdouble) t->unit[t->curTextureUnit].eyeRCoeff.w;
					break;
				case GL_Q:
					param[0] = (GLdouble) t->unit[t->curTextureUnit].eyeQCoeff.x;
					param[1] = (GLdouble) t->unit[t->curTextureUnit].eyeQCoeff.y;
					param[2] = (GLdouble) t->unit[t->curTextureUnit].eyeQCoeff.z;
					param[3] = (GLdouble) t->unit[t->curTextureUnit].eyeQCoeff.w;
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
					*param = (GLfloat) t->unit[t->curTextureUnit].gen.s;
					break;
				case GL_T:
					*param = (GLfloat) t->unit[t->curTextureUnit].gen.t;
					break;
				case GL_R:
					*param = (GLfloat) t->unit[t->curTextureUnit].gen.r;
					break;
				case GL_Q:
					*param = (GLfloat) t->unit[t->curTextureUnit].gen.q;
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
					param[0] = t->unit[t->curTextureUnit].objSCoeff.x;
					param[1] = t->unit[t->curTextureUnit].objSCoeff.y;
					param[2] = t->unit[t->curTextureUnit].objSCoeff.z;
					param[3] = t->unit[t->curTextureUnit].objSCoeff.w;
					break;
				case GL_T:
					param[0] =  t->unit[t->curTextureUnit].objTCoeff.x;
					param[1] =  t->unit[t->curTextureUnit].objTCoeff.y;
					param[2] =  t->unit[t->curTextureUnit].objTCoeff.z;
					param[3] =  t->unit[t->curTextureUnit].objTCoeff.w;
					break;
				case GL_R:
					param[0] =  t->unit[t->curTextureUnit].objRCoeff.x;
					param[1] =  t->unit[t->curTextureUnit].objRCoeff.y;
					param[2] =  t->unit[t->curTextureUnit].objRCoeff.z;
					param[3] =  t->unit[t->curTextureUnit].objRCoeff.w;
					break;
				case GL_Q:
					param[0] =  t->unit[t->curTextureUnit].objQCoeff.x;
					param[1] =  t->unit[t->curTextureUnit].objQCoeff.y;
					param[2] =  t->unit[t->curTextureUnit].objQCoeff.z;
					param[3] =  t->unit[t->curTextureUnit].objQCoeff.w;
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
					param[0] =  t->unit[t->curTextureUnit].eyeSCoeff.x;
					param[1] =  t->unit[t->curTextureUnit].eyeSCoeff.y;
					param[2] =  t->unit[t->curTextureUnit].eyeSCoeff.z;
					param[3] =  t->unit[t->curTextureUnit].eyeSCoeff.w;
					break;
				case GL_T:
					param[0] =  t->unit[t->curTextureUnit].eyeTCoeff.x;
					param[1] =  t->unit[t->curTextureUnit].eyeTCoeff.y;
					param[2] =  t->unit[t->curTextureUnit].eyeTCoeff.z;
					param[3] =  t->unit[t->curTextureUnit].eyeTCoeff.w;
					break;
				case GL_R:
					param[0] =  t->unit[t->curTextureUnit].eyeRCoeff.x;
					param[1] =  t->unit[t->curTextureUnit].eyeRCoeff.y;
					param[2] =  t->unit[t->curTextureUnit].eyeRCoeff.z;
					param[3] =  t->unit[t->curTextureUnit].eyeRCoeff.w;
					break;
				case GL_Q:
					param[0] =  t->unit[t->curTextureUnit].eyeQCoeff.x;
					param[1] =  t->unit[t->curTextureUnit].eyeQCoeff.y;
					param[2] =  t->unit[t->curTextureUnit].eyeQCoeff.z;
					param[3] =  t->unit[t->curTextureUnit].eyeQCoeff.w;
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
					*param = (GLint) t->unit[t->curTextureUnit].gen.s;
					break;
				case GL_T:
					*param = (GLint) t->unit[t->curTextureUnit].gen.t;
					break;
				case GL_R:
					*param = (GLint) t->unit[t->curTextureUnit].gen.r;
					break;
				case GL_Q:
					*param = (GLint) t->unit[t->curTextureUnit].gen.q;
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
					param[0] = (GLint) t->unit[t->curTextureUnit].objSCoeff.x;
					param[1] = (GLint) t->unit[t->curTextureUnit].objSCoeff.y;
					param[2] = (GLint) t->unit[t->curTextureUnit].objSCoeff.z;
					param[3] = (GLint) t->unit[t->curTextureUnit].objSCoeff.w;
					break;
				case GL_T:
					param[0] = (GLint) t->unit[t->curTextureUnit].objTCoeff.x;
					param[1] = (GLint) t->unit[t->curTextureUnit].objTCoeff.y;
					param[2] = (GLint) t->unit[t->curTextureUnit].objTCoeff.z;
					param[3] = (GLint) t->unit[t->curTextureUnit].objTCoeff.w;
					break;
				case GL_R:
					param[0] = (GLint) t->unit[t->curTextureUnit].objRCoeff.x;
					param[1] = (GLint) t->unit[t->curTextureUnit].objRCoeff.y;
					param[2] = (GLint) t->unit[t->curTextureUnit].objRCoeff.z;
					param[3] = (GLint) t->unit[t->curTextureUnit].objRCoeff.w;
					break;
				case GL_Q:
					param[0] = (GLint) t->unit[t->curTextureUnit].objQCoeff.x;
					param[1] = (GLint) t->unit[t->curTextureUnit].objQCoeff.y;
					param[2] = (GLint) t->unit[t->curTextureUnit].objQCoeff.z;
					param[3] = (GLint) t->unit[t->curTextureUnit].objQCoeff.w;
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
					param[0] = (GLint) t->unit[t->curTextureUnit].eyeSCoeff.x;
					param[1] = (GLint) t->unit[t->curTextureUnit].eyeSCoeff.y;
					param[2] = (GLint) t->unit[t->curTextureUnit].eyeSCoeff.z;
					param[3] = (GLint) t->unit[t->curTextureUnit].eyeSCoeff.w;
					break;
				case GL_T:
					param[0] = (GLint) t->unit[t->curTextureUnit].eyeTCoeff.x;
					param[1] = (GLint) t->unit[t->curTextureUnit].eyeTCoeff.y;
					param[2] = (GLint) t->unit[t->curTextureUnit].eyeTCoeff.z;
					param[3] = (GLint) t->unit[t->curTextureUnit].eyeTCoeff.w;
					break;
				case GL_R:
					param[0] = (GLint) t->unit[t->curTextureUnit].eyeRCoeff.x;
					param[1] = (GLint) t->unit[t->curTextureUnit].eyeRCoeff.y;
					param[2] = (GLint) t->unit[t->curTextureUnit].eyeRCoeff.z;
					param[3] = (GLint) t->unit[t->curTextureUnit].eyeRCoeff.w;
					break;
				case GL_Q:
					param[0] = (GLint) t->unit[t->curTextureUnit].eyeQCoeff.x;
					param[1] = (GLint) t->unit[t->curTextureUnit].eyeQCoeff.y;
					param[2] = (GLint) t->unit[t->curTextureUnit].eyeQCoeff.z;
					param[3] = (GLint) t->unit[t->curTextureUnit].eyeQCoeff.w;
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


/*
 * Return the texture image corresponding the given texture unit, target
 * and mipmap level.  Return NULL if any error.
 * This is a useful helper function.
 */
static CRTextureLevel * crStateGetTexLevel (CRContext *g, GLuint texUnit,
	GLenum target, GLint level)
{
	CRTextureState *t = &(g->texture);  /* FIXME: active texture unit! */
	CRTextureObj *tobj;
	CRTextureLevel *timg;

	assert(level >= 0);

	(void) texUnit;

	switch (target)
	{
		case GL_TEXTURE_1D:
			tobj = t->currentTexture1D;
			timg = tobj->level + level;
			break;

		case GL_PROXY_TEXTURE_1D:
			tobj = &(t->proxy1D);
			timg = tobj->level + level;
			break;

		case GL_TEXTURE_2D:
			tobj = t->currentTexture2D;
			timg = tobj->level + level;
			break;

		case GL_PROXY_TEXTURE_2D:
			tobj = &(t->proxy2D);
			timg = tobj->level + level;
			break;
#ifdef CR_OPENGL_VERSION_1_2
		case GL_TEXTURE_3D:
			tobj = t->currentTexture3D;
			timg = tobj->level + level;
			break;

		case GL_PROXY_TEXTURE_3D:
			tobj = t->proxyTexture3D;
			timg = tobj->level + level;
			break;
#endif
#ifdef CR_ARB_texture_cube_map
		case GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB:
			tobj = t->currentTextureCubeMap;
			timg = tobj->level + level;
			break;

		case GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB:
			tobj = t->currentTextureCubeMap;
			timg = tobj->negativeXlevel + level;
			break;

		case GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB:
			tobj = t->currentTextureCubeMap;
			timg = tobj->positiveYlevel + level;
			break;

		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB:
			tobj = t->currentTextureCubeMap;
			timg = tobj->negativeYlevel + level;
			break;

		case GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB:
			tobj = t->currentTextureCubeMap;
			timg = tobj->positiveZlevel + level;
			break;

		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB:
			tobj = t->currentTextureCubeMap;
			timg = tobj->negativeYlevel + level;
			break;
#endif
		default: 
			timg = NULL;
			break;
	}

	return timg;
}


/*
 * Return the texture object corresponding the given texture unit and target.
 * Return NULL if any error.
 * This is a useful helper function.
 */
static CRTextureObj * crStateGetTexObject (CRContext *g, GLuint texUnit,
	GLenum target)
{
	CRTextureState *t = &(g->texture);  /* FIXME: active texture unit! */
	CRTextureObj *tobj;

	(void) texUnit;

	switch (target)
	{
		case GL_TEXTURE_1D:
			tobj = t->currentTexture1D;
			break;

		case GL_PROXY_TEXTURE_1D:
			tobj = &(t->proxy1D);
			break;

		case GL_TEXTURE_2D:
			tobj = t->currentTexture2D;
			break;

		case GL_PROXY_TEXTURE_2D:
			tobj = &(t->proxy2D);
			break;
#ifdef CR_OPENGL_VERSION_1_2
		case GL_TEXTURE_3D:
			tobj = t->currentTexture3D;
			break;

		case GL_PROXY_TEXTURE_3D:
			tobj = &(t->proxy3D);
			break;
#endif
#ifdef CR_ARB_texture_cube_map
		case GL_TEXTURE_CUBE_MAP_ARB:
			tobj = t->currentTextureCubeMap;
			break;

		case GL_PROXY_TEXTURE_CUBE_MAP_ARB:
			tobj = &(t->proxyCubeMap);
			break;
#endif
		default: 
			tobj = NULL;
			break;
	}

	return tobj;
}



void STATE_APIENTRY crStateGetTexLevelParameterfv (GLenum target, GLint level, 
		GLenum pname, GLfloat *params) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRTextureLevel *timg;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
			"glGetTexLevelParameterfv called in begin/end");
		return;
	}

	if (level < 0 && level > t->maxLevel)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
			"glGetTexLevelParameterfv: level oob: %d", level);
		return;
	}

	timg = crStateGetTexLevel(g, t->curTextureUnit, target, level);
	if (!timg)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
			"GetTexLevelParameterfv: invalid target: 0x%x",
			target);
		return;
	}


	switch (pname) 
	{
		case GL_TEXTURE_WIDTH:
			*params = (GLfloat) timg->width;
			break;
		case GL_TEXTURE_HEIGHT:
			*params = (GLfloat) timg->height;
			break;
#ifdef CR_OPENGL_VERSION_1_2
		case GL_TEXTURE_DEPTH:
			*params = (GLfloat) timg->depth;
			break;
#endif
		case GL_TEXTURE_COMPONENTS:
			*params = (GLfloat) timg->components;
			break;
		case GL_TEXTURE_BORDER:
			*params = (GLfloat) timg->border;
			break;
		case GL_TEXTURE_RED_SIZE:
		case GL_TEXTURE_GREEN_SIZE:
		case GL_TEXTURE_BLUE_SIZE:
		case GL_TEXTURE_ALPHA_SIZE:
		case GL_TEXTURE_INTENSITY_SIZE:
		case GL_TEXTURE_LUMINANCE_SIZE:
			UNIMPLEMENTED();
			return;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
				"GetTexLevelParameterfv: invalid pname: 0x%x",
				pname);
			return;
	}
}

void STATE_APIENTRY crStateGetTexLevelParameteriv (GLenum target, GLint level, 
		GLenum pname, GLint *params) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRTextureLevel *timg;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
			"glGetTexLevelParameteriv called in begin/end");
		return;
	}

	if (level < 0 && level > t->maxLevel)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
			"glGetTexLevelParameteriv: level oob: %d", level);
		return;
	}

	timg = crStateGetTexLevel(g, t->curTextureUnit, target, level);
	if (!timg)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
			"GetTexLevelParameteriv: invalid target: 0x%x",
			target);
		return;
	}

	switch (pname) 
	{
		case GL_TEXTURE_WIDTH:
			*params = (GLint) timg->width;
			break;
		case GL_TEXTURE_HEIGHT:
			*params = (GLint) timg->height;
			break;
#ifdef CR_OPENGL_VERSION_1_2
		case GL_TEXTURE_DEPTH:
			*params = (GLint) timg->depth;
			break;
#endif
		case GL_TEXTURE_COMPONENTS:
			*params = (GLint) timg->components;
			break;
		case GL_TEXTURE_BORDER:
			*params = (GLint) timg->border;
			break;
		case GL_TEXTURE_RED_SIZE:
		case GL_TEXTURE_GREEN_SIZE:
		case GL_TEXTURE_BLUE_SIZE:
		case GL_TEXTURE_ALPHA_SIZE:
		case GL_TEXTURE_INTENSITY_SIZE:
		case GL_TEXTURE_LUMINANCE_SIZE:
			UNIMPLEMENTED();
			return;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
				"GetTexLevelParameteriv: invalid pname: 0x%x",
				pname);
			return;
	}
}

void STATE_APIENTRY crStateGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRTextureObj *tobj;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					"glGetTexParameter called in begin/end");
		return;
	}

	tobj = crStateGetTexObject(g, t->curTextureUnit, target);
	if (!tobj)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
			"glGetTexParameterfv: invalid target: 0x%x", target);
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
#ifdef CR_OPENGL_VERSION_1_2
		case GL_TEXTURE_WRAP_R:
			*params = (GLfloat) tobj->wrapR;
			break;
#endif
		case GL_TEXTURE_BORDER_COLOR:
			params[0] = tobj->borderColor.r;
			params[1] = tobj->borderColor.g;
			params[2] = tobj->borderColor.b;
			params[3] = tobj->borderColor.a;
			break;
        case GL_TEXTURE_RESIDENT:
			/* XXX todo */
#ifdef CR_OPENGL_VERSION_1_2
		case GL_TEXTURE_MIN_LOD:
		case GL_TEXTURE_MAX_LOD:
		case GL_TEXTURE_BASE_LEVEL:
		case GL_TEXTURE_MAX_LEVEL:
			/* XXX todo */
#endif
#ifdef CR_EXT_texture_filter_anisotropic
		case GL_TEXTURE_MAX_ANISOTROPY_EXT:
			if (g->extensions.EXT_texture_filter_anisotropic) {
				*params = (GLfloat) tobj->maxAnisotropy;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					"glGetTexParameter: invalid pname: 0x%x", pname);
				return;
			}
			break;
#endif
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
				"glGetTexParameter: invalid pname: %d", pname);
			return;
	}
}

void STATE_APIENTRY crStateGetTexParameteriv (GLenum target, GLenum pname, GLint *params) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRTextureObj *tobj;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					"glGetTexParameter called in begin/end");
		return;
	}

	tobj = crStateGetTexObject(g, t->curTextureUnit, target);
	if (!tobj)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
			"glGetTexParameteriv: invalid target: 0x%x", target);
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
#ifdef CR_OPENGL_VERSION_1_2
		case GL_TEXTURE_WRAP_R:
			*params = (GLint) tobj->wrapR;
			break;
#endif
		case GL_TEXTURE_BORDER_COLOR:
			params[0] = (GLint) (tobj->borderColor.r * GL_MAXINT);
			params[1] = (GLint) (tobj->borderColor.g * GL_MAXINT);
			params[2] = (GLint) (tobj->borderColor.b * GL_MAXINT);
			params[3] = (GLint) (tobj->borderColor.a * GL_MAXINT);
			break;
        case GL_TEXTURE_RESIDENT:
			/* XXX todo */
#ifdef CR_OPENGL_VERSION_1_2
		case GL_TEXTURE_MIN_LOD:
		case GL_TEXTURE_MAX_LOD:
		case GL_TEXTURE_BASE_LEVEL:
		case GL_TEXTURE_MAX_LEVEL:
			/* XXX todo */
#endif
#ifdef CR_EXT_texture_filter_anisotropic
		case GL_TEXTURE_MAX_ANISOTROPY_EXT:
			if (g->extensions.EXT_texture_filter_anisotropic) {
				*params = (GLint) tobj->maxAnisotropy;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, 
					"glGetTexParameter: invalid pname: 0x%x", pname);
				return;
			}
			break;
#endif
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, 
				"glGetTexParameter: invalid pname: %d", pname);
			return;
	}
}

/* TODO: */
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

void crStateTextureSwitch(CRContext *g, CRTextureBits *t, GLbitvalue bitID, 
						  CRTextureState *from, CRTextureState *to) 
{
	const GLbitvalue nbitID = ~bitID;
	int i;
	glAble able[2];
	able[0] = diff_api.Disable;
	able[1] = diff_api.Enable;

	for (i = 0 ; i < CR_MAX_TEXTURE_UNITS ; i++)
	{
		if (t->enable[i] & bitID) 
		{
			diff_api.ActiveTextureARB( i+GL_TEXTURE0_ARB );
			if (from->unit[i].enabled1D != to->unit[i].enabled1D) 
			{
				able[to->unit[i].enabled1D](GL_TEXTURE_1D);
				t->enable[i] = GLBITS_ONES;
				t->dirty = GLBITS_ONES;
			}
			if (from->unit[i].enabled2D != to->unit[i].enabled2D) 
			{
				able[to->unit[i].enabled2D](GL_TEXTURE_2D);
				t->enable[i] = GLBITS_ONES;
				t->dirty = GLBITS_ONES;
			}
#ifdef CR_OPENGL_VERSION_1_2
			if (from->unit[i].enabled3D != to->unit[i].enabled3D)
			{
				able[to->unit[i].enabled3D](GL_TEXTURE_3D);
				t->enable[i] = GLBITS_ONES;
				t->dirty = GLBITS_ONES;
			}
#endif
#ifdef CR_ARB_texture_cube_map
			if (g->extensions.ARB_texture_cube_map &&
				from->unit[i].enabledCubeMap != to->unit[i].enabledCubeMap)
			{
				able[to->unit[i].enabledCubeMap](GL_TEXTURE_CUBE_MAP_ARB);
				t->enable[i] = GLBITS_ONES;
				t->dirty = GLBITS_ONES;
			}
#endif
			if (from->unit[i].textureGen.s != to->unit[i].textureGen.s ||
					from->unit[i].textureGen.t != to->unit[i].textureGen.t ||
					from->unit[i].textureGen.r != to->unit[i].textureGen.r ||
					from->unit[i].textureGen.q != to->unit[i].textureGen.q) 
			{
				able[to->unit[i].textureGen.s](GL_TEXTURE_GEN_S);
				able[to->unit[i].textureGen.t](GL_TEXTURE_GEN_T);
				able[to->unit[i].textureGen.r](GL_TEXTURE_GEN_R);
				able[to->unit[i].textureGen.q](GL_TEXTURE_GEN_Q);
				t->enable[i] = GLBITS_ONES;
				t->dirty = GLBITS_ONES;
			}
			t->enable[i] &= nbitID;
		}

		/* 
		**  A thought on switching with textures:
		**  Since we are only performing a switch
		**  and not a sync, we won't need to 
		**  update individual textures, just
		**  the bindings....
		*/

		if (t->current[i] & bitID) 
		{
			diff_api.ActiveTextureARB( i + GL_TEXTURE0_ARB );
			if (from->unit[i].currentTexture1DName != to->unit[i].currentTexture1DName) 
			{
				diff_api.BindTexture(GL_TEXTURE_1D, to->unit[i].currentTexture1DName);
				t->current[i] = GLBITS_ONES;
				t->dirty = GLBITS_ONES;
			}
			if (from->unit[i].currentTexture2DName != to->unit[i].currentTexture2DName) 
			{
				diff_api.BindTexture(GL_TEXTURE_2D, to->unit[i].currentTexture2DName);
				t->current[i] = GLBITS_ONES;
				t->dirty = GLBITS_ONES;
			}
#ifdef CR_OPENGL_VERSION_1_2
			if (from->unit[i].currentTexture3DName != to->unit[i].currentTexture3DName) {
				diff_api.BindTexture(GL_TEXTURE_3D, to->unit[i].currentTexture3DName);
				t->current[i] = GLBITS_ONES;
				t->dirty = GLBITS_ONES;
			}
#endif
#ifdef CR_ARB_texture_cube_map
			if (g->extensions.ARB_texture_cube_map &&
				from->unit[i].currentTextureCubeMapName != to->unit[i].currentTextureCubeMapName) {
				diff_api.BindTexture(GL_TEXTURE_CUBE_MAP_ARB, to->unit[i].currentTextureCubeMapName);
				t->current[i] = GLBITS_ONES;
				t->dirty = GLBITS_ONES;
			}
#endif
		}

		if (t->objGen[i] & bitID) 
		{
			diff_api.ActiveTextureARB( i + GL_TEXTURE0_ARB );

			if (from->unit[i].objSCoeff.x != to->unit[i].objSCoeff.x ||
				  from->unit[i].objSCoeff.y != to->unit[i].objSCoeff.y ||
				  from->unit[i].objSCoeff.z != to->unit[i].objSCoeff.z ||
				  from->unit[i].objSCoeff.w != to->unit[i].objSCoeff.w) 
			{
				GLfloat f[4];
				f[0] = to->unit[i].objSCoeff.x;
				f[1] = to->unit[i].objSCoeff.y;
				f[2] = to->unit[i].objSCoeff.z;
				f[3] = to->unit[i].objSCoeff.w;
				diff_api.TexGenfv (GL_S, GL_OBJECT_PLANE, (const GLfloat *) f);
				t->objGen[i] = GLBITS_ONES;
				t->dirty = GLBITS_ONES;
			}
			if (from->unit[i].objTCoeff.x != to->unit[i].objTCoeff.x ||
				from->unit[i].objTCoeff.y != to->unit[i].objTCoeff.y ||
				from->unit[i].objTCoeff.z != to->unit[i].objTCoeff.z ||
				from->unit[i].objTCoeff.w != to->unit[i].objTCoeff.w) {
				GLfloat f[4];
				f[0] = to->unit[i].objTCoeff.x;
				f[1] = to->unit[i].objTCoeff.y;
				f[2] = to->unit[i].objTCoeff.z;
				f[3] = to->unit[i].objTCoeff.w;
				diff_api.TexGenfv (GL_T, GL_OBJECT_PLANE, (const GLfloat *) f);
				t->objGen[i] = GLBITS_ONES;
				t->dirty = GLBITS_ONES;
			}
			if (from->unit[i].objRCoeff.x != to->unit[i].objRCoeff.x ||
				from->unit[i].objRCoeff.y != to->unit[i].objRCoeff.y ||
				from->unit[i].objRCoeff.z != to->unit[i].objRCoeff.z ||
				from->unit[i].objRCoeff.w != to->unit[i].objRCoeff.w) {
				GLfloat f[4];
				f[0] = to->unit[i].objRCoeff.x;
				f[1] = to->unit[i].objRCoeff.y;
				f[2] = to->unit[i].objRCoeff.z;
				f[3] = to->unit[i].objRCoeff.w;
				diff_api.TexGenfv (GL_R, GL_OBJECT_PLANE, (const GLfloat *) f);
				t->objGen[i] = GLBITS_ONES;
				t->dirty = GLBITS_ONES;
			}
			if (from->unit[i].objQCoeff.x != to->unit[i].objQCoeff.x ||
				from->unit[i].objQCoeff.y != to->unit[i].objQCoeff.y ||
				from->unit[i].objQCoeff.z != to->unit[i].objQCoeff.z ||
				from->unit[i].objQCoeff.w != to->unit[i].objQCoeff.w) {
				GLfloat f[4];
				f[0] = to->unit[i].objQCoeff.x;
				f[1] = to->unit[i].objQCoeff.y;
				f[2] = to->unit[i].objQCoeff.z;
				f[3] = to->unit[i].objQCoeff.w;
				diff_api.TexGenfv (GL_Q, GL_OBJECT_PLANE, (const GLfloat *) f);
				t->objGen[i] = GLBITS_ONES;
				t->dirty = GLBITS_ONES;
			}
			t->objGen[i] &= nbitID;
		}
		if (t->eyeGen[i] & bitID) 
		{
			diff_api.ActiveTextureARB( i + GL_TEXTURE0_ARB );

			diff_api.MatrixMode(GL_MODELVIEW);
			diff_api.PushMatrix();
			diff_api.LoadIdentity();
			if (from->unit[i].eyeSCoeff.x != to->unit[i].eyeSCoeff.x ||
				from->unit[i].eyeSCoeff.y != to->unit[i].eyeSCoeff.y ||
				from->unit[i].eyeSCoeff.z != to->unit[i].eyeSCoeff.z ||
				from->unit[i].eyeSCoeff.w != to->unit[i].eyeSCoeff.w) {
				GLfloat f[4];
				f[0] = to->unit[i].eyeSCoeff.x;
				f[1] = to->unit[i].eyeSCoeff.y;
				f[2] = to->unit[i].eyeSCoeff.z;
				f[3] = to->unit[i].eyeSCoeff.w;
				diff_api.TexGenfv (GL_S, GL_EYE_PLANE, (const GLfloat *) f);
				t->eyeGen[i] = GLBITS_ONES;
				t->dirty = GLBITS_ONES;
			}
			if (from->unit[i].eyeTCoeff.x != to->unit[i].eyeTCoeff.x ||
				from->unit[i].eyeTCoeff.y != to->unit[i].eyeTCoeff.y ||
				from->unit[i].eyeTCoeff.z != to->unit[i].eyeTCoeff.z ||
				from->unit[i].eyeTCoeff.w != to->unit[i].eyeTCoeff.w) {
				GLfloat f[4];
				f[0] = to->unit[i].eyeTCoeff.x;
				f[1] = to->unit[i].eyeTCoeff.y;
				f[2] = to->unit[i].eyeTCoeff.z;
				f[3] = to->unit[i].eyeTCoeff.w;
				diff_api.TexGenfv (GL_T, GL_EYE_PLANE, (const GLfloat *) f);
				t->eyeGen[i] = GLBITS_ONES;
				t->dirty = GLBITS_ONES;
			}
			if (from->unit[i].eyeRCoeff.x != to->unit[i].eyeRCoeff.x ||
				from->unit[i].eyeRCoeff.y != to->unit[i].eyeRCoeff.y ||
				from->unit[i].eyeRCoeff.z != to->unit[i].eyeRCoeff.z ||
				from->unit[i].eyeRCoeff.w != to->unit[i].eyeRCoeff.w) {
				GLfloat f[4];
				f[0] = to->unit[i].eyeRCoeff.x;
				f[1] = to->unit[i].eyeRCoeff.y;
				f[2] = to->unit[i].eyeRCoeff.z;
				f[3] = to->unit[i].eyeRCoeff.w;
				diff_api.TexGenfv (GL_R, GL_EYE_PLANE, (const GLfloat *) f);
				t->eyeGen[i] = GLBITS_ONES;
				t->dirty = GLBITS_ONES;
			}
			if (from->unit[i].eyeQCoeff.x != to->unit[i].eyeQCoeff.x ||
				from->unit[i].eyeQCoeff.y != to->unit[i].eyeQCoeff.y ||
				from->unit[i].eyeQCoeff.z != to->unit[i].eyeQCoeff.z ||
				from->unit[i].eyeQCoeff.w != to->unit[i].eyeQCoeff.w) {
				GLfloat f[4];
				f[0] = to->unit[i].eyeQCoeff.x;
				f[1] = to->unit[i].eyeQCoeff.y;
				f[2] = to->unit[i].eyeQCoeff.z;
				f[3] = to->unit[i].eyeQCoeff.w;
				diff_api.TexGenfv (GL_Q, GL_EYE_PLANE, (const GLfloat *) f);
				t->eyeGen[i] = GLBITS_ONES;
				t->dirty = GLBITS_ONES;
			}
			diff_api.PopMatrix();
			t->eyeGen[i] &= nbitID;
		}
		if (t->gen[i] & bitID) 
		{
			diff_api.ActiveTextureARB( i + GL_TEXTURE0_ARB );
			if (from->unit[i].gen.s != to->unit[i].gen.s ||
				from->unit[i].gen.t != to->unit[i].gen.t ||
				from->unit[i].gen.r != to->unit[i].gen.r ||
				from->unit[i].gen.q != to->unit[i].gen.q) 
			{
				diff_api.TexGeni (GL_S, GL_TEXTURE_GEN_MODE, to->unit[i].gen.s);
				diff_api.TexGeni (GL_T, GL_TEXTURE_GEN_MODE, to->unit[i].gen.t);
				diff_api.TexGeni (GL_R, GL_TEXTURE_GEN_MODE, to->unit[i].gen.r);
				diff_api.TexGeni (GL_Q, GL_TEXTURE_GEN_MODE, to->unit[i].gen.q);	
				t->gen[i] = GLBITS_ONES;
				t->dirty = GLBITS_ONES;
			}
			t->gen[i] &= nbitID;
		}
		t->dirty &= nbitID;
/* Texture enviroment */
		if (t->envBit[i] & bitID) 
		{
			diff_api.ActiveTextureARB( i + GL_TEXTURE0_ARB );
			if (from->unit[i].envMode != to->unit[i].envMode) 
			{
				diff_api.TexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, to->unit[i].envMode);
				t->envBit[i] = GLBITS_ONES;
				t->dirty = GLBITS_ONES;
			}
			if (from->unit[i].envColor.r != to->unit[i].envColor.r ||
				from->unit[i].envColor.g != to->unit[i].envColor.g ||
				from->unit[i].envColor.b != to->unit[i].envColor.b ||
				from->unit[i].envColor.a != to->unit[i].envColor.a) 
			{
				GLfloat f[4];
				f[0] = to->unit[i].envColor.r;
				f[1] = to->unit[i].envColor.g;
				f[2] = to->unit[i].envColor.b;
				f[3] = to->unit[i].envColor.a;
				diff_api.TexEnvfv (GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, (const GLfloat *) f);
				t->envBit[i] = GLBITS_ONES;
				t->dirty = GLBITS_ONES;
			}
			t->envBit[i] &= nbitID;
		}
	}
}

void crStateTextureDiff(CRContext *g, CRTextureBits *t, GLbitvalue bitID, 
						 CRTextureState *from, CRTextureState *to) 
{
	const GLbitvalue nbitID = ~bitID;
	CRTextureObj *tobj = NULL;
	GLuint name=0;
	GLuint *cname=NULL;
	int i;
	glAble able[2];
	able[0] = diff_api.Disable;
	able[1] = diff_api.Enable;

	for (i = 0 ; i < CR_MAX_TEXTURE_UNITS ; i++)
	{
		/* First, try to create the current texture 
		 * objects before mucking with the individual 
		 * units. */

		if (to->unit[i].enabled1D == GL_TRUE) 
		{
			GET_TOBJ(tobj, to, to->unit[i].currentTexture1DName);
			if (!tobj)
			{
				tobj = to->currentTexture1D;
			}
			name = to->unit[i].currentTexture1DName;
			cname = &(from->unit[i].currentTexture1DName);
		}

		if (to->unit[i].enabled2D == GL_TRUE) 
		{
			GET_TOBJ(tobj, to, to->unit[i].currentTexture2DName);
			if (!tobj)
			{
				tobj = to->currentTexture2D;
			}
			name = to->unit[i].currentTexture2DName;
			cname = &(from->unit[i].currentTexture2DName);
		}

#ifdef CR_OPENGL_VERSION_1_2
		if (to->unit[i].enabled3D == GL_TRUE) 
		{
			GET_TOBJ(tobj, to, to->unit[i].currentTexture3DName);
			if (!tobj)
			{
				tobj = to->currentTexture3D;
			}
			name = to->unit[i].currentTexture3DName;
			cname = &(from->unit[i].currentTexture3DName);
		}
#endif
#ifdef CR_ARB_texture_cube_map
		if (g->extensions.ARB_texture_cube_map && to->unit[i].enabledCubeMap == GL_TRUE)
		{
			GET_TOBJ(tobj, to, to->unit[i].currentTextureCubeMapName);
			if (!tobj)
			{
				tobj = to->currentTextureCubeMap;
			}
			name = to->unit[i].currentTextureCubeMapName;
			cname = &(from->unit[i].currentTextureCubeMapName);
		}
#endif

		if (!tobj)
		{
			continue;
		}

		if (tobj->dirty & bitID) 
		{
			diff_api.ActiveTextureARB( i + GL_TEXTURE0_ARB );
			diff_api.BindTexture( tobj->target, name );
			if (tobj->paramsBit[i] & bitID) 
			{
				GLfloat f[4];
				f[0] = tobj->borderColor.r;
				f[1] = tobj->borderColor.g;
				f[2] = tobj->borderColor.b;
				f[3] = tobj->borderColor.a;
				diff_api.TexParameteri(tobj->target, GL_TEXTURE_MIN_FILTER, tobj->minFilter);
				diff_api.TexParameteri(tobj->target, GL_TEXTURE_MAG_FILTER, tobj->magFilter);
				diff_api.TexParameteri(tobj->target, GL_TEXTURE_WRAP_S, tobj->wrapS);
				diff_api.TexParameteri(tobj->target, GL_TEXTURE_WRAP_T, tobj->wrapT);
#ifdef CR_OPENGL_VERSION_1_2
				diff_api.TexParameteri(tobj->target, GL_TEXTURE_WRAP_R, tobj->wrapR);
#endif
				diff_api.TexParameterfv(tobj->target, GL_TEXTURE_BORDER_COLOR, (const GLfloat *) f);
#ifdef CR_EXT_texture_filter_anisotropic
				if (g->extensions.EXT_texture_filter_anisotropic)
					diff_api.TexParameterf(tobj->target, GL_TEXTURE_MAX_ANISOTROPY_EXT, tobj->maxAnisotropy);
#endif
				tobj->paramsBit[i] &= nbitID;
			}

			if (tobj->imageBit[i] & bitID) 
			{
				int j;
				switch (tobj->target)
				{
					case GL_TEXTURE_1D:
						for (j=0; j<to->maxLevel; j++) 
						{
							CRTextureLevel *tl = &(tobj->level[j]);
							if (tl->dirty[i] & bitID) 
							{
								diff_api.TexImage1D(GL_TEXTURE_1D, j,
													tl->components,
													tl->width, tl->border,
													tl->format,
													tl->type, tl->img);
								tl->dirty[i] &= nbitID;
							}
						}
						break;
					case GL_TEXTURE_2D:
						for (j=0; j<to->maxLevel; j++) 
						{
							CRTextureLevel *tl = &(tobj->level[j]);
							if (tl->dirty[i] & bitID) 
							{
								diff_api.TexImage2D(GL_TEXTURE_2D, j,
													tl->components,
													tl->width, tl->height,
													tl->border, tl->format,
													tl->type, tl->img);
								tl->dirty[i] &= nbitID;
							}
						}
						break;
#ifdef CR_OPENGL_VERSION_1_2
					case GL_TEXTURE_3D:
						for (j=0; j<to->maxLevel; j++) 
						{
							CRTextureLevel *tl = &(tobj->level[j]);
							if (tl->dirty[i] & bitID) 
							{
								diff_api.TexImage3D(GL_TEXTURE_3D, j,
													tl->components,
													tl->width, tl->height,
													tl->depth,
													tl->border, tl->format,
													tl->type, tl->img);
								tl->dirty[i] &= nbitID;
							}
						}
						break;
#endif
#ifdef CR_ARB_texture_cube_map
					case GL_TEXTURE_CUBE_MAP_ARB:
						for (j=0; j<to->maxLevel; j++) 
						{
							CRTextureLevel *tl;
							/* Positive X */
							tl = &(tobj->level[j]);
							if (tl->dirty[i] & bitID)
							{
								diff_api.TexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
													j, tl->components,
													tl->width, tl->height, tl->border,
													tl->format, tl->type, tl->img);
								tl->dirty[i] &= nbitID;
							}
							/* Negative X */
							tl = &(tobj->negativeXlevel[j]);
							if (tl->dirty[i] & bitID)
							{
								diff_api.TexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB,
													j, tl->components,
													tl->width, tl->height, tl->border,
													tl->format, tl->type, tl->img);
								tl->dirty[i] &= nbitID;
							}
							/* Positive Y */
							tl = &(tobj->positiveYlevel[j]);
							if (tl->dirty[i] & bitID) 
							{
								diff_api.TexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB,
													j, tl->components,
													tl->width, tl->height, tl->border,
													tl->format, tl->type, tl->img);
								tl->dirty[i] &= nbitID;
							}
							/* Negative Y */
							tl = &(tobj->negativeYlevel[j]);
							if (tl->dirty[i] & bitID)
							{
								diff_api.TexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB,
													j, tl->components,
													tl->width, tl->height, tl->border,
													tl->format, tl->type, tl->img);
								tl->dirty[i] &= nbitID;
							}
							/* Positive Z */
							tl = &(tobj->positiveZlevel[j]);
							if (tl->dirty[i] & bitID)
							{
								diff_api.TexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB,
													j, tl->components,
													tl->width, tl->height, tl->border,
													tl->format, tl->type, tl->img);
								tl->dirty[i] &= nbitID;
							}
							/* Negative Z */
							tl = &(tobj->negativeZlevel[j]);
							if (tl->dirty[i] & bitID)
							{
								diff_api.TexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB,
													j, tl->components,
													tl->width, tl->height, tl->border,
													tl->format, tl->type, tl->img);
								tl->dirty[i] &= nbitID;
							}
						}
						break;
#endif
					default:
						UNIMPLEMENTED();
				}	
			} /* if (tobj->imageBit[i] & bitID) */
			tobj->dirty &= nbitID;
		}
	}

	for (i = 0 ; i < CR_MAX_TEXTURE_UNITS ; i++)
	{
		if (t->enable[i] & bitID) 
		{
			diff_api.ActiveTextureARB( i + GL_TEXTURE0_ARB );
			if (from->unit[i].enabled1D != to->unit[i].enabled1D) 
			{
				able[to->unit[i].enabled1D](GL_TEXTURE_1D);
				from->unit[i].enabled1D = to->unit[i].enabled1D;
			}
			if (from->unit[i].enabled2D != to->unit[i].enabled2D) 
			{
				able[to->unit[i].enabled2D](GL_TEXTURE_2D);
				from->unit[i].enabled2D = to->unit[i].enabled2D;
			}
#ifdef CR_OPENGL_VERSION_1_2
			if (from->unit[i].enabled3D != to->unit[i].enabled3D)
			{
				able[to->unit[i].enabled3D](GL_TEXTURE_3D);
				from->unit[i].enabled3D = to->unit[i].enabled3D;
			}
#endif
#ifdef CR_ARB_texture_cube_map
			if (g->extensions.ARB_texture_cube_map &&
				from->unit[i].enabledCubeMap != to->unit[i].enabledCubeMap)
			{
				able[to->unit[i].enabledCubeMap](GL_TEXTURE_CUBE_MAP_ARB);
				from->unit[i].enabledCubeMap = to->unit[i].enabledCubeMap;
			}
#endif
			if (from->unit[i].textureGen.s != to->unit[i].textureGen.s ||
					from->unit[i].textureGen.t != to->unit[i].textureGen.t ||
					from->unit[i].textureGen.r != to->unit[i].textureGen.r ||
					from->unit[i].textureGen.q != to->unit[i].textureGen.q) 
			{
				able[to->unit[i].textureGen.s](GL_TEXTURE_GEN_S);
				able[to->unit[i].textureGen.t](GL_TEXTURE_GEN_T);
				able[to->unit[i].textureGen.r](GL_TEXTURE_GEN_R);
				able[to->unit[i].textureGen.q](GL_TEXTURE_GEN_Q);
				from->unit[i].textureGen = to->unit[i].textureGen;
			}
			t->enable[i] &= nbitID;
		}

		/* Get the active texture */
		if (to->unit[i].enabled1D == GL_TRUE) 
		{
			GET_TOBJ(tobj, to, to->unit[i].currentTexture1DName);
			/*tobj = to->currentTexture1D; */
			name = to->unit[i].currentTexture1DName;
			cname = &(from->unit[i].currentTexture1DName);
		}

		if (to->unit[i].enabled2D == GL_TRUE) 
		{
			GET_TOBJ(tobj, to, to->unit[i].currentTexture2DName);
			/*tobj = to->currentTexture2D; */
			name = to->unit[i].currentTexture2DName;
			cname = &(from->unit[i].currentTexture2DName);
		}

#ifdef CR_OPENGL_VERSION_1_2
		if (to->unit[i].enabled3d == GL_TRUE)
		{
			GET_TOBJ(tobj, to, to->unit[i].currentTexture3DName);
			/*tobj = to->currenttexture3d[i]; */
			name = to->unit[i].currenttexture3dname;
			cname = &(from->unit[i].currenttexture3dname);
		}
#endif
#ifdef CR_ARB_texture_cube_map
		if (g->extensions.ARB_texture_cube_map &&
			to->unit[i].enabledCubeMap == GL_TRUE)
		{
			GET_TOBJ(tobj, to, to->unit[i].currentTextureCubeMapName);
			/*tobj = to->currentTextureCubeMap; */
			name = to->unit[i].currentTextureCubeMapName;
			cname = &(from->unit[i].currentTextureCubeMapName);
		}
#endif

		if (!tobj) 
		{
			/* texturing is not enabled for this unit */
			continue;
		}

		if (t->current[i] & bitID) 
		{
			if (*cname != name) 
			{
				diff_api.ActiveTextureARB(i+GL_TEXTURE0_ARB);
				diff_api.BindTexture(tobj->target, name);
				*cname = name;
			}
			t->current[i] &= nbitID;
		}

		/*
		** Texture Restore 
		** Since textures are allocated objects
		** it seems wastefull to allocate each one
		** on all the pipes.
		** So instead, we'll skip the value compare
		** and just use the bit test.
		*/


		if (t->objGen[i] & bitID) 
		{
			diff_api.ActiveTextureARB( i + GL_TEXTURE0_ARB );
			if (from->unit[i].objSCoeff.x != to->unit[i].objSCoeff.x ||
					from->unit[i].objSCoeff.y != to->unit[i].objSCoeff.y ||
					from->unit[i].objSCoeff.z != to->unit[i].objSCoeff.z ||
					from->unit[i].objSCoeff.w != to->unit[i].objSCoeff.w) 
			{
				GLfloat f[4];
				f[0] = to->unit[i].objSCoeff.x;
				f[1] = to->unit[i].objSCoeff.y;
				f[2] = to->unit[i].objSCoeff.z;
				f[3] = to->unit[i].objSCoeff.w;
				diff_api.TexGenfv (GL_S, GL_OBJECT_PLANE, (const GLfloat *) f);
				from->unit[i].objSCoeff = to->unit[i].objSCoeff;
			}
			if (from->unit[i].objTCoeff.x != to->unit[i].objTCoeff.x ||
					from->unit[i].objTCoeff.y != to->unit[i].objTCoeff.y ||
					from->unit[i].objTCoeff.z != to->unit[i].objTCoeff.z ||
					from->unit[i].objTCoeff.w != to->unit[i].objTCoeff.w) 
			{
				GLfloat f[4];
				f[0] = to->unit[i].objTCoeff.x;
				f[1] = to->unit[i].objTCoeff.y;
				f[2] = to->unit[i].objTCoeff.z;
				f[3] = to->unit[i].objTCoeff.w;
				diff_api.TexGenfv (GL_T, GL_OBJECT_PLANE, (const GLfloat *) f);
				from->unit[i].objTCoeff = to->unit[i].objTCoeff;
			}
			if (from->unit[i].objRCoeff.x != to->unit[i].objRCoeff.x ||
					from->unit[i].objRCoeff.y != to->unit[i].objRCoeff.y ||
					from->unit[i].objRCoeff.z != to->unit[i].objRCoeff.z ||
					from->unit[i].objRCoeff.w != to->unit[i].objRCoeff.w) 
			{
				GLfloat f[4];
				f[0] = to->unit[i].objRCoeff.x;
				f[1] = to->unit[i].objRCoeff.y;
				f[2] = to->unit[i].objRCoeff.z;
				f[3] = to->unit[i].objRCoeff.w;
				diff_api.TexGenfv (GL_R, GL_OBJECT_PLANE, (const GLfloat *) f);
				from->unit[i].objRCoeff = to->unit[i].objRCoeff;
			}
			if (from->unit[i].objQCoeff.x != to->unit[i].objQCoeff.x ||
					from->unit[i].objQCoeff.y != to->unit[i].objQCoeff.y ||
					from->unit[i].objQCoeff.z != to->unit[i].objQCoeff.z ||
					from->unit[i].objQCoeff.w != to->unit[i].objQCoeff.w) 
			{
				GLfloat f[4];
				f[0] = to->unit[i].objQCoeff.x;
				f[1] = to->unit[i].objQCoeff.y;
				f[2] = to->unit[i].objQCoeff.z;
				f[3] = to->unit[i].objQCoeff.w;
				diff_api.TexGenfv (GL_Q, GL_OBJECT_PLANE, (const GLfloat *) f);
				from->unit[i].objQCoeff = to->unit[i].objQCoeff;
			}
			t->objGen[i] &= nbitID;
		}
		if (t->eyeGen[i] & bitID) 
		{
			diff_api.ActiveTextureARB( i + GL_TEXTURE0_ARB );
			diff_api.MatrixMode(GL_MODELVIEW);
			diff_api.PushMatrix();
			diff_api.LoadIdentity();
			if (from->unit[i].eyeSCoeff.x != to->unit[i].eyeSCoeff.x ||
					from->unit[i].eyeSCoeff.y != to->unit[i].eyeSCoeff.y ||
					from->unit[i].eyeSCoeff.z != to->unit[i].eyeSCoeff.z ||
					from->unit[i].eyeSCoeff.w != to->unit[i].eyeSCoeff.w) 
			{
				GLfloat f[4];
				f[0] = to->unit[i].eyeSCoeff.x;
				f[1] = to->unit[i].eyeSCoeff.y;
				f[2] = to->unit[i].eyeSCoeff.z;
				f[3] = to->unit[i].eyeSCoeff.w;
				diff_api.TexGenfv (GL_S, GL_EYE_PLANE, (const GLfloat *) f);
				from->unit[i].eyeSCoeff = to->unit[i].eyeSCoeff;
			}
			if (from->unit[i].eyeTCoeff.x != to->unit[i].eyeTCoeff.x ||
					from->unit[i].eyeTCoeff.y != to->unit[i].eyeTCoeff.y ||
					from->unit[i].eyeTCoeff.z != to->unit[i].eyeTCoeff.z ||
					from->unit[i].eyeTCoeff.w != to->unit[i].eyeTCoeff.w) 
			{
				GLfloat f[4];
				f[0] = to->unit[i].eyeTCoeff.x;
				f[1] = to->unit[i].eyeTCoeff.y;
				f[2] = to->unit[i].eyeTCoeff.z;
				f[3] = to->unit[i].eyeTCoeff.w;
				diff_api.TexGenfv (GL_T, GL_EYE_PLANE, (const GLfloat *) f);
				from->unit[i].eyeTCoeff = to->unit[i].eyeTCoeff;
			}
			if (from->unit[i].eyeRCoeff.x != to->unit[i].eyeRCoeff.x ||
					from->unit[i].eyeRCoeff.y != to->unit[i].eyeRCoeff.y ||
					from->unit[i].eyeRCoeff.z != to->unit[i].eyeRCoeff.z ||
					from->unit[i].eyeRCoeff.w != to->unit[i].eyeRCoeff.w) 
			{
				GLfloat f[4];
				f[0] = to->unit[i].eyeRCoeff.x;
				f[1] = to->unit[i].eyeRCoeff.y;
				f[2] = to->unit[i].eyeRCoeff.z;
				f[3] = to->unit[i].eyeRCoeff.w;
				diff_api.TexGenfv (GL_R, GL_EYE_PLANE, (const GLfloat *) f);
				from->unit[i].eyeRCoeff = to->unit[i].eyeRCoeff;
			}
			if (from->unit[i].eyeQCoeff.x != to->unit[i].eyeQCoeff.x ||
					from->unit[i].eyeQCoeff.y != to->unit[i].eyeQCoeff.y ||
					from->unit[i].eyeQCoeff.z != to->unit[i].eyeQCoeff.z ||
					from->unit[i].eyeQCoeff.w != to->unit[i].eyeQCoeff.w) 
			{
				GLfloat f[4];
				f[0] = to->unit[i].eyeQCoeff.x;
				f[1] = to->unit[i].eyeQCoeff.y;
				f[2] = to->unit[i].eyeQCoeff.z;
				f[3] = to->unit[i].eyeQCoeff.w;
				diff_api.TexGenfv (GL_Q, GL_EYE_PLANE, (const GLfloat *) f);
				from->unit[i].eyeQCoeff = to->unit[i].eyeQCoeff;
			}
			diff_api.PopMatrix();
			t->eyeGen[i] &= nbitID;
		}
		if (t->gen[i] & bitID) 
		{
			diff_api.ActiveTextureARB( i + GL_TEXTURE0_ARB );
			if (from->unit[i].gen.s != to->unit[i].gen.s ||
					from->unit[i].gen.t != to->unit[i].gen.t ||
					from->unit[i].gen.r != to->unit[i].gen.r ||
					from->unit[i].gen.q != to->unit[i].gen.q) 
			{
				diff_api.TexGeni (GL_S, GL_TEXTURE_GEN_MODE, to->unit[i].gen.s);
				diff_api.TexGeni (GL_T, GL_TEXTURE_GEN_MODE, to->unit[i].gen.t);
				diff_api.TexGeni (GL_R, GL_TEXTURE_GEN_MODE, to->unit[i].gen.r);
				diff_api.TexGeni (GL_Q, GL_TEXTURE_GEN_MODE, to->unit[i].gen.q);	
				from->unit[i].gen = to->unit[i].gen;
			}
			t->gen[i] &= nbitID;
		}
		t->dirty &= nbitID;

		/* Texture enviroment  */
		if (t->envBit[i] & bitID) 
		{
			if (from->unit[i].envMode != to->unit[i].envMode) 
			{
				diff_api.TexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, to->unit[i].envMode);
				from->unit[i].envMode = to->unit[i].envMode;
			}
			if (from->unit[i].envColor.r != to->unit[i].envColor.r ||
					from->unit[i].envColor.g != to->unit[i].envColor.g ||
					from->unit[i].envColor.b != to->unit[i].envColor.b ||
					from->unit[i].envColor.a != to->unit[i].envColor.a) 
			{
				GLfloat f[4];
				f[0] = to->unit[i].envColor.r;
				f[1] = to->unit[i].envColor.g;
				f[2] = to->unit[i].envColor.b;
				f[3] = to->unit[i].envColor.a;
				diff_api.TexEnvfv (GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, (const GLfloat *) f);
				from->unit[i].envColor = to->unit[i].envColor;
			}
			t->envBit[i] &= nbitID;
		}
	}
}
