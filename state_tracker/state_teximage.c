/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */


#include "state.h"
#include "state/cr_statetypes.h"
#include "state/cr_texture.h"
#include "cr_pixeldata.h"
#include "cr_string.h"
#include "cr_mem.h"
#include "cr_version.h"
#include "state_internals.h"


/*
 * If GL_GENERATE_MIPMAP_SGIS is true and we modify the base level texture
 * image we have to finish the mipmap.
 * All we really have to do is fill in the width/height/format/etc for the
 * remaining image levels.  The image data doesn't matter here - the back-
 * end OpenGL library will fill those in.
 */
static void
generate_mipmap(CRTextureObj *tobj, GLenum target)
{
	CRTextureLevel *levels;
	GLint level, width, height, depth;

	switch (target) {
		case GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB:
			levels = tobj->level;
			break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB:
			levels = tobj->negativeXlevel;
			break;
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB:
			levels = tobj->positiveYlevel;
			break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB:
			levels = tobj->negativeYlevel;
			break;
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB:
			levels = tobj->positiveZlevel;
			break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB:
			levels = tobj->negativeZlevel;
			break;
		default:
			levels = tobj->level;
	}

	width = levels[tobj->baseLevel].width;
	height = levels[tobj->baseLevel].height;
	depth = levels[tobj->baseLevel].depth;

	for (level = tobj->baseLevel + 1; level <= tobj->maxLevel; level++)
	{
		if (width > 1)
			width /= 2;
		if (height > 1)
			height /= 2;
		if (depth > 1)
			depth /= 2;
		levels[level].width = width;
		levels[level].height = height;
		levels[level].depth = depth;
		levels[level].internalFormat = levels[tobj->baseLevel].internalFormat;
		levels[level].format = levels[tobj->baseLevel].format;
		levels[level].type = levels[tobj->baseLevel].type;
#ifdef CR_ARB_texture_compression
		levels[level].compressed = levels[tobj->baseLevel].compressed;
#endif
		levels[level].texFormat = levels[tobj->baseLevel].texFormat;
		if (width == 1 && height == 1 && depth == 1)
			break;
	}
}



