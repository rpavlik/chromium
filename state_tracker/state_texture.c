/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "state.h"
#include "state/cr_statetypes.h"
#include "state/cr_texture.h"
#include "cr_hash.h"
#include "cr_idpool.h"
#include "cr_string.h"
#include "cr_mem.h"
#include "cr_version.h"
#include "state_internals.h"

#define UNIMPLEMENTED() crStateError(__LINE__,__FILE__,GL_INVALID_OPERATION, "Unimplemented something or other" )
#define UNUSED(x) ((void) (x))

#define GET_TOBJ(tobj, state, id) 	tobj = (CRTextureObj *) crHashtableSearch(state->idHash, id);


static void
crStateTextureDelete_t(CRTextureState *t, CRTextureObj *tobj);


void crStateTextureDestroy(CRContext *ctx)
{
	CRTextureState *t = &ctx->texture;

	crStateTextureDelete_t(t, &(t->base1D));
	crStateTextureDelete_t(t, &(t->base2D));
#ifdef CR_OPENGL_VERSION_1_2
	crStateTextureDelete_t(t, &(t->base3D));
#endif
#ifdef CR_ARB_texture_cube_map
	crStateTextureDelete_t(t, &(t->baseCubeMap));
#endif
	crStateTextureDelete_t(t, &(t->proxy1D));
	crStateTextureDelete_t(t, &(t->proxy2D));
#ifdef CR_OPENGL_VERSION_1_2
	crStateTextureDelete_t(t, &(t->proxy3D));
#endif
#ifdef CR_ARB_texture_cube_map
	crStateTextureDelete_t(t, &(t->proxyCubeMap));
#endif
}

void crStateTextureInit(CRContext *ctx)
{
	CRLimitsState *limits = &ctx->limits;
	CRTextureState *t = &ctx->texture;
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);
	unsigned int i;
	unsigned int a;
	GLvectorf zero_vector = {0.0f, 0.0f, 0.0f, 0.0f};
	GLcolorf zero_color = {0.0f, 0.0f, 0.0f, 0.0f};
	GLvectorf x_vector = {1.0f, 0.0f, 0.0f, 0.0f};
	GLvectorf y_vector = {0.0f, 1.0f, 0.0f, 0.0f};

	/* compute max levels from max sizes */
	for (i=0, a=limits->maxTextureSize; a; i++, a=a>>1);
	t->maxLevel = i;
	for (i=0, a=limits->max3DTextureSize; a; i++, a=a>>1);
	t->max3DLevel = i;
#ifdef CR_ARB_texture_cube_map
	for (i=0, a=limits->maxCubeMapTextureSize; a; i++, a=a>>1);
	t->maxCubeMapLevel = i;
#endif

	/* Initalize id pool and hash table */
	t->idPool = crAllocIdPool();
	t->idHash = crAllocHashtable();

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
		t->unit[i].currentTexture1D = &(t->base1D);
		t->unit[i].currentTexture2D = &(t->base2D);
		t->unit[i].currentTexture3D = &(t->base3D);
#ifdef CR_ARB_texture_cube_map
		t->unit[i].currentTextureCubeMap = &(t->baseCubeMap);
#endif

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

		t->unit[i].combineModeRGB = GL_MODULATE;
		t->unit[i].combineModeA = GL_MODULATE;
		t->unit[i].combineSourceRGB[0] = GL_TEXTURE;
		t->unit[i].combineSourceRGB[1] = GL_PREVIOUS_EXT;
		t->unit[i].combineSourceRGB[2] = GL_CONSTANT_EXT;
		t->unit[i].combineSourceA[0] = GL_TEXTURE;
		t->unit[i].combineSourceA[1] = GL_PREVIOUS_EXT;
		t->unit[i].combineSourceA[2] = GL_CONSTANT_EXT;
		t->unit[i].combineOperandRGB[0] = GL_SRC_COLOR;
		t->unit[i].combineOperandRGB[1] = GL_SRC_COLOR;
		t->unit[i].combineOperandRGB[2] = GL_SRC_ALPHA;
		t->unit[i].combineOperandA[0] = GL_SRC_ALPHA;
		t->unit[i].combineOperandA[1] = GL_SRC_ALPHA;
		t->unit[i].combineOperandA[2] = GL_SRC_ALPHA;
		t->unit[i].combineScaleRGB = 1.0F;
		t->unit[i].combineScaleA = 1.0F;
#ifdef CR_EXT_texture_lod_bias
		t->unit[i].lodBias = 0.0F;
#endif
		RESET(tb->enable[i], ctx->bitid);
		RESET(tb->current[i], ctx->bitid);
		RESET(tb->objGen[i], ctx->bitid);
		RESET(tb->eyeGen[i], ctx->bitid);
		RESET(tb->gen[i], ctx->bitid);
		RESET(tb->envBit[i], ctx->bitid);
	}
	RESET(tb->dirty, ctx->bitid);
}


/*
 * Free all the texture state associated with t.
 */
