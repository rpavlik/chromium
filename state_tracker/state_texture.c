/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "state.h"
#include "state/cr_statetypes.h"
#include "cr_pixeldata.h"
#include "cr_string.h"
#include "cr_mem.h"
#include "cr_version.h"
#include "state_internals.h"

#define UNIMPLEMENTED() crStateError(__LINE__,__FILE__,GL_INVALID_OPERATION, "Unimplemented something or other" )
#define UNUSED(x) ((void) (x))

#define GET_TOBJ(tobj,state,id) 	for (tobj = state->mapping[id%CRTEXTURE_HASHSIZE]; tobj && tobj->name != id; tobj = tobj->next){}

void crStateTextureInitTextureObj (CRTextureState *t, CRTextureObj *tobj, GLuint name, GLenum target);
void crStateTextureInitTextureFormat( CRTextureLevel *tl, GLenum internalFormat );

CRTextureObj *crStateTextureAllocate_t(CRTextureState *t, GLuint name);

void crStateTextureDelete_t(CRTextureState *t, GLuint name);

void crStateTextureInit(const CRLimitsState *limits, CRTextureState *t) 
{
  unsigned int i, h;
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
	for (i = 0; i < limits->maxTextureUnits; i++)
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
	int k;
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
	tobj->priority      = 1.0f;
	tobj->minLod        = -1000.0;
	tobj->maxLod        = 1000.0;
	tobj->baseLevel     = 0;
	tobj->maxLevel      = 1000;
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
		tl->internalFormat    = 1;										\
		tl->bytesPerPixel = 0;										\
		tl->format        = GL_RGBA;								\
		tl->type          = GL_UNSIGNED_BYTE;						\
		crStateTextureInitTextureFormat( tl, tl->internalFormat );				\
		for (j = 0; j < CR_MAX_TEXTURE_UNITS; j++)					\
		{															\
			for (k = 0; k < CR_MAX_BITARRAY; k++) \
				tl->dirty[j][k]     = 0;  /* By default this level is ignored.*/	\
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
	FILLDIRTY(tobj->dirty);
	for (i = 0; i < CR_MAX_TEXTURE_UNITS; i++)
	{
		FILLDIRTY(tobj->paramsBit[i]);
		FILLDIRTY(tobj->imageBit[i]);
	}
}

/* ================================================================
 * Texture internal formats:
 */

const struct CRTextureFormat _texformat_rgba8888 = {
   8,				/* RedBits */
   8,				/* GreenBits */
   8,				/* BlueBits */
   8,				/* AlphaBits */
   0,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
};

const struct CRTextureFormat _texformat_abgr8888 = {
   8,				/* RedBits */
   8,				/* GreenBits */
   8,				/* BlueBits */
   8,				/* AlphaBits */
   0,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
};

const struct CRTextureFormat _texformat_argb8888 = {
   8,				/* RedBits */
   8,				/* GreenBits */
   8,				/* BlueBits */
   8,				/* AlphaBits */
   0,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
};

const struct CRTextureFormat _texformat_rgb888 = {
   8,				/* RedBits */
   8,				/* GreenBits */
   8,				/* BlueBits */
   0,				/* AlphaBits */
   0,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
};

const struct CRTextureFormat _texformat_bgr888 = {
   8,				/* RedBits */
   8,				/* GreenBits */
   8,				/* BlueBits */
   0,				/* AlphaBits */
   0,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
};

const struct CRTextureFormat _texformat_rgb565 = {
   5,				/* RedBits */
   6,				/* GreenBits */
   5,				/* BlueBits */
   0,				/* AlphaBits */
   0,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
};

const struct CRTextureFormat _texformat_argb4444 = {
   4,				/* RedBits */
   4,				/* GreenBits */
   4,				/* BlueBits */
   4,				/* AlphaBits */
   0,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
};

const struct CRTextureFormat _texformat_argb1555 = {
   5,				/* RedBits */
   5,				/* GreenBits */
   5,				/* BlueBits */
   1,				/* AlphaBits */
   0,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
};

const struct CRTextureFormat _texformat_al88 = {
   0,				/* RedBits */
   0,				/* GreenBits */
   0,				/* BlueBits */
   8,				/* AlphaBits */
   8,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
};

const struct CRTextureFormat _texformat_rgb332 = {
   3,				/* RedBits */
   3,				/* GreenBits */
   2,				/* BlueBits */
   0,				/* AlphaBits */
   0,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
};

const struct CRTextureFormat _texformat_a8 = {
   0,				/* RedBits */
   0,				/* GreenBits */
   0,				/* BlueBits */
   8,				/* AlphaBits */
   0,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
};

const struct CRTextureFormat _texformat_l8 = {
   0,				/* RedBits */
   0,				/* GreenBits */
   0,				/* BlueBits */
   0,				/* AlphaBits */
   8,				/* LuminanceBits */
   0,				/* IntensityBits */
   0,				/* IndexBits */
};

const struct CRTextureFormat _texformat_i8 = {
   0,				/* RedBits */
   0,				/* GreenBits */
   0,				/* BlueBits */
   0,				/* AlphaBits */
   0,				/* LuminanceBits */
   8,				/* IntensityBits */
   0,				/* IndexBits */
};

const struct CRTextureFormat _texformat_ci8 = {
   0,				/* RedBits */
   0,				/* GreenBits */
   0,				/* BlueBits */
   0,				/* AlphaBits */
   0,				/* LuminanceBits */
   0,				/* IntensityBits */
   8,				/* IndexBits */
};

/*
 * Given an internal texture format enum or 1, 2, 3, 4 return the
 * corresponding _base_ internal format:  GL_ALPHA, GL_LUMINANCE,
 * GL_LUMANCE_ALPHA, GL_INTENSITY, GL_RGB, or GL_RGBA.
 * Return -1 if invalid enum.
 */
void crStateTextureInitTextureFormat( CRTextureLevel *tl, GLenum internalFormat )
{
   switch ( internalFormat ) {
   case 4:
   case GL_RGBA:
      tl->texFormat = &_texformat_abgr8888;
      break;

   case 3:
   case GL_RGB:
      tl->texFormat = &_texformat_bgr888;
      break;

   case GL_RGBA2:
   case GL_RGBA4:
   case GL_RGB5_A1:
   case GL_RGBA8:
   case GL_RGB10_A2:
   case GL_RGBA12:
   case GL_RGBA16:
      tl->texFormat = &_texformat_abgr8888;
      break;

   case GL_R3_G3_B2:
   case GL_RGB4:
   case GL_RGB5:
   case GL_RGB8:
   case GL_RGB10:
   case GL_RGB12:
   case GL_RGB16:
      tl->texFormat = &_texformat_bgr888;
      break;

   case GL_ALPHA:
   case GL_ALPHA4:
   case GL_ALPHA8:
   case GL_ALPHA12:
   case GL_ALPHA16:
      tl->texFormat = &_texformat_a8;
      break;

   case 1:
   case GL_LUMINANCE:
   case GL_LUMINANCE4:
   case GL_LUMINANCE8:
   case GL_LUMINANCE12:
   case GL_LUMINANCE16:
      tl->texFormat = &_texformat_l8;
      break;

   case 2:
   case GL_LUMINANCE_ALPHA:
   case GL_LUMINANCE4_ALPHA4:
   case GL_LUMINANCE6_ALPHA2:
   case GL_LUMINANCE8_ALPHA8:
   case GL_LUMINANCE12_ALPHA4:
   case GL_LUMINANCE12_ALPHA12:
   case GL_LUMINANCE16_ALPHA16:
      tl->texFormat = &_texformat_al88;
      break;

   case GL_INTENSITY:
   case GL_INTENSITY4:
   case GL_INTENSITY8:
   case GL_INTENSITY12:
   case GL_INTENSITY16:
      tl->texFormat = &_texformat_i8;
      break;

   case GL_COLOR_INDEX:
   case GL_COLOR_INDEX1_EXT:
   case GL_COLOR_INDEX2_EXT:
   case GL_COLOR_INDEX4_EXT:
   case GL_COLOR_INDEX8_EXT:
   case GL_COLOR_INDEX12_EXT:
   case GL_COLOR_INDEX16_EXT:
      tl->texFormat = &_texformat_ci8;
      break;

   default:
      return;
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
			if (name == t->unit[t->curTextureUnit].currentTexture3DName) 
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

	DIRTY(tb->dirty, g->neg_bitid);
	DIRTY(tb->current[t->curTextureUnit], g->neg_bitid);
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

	if ( texture < GL_TEXTURE0_ARB || texture >= GL_TEXTURE0_ARB + g->limits.maxTextureUnits)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "Bad texture unit passed to crStateClientActiveTexture: %d (max is %d)", texture, g->limits.maxTextureUnits );
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

	if ( texture < GL_TEXTURE0_ARB || texture >= GL_TEXTURE0_ARB + g->limits.maxTextureUnits)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "Bad texture unit passed to crStateActiveTexture: %d (max is %d)", texture, g->limits.maxTextureUnits );
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
				t->currentTexture3D = &(t->base3D);
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

		DIRTY(tb->dirty, g->neg_bitid);
		DIRTY(tb->current[t->curTextureUnit], g->neg_bitid);
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
			t->currentTexture3D = tobj;
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

	DIRTY(tb->dirty, g->neg_bitid);
	DIRTY(tb->current[t->curTextureUnit], g->neg_bitid);
}