void STATE_APIENTRY
crStateTexImage1D(GLenum target, GLint level, GLint internalFormat,
									GLsizei width, GLint border, GLenum format,
									GLenum type, const GLvoid * pixels)
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
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glTexImage1D called in Begin/End");
		return;
	}

	FLUSH();

	if (target != GL_TEXTURE_1D && target != GL_PROXY_TEXTURE_1D)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								 "glTexImage1D target != (GL_TEXTURE_1D | GL_PROXY_TEXTURE_1D): 0x%x",
								 target);
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
	i = 1;
	if (width > 0)
	{
		for (i = width - 2 * border; i > 0 && !(i & 0x1); i = i >> 1)
		{
			/* EMPTY BODY */
		}
	}
	if (width < 0 || i != 1)
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
		tobj = &(t->proxy1D);
		tl = tobj->level + level;
		tl->bytes = 0;
	}
	else
	{
		CRASSERT(target == GL_TEXTURE_1D);
		tobj = t->unit[t->curTextureUnit].currentTexture1D;
		tobj->target = GL_TEXTURE_1D;
		tl = tobj->level + level;
		tl->bytes = crImageSize(format, type, width, 1);
	}

	if (tl->bytes)
	{
		/* this is not a proxy texture target so alloc storage */
		if (tl->img)
			crFree(tl->img);
		tl->img = (GLubyte *) crAlloc(tl->bytes);
		if (!tl->img)
		{
			crStateError(__LINE__, __FILE__, GL_OUT_OF_MEMORY,
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
	crStateTextureInitTextureFormat(tl, internalFormat);
	tl->type = type;
	if (width)
		tl->bytesPerPixel = tl->bytes / width;
	else
		tl->bytesPerPixel = 0;

#ifdef CR_SGIS_generate_mipmap
	if (level == tobj->baseLevel && tobj->generateMipmap) {
		generate_mipmap(tobj, target);
	}
#endif

	/* XXX may need to do some fine-tuning here for proxy textures */
	DIRTY(tobj->dirty, g->neg_bitid);
	for (i = 0; i < g->limits.maxTextureUnits; i++)
	{
		DIRTY(tl->dirty[i], g->neg_bitid);
		DIRTY(tobj->imageBit[i], g->neg_bitid);
	}
	DIRTY(tb->dirty, g->neg_bitid);
}

void STATE_APIENTRY
crStateTexImage2D(GLenum target, GLint level, GLint internalFormat,
									GLsizei width, GLsizei height, GLint border,
									GLenum format, GLenum type, const GLvoid * pixels)
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRClientState *c = &(g->client);
	CRTextureObj *tobj = NULL;
	CRTextureLevel *tl = NULL;
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);
	unsigned int i;
	int is_distrib = ((type == GL_TRUE) || (type == GL_FALSE));
	CRTextureUnit *unit;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glTexImage2D called in Begin/End");
		return;
	}

	FLUSH();

	switch (target)
	{
	case GL_TEXTURE_2D:
	case GL_PROXY_TEXTURE_2D:
		break;											/* legal */
#ifdef CR_ARB_texture_cube_map
	case GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB:
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB:
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB:
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB:
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB:
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB:
	case GL_PROXY_TEXTURE_CUBE_MAP_ARB:
		if (g->extensions.ARB_texture_cube_map)
			break;										/* legal */
#endif
	default:
		/* error */
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								 "glTexImage2D invalid target: 0x%x", target);
		return;
	}

	if (level < 0
			|| ((target == GL_TEXTURE_2D || target == GL_PROXY_TEXTURE_2D)
					&& level > t->maxLevel)
#ifdef CR_ARB_texture_cube_map
			|| ((target != GL_TEXTURE_2D && target != GL_PROXY_TEXTURE_2D)
					&& level > t->maxCubeMapLevel)
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
		for (i = width - 2 * border; i > 0 && !(i & 0x1); i = i >> 1)
		{
			/* EMPTY BODY */
		}
	}
	if (width < 0 || ((i != 1) && (!is_distrib)))
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
	i = 1;
	if (height > 0)
	{
		for (i = height - 2 * border; i > 0 && !(i & 0x1); i = i >> 1)
		{
			/* EMPTY BODY */
		}
	}
	if (height < 0 || ((i != 1) && (!is_distrib)))
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
								 "glTexImage2D height is not valid: %d", height);
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
		if (width > ((int) g->limits.maxTextureSize + 2)
				|| height > ((int) g->limits.maxTextureSize + 2))
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
				height > (int) g->limits.maxCubeMapTextureSize || width != height)
		{
			/* clear all the texture object state */
			crStateTextureInitTextureObj(t, &(t->proxyCubeMap), 0,
																	 GL_TEXTURE_CUBE_MAP_ARB);
			return;
		}
	}
	else													/* Cube map */
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
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glTexImage2D cube map width and height are unequal: %dx%d",
									 width, height);
			return;
		}
#endif
	}

	unit = t->unit + t->curTextureUnit;

	/*
	 ** Only set these fields if 
	 ** defining the base texture.
	 */
	if (target == GL_TEXTURE_2D)
	{
		tobj = unit->currentTexture2D;
		tl = tobj->level + level;
		if (is_distrib)
		{
			tl->bytes = crStrlen(pixels) + 1 +
				(type == GL_TRUE ? width * height * 3 : 0);
		}
		else
		{
			tl->bytes = crImageSize(format, type, width, height);
		}
	}
	else if (target == GL_PROXY_TEXTURE_2D)
	{
		tobj = &(t->proxy2D);
		tl = tobj->level + level;
		tl->bytes = 0;
	}