void crStateTextureFree( CRTextureState *t ) 
{
	/* walk hash table, freeing texture objects */
	CR_HASHTABLE_WALK(t->idHash, entry)

		CRTextureObj *tobj = (CRTextureObj *) entry->data;
		crStateTextureDelete_t(t, tobj);
		entry->data = NULL;

	CR_HASHTABLE_WALK_END(t->idHash);

	crFreeIdPool(t->idPool);
	crFreeHashtable(t->idHash);
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
												 * t->maxLevel);				\
	if (!tobj->ARRAY)															\
		return; /* out of memory */									\
	for (i=0; i<t->maxLevel; i++)									\
	{																							\
		tl                = &(tobj->ARRAY[i]);			\
		tl->compressed    = GL_FALSE;								\
		tl->bytes         = 0;											\
		tl->img           = NULL;										\
		tl->width         = 0;											\
		tl->height        = 0;											\
		tl->depth         = 0;											\
		tl->border        = 0;											\
		tl->internalFormat= GL_RGBA;								\
		tl->bytesPerPixel = 0;											\
		tl->format        = GL_RGBA;								\
		tl->type          = GL_UNSIGNED_BYTE;				\
		crStateTextureInitTextureFormat( tl, tl->internalFormat );						\
		for (j = 0; j < CR_MAX_TEXTURE_UNITS; j++)														\
		{																																			\
			for (k = 0; k < CR_MAX_BITARRAY; k++)																\
				tl->dirty[j][k]     = 0;  /* By default this level is ignored.*/	\
		}																																			\
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

#ifdef CR_ARB_depth_texture
	tobj->depthMode = GL_LUMINANCE;
#endif

#ifdef CR_ARB_shadow
	tobj->compareMode = GL_NONE;
	tobj->compareFunc = GL_LEQUAL;
#endif

#ifdef CR_ARB_shadow_ambient
	tobj->compareFailValue = 0.0;
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
 * Given an internal texture format enum or 1, 2, 3, 4 initialize the
 * texture levels texture format.  This basically just indicates the
 * number of red, green, blue, alpha, luminance, etc. bits are used to
 * store the image.
 */
void crStateTextureInitTextureFormat( CRTextureLevel *tl, GLenum internalFormat )
{
   switch ( internalFormat ) {
   case 4:
   case GL_RGBA:
	 case GL_COMPRESSED_RGBA_ARB:
      tl->texFormat = &_texformat_rgba8888;
      break;

   case 3:
   case GL_RGB:
	 case GL_COMPRESSED_RGB_ARB:
      tl->texFormat = &_texformat_rgb888;
      break;

   case GL_RGBA2:
   case GL_RGBA4:
   case GL_RGB5_A1:
   case GL_RGBA8:
   case GL_RGB10_A2:
   case GL_RGBA12:
   case GL_RGBA16:
      tl->texFormat = &_texformat_rgba8888;
      break;

   case GL_R3_G3_B2:
      tl->texFormat = &_texformat_rgb332;
      break;
   case GL_RGB4:
   case GL_RGB5:
   case GL_RGB8:
   case GL_RGB10:
   case GL_RGB12:
   case GL_RGB16:
      tl->texFormat = &_texformat_rgb888;
      break;

   case GL_ALPHA:
   case GL_ALPHA4:
   case GL_ALPHA8:
   case GL_ALPHA12:
   case GL_ALPHA16:
	 case GL_COMPRESSED_ALPHA_ARB:
      tl->texFormat = &_texformat_a8;
      break;

   case 1:
   case GL_LUMINANCE:
   case GL_LUMINANCE4:
   case GL_LUMINANCE8:
   case GL_LUMINANCE12:
   case GL_LUMINANCE16:
	 case GL_COMPRESSED_LUMINANCE_ARB:
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
	 case GL_COMPRESSED_LUMINANCE_ALPHA_ARB:
      tl->texFormat = &_texformat_al88;
      break;

   case GL_INTENSITY:
   case GL_INTENSITY4:
   case GL_INTENSITY8:
   case GL_INTENSITY12:
   case GL_INTENSITY16:
	 case GL_COMPRESSED_INTENSITY_ARB:
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
#endif

CRTextureObj * crStateTextureGet(GLenum target, GLuint name) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRTextureObj *tobj;

	if (name == 0)
	{
		switch (target) {
		case GL_TEXTURE_1D:
			return &t->base1D;
		case GL_TEXTURE_2D:
			return &t->base2D;
		case GL_TEXTURE_3D:
			return &t->base3D;
#ifdef CR_ARB_texture_cube_map
		case GL_TEXTURE_CUBE_MAP_ARB:
			return &t->baseCubeMap;
#endif
		default:
			return NULL;
		}
	}

	GET_TOBJ(tobj, t, name);

	return tobj;
}


/*
 * Allocate a new texture object with the given name
 */
static CRTextureObj *
crStateTextureAllocate_t (CRTextureState *t, GLuint name) 
{
	CRTextureObj *tobj;

	if (!name) 
		return NULL;

	tobj = crCalloc(sizeof(CRTextureObj));
	if (!tobj)
		return NULL;

	/* reserve the ID and insert into hash table */
	crIdPoolAllocId( t->idPool, name );
	crHashtableAdd( t->idHash, name, (void *) tobj );

	crStateTextureInitTextureObj(t, tobj, name, GL_NONE);

	return tobj;
}


static void
crStateTextureDelete_t(CRTextureState *t, CRTextureObj *tobj) 
{
	int k;

	CRASSERT(t);
	CRASSERT(tobj);

	/* remove from hash table */
	crHashtableDelete( t->idHash, tobj->name, GL_FALSE );
	crIdPoolFreeBlock( t->idPool, tobj->name, 1 );

	/* Free the images */
	for (k = 0; k < t->maxLevel; k++) 
	{
		CRTextureLevel *tl = tobj->level + k;
		if (tl->img) 
		{
			crFree(tl->img);
			tl->img = NULL;
			tl->bytes = 0;
		}
	}
#ifdef CR_ARB_texture_cube_map
	crFree(tobj->negativeXlevel);
	crFree(tobj->positiveYlevel);
	crFree(tobj->negativeYlevel);
	crFree(tobj->positiveZlevel);
	crFree(tobj->negativeZlevel);
#endif
	crFree(tobj->level);
}


void STATE_APIENTRY crStateGenTextures(GLsizei n, GLuint *textures) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	GLint start;

	FLUSH();

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glGenTextures called in Begin/End");
		return;
	}

	if (n < 0)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
								 "Negative n passed to glGenTextures: %d", n);
		return;
	}

	start = crIdPoolAllocBlock(t->idPool, n);
	if (start)
	{
		GLint i;
		for (i = 0; i < n; i++)
			textures[i] = (GLuint) (start + i);
	}
	else
	{
		crStateError(__LINE__, __FILE__, GL_OUT_OF_MEMORY, "glGenTextures");
	}
}

void STATE_APIENTRY crStateDeleteTextures(GLsizei n, const GLuint *textures) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);
	int i;

	FLUSH();

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glDeleteTextures called in Begin/End");
		return;
	}

	if (n < 0)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
								 "Negative n passed to glDeleteTextures: %d", n);
		return;
	}

	for (i=0; i<n; i++) 
	{
		GLuint name = textures[i];
		CRTextureObj *tObj;
		GET_TOBJ(tObj, t, name);
		if (name && tObj)
		{
			GLuint u;
			crStateTextureDelete_t(t, tObj);
			/* if the currentTexture is deleted, 
			 ** reset back to the base texture.
			 */
			for (u = 0; u < g->limits.maxTextureUnits; u++)
			{
				if (tObj == t->unit[u].currentTexture1D) 
				{
					t->unit[u].currentTexture1D = &(t->base1D);
				}
				if (tObj == t->unit[u].currentTexture2D) 
				{
					t->unit[u].currentTexture2D = &(t->base2D);
				}
#ifdef CR_OPENGL_VERSION_1_2
				if (tObj == t->unit[u].currentTexture3D) 
				{
					t->unit[u].currentTexture3D = &(t->base3D);
				}
#endif
#ifdef CR_ARB_texture_cube_map
				if (tObj == t->unit[u].currentTextureCubeMap)
				{
					t->unit[u].currentTextureCubeMap = &(t->baseCubeMap);
				}
#endif
			}
		}
	}

	DIRTY(tb->dirty, g->neg_bitid);
	DIRTY(tb->current[t->curTextureUnit], g->neg_bitid);
}



void STATE_APIENTRY crStateClientActiveTextureARB( GLenum texture )
{
	CRContext *g = GetCurrentContext();
	CRClientState *c = &(g->client);

	FLUSH();

	if (!g->extensions.ARB_multitexture) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glClientActiveTextureARB not available");
		return;
	}

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glClientActiveTextureARB called in Begin/End");
		return;
	}

	if ( texture < GL_TEXTURE0_ARB ||
			 texture >= GL_TEXTURE0_ARB + g->limits.maxTextureUnits)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "crStateClientActiveTexture: unit = %d (max is %d)",
								 texture, g->limits.maxTextureUnits );
		return;
	}

	c->curClientTextureUnit = texture - GL_TEXTURE0_ARB;
}

void STATE_APIENTRY crStateActiveTextureARB( GLenum texture )
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);

	FLUSH();

	if (!g->extensions.ARB_multitexture) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glActiveTextureARB not available");
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

	t->curTextureUnit = texture - GL_TEXTURE0_ARB;
}