void STATE_APIENTRY crStateTexImage1D (GLenum target, GLint level, GLint internalFormat, 
		GLsizei width, GLint border, GLenum format,
		GLenum type, const GLvoid *pixels  ) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRClientState *c = &(g->client);
	CRTextureObj *tobj;
	CRTextureLevel *tl;
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);
  unsigned int i;

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

	if (width > ((int) g->limits.maxTextureSize + 2))
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
		tl->bytes = crImageSize(format, type, width, 1);
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
			crPixelCopy1D((GLvoid *) tl->img, format, type,
										pixels, format, type, width, &(c->unpack));
	}

	tl->width = width;
	tl->height = 1;
	tl->depth = 1;
	tl->format = format;
	tl->border = border;
	tl->internalFormat = internalFormat;
	crStateTextureInitTextureFormat( tl, internalFormat );
	tl->type = type;
	if (width)
		tl->bytesPerPixel = tl->bytes / width;
	else
		tl->bytesPerPixel = 0;

	/* XXX may need to do some fine-tuning here for proxy textures */
	DIRTY(tobj->dirty, g->neg_bitid);
	for (i = 0; i < g->limits.maxTextureUnits; i++)
	{
		DIRTY(tl->dirty[i], g->neg_bitid);
		DIRTY(tobj->imageBit[i], g->neg_bitid);
	}
	DIRTY(tb->dirty, g->neg_bitid);
}

void STATE_APIENTRY crStateTexImage2D (GLenum target, GLint level, GLint internalFormat, 
		GLsizei width, GLsizei height, GLint border,
		GLenum format, GLenum type, const GLvoid *pixels  ) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRClientState *c = &(g->client);
	CRTextureObj *tobj = NULL;
	CRTextureLevel *tl = NULL;
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);
	unsigned int i;
	int is_distrib = ( (type == GL_TRUE) || (type == GL_FALSE) ) ;

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

	if (level < 0 || ((target == GL_TEXTURE_2D || target == GL_PROXY_TEXTURE_2D) && level > t->maxLevel) 
#ifdef CR_ARB_texture_cube_map
			|| ((target != GL_TEXTURE_2D && target != GL_PROXY_TEXTURE_2D) && level > t->maxCubeMapLevel)
#endif
	   )
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
	if ( width < 0 || ((i!=1) && (!is_distrib)) )
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
	if ( height < 0 || ((i!=1) && (!is_distrib)) )
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexImage2D height is not valid: %d", height);
		return;
	}

	if (target == GL_TEXTURE_2D)
	{
		if (width > ((int) g->limits.maxTextureSize + 2))
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
				     "glTexImage2D width oob: %d", width);
			return;
		}
		if (height > ((int) g->limits.maxTextureSize + 2))
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
				     "glTexImage2D height oob: %d", height);
			return;
		}
	}
	else if (target == GL_PROXY_TEXTURE_2D)
	{
		if (width > ((int) g->limits.maxTextureSize + 2) || height > ((int) g->limits.maxTextureSize + 2))
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
		if ( is_distrib )
		{
			tl->bytes = crStrlen(pixels) + 1 +
				( type == GL_TRUE ? width*height*3 : 0 ) ;
		}
		else
		{
			tl->bytes = crImageSize(format, type, width, height);
		}
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
		tl->bytes = crImageSize(format, type, width, height);
	}
	else if (target == GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB)
	{
		tobj = t->currentTextureCubeMap;
		tl = tobj->negativeXlevel + level;
		tl->bytes = crImageSize(format, type, width, height);
	}
	else if (target == GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB)
	{
		tobj = t->currentTextureCubeMap;
		tl = tobj->positiveYlevel + level;
		tl->bytes = crImageSize(format, type, width, height);
	}
	else if (target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB)
	{
		tobj = t->currentTextureCubeMap;
		tl = tobj->negativeYlevel + level;
		tl->bytes = crImageSize(format, type, width, height);
	}
	else if (target == GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB)
	{
		tobj = t->currentTextureCubeMap;
		tl = tobj->positiveZlevel + level;
		tl->bytes = crImageSize(format, type, width, height);
	}
	else if (target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB)
	{
		tobj = t->currentTextureCubeMap;
		tl = tobj->negativeZlevel + level;
		tl->bytes = crImageSize(format, type, width, height);
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
		{
			if ( is_distrib )
			{
				crMemcpy( (void*)tl->img, (void*)pixels, tl->bytes ) ;
			}
			else
			{
				crPixelCopy2D( width, height,
										 (GLvoid *) (tl->img), format, type, NULL, /* dst */
										 pixels, format,type, &(c->unpack) );  /* src */
			}
		}
	}

	tl->width = width;
	tl->height = height;
	tl->depth = 1;
	tl->format = format;
	tl->internalFormat = internalFormat;
	crStateTextureInitTextureFormat( tl, internalFormat );
	tl->border = border;
	tl->type = type;
	if (width && height)
	{
		if ( is_distrib )
			tl->bytesPerPixel = 3 ;
		else
			tl->bytesPerPixel = tl->bytes / (width * height);
	}
	else
		tl->bytesPerPixel = 0;

	/* XXX may need to do some fine-tuning here for proxy textures */
	DIRTY(tobj->dirty, g->neg_bitid);
	for (i = 0; i < g->limits.maxTextureUnits; i++)
	{
		DIRTY(tl->dirty[i], g->neg_bitid);
		DIRTY(tobj->imageBit[i], g->neg_bitid);
	}
	DIRTY(tb->dirty, g->neg_bitid);
}