#ifdef CR_ARB_texture_cube_map
	else if (target == GL_PROXY_TEXTURE_CUBE_MAP_ARB)
	{
		tobj = &(t->proxyCubeMap);
		tl = tobj->level + level;
		tl->bytes = 0;
	}
	else if (target == GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB)
	{
		tobj = unit->currentTextureCubeMap;
		tl = tobj->level + level;
		tl->bytes = crImageSize(format, type, width, height);
	}
	else if (target == GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB)
	{
		tobj = unit->currentTextureCubeMap;
		tl = tobj->negativeXlevel + level;
		tl->bytes = crImageSize(format, type, width, height);
	}
	else if (target == GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB)
	{
		tobj = unit->currentTextureCubeMap;
		tl = tobj->positiveYlevel + level;
		tl->bytes = crImageSize(format, type, width, height);
	}
	else if (target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB)
	{
		tobj = unit->currentTextureCubeMap;
		tl = tobj->negativeYlevel + level;
		tl->bytes = crImageSize(format, type, width, height);
	}
	else if (target == GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB)
	{
		tobj = unit->currentTextureCubeMap;
		tl = tobj->positiveZlevel + level;
		tl->bytes = crImageSize(format, type, width, height);
	}
	else if (target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB)
	{
		tobj = unit->currentTextureCubeMap;
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
			crStateError(__LINE__, __FILE__, GL_OUT_OF_MEMORY,
									 "glTexImage2D out of memory");
			return;
		}
		if (pixels)
		{
			if (is_distrib)
			{
				crMemcpy((void *) tl->img, (void *) pixels, tl->bytes);
			}
			else
			{
				crPixelCopy2D(width, height, (GLvoid *) (tl->img), format, type, NULL,	/* dst */
											pixels, format, type, &(c->unpack));	/* src */
			}
		}
	}

	tl->width = width;
	tl->height = height;
	tl->depth = 1;
	tl->format = format;
	tl->internalFormat = internalFormat;
	crStateTextureInitTextureFormat(tl, internalFormat);
	tl->border = border;
	tl->type = type;
	if (width && height)
	{
		if (is_distrib)
			tl->bytesPerPixel = 3;
		else
			tl->bytesPerPixel = tl->bytes / (width * height);
	}
	else
		tl->bytesPerPixel = 0;

#ifdef CR_SGIS_generate_mipmap
	if (level == tobj->baseLevel && tobj->generateMipmap) {
		generate_mipmap(tobj, target);
	}
#endif

	/* XXX may need to do some fine-tuning here for proxy textures */
	DIRTY(tobj->dirty, g->neg_bitid);
	for (i = 0; i < g->limits.maxTextureUnits; i++)
	{
		DIRTY(tl->dirty[i], g->neg_bitid);
		DIRTY(tobj->imageBit[i], g->neg_bitid);
	}
	DIRTY(tb->dirty, g->neg_bitid);
}



void STATE_APIENTRY
crStateTexSubImage1D(GLenum target, GLint level, GLint xoffset,
										 GLsizei width, GLenum format,
										 GLenum type, const GLvoid * pixels)
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);
	CRTextureUnit *unit = t->unit + t->curTextureUnit;
	CRTextureObj *tobj = unit->currentTexture1D;
	CRTextureLevel *tl = tobj->level + level;
	unsigned int i;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glTexSubImage1D called in Begin/End");
		return;
	}

	FLUSH();

	if (target != GL_TEXTURE_1D && target != GL_PROXY_TEXTURE_1D)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								 "glTexSubImage1D target != GL_TEXTURE_1D: %d", target);
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
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
								 "glTexSubImage1D(bad width or xoffset)");
		return;
	}

	xoffset += tl->border;

	crPixelCopy1D((void *) (tl->img + xoffset * tl->bytesPerPixel),
								tl->format, tl->type,
								pixels, format, type, width, &(c->unpack));

#ifdef CR_SGIS_generate_mipmap
	if (level == tobj->baseLevel && tobj->generateMipmap) {
		generate_mipmap(tobj, target);
	}
#endif

	DIRTY(tobj->dirty, g->neg_bitid);
	for (i = 0; i < g->limits.maxTextureUnits; i++)
	{
		DIRTY(tl->dirty[i], g->neg_bitid);
		DIRTY(tobj->imageBit[i], g->neg_bitid);
	}
	DIRTY(tb->dirty, g->neg_bitid);
}