void STATE_APIENTRY crStateBindTexture(GLenum target, GLuint texture) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRTextureObj *tobj;
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);

	FLUSH();

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glBindTexture called in Begin/End");
		return;
	}

	/* Special Case name = 0 */
	if (!texture) 
	{
		switch (target) 
		{
			case GL_TEXTURE_1D:
				t->unit[t->curTextureUnit].currentTexture1D = &(t->base1D);
				break;
			case GL_TEXTURE_2D:
				t->unit[t->curTextureUnit].currentTexture2D = &(t->base2D);
				break;
#ifdef CR_OPENGL_VERSION_1_2
			case GL_TEXTURE_3D:
				t->unit[t->curTextureUnit].currentTexture3D = &(t->base3D);
				break;
#endif
#ifdef CR_ARB_texture_cube_map
			case GL_TEXTURE_CUBE_MAP_ARB:
				if (!g->extensions.ARB_texture_cube_map) {
					crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"Invalid target passed to glBindTexture: %d", target);
					return;
				}
				t->unit[t->curTextureUnit].currentTextureCubeMap = &(t->baseCubeMap);
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
	else if (tobj->target != target)
	{
		crWarning( "You called glBindTexture with a target of 0x%x, but the texture you wanted was target 0x%x", target, tobj->target );
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "Attempt to bind a texture of diffent dimenions");
		return;
	}

	/* Set the current texture */
	switch (target) 
	{
		case GL_TEXTURE_1D:
			t->unit[t->curTextureUnit].currentTexture1D = tobj;
			break;
		case GL_TEXTURE_2D:
			t->unit[t->curTextureUnit].currentTexture2D = tobj;
			break;
#ifdef CR_OPENGL_VERSION_1_2
		case GL_TEXTURE_3D:
			t->unit[t->curTextureUnit].currentTexture3D = tobj;
			break;
#endif
#ifdef CR_ARB_texture_cube_map
		case GL_TEXTURE_CUBE_MAP_ARB:
			t->unit[t->curTextureUnit].currentTextureCubeMap = tobj;
			break;
#endif
		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
									 "Invalid target passed to glBindTexture: %d", target);
			return;
	}

	DIRTY(tb->dirty, g->neg_bitid);
	DIRTY(tb->current[t->curTextureUnit], g->neg_bitid);
}



void STATE_APIENTRY crStateTexParameterfv(GLenum target, GLenum pname, const GLfloat *param) 
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRTextureUnit *unit = t->unit + t->curTextureUnit;
	CRTextureObj *tobj = NULL;
	GLenum e = (GLenum) *param;
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);
	unsigned int i;

	FLUSH();

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					"TexParameterfv called in Begin/End");
		return;
	}

	switch (target) 
	{
		case GL_TEXTURE_1D:
			tobj = unit->currentTexture1D;
			break;
		case GL_TEXTURE_2D:
			tobj = unit->currentTexture2D;
			break;
#ifdef CR_OPENGL_VERSION_1_2
		case GL_TEXTURE_3D:
			tobj = unit->currentTexture3D;
			break;
#endif
#ifdef CR_ARB_texture_cube_map
		case GL_TEXTURE_CUBE_MAP_ARB:
			if (!g->extensions.ARB_texture_cube_map) {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					"TexParamterfv: target is invalid: %d", target);
				return;
			}
			tobj = unit->currentTextureCubeMap;
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
#ifdef CR_ARB_texture_border_clamp
			else if (e == GL_CLAMP_TO_BORDER_SGIS && g->extensions.ARB_texture_border_clamp) {
				tobj->wrapS = e;
			}
#endif
#ifdef CR_ARB_texture_mirrored_repeat
			else if (e == GL_MIRRORED_REPEAT_ARB && g->extensions.ARB_texture_mirrored_repeat) {
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
#ifdef CR_ARB_texture_border_clamp
			else if (e == GL_CLAMP_TO_BORDER_SGIS && g->extensions.ARB_texture_border_clamp) {
				tobj->wrapT = e;
			}
#endif
#ifdef CR_ARB_texture_mirrored_repeat
			else if (e == GL_MIRRORED_REPEAT_ARB && g->extensions.ARB_texture_mirrored_repeat) {
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
#ifdef CR_ARB_texture_border_clamp
			else if (e == GL_CLAMP_TO_BORDER_SGIS && g->extensions.ARB_texture_border_clamp) {
				tobj->wrapR = e;
			}
#endif
#ifdef CR_ARB_texture_mirrored_repeat
			else if (e == GL_MIRRORED_REPEAT_ARB && g->extensions.ARB_texture_mirrored_repeat) {
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
#ifdef CR_ARB_depth_texture
		case GL_DEPTH_TEXTURE_MODE_ARB:
			if (g->extensions.ARB_depth_texture) {
				if (param[0] == GL_LUMINANCE || 
				    param[0] == GL_INTENSITY ||
				    param[0] == GL_ALPHA) {
					tobj->depthMode = (GLenum) param[0];
				}
				else
				{
					crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
						"TexParameterfv: GL_DEPTH_TEXTURE_MODE_ARB called with invalid parameter: 0x%x", param[0]);
					return;
				}
			}
			break;
#endif
#ifdef CR_ARB_shadow
		case GL_TEXTURE_COMPARE_MODE_ARB:
			if (g->extensions.ARB_shadow) {
				if (param[0] == GL_NONE ||
				    param[0] == GL_COMPARE_R_TO_TEXTURE_ARB) {
					tobj->compareMode = (GLenum) param[0];
				}
				else
				{
					crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
						"TexParameterfv: GL_TEXTURE_COMPARE_MODE_ARB called with invalid parameter: 0x%x", param[0]);
					return;
				}
			}
			break;
		case GL_TEXTURE_COMPARE_FUNC_ARB:
			if (g->extensions.ARB_shadow) {
				if (param[0] == GL_LEQUAL ||
				    param[0] == GL_GEQUAL) {
					tobj->compareFunc = (GLenum) param[0];
				}
				else
				{
					crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
						"TexParameterfv: GL_TEXTURE_COMPARE_FUNC_ARB called with invalid parameter: 0x%x", param[0]);
					return;
				}
			}
			break;
#endif
#ifdef CR_ARB_shadow_ambient
		case GL_TEXTURE_COMPARE_FAIL_VALUE_ARB:
			if (g->extensions.ARB_shadow_ambient) {
				tobj->compareFailValue = param[0];
			}
			break;
#endif
#ifdef CR_SGIS_generate_mipmap
		case GL_GENERATE_MIPMAP_SGIS:
			if (g->extensions.SGIS_generate_mipmap) {
				tobj->generateMipmap = param[0] ? GL_TRUE : GL_FALSE;
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
#ifdef CR_ARB_depth_texture
		case GL_DEPTH_TEXTURE_MODE_ARB:
#endif
#ifdef CR_ARB_shadow
		case GL_TEXTURE_COMPARE_MODE_ARB:
		case GL_TEXTURE_COMPARE_FUNC_ARB:
#endif
#ifdef CR_ARB_shadow_ambinet
		case GL_TEXTURE_COMPARE_FAIL_VALUE_ARB:
#endif
#ifdef CR_SGIS_generate_mipmap
		case GL_GENERATE_MIPMAP_SGIS:
#endif
			f_param = (GLfloat) (*param);
			crStateTexParameterfv( target, pname, &(f_param) );
			break;
		case GL_TEXTURE_BORDER_COLOR:
			f_color.r = ((GLfloat) param[0])/CR_MAXINT;
			f_color.g = ((GLfloat) param[1])/CR_MAXINT;
			f_color.b = ((GLfloat) param[2])/CR_MAXINT;
			f_color.a = ((GLfloat) param[3])/CR_MAXINT;
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
	GLuint stage;

	(void) stage;

	FLUSH();

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					"glTexEnvfv called in begin/end");
		return;
	}