void STATE_APIENTRY crStateTexSubImage1D (GLenum target, GLint level, GLint xoffset, 
		GLsizei width, GLenum format,
		GLenum type, const GLvoid *pixels  ) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);
	CRTextureObj *tobj = t->currentTexture1D;
	CRTextureLevel *tl = tobj->level + level;
  unsigned int i;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glTexSubImage1D called in Begin/End");
		return;
	}

	FLUSH();

	if (target != GL_TEXTURE_1D && target != GL_PROXY_TEXTURE_1D)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glTexSubImage1D target != GL_TEXTURE_1D: %d", target);
		return;
	}

	if (level < 0 || level > t->maxLevel) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
			"glTexSubImage1D level oob: %d", level);
		return;
	}

	if (width > ((int) g->limits.maxTextureSize + 2))
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
					"glTexSubImage1D width oob: %d", width);
		return;
	}

	if (width + xoffset > tl->width) 
	{
		crStateError( __LINE__, __FILE__, GL_INVALID_VALUE, "glTexSubImage1D(bad width or xoffset)" );
		return;
	}

	xoffset += tl->border;

	crPixelCopy1D((void *)(tl->img + xoffset*tl->bytesPerPixel),
								tl->format, tl->type,
								pixels, format, type, width, &(c->unpack) );

	DIRTY(tobj->dirty, g->neg_bitid);
	for (i = 0; i < g->limits.maxTextureUnits; i++)
	{
		DIRTY(tl->dirty[i], g->neg_bitid);
		DIRTY(tobj->imageBit[i], g->neg_bitid);
	}
	DIRTY(tb->dirty, g->neg_bitid);
}

void STATE_APIENTRY crStateTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, 
		GLsizei width, GLsizei height,
		GLenum format, GLenum type, const GLvoid *pixels  ) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRClientState *c = &(g->client);
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
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glTexSubImage2D called in Begin/End");
		return;
	}

	FLUSH();

	if (target == GL_TEXTURE_2D)
	{
		if (level < 0 || level > t->maxLevel)
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glTexSubImage2D level oob: %d", level);
			return;
		}
		if (width < 0 || width > ((int) g->limits.maxTextureSize + 2))
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
				     "glTexSubImage2D width oob: %d", width);
			return;
		}
		if (height < 0 || height > ((int) g->limits.maxTextureSize + 2))
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
				     "glTexSubImage2D height oob: %d", height);
			return;
		}
	}
#ifdef CR_ARB_texture_cube_map
	else if (target >= GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB && target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB && g->extensions.ARB_texture_cube_map)
	{
		if (level < 0 || level > t->maxCubeMapLevel)
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glTexSubImage2D level oob: %d", level);
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
				     "glTexSubImage2D height oob: %d", height);
			return;
		}
	}
#endif /* CR_ARB_texture_cube_map */
	else
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glTexSubImage2D target != GL_TEXTURE_2D: %d", target);
		return;
	}

	if (width + xoffset > tl->width) 
	{
		crStateError( __LINE__, __FILE__, GL_INVALID_VALUE,
									"glTexSubImage2D(bad width or xoffset)" );
		return;
	}
	if (height + yoffset > tl->height) 
	{
		crStateError( __LINE__, __FILE__, GL_INVALID_VALUE,
									"glTexSubImage2D(bad heigh or yoffset)" );
		return;
	}

	xoffset += tl->border;
	yoffset += tl->border;

	subimg = (GLubyte *) crAlloc (crImageSize(tl->format, tl->type, width, height));

	crPixelCopy2D( width, height,
								 subimg, tl->format, tl->type, NULL,  /* dst */
								 pixels, format, type, &(c->unpack) );  /* src */

	img = tl->img +
			xoffset * tl->bytesPerPixel +
			yoffset * tl->width * tl->bytesPerPixel;

	src = subimg;

	/* Copy the data into the texture */
	for (i=0; i<height; i++) 
	{
		crMemcpy (img, src, tl->bytesPerPixel * width);
		img += tl->width * tl->bytesPerPixel;
		src += width * tl->bytesPerPixel;
	}

	crFree (subimg);

	DIRTY(tobj->dirty, g->neg_bitid);
  {
    unsigned int i;
	for (i = 0; i < g->limits.maxTextureUnits; i++)
	{
		DIRTY(tobj->imageBit[i], g->neg_bitid);
		DIRTY(tl->dirty[i], g->neg_bitid);
	}
  }
	DIRTY(tb->dirty, g->neg_bitid);
}

#if defined( CR_OPENGL_VERSION_1_2 )
void STATE_APIENTRY crStateTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels  )
{
        CRContext *g = GetCurrentContext();
        CRTextureState *t = &(g->texture);
        CRClientState *c = &(g->client);
        CRStateBits *sb = GetCurrentBits();
        CRTextureBits *tb = &(sb->texture);
        CRTextureObj *tobj = t->currentTexture3D;
        CRTextureLevel *tl = tobj->level + level;
        int i;

        GLubyte *subimg = NULL;
        GLubyte *img = NULL;
        GLubyte *src;

        if (g->current.inBeginEnd)
        {
                crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
                                                                 "glTexSubImage3D called in Begin/End");
                return;
        }

        FLUSH();

        if (target == GL_TEXTURE_3D)
        {
                if (level < 0 || level > t->maxLevel)
                {
                        crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
                                                                         "glTexSubImage3D level oob: %d", level);
                        return;
                }
                if (width < 0 || width > ((int) g->limits.maxTextureSize + 2))
                {
                        crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
                                     "glTexSubImage3D width oob: %d", width);
                        return;
                }
                if (height < 0 || height > ((int) g->limits.maxTextureSize + 2))
                {
                        crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
                                     "glTexSubImage3D height oob: %d", height);
                        return;
                }
		if (depth < 0 || depth > ((int) g->limits.maxTextureSize + 2))
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexSubImage3D depth oob: %d", depth);
			return;
		}
        }