void STATE_APIENTRY
crStateTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset,
										 GLsizei width, GLsizei height,
										 GLenum format, GLenum type, const GLvoid * pixels)
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);
	CRTextureUnit *unit = t->unit + t->curTextureUnit;
	CRTextureObj *tobj = unit->currentTexture2D;
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
	else if (target >= GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB
					 && target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB
					 && g->extensions.ARB_texture_cube_map)
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
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								 "glTexSubImage2D target != GL_TEXTURE_2D: %d", target);
		return;
	}

	if (width + xoffset > tl->width)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
								 "glTexSubImage2D(bad width or xoffset)");
		return;
	}
	if (height + yoffset > tl->height)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
								 "glTexSubImage2D(bad heigh or yoffset)");
		return;
	}

	xoffset += tl->border;
	yoffset += tl->border;

	subimg =
		(GLubyte *) crAlloc(crImageSize(tl->format, tl->type, width, height));

	crPixelCopy2D(width, height, subimg, tl->format, tl->type, NULL,	/* dst */
								pixels, format, type, &(c->unpack));	/* src */

	img = tl->img +
		xoffset * tl->bytesPerPixel + yoffset * tl->width * tl->bytesPerPixel;

	src = subimg;

	/* Copy the data into the texture */
	for (i = 0; i < height; i++)
	{
		crMemcpy(img, src, tl->bytesPerPixel * width);
		img += tl->width * tl->bytesPerPixel;
		src += width * tl->bytesPerPixel;
	}

	crFree(subimg);

#ifdef CR_SGIS_generate_mipmap
	if (level == tobj->baseLevel && tobj->generateMipmap) {
		generate_mipmap(tobj, target);
	}
#endif

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
void STATE_APIENTRY
crStateTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset,
										 GLint zoffset, GLsizei width, GLsizei height,
										 GLsizei depth, GLenum format, GLenum type,
										 const GLvoid * pixels)
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRClientState *c = &(g->client);
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);
	CRTextureUnit *unit = t->unit + t->curTextureUnit;
	CRTextureObj *tobj = unit->currentTexture3D;
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
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glTexSubImage3D depth oob: %d", depth);
			return;
		}
	}
#ifdef CR_ARB_texture_cube_map
	else if (target >= GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB
					 && target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB
					 && g->extensions.ARB_texture_cube_map)
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
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glTexSubImage3D depth oob:  %d", depth);
		}
	}
#endif /* CR_ARB_texture_cube_map */
	else
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								 "glTexSubImage3D target != GL_TEXTURE_3D: %d", target);
		return;
	}

	if (width + xoffset > tl->width)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
								 "glTexSubImage3D(bad width or xoffset)");
		return;
	}
	if (height + yoffset > tl->height)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
								 "glTexSubImage3D(bad height or yoffset)");
		return;
	}
	if (depth + zoffset > tl->depth)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
								 "glTexSubImage3D(bad depth or zoffset)");
		return;
	}

	xoffset += tl->border;
	yoffset += tl->border;
	zoffset += tl->border;

	subimg =
		(GLubyte *)
		crAlloc(crTextureSize(tl->format, tl->type, width, height, depth));

	crPixelCopy3D(width, height, depth, subimg, tl->format, tl->type, NULL,
								pixels, format, type, &(c->unpack));

	img = tl->img + xoffset * tl->bytesPerPixel +
		yoffset * tl->width * tl->bytesPerPixel +
		zoffset * tl->width * tl->height * tl->bytesPerPixel;

	src = subimg;

	/* Copy the data into the texture */
	for (i = 0; i < depth; i++)
	{
		crMemcpy(img, src, tl->bytesPerPixel * width * height);
		img += tl->width * tl->height * tl->bytesPerPixel;
		src += width * height * tl->bytesPerPixel;
	}

	crFree(subimg);

#ifdef CR_SGIS_generate_mipmap
	if (level == tobj->baseLevel && tobj->generateMipmap) {
		generate_mipmap(tobj, target);
	}