#if CR_EXT_texture_lod_bias
	if (target == GL_TEXTURE_FILTER_CONTROL_EXT) {
		if (!g->extensions.EXT_texture_lod_bias || pname != GL_TEXTURE_LOD_BIAS_EXT) {
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glTexEnv");
		}
		else {
			t->unit[t->curTextureUnit].lodBias = *param;
		}
		DIRTY(tb->envBit[t->curTextureUnit], g->neg_bitid);
		DIRTY(tb->dirty, g->neg_bitid);
		return;
	}
	else
#endif
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
					e != GL_REPLACE &&
					e != GL_COMBINE_ARB)
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

#ifdef CR_ARB_texture_env_combine
		case GL_COMBINE_RGB_ARB:
			e = (GLenum) (GLint) *param;
			if (g->extensions.ARB_texture_env_combine &&
					(e == GL_REPLACE ||
					 e == GL_MODULATE ||
					 e == GL_ADD ||
					 e == GL_ADD_SIGNED_ARB ||
					 e == GL_INTERPOLATE_ARB ||
					 e == GL_SUBTRACT_ARB)) {
				 t->unit[t->curTextureUnit].combineModeRGB = e;
			}
#ifdef CR_ARB_texture_env_dot3
			else if (g->extensions.ARB_texture_env_dot3 &&
							 (e == GL_DOT3_RGB_ARB ||
								e == GL_DOT3_RGBA_ARB ||
								e == GL_DOT3_RGB_EXT ||
								e == GL_DOT3_RGBA_EXT)) {
				 t->unit[t->curTextureUnit].combineModeRGB = e;
			}
#endif
			else {
				 crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glTexEnvfv(param=0x%x", e);
				 return;
			}
			break;
		case GL_COMBINE_ALPHA_EXT:
			e = (GLenum) *param;
			if (g->extensions.ARB_texture_env_combine &&
					(e == GL_REPLACE ||
					 e == GL_MODULATE ||
					 e == GL_ADD ||
					 e == GL_ADD_SIGNED_ARB ||
					 e == GL_INTERPOLATE_ARB ||
					 e == GL_SUBTRACT_ARB)) {
				 t->unit[t->curTextureUnit].combineModeA = e;
			}
			else {
				 crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glTexEnvfv");
				 return;
			}
			break;
		case GL_SOURCE0_RGB_ARB:
		case GL_SOURCE1_RGB_ARB:
		case GL_SOURCE2_RGB_ARB:
			e = (GLenum) *param;
	    stage = pname - GL_SOURCE0_RGB_ARB;
			if (g->extensions.ARB_texture_env_combine &&
					(e == GL_TEXTURE ||
					 e == GL_CONSTANT_ARB ||
					 e == GL_PRIMARY_COLOR_ARB ||
					 e == GL_PREVIOUS_ARB)) {
				t->unit[t->curTextureUnit].combineSourceRGB[stage] = e;
			}
			else if (g->extensions.ARB_texture_env_crossbar &&
							 e >= GL_TEXTURE0_ARB &&
							 e < GL_TEXTURE0_ARB + g->limits.maxTextureUnits) {
				t->unit[t->curTextureUnit].combineSourceRGB[stage] = e;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glTexEnvfv");
				return;
			}
			break;
		case GL_SOURCE0_ALPHA_ARB:
		case GL_SOURCE1_ALPHA_ARB:
		case GL_SOURCE2_ALPHA_ARB:
	    e = (GLenum) *param;
	    stage = pname - GL_SOURCE0_ALPHA_ARB;
			if (g->extensions.ARB_texture_env_combine &&
					(e == GL_TEXTURE ||
					 e == GL_CONSTANT_ARB ||
					 e == GL_PRIMARY_COLOR_ARB ||
					 e == GL_PREVIOUS_ARB)) {
				t->unit[t->curTextureUnit].combineSourceA[stage] = e;
			}
			else if (g->extensions.ARB_texture_env_crossbar &&
							 e >= GL_TEXTURE0_ARB &&
							 e < GL_TEXTURE0_ARB + g->limits.maxTextureUnits) {
				t->unit[t->curTextureUnit].combineSourceA[stage] = e;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glTexEnvfv");
				return;
			}
			break;
		case GL_OPERAND0_RGB_ARB:
		case GL_OPERAND1_RGB_ARB:
		case GL_OPERAND2_RGB_ARB:
			e = (GLenum) *param;
			stage = pname - GL_OPERAND0_RGB_ARB;
			if (g->extensions.ARB_texture_env_combine &&
					(e == GL_SRC_COLOR ||
					 e == GL_ONE_MINUS_SRC_COLOR ||
					 e == GL_SRC_ALPHA ||
					 e == GL_ONE_MINUS_SRC_ALPHA)) {
				t->unit[t->curTextureUnit].combineOperandRGB[stage] = e;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glTexEnvfv");
				return;
			}
			break;
		case GL_OPERAND0_ALPHA_ARB:
		case GL_OPERAND1_ALPHA_ARB:
		case GL_OPERAND2_ALPHA_ARB:
			e = (GLenum) *param;
			stage = pname - GL_OPERAND0_ALPHA_ARB;
			if (g->extensions.ARB_texture_env_combine &&
					(e == GL_SRC_ALPHA ||
					 e == GL_ONE_MINUS_SRC_ALPHA)) {
				t->unit[t->curTextureUnit].combineOperandA[stage] = e;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glTexEnvfv(param=0x%x)", e);
				return;
			}
			break;
		case GL_RGB_SCALE_ARB:
			if (g->extensions.ARB_texture_env_combine &&
					(*param == 1.0 ||
					 *param == 2.0 ||
					 *param == 4.0)) {
				t->unit[t->curTextureUnit].combineScaleRGB = *param;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexEnvfv");
				return;
			}
			break;
		case GL_ALPHA_SCALE:
			if (g->extensions.ARB_texture_env_combine &&
					(*param == 1.0 ||
					 *param == 2.0 ||
					 *param == 4.0)) {
				t->unit[t->curTextureUnit].combineScaleA = *param;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glTexEnvfv");
				return;
			}
		break;
#endif /* CR_ARB_texture_env_combine */

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
			f_color.r = ((GLfloat) param[0]) / CR_MAXINT;
			f_color.g = ((GLfloat) param[1]) / CR_MAXINT;
			f_color.b = ((GLfloat) param[2]) / CR_MAXINT;
			f_color.a = ((GLfloat) param[3]) / CR_MAXINT;
			crStateTexEnvfv( target, pname, (const GLfloat *) &f_color );
			break;
#ifdef CR_ARB_texture_env_combine
		case GL_COMBINE_RGB_ARB:
		case GL_COMBINE_ALPHA_EXT:
		case GL_SOURCE0_RGB_ARB:
		case GL_SOURCE1_RGB_ARB:
		case GL_SOURCE2_RGB_ARB:
		case GL_SOURCE0_ALPHA_ARB:
		case GL_SOURCE1_ALPHA_ARB:
		case GL_SOURCE2_ALPHA_ARB:
		case GL_OPERAND0_RGB_ARB:
		case GL_OPERAND1_RGB_ARB:
		case GL_OPERAND2_RGB_ARB:
		case GL_OPERAND0_ALPHA_ARB:
		case GL_OPERAND1_ALPHA_ARB:
		case GL_OPERAND2_ALPHA_ARB:
		case GL_RGB_SCALE_ARB:
		case GL_ALPHA_SCALE:
			f_param = (GLfloat) (*param);
			crStateTexEnvfv( target, pname, &f_param );
			break;
#endif
#ifdef CR_EXT_texture_lod_bias
		case GL_TEXTURE_LOD_BIAS_EXT:
			f_param = (GLfloat) (*param);
			crStateTexEnvfv( target, pname, &f_param);
			break;
#endif
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

#if CR_EXT_texture_lod_bias
	if (target == GL_TEXTURE_FILTER_CONTROL_EXT) {
		if (!g->extensions.EXT_texture_lod_bias || pname != GL_TEXTURE_LOD_BIAS_EXT) {
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnv");
		}
		else {
			*param = t->unit[t->curTextureUnit].lodBias;
		}
		return;
	}
	else
#endif
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
		case GL_COMBINE_RGB_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLfloat) t->unit[t->curTextureUnit].combineModeRGB;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnvfv(pname)");
				return;
			}
			break;
		case GL_COMBINE_ALPHA_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLfloat) t->unit[t->curTextureUnit].combineModeA;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnvfv(pname)");
				return;
			}
			break;
		case GL_SOURCE0_RGB_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLfloat) t->unit[t->curTextureUnit].combineSourceRGB[0];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnvfv(pname)");
				return;
			}
			break;
		case GL_SOURCE1_RGB_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLfloat) t->unit[t->curTextureUnit].combineSourceRGB[1];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnvfv(pname)");
				return;
			}
			break;
		case GL_SOURCE2_RGB_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLfloat) t->unit[t->curTextureUnit].combineSourceRGB[2];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnvfv(pname)");
				return;
			}
			break;
		case GL_SOURCE0_ALPHA_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLfloat) t->unit[t->curTextureUnit].combineSourceA[0];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnvfv(pname)");
				return;
			}
			break;
		case GL_SOURCE1_ALPHA_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLfloat) t->unit[t->curTextureUnit].combineSourceA[1];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnvfv(pname)");
				return;
			}
			break;
		case GL_SOURCE2_ALPHA_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLfloat) t->unit[t->curTextureUnit].combineSourceA[2];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnvfv(pname)");
				return;
			}
			break;
		case GL_OPERAND0_RGB_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLfloat) t->unit[t->curTextureUnit].combineOperandRGB[0];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnvfv(pname)");
				return;
			}
			break;
		case GL_OPERAND1_RGB_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLfloat) t->unit[t->curTextureUnit].combineOperandRGB[1];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnvfv(pname)");
				return;
			}
			break;
		case GL_OPERAND2_RGB_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLfloat) t->unit[t->curTextureUnit].combineOperandRGB[2];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnvfv(pname)");
				return;
			}
			break;
		case GL_OPERAND0_ALPHA_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLfloat) t->unit[t->curTextureUnit].combineOperandA[0];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnvfv(pname)");
				return;
			}
			break;
		case GL_OPERAND1_ALPHA_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLfloat) t->unit[t->curTextureUnit].combineOperandA[1];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnvfv(pname)");
				return;
			}
			break;
		case GL_OPERAND2_ALPHA_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLfloat) t->unit[t->curTextureUnit].combineOperandA[2];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnvfv(pname)");
				return;
			}
			break;
		case GL_RGB_SCALE_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = t->unit[t->curTextureUnit].combineScaleRGB;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnvfv(pname)");
				return;
			}
			break;
		case GL_ALPHA_SCALE:
			if (g->extensions.ARB_texture_env_combine) {
				*param = t->unit[t->curTextureUnit].combineScaleA;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnvfv(pname)");
				return;
			}
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