#ifdef CR_ARB_texture_cube_map
        else if (target >= GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB && target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB && g->extensions.ARB_texture_cube_map)
        {
                if (level < 0 || level > t->maxCubeMapLevel)
                {
                        crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
                                                                         "glTexSubImage3D level oob: %d", level);
                        return;
                }
                if (width < 0 || width > (int) g->limits.maxCubeMapTextureSize)
                {
                        crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
                                     "glTexSubImage3D width oob: %d", width);
                        return;
                }
                if (height < 0 || height > (int) g->limits.maxCubeMapTextureSize)
                {
                        crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
                                     "glTexSubImage3D height oob: %d", height);
                        return;
                }
		if (depth < 0 || depth > (int) g->limits.maxCubeMapTextureSize)
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexSubImage3D depth oob:  %d", depth);
		}
        }
#endif /* CR_ARB_texture_cube_map */
        else
        {
                crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glTexSubImage3D target != GL_TEXTURE_3D: %d", target);
                return;
        }

        if (width + xoffset > tl->width)
        {
                crStateError( __LINE__, __FILE__, GL_INVALID_VALUE, "glTexSubImage3D(bad width or xoffset)" );
                return;
        }
        if (height + yoffset > tl->height)
        {
                crStateError( __LINE__, __FILE__, GL_INVALID_VALUE, "glTexSubImage3D(bad height or yoffset)" );
                return;
        }
	if (depth + zoffset > tl->depth)
	{
		crStateError( __LINE__, __FILE__, GL_INVALID_VALUE, "glTexSubImage3D(bad depth or zoffset)" );
		return;
	}

        xoffset += tl->border;
        yoffset += tl->border;
	zoffset += tl->border;

        subimg = (GLubyte *) crAlloc (crTextureSize(tl->format, tl->type, width, height, depth));

        crPixelCopy3D( width, height, depth, subimg, tl->format, tl->type, NULL,  pixels, format, type, &(c->unpack) );

        img = tl->img + xoffset * tl->bytesPerPixel +
                        yoffset * tl->width * tl->bytesPerPixel +
			zoffset * tl->width * tl->height * tl->bytesPerPixel;

        src = subimg;

        /* Copy the data into the texture */
       	for (i=0; i<depth; i++)
       	{
               	crMemcpy (img, src, tl->bytesPerPixel * width * height);
               	img += tl->width * tl->height * tl->bytesPerPixel;
               	src += width * height * tl->bytesPerPixel;
       	}

        crFree (subimg);

        DIRTY(tobj->dirty, g->neg_bitid);
  {
    unsigned int i;
        for (i = 0; i < g->limits.maxTextureUnits; i++)
        {
                DIRTY(tobj->imageBit[i], g->neg_bitid);
                DIRTY(tl->dirty[i], g->neg_bitid);
        }
  }
        DIRTY(tb->dirty, g->neg_bitid);
}
#endif /* CR_OPENGL_VERSION_1_2 || GL_EXT_texture3D */

#if defined( CR_OPENGL_VERSION_1_2 ) || defined( GL_EXT_texture3D ) 
void STATE_APIENTRY crStateTexImage3D (GLenum target, GLint level,
#if defined(IRIX) || defined(IRIX64) || defined(AIX) || defined (SunOS)
                                                                         GLenum internalFormat,
#else
                                                                         GLint internalFormat,
#endif
		GLsizei width, GLsizei height, GLsizei depth,
		GLint border, GLenum format, GLenum type, const GLvoid *pixels  ) {
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRClientState *c = &(g->client);
	CRTextureObj *tobj = NULL;
	CRTextureLevel *tl = NULL;
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);
  	unsigned int i;

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

	if (level < 0 || level > t->max3DLevel) 
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

	/* check the bits in depth */
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

	if (width > (int)(g->limits.max3DTextureSize + 2))
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
		}
		return;
	}

	if (height > (int)(g->limits.max3DTextureSize + 2))
	{
		if (target == GL_PROXY_TEXTURE_3D)
		{
			/* clear all the texture object state */
			crStateTextureInitTextureObj(t, &(t->proxy3D), 0, GL_TEXTURE_3D);
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
		tl->bytes = crTextureSize( format, type, width, height, depth );
	}
	else if (target == GL_PROXY_TEXTURE_3D)
	{
		tobj = &(t->proxy3D);  /* FIXME: per unit! */
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
			crPixelCopy3D( width, height, depth, (GLvoid *)(tl->img), format, type, NULL, pixels, format, type, &(c->unpack) );
	}

	tl->internalFormat = internalFormat;
	tl->border = border;
	tl->width = width;
	tl->height = height;
	tl->depth = depth;
	tl->format = format;
	tl->type = type;

	/* XXX may need to do some fine-tuning here for proxy textures */
	DIRTY(tobj->dirty, g->neg_bitid);
	for (i = 0; i < g->limits.maxTextureUnits; i++)
	{
		DIRTY(tl->dirty[i], g->neg_bitid);
		DIRTY(tobj->imageBit[i], g->neg_bitid);
	}
	DIRTY(tb->dirty, g->neg_bitid);
}
#endif /* CR_OPENGL_VERSION_1_2 || GL_EXT_texture3D */

#ifdef GL_EXT_texture3D
void STATE_APIENTRY crStateTexImage3DEXT (GLenum target, GLint level,
                                          GLenum internalFormat,
		GLsizei width, GLsizei height, GLsizei depth,
		GLint border, GLenum format, GLenum type, const GLvoid *pixels  ) {
	crStateTexImage3D( target, level, (GLint)internalFormat, width, height, depth, border, format, type, pixels );
}
#endif /* GL_EXT_texture3D */

void STATE_APIENTRY crStateTexParameterfv (GLenum target, GLenum pname, const GLfloat *param) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRTextureObj *tobj = NULL;
	GLenum e = (GLenum) *param;
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);
  	unsigned int i;

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
#ifdef CR_OPENGL_VERSION_1_2
			else if (e == GL_CLAMP_TO_EDGE) {
				tobj->wrapS = e;
			}
#endif
#ifdef GL_CLAMP_TO_EDGE_EXT
			else if (e == GL_CLAMP_TO_EDGE_EXT && g->extensions.EXT_texture_edge_clamp) {
				tobj->wrapS = e;
			}
#endif
#ifdef GL_CLAMP_TO_BORDER_SGIS
			else if (e == GL_CLAMP_TO_BORDER_SGIS && g->extensions.ARB_texture_border_clamp) {
				tobj->wrapS = e;
			}
#endif
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
#ifdef CR_OPENGL_VERSION_1_2
			else if (e == GL_CLAMP_TO_EDGE) {
				tobj->wrapT = e;
			}
#endif
#ifdef GL_CLAMP_TO_EDGE_EXT
			else if (e == GL_CLAMP_TO_EDGE_EXT && g->extensions.EXT_texture_edge_clamp) {
				tobj->wrapT = e;
			}