#endif

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
void STATE_APIENTRY
crStateTexImage3D(GLenum target, GLint level,
									GLint internalFormat,
									GLsizei width, GLsizei height,
									GLsizei depth, GLint border,
									GLenum format, GLenum type, const GLvoid * pixels)
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRClientState *c = &(g->client);
	CRTextureUnit *unit = t->unit + t->curTextureUnit;
	CRTextureObj *tobj = NULL;
	CRTextureLevel *tl = NULL;
	CRStateBits *sb = GetCurrentBits();
	CRTextureBits *tb = &(sb->texture);
	unsigned int i;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glTexImage3D called in Begin/End");
		return;
	}


	if (target != GL_TEXTURE_3D && target != GL_PROXY_TEXTURE_3D)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								 "glTexImage3D invalid target: 0x%x", target);
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
	i = 1;
	if (width > 0)
		for (i = width - 2 * border; i > 0 && !(i & 0x1); i = i >> 1);
	if (width < 0 || i != 1)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
								 "glTexImage3D width is not valid: %d", width);
		return;
	}

	/* check the bits in height */
	i = 1;
	if (height > 0)
		for (i = height - 2 * border; i > 0 && !(i & 0x1); i = i >> 1);
	if (height < 0 || i != 1)
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
	i = 1;
	if (depth > 0)
		for (i = depth - 2 * border; i > 0 && !(i & 0x1); i = i >> 1);
	if (depth < 0 || i != 1)
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

	if (width > (int) (g->limits.max3DTextureSize + 2))
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

	if (height > (int) (g->limits.max3DTextureSize + 2))
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
		tobj = unit->currentTexture3D;	/* FIXME: per unit! */
		tl = tobj->level + level;
		tl->bytes = crTextureSize(format, type, width, height, depth);
	}
	else if (target == GL_PROXY_TEXTURE_3D)
	{
		tobj = &(t->proxy3D);				/* FIXME: per unit! */
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
			crStateError(__LINE__, __FILE__, GL_OUT_OF_MEMORY,
									 "glTexImage3D out of memory");
			return;
		}
		if (pixels)
			crPixelCopy3D(width, height, depth, (GLvoid *) (tl->img), format, type,
										NULL, pixels, format, type, &(c->unpack));
	}

	tl->internalFormat = internalFormat;
	tl->border = border;
	tl->width = width;
	tl->height = height;
	tl->depth = depth;
	tl->format = format;
	tl->type = type;

#ifdef CR_SGIS_generate_mipmap
	if (level == tobj->baseLevel && tobj->generateMipmap) {
		generate_mipmap(tobj, target);
	}
#endif

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
void STATE_APIENTRY
crStateTexImage3DEXT(GLenum target, GLint level,
										 GLenum internalFormat,
										 GLsizei width, GLsizei height, GLsizei depth,
										 GLint border, GLenum format, GLenum type,
										 const GLvoid * pixels)
{
	crStateTexImage3D(target, level, (GLint) internalFormat, width, height,
										depth, border, format, type, pixels);
}
#endif /* GL_EXT_texture3D */



void STATE_APIENTRY
crStateCompressedTexImage1DARB(GLenum target, GLint level,
															 GLenum internalformat, GLsizei width,
															 GLint border, GLsizei imageSize,
															 const GLvoid * data)
{
	crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
							 "glCompressedTexImage1DARB - no compression formats supported");
}


void STATE_APIENTRY
crStateCompressedTexImage2DARB(GLenum target, GLint level,
															 GLenum internalformat, GLsizei width,
															 GLsizei height, GLint border,
															 GLsizei imageSize, const GLvoid * data)
{
	crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
							 "glCompressedTexImage2DARB - no compression formats supported");
}


void STATE_APIENTRY
crStateCompressedTexImage3DARB(GLenum target, GLint level,
															 GLenum internalformat, GLsizei width,
															 GLsizei height, GLsizei depth, GLint border,
															 GLsizei imageSize, const GLvoid * data)
{
	crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
							 "glCompressedTexImage3DARB - no compression formats supported");
}