#if CR_EXT_texture_lod_bias
	if (target == GL_TEXTURE_FILTER_CONTROL_EXT) {
		if (!g->extensions.EXT_texture_lod_bias || pname != GL_TEXTURE_LOD_BIAS_EXT) {
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnv");
		}
		else {
			*param = (GLint) t->unit[t->curTextureUnit].lodBias;
		}
		return;
	}
	else
#endif
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
			param[0] = (GLint) (t->unit[t->curTextureUnit].envColor.r * CR_MAXINT);
			param[1] = (GLint) (t->unit[t->curTextureUnit].envColor.g * CR_MAXINT);
			param[2] = (GLint) (t->unit[t->curTextureUnit].envColor.b * CR_MAXINT);
			param[3] = (GLint) (t->unit[t->curTextureUnit].envColor.a * CR_MAXINT);
			break;
		case GL_COMBINE_RGB_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLint) t->unit[t->curTextureUnit].combineModeRGB;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnviv(pname)");
				return;
			}
			break;
		case GL_COMBINE_ALPHA_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLint) t->unit[t->curTextureUnit].combineModeA;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnviv(pname)");
				return;
			}
			break;
		case GL_SOURCE0_RGB_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLint) t->unit[t->curTextureUnit].combineSourceRGB[0];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnviv(pname)");
				return;
			}
			break;
		case GL_SOURCE1_RGB_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLint) t->unit[t->curTextureUnit].combineSourceRGB[1];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnviv(pname)");
				return;
			}
			break;
		case GL_SOURCE2_RGB_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLint) t->unit[t->curTextureUnit].combineSourceRGB[2];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnviv(pname)");
				return;
			}
			break;
		case GL_SOURCE0_ALPHA_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLint) t->unit[t->curTextureUnit].combineSourceA[0];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnviv(pname)");
				return;
			}
			break;
		case GL_SOURCE1_ALPHA_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLint) t->unit[t->curTextureUnit].combineSourceA[1];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnviv(pname)");
				return;
			}
			break;
		case GL_SOURCE2_ALPHA_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLint) t->unit[t->curTextureUnit].combineSourceA[2];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnviv(pname)");
				return;
			}
			break;
		case GL_OPERAND0_RGB_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLint) t->unit[t->curTextureUnit].combineOperandRGB[0];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnviv(pname)");
				return;
			}
			break;
		case GL_OPERAND1_RGB_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLint) t->unit[t->curTextureUnit].combineOperandRGB[1];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnviv(pname)");
				return;
			}
			break;
		case GL_OPERAND2_RGB_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLint) t->unit[t->curTextureUnit].combineOperandRGB[2];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnviv(pname)");
				return;
			}
			break;
		case GL_OPERAND0_ALPHA_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLint) t->unit[t->curTextureUnit].combineOperandA[0];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnviv(pname)");
				return;
			}
			break;
		case GL_OPERAND1_ALPHA_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLint) t->unit[t->curTextureUnit].combineOperandA[1];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnviv(pname)");
				return;
			}
			break;
		case GL_OPERAND2_ALPHA_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLint) t->unit[t->curTextureUnit].combineOperandA[2];
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnviv(pname)");
				return;
			}
			break;
		case GL_RGB_SCALE_ARB:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLint) t->unit[t->curTextureUnit].combineScaleRGB;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnviv(pname)");
				return;
			}
			break;
		case GL_ALPHA_SCALE:
			if (g->extensions.ARB_texture_env_combine) {
				*param = (GLint) t->unit[t->curTextureUnit].combineScaleA;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "glGetTexEnviv(pname)");
				return;
			}
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
	CRmatrix inv;
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);

	FLUSH();

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					"glTexGen called in begin/end");
		return;
	}

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