#endif
#ifdef GL_CLAMP_TO_BORDER_SGIS
			else if (e == GL_CLAMP_TO_BORDER_SGIS && g->extensions.ARB_texture_border_clamp) {
				tobj->wrapT = e;
			}
#endif
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
			else if (e == GL_CLAMP_TO_EDGE) {
				tobj->wrapR = e;
			}
#ifdef GL_CLAMP_TO_EDGE_EXT
			else if (e == GL_CLAMP_TO_EDGE_EXT && g->extensions.EXT_texture_edge_clamp) {
				tobj->wrapR = e;
			}
#endif
#ifdef GL_CLAMP_TO_BORDER_SGIS
			else if (e == GL_CLAMP_TO_BORDER_SGIS && g->extensions.ARB_texture_border_clamp) {
				tobj->wrapR = e;
			}
#endif
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					"TexParameterfv: GL_TEXTURE_WRAP_R invalid param: 0x%x", e);
				return;
			}
			break;
		case GL_TEXTURE_PRIORITY:
			tobj->priority = param[0];
			break;
		case GL_TEXTURE_MIN_LOD:
			tobj->minLod = param[0];
			break;
		case GL_TEXTURE_MAX_LOD:
			tobj->maxLod = param[0];
			break;
		case GL_TEXTURE_BASE_LEVEL:
			if (e < 0.0f) 
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					"TexParameterfv: GL_TEXTURE_BASE_LEVEL invalid param: 0x%x", e);
				return;
			}
			tobj->baseLevel = e;
			break;
		case GL_TEXTURE_MAX_LEVEL:
			if (e < 0.0f) 
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					"TexParameterfv: GL_TEXTURE_MAX_LEVEL invalid param: 0x%x", e);
				return;
			}
			tobj->maxLevel = e;
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

	DIRTY(tobj->dirty, g->neg_bitid);
	for (i = 0; i < g->limits.maxTextureUnits; i++)
	{
		DIRTY(tobj->paramsBit[i], g->neg_bitid);
	}
	DIRTY(tb->dirty, g->neg_bitid);
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
#ifdef CR_OPENGL_VERSION_1_2
		case GL_TEXTURE_WRAP_R:
		case GL_TEXTURE_PRIORITY:
		case GL_TEXTURE_MIN_LOD:
		case GL_TEXTURE_MAX_LOD:
		case GL_TEXTURE_BASE_LEVEL:
		case GL_TEXTURE_MAX_LEVEL:
#endif
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

	DIRTY(tb->envBit[t->curTextureUnit], g->neg_bitid);
	DIRTY(tb->dirty, g->neg_bitid);
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
			param[1] = (GLint) (t->unit[t->curTextureUnit].envColor.g * GL_MAXINT);
			param[2] = (GLint) (t->unit[t->curTextureUnit].envColor.b * GL_MAXINT);
			param[3] = (GLint) (t->unit[t->curTextureUnit].envColor.a * GL_MAXINT);
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
						DIRTY(tb->gen[t->curTextureUnit], g->neg_bitid);
						DIRTY(tb->dirty, g->neg_bitid);
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
					DIRTY(tb->objGen[t->curTextureUnit], g->neg_bitid);
					DIRTY(tb->dirty, g->neg_bitid);
					break;
				case GL_EYE_PLANE:
					v.x = (GLfloat) param[0];
					v.y = (GLfloat) param[1];
					v.z = (GLfloat) param[2];
					v.w = (GLfloat) param[3];
					crStateTransformInvertTransposeMatrix(&inv, trans->modelView+trans->modelViewDepth);
					crStateTransformXformPointMatrixf(&inv, &v);
					t->unit[t->curTextureUnit].eyeSCoeff = v;
					DIRTY(tb->eyeGen[t->curTextureUnit], g->neg_bitid);
					DIRTY(tb->dirty, g->neg_bitid);
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
						DIRTY(tb->gen[t->curTextureUnit], g->neg_bitid);
						DIRTY(tb->dirty, g->neg_bitid);
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
					DIRTY(tb->objGen[t->curTextureUnit], g->neg_bitid);
					DIRTY(tb->dirty, g->neg_bitid);
					break;
				case GL_EYE_PLANE:
					v.x = (GLfloat) param[0];
					v.y = (GLfloat) param[1];
					v.z = (GLfloat) param[2];
					v.w = (GLfloat) param[3];
					crStateTransformInvertTransposeMatrix(&inv, trans->modelView+trans->modelViewDepth);
					crStateTransformXformPointMatrixf(&inv, &v);
					t->unit[t->curTextureUnit].eyeTCoeff = v;
					DIRTY(tb->eyeGen[t->curTextureUnit], g->neg_bitid);
					DIRTY(tb->dirty, g->neg_bitid);
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
						DIRTY(tb->gen[t->curTextureUnit], g->neg_bitid);
						DIRTY(tb->dirty, g->neg_bitid);
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
					DIRTY(tb->objGen[t->curTextureUnit], g->neg_bitid);
					DIRTY(tb->dirty, g->neg_bitid);
					break;
				case GL_EYE_PLANE:
					v.x = (GLfloat) param[0];
					v.y = (GLfloat) param[1];
					v.z = (GLfloat) param[2];
					v.w = (GLfloat) param[3];
					crStateTransformInvertTransposeMatrix(&inv, trans->modelView+trans->modelViewDepth);
					crStateTransformXformPointMatrixf(&inv, &v);
					t->unit[t->curTextureUnit].eyeRCoeff = v;
					DIRTY(tb->eyeGen[t->curTextureUnit], g->neg_bitid);
					DIRTY(tb->dirty, g->neg_bitid);
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
						DIRTY(tb->gen[t->curTextureUnit], g->neg_bitid);
						DIRTY(tb->dirty, g->neg_bitid);
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
					DIRTY(tb->objGen[t->curTextureUnit], g->neg_bitid);
					DIRTY(tb->dirty, g->neg_bitid);
					break;
				case GL_EYE_PLANE:
					v.x = (GLfloat) param[0];
					v.y = (GLfloat) param[1];
					v.z = (GLfloat) param[2];
					v.w = (GLfloat) param[3];
					crStateTransformInvertTransposeMatrix(&inv, trans->modelView+trans->modelViewDepth);
					crStateTransformXformPointMatrixf(&inv, &v);
					t->unit[t->curTextureUnit].eyeQCoeff = v;
					DIRTY(tb->eyeGen[t->curTextureUnit], g->neg_bitid);
					DIRTY(tb->dirty, g->neg_bitid);
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
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRClientState *c = &(g->client);
	CRTextureObj *tobj = NULL;
	CRTextureLevel *tl = NULL;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					"glGetTexImage called in begin/end");
		return;
	}

#ifdef CR_OPENGL_VERSION_1_2
	if (target == GL_TEXTURE_3D )
	{
		tobj = t->currentTexture3D;
		tl = tobj->level+level;
	} else