void STATE_APIENTRY
crStateCompressedTexSubImage1DARB(GLenum target, GLint level, GLint xoffset,
																	GLsizei width, GLenum format,
																	GLsizei imageSize, const GLvoid * data)
{
	crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
							 "glCompressedTexSubImage1DARB - no compression formats supported");
}


void STATE_APIENTRY
crStateCompressedTexSubImage2DARB(GLenum target, GLint level, GLint xoffset,
																	GLint yoffset, GLsizei width,
																	GLsizei height, GLenum format,
																	GLsizei imageSize, const GLvoid * data)
{
	crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
							 "glCompressedTexSubImage2DARB - no compression formats supported");
}


void STATE_APIENTRY
crStateCompressedTexSubImage3DARB(GLenum target, GLint level, GLint xoffset,
																	GLint yoffset, GLint zoffset, GLsizei width,
																	GLsizei height, GLsizei depth,
																	GLenum format, GLsizei imageSize,
																	const GLvoid * data)
{
	crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
							 "glCompressedTexSubImage3DARB - no compression formats supported");
}


void STATE_APIENTRY
crStateGetCompressedTexImageARB(GLenum target, GLint level, GLvoid * img)
{
	crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
							 "glGetCompressedTexImageARB - no compression formats supported");
}




void STATE_APIENTRY
crStateGetTexImage(GLenum target, GLint level, GLenum format,
									 GLenum type, GLvoid * pixels)
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);
	CRTextureUnit *unit = t->unit + t->curTextureUnit;
	CRClientState *c = &(g->client);
	CRTextureObj *tobj = NULL;
	CRTextureLevel *tl = NULL;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glGetTexImage called in begin/end");
		return;
	}

	if (target == GL_TEXTURE_2D)
	{
		tobj = unit->currentTexture2D;
		tl = tobj->level + level;
	}
	else if (target == GL_TEXTURE_1D)
	{
		tobj = unit->currentTexture1D;
		tl = tobj->level + level;
	}
#ifdef CR_OPENGL_VERSION_1_2
	else if (target == GL_TEXTURE_3D)
	{
		tobj = unit->currentTexture3D;
		tl = tobj->level + level;
	}
#endif
#ifdef CR_ARB_texture_cube_map
	else if (target == GL_TEXTURE_CUBE_MAP_POSITIVE_X)
	{
		tobj = unit->currentTextureCubeMap;
		tl = tobj->level + level;
	}
	else if (target == GL_TEXTURE_CUBE_MAP_NEGATIVE_X)
	{
		tobj = unit->currentTextureCubeMap;
		tl = tobj->negativeXlevel + level;
	}
	else if (target == GL_TEXTURE_CUBE_MAP_POSITIVE_Y)
	{
		tobj = unit->currentTextureCubeMap;
		tl = tobj->positiveYlevel + level;
	}
	else if (target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y)
	{
		tobj = unit->currentTextureCubeMap;
		tl = tobj->negativeYlevel + level;
	}
	else if (target == GL_TEXTURE_CUBE_MAP_POSITIVE_Z)
	{
		tobj = unit->currentTextureCubeMap;
		tl = tobj->positiveZlevel + level;
	}
	else if (target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
	{
		tobj = unit->currentTextureCubeMap;
		tl = tobj->negativeZlevel + level;
	}
#endif
	else
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								 "glGetTexImage called with bogus target: %d", target);
		return;
	}

	switch (format)
	{
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

	switch (type)
	{
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
	if (target == GL_TEXTURE_3D)
	{
		crPixelCopy3D(tl->width, tl->height, tl->depth, (GLvoid *) pixels, format,
									type, NULL, (tl->img), format, type, &(c->pack));
	}
	else
#endif
	if ((target == GL_TEXTURE_2D) || (target == GL_TEXTURE_1D))
	{
		crPixelCopy2D(tl->width, tl->height, (GLvoid *) pixels, format, type, NULL,	/* dst */
									(tl->img), format, type, &(c->pack));	/* src */
	}
}