/*
 * Return the texture image corresponding the given texture unit, target
 * and mipmap level.  Return NULL if any error.
 * This is a useful helper function.
 */
static CRTextureLevel * crStateGetTexLevel (CRContext *g, GLuint texUnit,
	GLenum target, GLint level)
{
	CRTextureState *t = &(g->texture);  /* FIXME: active texture unit! */
	CRTextureUnit *unit = t->unit + t->curTextureUnit;
	CRTextureObj *tobj;
	CRTextureLevel *timg;

	CRASSERT(level >= 0);

	(void) texUnit;

	switch (target)
	{
		case GL_TEXTURE_1D:
			tobj = unit->currentTexture1D;
			timg = tobj->level + level;
			break;

		case GL_PROXY_TEXTURE_1D:
			tobj = &(t->proxy1D);
			timg = tobj->level + level;
			break;

		case GL_TEXTURE_2D:
			tobj = unit->currentTexture2D;
			timg = tobj->level + level;
			break;

		case GL_PROXY_TEXTURE_2D:
			tobj = &(t->proxy2D);
			timg = tobj->level + level;
			break;
#ifdef CR_OPENGL_VERSION_1_2
		case GL_TEXTURE_3D:
			tobj = unit->currentTexture3D;
			timg = tobj->level + level;
			break;

		case GL_PROXY_TEXTURE_3D:
			tobj = &(t->proxy3D);
			timg = tobj->level + level;
			break;
#endif
#ifdef CR_ARB_texture_cube_map
		case GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB:
			tobj = unit->currentTextureCubeMap;
			timg = tobj->level + level;
			break;

		case GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB:
			tobj = unit->currentTextureCubeMap;
			timg = tobj->negativeXlevel + level;
			break;

		case GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB:
			tobj = unit->currentTextureCubeMap;
			timg = tobj->positiveYlevel + level;
			break;

		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB:
			tobj = unit->currentTextureCubeMap;
			timg = tobj->negativeYlevel + level;
			break;

		case GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB:
			tobj = unit->currentTextureCubeMap;
			timg = tobj->positiveZlevel + level;
			break;

		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB:
			tobj = unit->currentTextureCubeMap;
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
	CRTextureUnit *unit = t->unit + t->curTextureUnit;
	CRTextureObj *tobj;

	(void) texUnit;

	switch (target)
	{
		case GL_TEXTURE_1D:
			tobj = unit->currentTexture1D;
			break;

		case GL_PROXY_TEXTURE_1D:
			tobj = &(t->proxy1D);
			break;

		case GL_TEXTURE_2D:
			tobj = unit->currentTexture2D;
			break;

		case GL_PROXY_TEXTURE_2D:
			tobj = &(t->proxy2D);
			break;
#ifdef CR_OPENGL_VERSION_1_2
		case GL_TEXTURE_3D:
			tobj = unit->currentTexture3D;
			break;

		case GL_PROXY_TEXTURE_3D:
			tobj = &(t->proxy3D);
			break;
#endif
#ifdef CR_ARB_texture_cube_map
		case GL_TEXTURE_CUBE_MAP_ARB:
			tobj = unit->currentTextureCubeMap;
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
#if CR_ARB_texture_compression
		case GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB:
			 *params = (GLfloat) timg->bytes;
			 break;
		case GL_TEXTURE_COMPRESSED_ARB:
			 *params = (GLfloat) timg->compressed;
			 break;
#endif
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
#if CR_ARB_texture_compression
		case GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB:
			*params = (GLint) timg->bytes;
			break;
		case GL_TEXTURE_COMPRESSED_ARB:
			*params = (GLint) timg->compressed;
			break;
#endif
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
#ifdef CR_ARB_depth_texture
		case GL_DEPTH_TEXTURE_MODE_ARB:
			if (g->extensions.ARB_depth_texture) {
				*params = (GLfloat) tobj->depthMode;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					"glGetTexParameter: invalid pname: 0x%x", pname);
				return;
			}
			break;
#endif
#ifdef CR_ARB_shadow
		case GL_TEXTURE_COMPARE_MODE_ARB:
			if (g->extensions.ARB_shadow) {
				*params = (GLfloat) tobj->compareMode;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					"glGetTexParameter: invalid pname: 0x%x", pname);
				return;
			}
			break;
		case GL_TEXTURE_COMPARE_FUNC_ARB:
			if (g->extensions.ARB_shadow) {
				*params = (GLfloat) tobj->compareFunc;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					"glGetTexParameter: invalid pname: 0x%x", pname);
				return;
			}
			break;
#endif
#ifdef CR_ARB_shadow_ambient
		case GL_TEXTURE_COMPARE_FAIL_VALUE_ARB:
			if (g->extensions.ARB_shadow_ambient) {
				*params = (GLfloat) tobj->compareFailValue;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					"glGetTexParameter: invalid pname: 0x%x", pname);
				return;
			}
			break;
#endif
#ifdef CR_SGIS_generate_mipmap
		case GL_GENERATE_MIPMAP_SGIS:
			if (g->extensions.SGIS_generate_mipmap) {
				*params = (GLfloat) tobj->generateMipmap;
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
			params[0] = (GLint) (tobj->borderColor.r * CR_MAXINT);
			params[1] = (GLint) (tobj->borderColor.g * CR_MAXINT);
			params[2] = (GLint) (tobj->borderColor.b * CR_MAXINT);
			params[3] = (GLint) (tobj->borderColor.a * CR_MAXINT);
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
#ifdef CR_ARB_depth_texture
		case GL_DEPTH_TEXTURE_MODE_ARB:
			if (g->extensions.ARB_depth_texture) {
				*params = (GLint) tobj->depthMode;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					"glGetTexParameter: invalid pname: 0x%x", pname);
				return;
			}
			break;
#endif
#ifdef CR_ARB_shadow
		case GL_TEXTURE_COMPARE_MODE_ARB:
			if (g->extensions.ARB_shadow) {
				*params = (GLint) tobj->compareMode;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					"glGetTexParameter: invalid pname: 0x%x", pname);
				return;
			}
			break;
		case GL_TEXTURE_COMPARE_FUNC_ARB:
			if (g->extensions.ARB_shadow) {
				*params = (GLint) tobj->compareFunc;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					"glGetTexParameter: invalid pname: 0x%x", pname);
				return;
			}
			break;
#endif
#ifdef CR_ARB_shadow_ambient
		case GL_TEXTURE_COMPARE_FAIL_VALUE_ARB:
			if (g->extensions.ARB_shadow_ambient) {
				*params = (GLint) tobj->compareFailValue;
			}
			else {
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					"glGetTexParameter: invalid pname: 0x%x", pname);
				return;
			}
			break;