#endif
	if (target == GL_TEXTURE_2D )
	{
		tobj = t->currentTexture2D;
		tl = tobj->level+level;
	} else
	if (target == GL_TEXTURE_1D )
	{
		tobj = t->currentTexture1D;
		tl = tobj->level+level;
	} else {
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
			"glGetTexImage called with bogus target: %d", target);
		return;
	}

	switch (format) {
		case GL_RED:
		case GL_GREEN:
		case GL_BLUE:
		case GL_ALPHA:
		case GL_RGB:
		case GL_RGBA:
		case GL_LUMINANCE:
		case GL_LUMINANCE_ALPHA:
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
				"glGetTexImage called with bogus format: %d", format);
		return;
	}

	switch (type) {
		case GL_UNSIGNED_BYTE:
		case GL_BYTE:
		case GL_UNSIGNED_SHORT:
		case GL_SHORT:
		case GL_UNSIGNED_INT:
		case GL_INT:
		case GL_FLOAT:
			break;
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
				"glGetTexImage called with bogus type: %d", type);
		return;
	}

#ifdef CR_OPENGL_VERSION_1_2
	if ( target == GL_TEXTURE_3D )
	{
		crPixelCopy3D( tl->width, tl->height, tl->depth, (GLvoid *) pixels, format, type, NULL, (tl->img), format, type, &(c->pack) );
	} else
#endif
	if ( ( target == GL_TEXTURE_2D ) || ( target == GL_TEXTURE_1D ) )
	{
		crPixelCopy2D( tl->width, tl->height,
								 (GLvoid *) pixels, format, type, NULL,  /* dst */
								 (tl->img), format, type, &(c->pack) );  /* src */
	}
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
			tobj = &(t->proxy3D);
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
		case GL_TEXTURE_INTERNAL_FORMAT:
			*params = (GLfloat) timg->internalFormat;
			break;
		case GL_TEXTURE_BORDER:
			*params = (GLfloat) timg->border;
			break;
		case GL_TEXTURE_RED_SIZE:
			*params = (GLfloat) timg->texFormat->redbits;
			break;
		case GL_TEXTURE_GREEN_SIZE:
			*params = (GLfloat) timg->texFormat->greenbits;
			break;
		case GL_TEXTURE_BLUE_SIZE:
			*params = (GLfloat) timg->texFormat->bluebits;
			break;
		case GL_TEXTURE_ALPHA_SIZE:
			*params = (GLfloat) timg->texFormat->alphabits;
			break;
		case GL_TEXTURE_INTENSITY_SIZE:
			*params = (GLfloat) timg->texFormat->intensitybits;
			break;
		case GL_TEXTURE_LUMINANCE_SIZE:
			*params = (GLfloat) timg->texFormat->luminancebits;
			break;
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
		case GL_TEXTURE_INTERNAL_FORMAT:
			*params = (GLint) timg->internalFormat;
			break;
		case GL_TEXTURE_BORDER:
			*params = (GLint) timg->border;
			break;
		case GL_TEXTURE_RED_SIZE:
			*params = (GLint) timg->texFormat->redbits;
			break;
		case GL_TEXTURE_GREEN_SIZE:
			*params = (GLint) timg->texFormat->greenbits;
			break;
		case GL_TEXTURE_BLUE_SIZE:
			*params = (GLint) timg->texFormat->bluebits;
			break;
		case GL_TEXTURE_ALPHA_SIZE:
			*params = (GLint) timg->texFormat->alphabits;
			break;
		case GL_TEXTURE_INTENSITY_SIZE:
			*params = (GLint) timg->texFormat->intensitybits;
			break;
		case GL_TEXTURE_LUMINANCE_SIZE:
			*params = (GLint) timg->texFormat->luminancebits;
			break;
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
		case GL_TEXTURE_PRIORITY:
			*params = (GLfloat) tobj->priority;
			break;
#endif
		case GL_TEXTURE_BORDER_COLOR:
			params[0] = tobj->borderColor.r;
			params[1] = tobj->borderColor.g;
			params[2] = tobj->borderColor.b;
			params[3] = tobj->borderColor.a;
			break;
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
#ifdef CR_OPENGL_VERSION_1_2
		case GL_TEXTURE_MIN_LOD:
			*params = (GLfloat) tobj->minLod;
			break;
		case GL_TEXTURE_MAX_LOD:
			*params = (GLfloat) tobj->maxLod;
			break;
		case GL_TEXTURE_BASE_LEVEL:
			*params = (GLfloat) tobj->baseLevel;
			break;
		case GL_TEXTURE_MAX_LEVEL:
			*params = (GLfloat) tobj->maxLevel;
			break;
#endif
        	case GL_TEXTURE_RESIDENT:
			/* XXX todo */
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
		case GL_TEXTURE_PRIORITY:
			*params = (GLint) tobj->priority;
			break;
#endif
		case GL_TEXTURE_BORDER_COLOR:
			params[0] = (GLint) (tobj->borderColor.r * GL_MAXINT);
			params[1] = (GLint) (tobj->borderColor.g * GL_MAXINT);
			params[2] = (GLint) (tobj->borderColor.b * GL_MAXINT);
			params[3] = (GLint) (tobj->borderColor.a * GL_MAXINT);
			break;
#ifdef CR_OPENGL_VERSION_1_2
		case GL_TEXTURE_MIN_LOD:
			*params = (GLint) tobj->minLod;
			break;
		case GL_TEXTURE_MAX_LOD:
			*params = (GLint) tobj->maxLod;
			break;
		case GL_TEXTURE_BASE_LEVEL:
			*params = (GLint) tobj->baseLevel;
			break;
		case GL_TEXTURE_MAX_LEVEL:
			*params = (GLint) tobj->maxLevel;
			break;
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
        	case GL_TEXTURE_RESIDENT:
			/* XXX todo */
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


GLboolean STATE_APIENTRY crStateIsTexture(GLuint texture)
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRTextureObj *tobj;

	GET_TOBJ(tobj, t, texture);
	return tobj != NULL;
}