#endif
#ifdef CR_SGIS_generate_mipmap
		case GL_GENERATE_MIPMAP_SGIS:
			if (g->extensions.SGIS_generate_mipmap) {
				*params = (GLint) tobj->generateMipmap;
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


void crStateTextureSwitch(CRContext *g, CRTextureBits *t, CRbitvalue *bitID, 
						  CRTextureState *from, CRTextureState *to) 
{
	unsigned int i,j;
	glAble able[2];
	CRbitvalue nbitID[CR_MAX_BITARRAY];

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
			if (from->unit[i].currentTexture1D->name != to->unit[i].currentTexture1D->name) 
			{
				diff_api.BindTexture(GL_TEXTURE_1D, to->unit[i].currentTexture1D->name);
				FILLDIRTY(t->current[i]);
				FILLDIRTY(t->dirty);
			}
			if (from->unit[i].currentTexture2D->name != to->unit[i].currentTexture2D->name) 
			{
				diff_api.BindTexture(GL_TEXTURE_2D, to->unit[i].currentTexture2D->name);
				FILLDIRTY(t->current[i]);
				FILLDIRTY(t->dirty);
			}
#ifdef CR_OPENGL_VERSION_1_2
			if (from->unit[i].currentTexture3D->name != to->unit[i].currentTexture3D->name) {
				diff_api.BindTexture(GL_TEXTURE_3D, to->unit[i].currentTexture3D->name);
				FILLDIRTY(t->current[i]);
				FILLDIRTY(t->dirty);
			}
#endif
#ifdef CR_ARB_texture_cube_map
			if (g->extensions.ARB_texture_cube_map &&
				from->unit[i].currentTextureCubeMap->name != to->unit[i].currentTextureCubeMap->name) {
				diff_api.BindTexture(GL_TEXTURE_CUBE_MAP_ARB, to->unit[i].currentTextureCubeMap->name);
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
			if (from->unit[i].combineModeRGB != to->unit[i].combineModeRGB)
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, to->unit[i].combineModeRGB);
				FILLDIRTY(t->envBit[i]);
				FILLDIRTY(t->dirty);
			}
			if (from->unit[i].combineModeA != to->unit[i].combineModeA)
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, to->unit[i].combineModeA);
				FILLDIRTY(t->envBit[i]);
				FILLDIRTY(t->dirty);
			}
			if (from->unit[i].combineSourceRGB[0] != to->unit[i].combineSourceRGB[0])
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, to->unit[i].combineSourceRGB[0]);
				FILLDIRTY(t->envBit[i]);
				FILLDIRTY(t->dirty);
			}
			if (from->unit[i].combineSourceRGB[1] != to->unit[i].combineSourceRGB[1])
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, to->unit[i].combineSourceRGB[1]);
				FILLDIRTY(t->envBit[i]);
				FILLDIRTY(t->dirty);
			}
			if (from->unit[i].combineSourceRGB[2] != to->unit[i].combineSourceRGB[2])
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, to->unit[i].combineSourceRGB[2]);
				FILLDIRTY(t->envBit[i]);
				FILLDIRTY(t->dirty);
			}
			if (from->unit[i].combineSourceA[0] != to->unit[i].combineSourceA[0])
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, to->unit[i].combineSourceA[0]);
				FILLDIRTY(t->envBit[i]);
				FILLDIRTY(t->dirty);
			}
			if (from->unit[i].combineSourceA[1] != to->unit[i].combineSourceA[1])
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, to->unit[i].combineSourceA[1]);
				FILLDIRTY(t->envBit[i]);
				FILLDIRTY(t->dirty);
			}
			if (from->unit[i].combineSourceA[2] != to->unit[i].combineSourceA[2])
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_ARB, to->unit[i].combineSourceA[2]);
				FILLDIRTY(t->envBit[i]);
				FILLDIRTY(t->dirty);
			}
			if (from->unit[i].combineOperandRGB[0] != to->unit[i].combineOperandRGB[0])
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, to->unit[i].combineOperandRGB[0]);
				FILLDIRTY(t->envBit[i]);
				FILLDIRTY(t->dirty);
			}
			if (from->unit[i].combineOperandRGB[1] != to->unit[i].combineOperandRGB[1])
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, to->unit[i].combineOperandRGB[1]);
				FILLDIRTY(t->envBit[i]);
				FILLDIRTY(t->dirty);
			}
			if (from->unit[i].combineOperandRGB[2] != to->unit[i].combineOperandRGB[2])
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, to->unit[i].combineOperandRGB[2]);
				FILLDIRTY(t->envBit[i]);
				FILLDIRTY(t->dirty);
			}
			if (from->unit[i].combineOperandA[0] != to->unit[i].combineOperandA[0])
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, to->unit[i].combineOperandA[0]);
				FILLDIRTY(t->envBit[i]);
				FILLDIRTY(t->dirty);
			}
			if (from->unit[i].combineOperandA[1] != to->unit[i].combineOperandA[1])
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB, to->unit[i].combineOperandA[1]);
				FILLDIRTY(t->envBit[i]);
				FILLDIRTY(t->dirty);
			}
			if (from->unit[i].combineOperandA[2] != to->unit[i].combineOperandA[2])
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_ARB, to->unit[i].combineOperandA[2]);
				FILLDIRTY(t->envBit[i]);
				FILLDIRTY(t->dirty);
			}
			if (from->unit[i].combineScaleRGB != to->unit[i].combineScaleRGB)
			{
				diff_api.TexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, to->unit[i].combineScaleRGB);
				FILLDIRTY(t->envBit[i]);
				FILLDIRTY(t->dirty);
			}
			if (from->unit[i].combineScaleA != to->unit[i].combineScaleA)
			{
				diff_api.TexEnvf(GL_TEXTURE_ENV, GL_ALPHA_SCALE, to->unit[i].combineScaleA);
				FILLDIRTY(t->envBit[i]);
				FILLDIRTY(t->dirty);
			}
			INVERTDIRTY(t->envBit[i], nbitID);
		}
	}
	diff_api.ActiveTextureARB( GL_TEXTURE0_ARB + to->curTextureUnit );
}

void crStateTextureDiff(CRContext *g, CRTextureBits *t, CRbitvalue *bitID, 
						 CRTextureState *from, CRTextureState *to) 
{
	CRTextureObj *tobj = NULL;
	GLuint name=0;
	GLuint *cname=NULL;
	unsigned int i;
	int j;
	glAble able[2];
	CRbitvalue nbitID[CR_MAX_BITARRAY];

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
			tobj = to->unit[i].currentTexture1D;
			name = tobj->name;
			cname = &(from->unit[i].currentTexture1D->name);
		}

		if (to->unit[i].enabled2D == GL_TRUE) 
		{
			tobj = to->unit[i].currentTexture2D;
			name = tobj->name;
			cname = &(from->unit[i].currentTexture2D->name);
		}

#ifdef CR_OPENGL_VERSION_1_2
		if (to->unit[i].enabled3D == GL_TRUE) 
		{
			tobj = to->unit[i].currentTexture3D;
			name = tobj->name;
			cname = &(from->unit[i].currentTexture3D->name);
		}
#endif
#ifdef CR_ARB_texture_cube_map
		if (g->extensions.ARB_texture_cube_map && to->unit[i].enabledCubeMap == GL_TRUE)
		{
			tobj = to->unit[i].currentTextureCubeMap;
			name = tobj->name;
			cname = &(from->unit[i].currentTextureCubeMap->name);
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
				if (g->extensions.EXT_texture_filter_anisotropic) {
					diff_api.TexParameterf(tobj->target, GL_TEXTURE_MAX_ANISOTROPY_EXT, tobj->maxAnisotropy);
				}
#endif
#ifdef CR_ARB_depth_texture
				if (g->extensions.ARB_depth_texture)
					diff_api.TexParameteri(tobj->target, GL_DEPTH_TEXTURE_MODE_ARB, tobj->depthMode);
#endif
#ifdef CR_ARB_shadow
				if (g->extensions.ARB_shadow) {
					diff_api.TexParameteri(tobj->target, GL_TEXTURE_COMPARE_MODE_ARB, tobj->compareMode);
					diff_api.TexParameteri(tobj->target, GL_TEXTURE_COMPARE_FUNC_ARB, tobj->compareFunc);
				}
#endif
#ifdef CR_ARB_shadow_ambient
				if (g->extensions.ARB_shadow_ambient) {
					diff_api.TexParameterf(tobj->target, GL_TEXTURE_COMPARE_FAIL_VALUE_ARB, tobj->compareFailValue);
				}
#endif
#ifdef CR_SGIS_generate_mipmap
				if (g->extensions.SGIS_generate_mipmap) {
					diff_api.TexParameteri(tobj->target, GL_GENERATE_MIPMAP_SGIS, tobj->generateMipmap);
				}
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
			tobj = to->unit[i].currentTexture1D;
			name = tobj->name;
			cname = &(from->unit[i].currentTexture1D->name);
		}

		if (to->unit[i].enabled2D == GL_TRUE) 
		{
			tobj = to->unit[i].currentTexture2D;
			name = tobj->name;
			cname = &(from->unit[i].currentTexture2D->name);
		}

#ifdef CR_OPENGL_VERSION_1_2
		if (to->unit[i].enabled3D == GL_TRUE)
		{
			tobj = to->unit[i].currentTexture3D;
			name = tobj->name;
			cname = &(from->unit[i].currentTexture3D->name);
		}
#endif
#ifdef CR_ARB_texture_cube_map
		if (g->extensions.ARB_texture_cube_map &&
			to->unit[i].enabledCubeMap == GL_TRUE)
		{
			tobj = to->unit[i].currentTextureCubeMap;
			name = tobj->name;
			cname = &(from->unit[i].currentTextureCubeMap->name);
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
#ifdef CR_ARB_texture_env_combine
			if (from->unit[i].combineModeRGB != to->unit[i].combineModeRGB)
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, to->unit[i].combineModeRGB);
				from->unit[i].combineModeRGB = to->unit[i].combineModeRGB;
			}
			if (from->unit[i].combineModeA != to->unit[i].combineModeA)
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, to->unit[i].combineModeA);
				from->unit[i].combineModeA = to->unit[i].combineModeA;
			}
			if (from->unit[i].combineSourceRGB[0] != to->unit[i].combineSourceRGB[0])
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, to->unit[i].combineSourceRGB[0]);
				from->unit[i].combineSourceRGB[0] = to->unit[i].combineSourceRGB[0];
			}
			if (from->unit[i].combineSourceRGB[1] != to->unit[i].combineSourceRGB[1])
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, to->unit[i].combineSourceRGB[1]);
				from->unit[i].combineSourceRGB[1] = to->unit[i].combineSourceRGB[1];
			}
			if (from->unit[i].combineSourceRGB[2] != to->unit[i].combineSourceRGB[2])
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, to->unit[i].combineSourceRGB[2]);
				from->unit[i].combineSourceRGB[2] = to->unit[i].combineSourceRGB[2];
			}
			if (from->unit[i].combineSourceA[0] != to->unit[i].combineSourceA[0])
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, to->unit[i].combineSourceA[0]);
				from->unit[i].combineSourceA[0] = to->unit[i].combineSourceA[0];
			}
			if (from->unit[i].combineSourceA[1] != to->unit[i].combineSourceA[1])
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, to->unit[i].combineSourceA[1]);
				from->unit[i].combineSourceA[1] = to->unit[i].combineSourceA[1];
			}
			if (from->unit[i].combineSourceA[2] != to->unit[i].combineSourceA[2])
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_ARB, to->unit[i].combineSourceA[2]);
				from->unit[i].combineSourceA[2] = to->unit[i].combineSourceA[2];
			}
			if (from->unit[i].combineOperandRGB[0] != to->unit[i].combineOperandRGB[0])
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, to->unit[i].combineOperandRGB[0]);
				from->unit[i].combineOperandRGB[0] = to->unit[i].combineOperandRGB[0];
			}
			if (from->unit[i].combineOperandRGB[1] != to->unit[i].combineOperandRGB[1])
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, to->unit[i].combineOperandRGB[1]);
				from->unit[i].combineOperandRGB[1] = to->unit[i].combineOperandRGB[1];
			}
			if (from->unit[i].combineOperandRGB[2] != to->unit[i].combineOperandRGB[2])
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, to->unit[i].combineOperandRGB[2]);
				from->unit[i].combineOperandRGB[2] = to->unit[i].combineOperandRGB[2];
			}
			if (from->unit[i].combineOperandA[0] != to->unit[i].combineOperandA[0])
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, to->unit[i].combineOperandA[0]);
				from->unit[i].combineOperandA[0] = to->unit[i].combineOperandA[0];
			}
			if (from->unit[i].combineOperandA[1] != to->unit[i].combineOperandA[1])
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB, to->unit[i].combineOperandA[1]);
				from->unit[i].combineOperandA[1] = to->unit[i].combineOperandA[1];
			}
			if (from->unit[i].combineOperandA[2] != to->unit[i].combineOperandA[2])
			{
				diff_api.TexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_ARB, to->unit[i].combineOperandA[2]);
				from->unit[i].combineOperandA[2] = to->unit[i].combineOperandA[2];
			}
			if (from->unit[i].combineScaleRGB != to->unit[i].combineScaleRGB)
			{
				diff_api.TexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, to->unit[i].combineScaleRGB);
				from->unit[i].combineScaleRGB = to->unit[i].combineScaleRGB;
			}
			if (from->unit[i].combineScaleA != to->unit[i].combineScaleA)
			{
				diff_api.TexEnvf(GL_TEXTURE_ENV, GL_ALPHA_SCALE, to->unit[i].combineScaleA);
				from->unit[i].combineScaleA = to->unit[i].combineScaleA;
			}
#endif
#if CR_EXT_texture_lod_bias
			if (from->unit[i].lodBias != to->unit[i].lodBias)
			{
				diff_api.TexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, to->unit[i].lodBias);
				from->unit[i].lodBias = to->unit[i].lodBias;
			}
#endif
			INVERTDIRTY(t->envBit[i], nbitID);
		}
	}
	diff_api.ActiveTextureARB( GL_TEXTURE0_ARB + to->curTextureUnit );
}