void crStateTextureSwitch(CRContext *g, CRTextureBits *t, GLbitvalue *bitID, 
						  CRTextureState *from, CRTextureState *to) 
{
	unsigned int i,j;
	glAble able[2];
	GLbitvalue nbitID[CR_MAX_BITARRAY];

	for (j=0;j<CR_MAX_BITARRAY;j++)
		nbitID[j] = ~bitID[j];
	able[0] = diff_api.Disable;
	able[1] = diff_api.Enable;

	for (i = 0; i < g->limits.maxTextureUnits; i++)
	{
		if (CHECKDIRTY(t->enable[i], bitID)) 
		{
			diff_api.ActiveTextureARB( i+GL_TEXTURE0_ARB );
			if (from->unit[i].enabled1D != to->unit[i].enabled1D) 
			{
				able[to->unit[i].enabled1D](GL_TEXTURE_1D);
				FILLDIRTY(t->enable[i]);
				FILLDIRTY(t->dirty);
			}
			if (from->unit[i].enabled2D != to->unit[i].enabled2D) 
			{
				able[to->unit[i].enabled2D](GL_TEXTURE_2D);
				FILLDIRTY(t->enable[i]);
				FILLDIRTY(t->dirty);
			}
#ifdef CR_OPENGL_VERSION_1_2
			if (from->unit[i].enabled3D != to->unit[i].enabled3D)
			{
				able[to->unit[i].enabled3D](GL_TEXTURE_3D);
				FILLDIRTY(t->enable[i]);
				FILLDIRTY(t->dirty);
			}
#endif
#ifdef CR_ARB_texture_cube_map
			if (g->extensions.ARB_texture_cube_map &&
				from->unit[i].enabledCubeMap != to->unit[i].enabledCubeMap)
			{
				able[to->unit[i].enabledCubeMap](GL_TEXTURE_CUBE_MAP_ARB);
				FILLDIRTY(t->enable[i]);
				FILLDIRTY(t->dirty);
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
				FILLDIRTY(t->enable[i]);
				FILLDIRTY(t->dirty);
			}
			INVERTDIRTY(t->enable[i], nbitID);
		}

		/* 
		**  A thought on switching with textures:
		**  Since we are only performing a switch
		**  and not a sync, we won't need to 
		**  update individual textures, just
		**  the bindings....
		*/

		if (CHECKDIRTY(t->current[i], bitID)) 
		{
			diff_api.ActiveTextureARB( i + GL_TEXTURE0_ARB );
			if (from->unit[i].currentTexture1DName != to->unit[i].currentTexture1DName) 
			{
				diff_api.BindTexture(GL_TEXTURE_1D, to->unit[i].currentTexture1DName);
				FILLDIRTY(t->current[i]);
				FILLDIRTY(t->dirty);
			}
			if (from->unit[i].currentTexture2DName != to->unit[i].currentTexture2DName) 
			{
				diff_api.BindTexture(GL_TEXTURE_2D, to->unit[i].currentTexture2DName);
				FILLDIRTY(t->current[i]);
				FILLDIRTY(t->dirty);
			}
#ifdef CR_OPENGL_VERSION_1_2
			if (from->unit[i].currentTexture3DName != to->unit[i].currentTexture3DName) {
				diff_api.BindTexture(GL_TEXTURE_3D, to->unit[i].currentTexture3DName);
				FILLDIRTY(t->current[i]);
				FILLDIRTY(t->dirty);
			}
#endif
#ifdef CR_ARB_texture_cube_map
			if (g->extensions.ARB_texture_cube_map &&
				from->unit[i].currentTextureCubeMapName != to->unit[i].currentTextureCubeMapName) {
				diff_api.BindTexture(GL_TEXTURE_CUBE_MAP_ARB, to->unit[i].currentTextureCubeMapName);
				FILLDIRTY(t->current[i]);
				FILLDIRTY(t->dirty);
			}
#endif
		}

		if (CHECKDIRTY(t->objGen[i], bitID)) 
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
				FILLDIRTY(t->objGen[i]);
				FILLDIRTY(t->dirty);
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
				FILLDIRTY(t->objGen[i]);
				FILLDIRTY(t->dirty);
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
				FILLDIRTY(t->objGen[i]);
				FILLDIRTY(t->dirty);
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
				FILLDIRTY(t->objGen[i]);
				FILLDIRTY(t->dirty);
			}
			INVERTDIRTY(t->objGen[i], nbitID);
		}
		if (CHECKDIRTY(t->eyeGen[i], bitID)) 
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
				FILLDIRTY(t->eyeGen[i]);
				FILLDIRTY(t->dirty);
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
				FILLDIRTY(t->eyeGen[i]);
				FILLDIRTY(t->dirty);
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
				FILLDIRTY(t->eyeGen[i]);
				FILLDIRTY(t->dirty);
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
				FILLDIRTY(t->eyeGen[i]);
				FILLDIRTY(t->dirty);
			}
			diff_api.PopMatrix();
			INVERTDIRTY(t->eyeGen[i], nbitID);
		}
		if (CHECKDIRTY(t->gen[i], bitID)) 
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
				FILLDIRTY(t->gen[i]);
				FILLDIRTY(t->dirty);
			}
			INVERTDIRTY(t->gen[i], nbitID);
		}
		INVERTDIRTY(t->dirty, nbitID);
/* Texture enviroment */
		if (CHECKDIRTY(t->envBit[i], bitID)) 
		{
			diff_api.ActiveTextureARB( i + GL_TEXTURE0_ARB );
			if (from->unit[i].envMode != to->unit[i].envMode) 
			{
				diff_api.TexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, to->unit[i].envMode);
				FILLDIRTY(t->envBit[i]);
				FILLDIRTY(t->dirty);
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
				FILLDIRTY(t->envBit[i]);
				FILLDIRTY(t->dirty);
			}
			INVERTDIRTY(t->envBit[i], nbitID);
		}
	}
	diff_api.ActiveTextureARB( GL_TEXTURE0_ARB + to->curTextureUnit );
}

void crStateTextureDiff(CRContext *g, CRTextureBits *t, GLbitvalue *bitID, 
						 CRTextureState *from, CRTextureState *to) 
{
	CRTextureObj *tobj = NULL;
	GLuint name=0;
	GLuint *cname=NULL;
	unsigned int i;
	int j;
	glAble able[2];
	GLbitvalue nbitID[CR_MAX_BITARRAY];

	for (j=0;j<CR_MAX_BITARRAY;j++)
		nbitID[j] = ~bitID[j];
	able[0] = diff_api.Disable;
	able[1] = diff_api.Enable;

	for (i = 0; i < g->limits.maxTextureUnits; i++)
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

		if (CHECKDIRTY(tobj->dirty, bitID)) 
		{
			diff_api.ActiveTextureARB( i + GL_TEXTURE0_ARB );
			diff_api.BindTexture( tobj->target, name );
			if (CHECKDIRTY(tobj->paramsBit[i], bitID)) 
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
				diff_api.TexParameterf(tobj->target, GL_TEXTURE_PRIORITY, tobj->priority);
#endif
				diff_api.TexParameterfv(tobj->target, GL_TEXTURE_BORDER_COLOR, (const GLfloat *) f);
#ifdef CR_EXT_texture_filter_anisotropic
				if (g->extensions.EXT_texture_filter_anisotropic)
					diff_api.TexParameterf(tobj->target, GL_TEXTURE_MAX_ANISOTROPY_EXT, tobj->maxAnisotropy);
#endif
				INVERTDIRTY(tobj->paramsBit[i], nbitID);
			}

			if (CHECKDIRTY(tobj->imageBit[i], bitID)) 
			{
				int j;
				switch (tobj->target)
				{
					case GL_TEXTURE_1D:
						for (j=0; j<to->maxLevel; j++) 
						{
							CRTextureLevel *tl = &(tobj->level[j]);
							if (CHECKDIRTY(tl->dirty[i], bitID)) 
							{
								diff_api.TexImage1D(GL_TEXTURE_1D, j,
													tl->internalFormat,
													tl->width, tl->border,
													tl->format,
													tl->type, tl->img);
								INVERTDIRTY(tl->dirty[i], nbitID);
							}
						}
						break;
					case GL_TEXTURE_2D:
						for (j=0; j<to->maxLevel; j++) 
						{
							CRTextureLevel *tl = &(tobj->level[j]);
							if (CHECKDIRTY(tl->dirty[i], bitID)) 
							{
								diff_api.TexImage2D(GL_TEXTURE_2D, j,
													tl->internalFormat,
													tl->width, tl->height,
													tl->border, tl->format,
													tl->type, tl->img);
								INVERTDIRTY(tl->dirty[i], nbitID);
							}
						}
						break;
#ifdef CR_OPENGL_VERSION_1_2
					case GL_TEXTURE_3D:
						for (j=0; j<to->maxLevel; j++) 
						{
							CRTextureLevel *tl = &(tobj->level[j]);
							if (CHECKDIRTY(tl->dirty[i], bitID)) 
							{
								diff_api.TexImage3D(GL_TEXTURE_3D, j,
													tl->internalFormat,
													tl->width, tl->height,
													tl->depth,
													tl->border, tl->format,
													tl->type, tl->img);
								INVERTDIRTY(tl->dirty[i], nbitID);
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
							if (CHECKDIRTY(tl->dirty[i], bitID))
							{
								diff_api.TexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
													j, tl->internalFormat,
													tl->width, tl->height, tl->border,
													tl->format, tl->type, tl->img);
								INVERTDIRTY(tl->dirty[i], nbitID);
							}
							/* Negative X */
							tl = &(tobj->negativeXlevel[j]);
							if (CHECKDIRTY(tl->dirty[i], bitID))
							{
								diff_api.TexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB,
													j, tl->internalFormat,
													tl->width, tl->height, tl->border,
													tl->format, tl->type, tl->img);
								INVERTDIRTY(tl->dirty[i], nbitID);
							}
							/* Positive Y */
							tl = &(tobj->positiveYlevel[j]);
							if (CHECKDIRTY(tl->dirty[i], bitID)) 
							{
								diff_api.TexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB,
													j, tl->internalFormat,
													tl->width, tl->height, tl->border,
													tl->format, tl->type, tl->img);
								INVERTDIRTY(tl->dirty[i], nbitID);
							}
							/* Negative Y */
							tl = &(tobj->negativeYlevel[j]);
							if (CHECKDIRTY(tl->dirty[i], bitID))
							{
								diff_api.TexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB,
													j, tl->internalFormat,
													tl->width, tl->height, tl->border,
													tl->format, tl->type, tl->img);
								INVERTDIRTY(tl->dirty[i], nbitID);
							}
							/* Positive Z */
							tl = &(tobj->positiveZlevel[j]);
							if (CHECKDIRTY(tl->dirty[i], bitID))
							{
								diff_api.TexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB,
													j, tl->internalFormat,
													tl->width, tl->height, tl->border,
													tl->format, tl->type, tl->img);
								INVERTDIRTY(tl->dirty[i], nbitID);
							}
							/* Negative Z */
							tl = &(tobj->negativeZlevel[j]);
							if (CHECKDIRTY(tl->dirty[i], bitID))
							{
								diff_api.TexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB,
													j, tl->internalFormat,
													tl->width, tl->height, tl->border,
													tl->format, tl->type, tl->img);
								INVERTDIRTY(tl->dirty[i], nbitID);
							}
						}
						break;
#endif
					default:
						UNIMPLEMENTED();
				}	
			} /* if (CHECKDIRTY(tobj->imageBit[i], bitID)) */
			INVERTDIRTY(tobj->dirty, nbitID);
		}
	}

	for (i = 0; i < g->limits.maxTextureUnits; i++)
	{
		if (CHECKDIRTY(t->enable[i], bitID)) 
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
			INVERTDIRTY(t->enable[i], nbitID);
		}

		/* Get the active texture */
		if (to->unit[i].enabled1D == GL_TRUE) 
		{
			GET_TOBJ(tobj, to, to->unit[i].currentTexture1DName);
			if (!tobj)
				tobj = to->currentTexture1D;
			/*tobj = to->currentTexture1D; */
			name = to->unit[i].currentTexture1DName;
			cname = &(from->unit[i].currentTexture1DName);
		}

		if (to->unit[i].enabled2D == GL_TRUE) 
		{
			GET_TOBJ(tobj, to, to->unit[i].currentTexture2DName);
			if (!tobj)
				tobj = to->currentTexture2D;
			/*tobj = to->currentTexture2D; */
			name = to->unit[i].currentTexture2DName;
			cname = &(from->unit[i].currentTexture2DName);
		}

#ifdef CR_OPENGL_VERSION_1_2
		if (to->unit[i].enabled3D == GL_TRUE)
		{
			GET_TOBJ(tobj, to, to->unit[i].currentTexture3DName);
			if (!tobj)
				tobj = to->currentTexture3D;
			/*tobj = to->currenttexture3D[i]; */
			name = to->unit[i].currentTexture3DName;
			cname = &(from->unit[i].currentTexture3DName);
		}
#endif
#ifdef CR_ARB_texture_cube_map
		if (g->extensions.ARB_texture_cube_map &&
			to->unit[i].enabledCubeMap == GL_TRUE)
		{
			GET_TOBJ(tobj, to, to->unit[i].currentTextureCubeMapName);
			if (!tobj)
				tobj = to->currentTextureCubeMap;
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

		if (CHECKDIRTY(t->current[i], bitID)) 
		{
			if (*cname != name) 
			{
				diff_api.ActiveTextureARB(i+GL_TEXTURE0_ARB);
				diff_api.BindTexture(tobj->target, name);
				*cname = name;
			}
			INVERTDIRTY(t->current[i], nbitID);
		}

		/*
		** Texture Restore 
		** Since textures are allocated objects
		** it seems wastefull to allocate each one
		** on all the pipes.
		** So instead, we'll skip the value compare
		** and just use the bit test.
		*/


		if (CHECKDIRTY(t->objGen[i], bitID)) 
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
			INVERTDIRTY(t->objGen[i], nbitID);
		}
		if (CHECKDIRTY(t->eyeGen[i], bitID)) 
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
			INVERTDIRTY(t->eyeGen[i], nbitID);
		}
		if (CHECKDIRTY(t->gen[i], bitID)) 
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
			INVERTDIRTY(t->gen[i], nbitID);
		}
		INVERTDIRTY(t->dirty, nbitID);

		/* Texture enviroment  */
		if (CHECKDIRTY(t->envBit[i], bitID)) 
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
			INVERTDIRTY(t->envBit[i], nbitID);
		}
	}
	diff_api.ActiveTextureARB( GL_TEXTURE0_ARB + to->curTextureUnit );
}
