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



static int
bitcount(int value)
{
	int bits = 0;
	for (; value > 0; value >>= 1) {
		if (value & 0x1)
			bits++;
	}
	return bits;
}


static GLboolean
IsProxyTarget(GLenum target)
{
	return (target == GL_PROXY_TEXTURE_1D ||
					target == GL_PROXY_TEXTURE_2D ||
					target == GL_PROXY_TEXTURE_3D ||
					target == GL_PROXY_TEXTURE_RECTANGLE_NV ||
					target == GL_PROXY_TEXTURE_CUBE_MAP);
}


/*
 * Test if a texture width, height or depth is legal.
 * It must be true that 0 <= size <= max.
 * Furthermore, if the ARB_texture_non_power_of_two extension isn't
 * present, size must also be a power of two.
 */
static GLboolean
isLegalSize(CRContext *g, GLsizei size, GLsizei max)
{
	if (size < 0 || size > max)
		return GL_FALSE;
	if (!g->extensions.ARB_texture_non_power_of_two) {
		/* check for power of two */
		if (size > 0 && bitcount(size) != 1)
			return GL_FALSE;
	}
	return GL_TRUE;
}



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
			crStateTextureInitTextureObj(g, &(t->proxy1D), 0, GL_TEXTURE_1D);
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
			crStateTextureInitTextureObj(g, &(t->proxy1D), 0, GL_TEXTURE_1D);
		}
		else
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glTexImage1D border oob: %d", border);
		}
		return;
	}

	if (!isLegalSize(g, width - 2 * border, g->limits.maxTextureSize)) {
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
								 "glTexImage1D(width)");
		if (target == GL_PROXY_TEXTURE_1D) {
			/* clear all the texture object state */
			crStateTextureInitTextureObj(g, &(t->proxy1D), 0, GL_TEXTURE_1D);
		}
		else {
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glTexImage1D(width=%d)", width);
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
	DIRTY(tobj->imageBit, g->neg_bitid);
	DIRTY(tl->dirty, g->neg_bitid);
	DIRTY(tb->dirty, g->neg_bitid);
}


/*
 * Do parameter error checking for glTexImage2D
 * Return GL_TRUE if any errors, GL_FALSE if no errors.
 */
static GLboolean
ErrorCheckTexImage2D(GLenum target, GLint level, GLsizei width, GLsizei height,
										 GLint border)
{
	CRContext *g = GetCurrentContext();
	CRTextureState *t = &(g->texture);

	/*
	 * Test target
	 */
	switch (target)
	{
	case GL_TEXTURE_2D:
	case GL_PROXY_TEXTURE_2D:
		break;											/* legal */
#ifdef CR_NV_texture_rectangle
	case GL_TEXTURE_RECTANGLE_NV:
	case GL_PROXY_TEXTURE_RECTANGLE_NV:
		if (!g->extensions.NV_texture_rectangle) {
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
									 "glTexImage2D invalid target: 0x%x", target);
			return GL_TRUE;
		}
		break;
#endif
#ifdef CR_ARB_texture_cube_map
	case GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB:
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB:
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB:
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB:
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB:
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB:
	case GL_PROXY_TEXTURE_CUBE_MAP_ARB:
		if (!g->extensions.ARB_texture_cube_map) {
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
									 "glTexImage2D invalid target: 0x%x", target);
			return GL_TRUE;
		}
		break;
#endif
	default:
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								 "glTexImage2D invalid target: 0x%x", target);
		return GL_TRUE;
	}

	/*
	 * Test level
	 */
	if (level < 0) {
		if (!IsProxyTarget(target))
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glTexImage2D(level=%d)", level);
		return GL_TRUE;
	}
	if ((target == GL_TEXTURE_2D || target == GL_PROXY_TEXTURE_2D)
			&& level > t->maxLevel) {
		if (!IsProxyTarget(target))
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glTexImage2D(level=%d)", level);
		return GL_TRUE;
	}
#ifdef CR_ARB_texture_cube_map
	else if ((target != GL_TEXTURE_2D && target != GL_PROXY_TEXTURE_2D)
					 && level > t->maxCubeMapLevel) {
		if (!IsProxyTarget(target))
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glTexImage2D(level=%d)", level);
		return GL_TRUE;
	}
#endif
#ifdef CR_NV_texture_rectangle
	else if ((target != GL_TEXTURE_2D && target != GL_PROXY_TEXTURE_2D)
					 && level > t->maxRectLevel) {
		if (!IsProxyTarget(target))
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glTexImage2D(level=%d)", level);
		return GL_TRUE;
	}
#endif

	/*
	 * Test border
	 */
	if (border != 0 && border != 1) {
		if (!IsProxyTarget(target))
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glTexImage2D(border=%d)", border);
		return GL_TRUE;
	}

	if ((target == GL_PROXY_TEXTURE_RECTANGLE_NV ||
			 target == GL_TEXTURE_RECTANGLE_NV) && border != 0) {
		if (!IsProxyTarget(target))
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glTexImage2D(border=%d)", border);
		return GL_TRUE;
	}

	/*
	 * Test width and height
	 */
	if (target == GL_PROXY_TEXTURE_2D || target == GL_TEXTURE_2D) {
		if (!isLegalSize(g, width - 2 * border, g->limits.maxTextureSize) ||
				!isLegalSize(g, height - 2 * border, g->limits.maxTextureSize)) {
			if (!IsProxyTarget(target))
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
										 "glTexImage2D(width=%d, height=%d)", width, height);
			return GL_TRUE;
		}
	}
	else if (target == GL_PROXY_TEXTURE_RECTANGLE_NV ||
					 target == GL_TEXTURE_RECTANGLE_NV) {
		if (width < 0 || width > (int) g->limits.maxRectTextureSize ||
				height < 0 || height > (int) g->limits.maxRectTextureSize) {
			if (!IsProxyTarget(target))
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
										 "glTexImage2D(width=%d, height=%d)", width, height);
			return GL_TRUE;
		}
	}
	else {
		/* cube map */
		if (!isLegalSize(g, width - 2*border, g->limits.maxCubeMapTextureSize) ||
				!isLegalSize(g, height - 2*border, g->limits.maxCubeMapTextureSize) ||
				width != height) {
			if (!IsProxyTarget(target))
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
										 "glTexImage2D(width=%d, height=%d)", width, height);
			return GL_TRUE;
		}
	}

	/* OK, no errors */
	return GL_FALSE;
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
	const int is_distrib = ((type == GL_TRUE) || (type == GL_FALSE));
	CRTextureUnit *unit;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "glTexImage2D called in Begin/End");
		return;
	}

	FLUSH();

	/* NOTE: we skip parameter error checking if this is a distributed
	 * texture!  The user better provide correct parameters!!!
	 */
	if (!is_distrib
			&& ErrorCheckTexImage2D(target, level, width, height, border)) {
		if (IsProxyTarget(target)) {
			/* clear all state, but don't generate error */
			crStateTextureInitTextureObj(g, &(t->proxy2D), 0, GL_TEXTURE_2D);
		}
		else {
			/* error was already recorded */
		}
		return;
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
#ifdef CR_NV_texture_rectangle 
	else if (target == GL_PROXY_TEXTURE_RECTANGLE_NV)
	{
		tobj = &(t->proxyRect);
		tl = tobj->level + level;
		tl->bytes = 0;
	}
	else if (target == GL_TEXTURE_RECTANGLE_NV)
	{
		tobj = unit->currentTextureRect;
		tl = tobj->level + level;
		tl->bytes = crImageSize(format, type, width, height);
	}
#endif
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
	DIRTY(tobj->imageBit, g->neg_bitid);
	DIRTY(tl->dirty, g->neg_bitid);
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
	DIRTY(tobj->imageBit, g->neg_bitid);
	DIRTY(tl->dirty, g->neg_bitid);
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
	CRTextureObj *tobj;
	CRTextureLevel *tl;
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
		tobj = unit->currentTexture2D;
	}
#ifdef CR_NV_texture_rectangle
	else if (target == GL_TEXTURE_RECTANGLE_NV
					 && g->extensions.NV_texture_rectangle)
	{
		if (level < 0 || level > t->maxRectLevel)
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glTexSubImage2D level oob: %d", level);
			return;
		}
		if (width < 0 || width > (int) g->limits.maxRectTextureSize)
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glTexSubImage2D width oob: %d", width);
			return;
		}
		if (height < 0 || height > (int) g->limits.maxRectTextureSize)
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glTexSubImage2D height oob: %d", height);
			return;
		}
		tobj = unit->currentTextureRect;
	}
#endif
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
		tobj = unit->currentTextureCubeMap;
	}
#endif /* CR_ARB_texture_cube_map */
	else
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
								 "glTexSubImage2D target != GL_TEXTURE_2D: %d", target);
		return;
	}

	tl = tobj->level + level;

	if (width + xoffset > tl->width)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
								 "glTexSubImage2D(width %d + xoffset %d > %d)",
								 width, xoffset, tl->width);
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
	DIRTY(tobj->imageBit, g->neg_bitid);
	DIRTY(tl->dirty, g->neg_bitid);
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
	DIRTY(tobj->imageBit, g->neg_bitid);
	DIRTY(tl->dirty, g->neg_bitid);
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
			crStateTextureInitTextureObj(g, &(t->proxy3D), 0, GL_TEXTURE_3D);
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
			crStateTextureInitTextureObj(g, &(t->proxy3D), 0, GL_TEXTURE_3D);
		}
		else
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glTexImage3D border oob: %d", border);
		}
		return;
	}

	/* check the bits in width */
	if (!isLegalSize(g, width - 2 * border, g->limits.max3DTextureSize) ||
			!isLegalSize(g, height - 2 * border, g->limits.max3DTextureSize) ||
			!isLegalSize(g, depth - 2 * border, g->limits.max3DTextureSize)) {
		if (target == GL_PROXY_TEXTURE_3D) {
			/* clear all the texture object state */
			crStateTextureInitTextureObj(g, &(t->proxy3D), 0, GL_TEXTURE_3D);
		}
		else {
			crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "glTexImage3D(invalide size %d x %d x %d)",
									 width, height, depth);
		}
		return;
	}

	if (target == GL_TEXTURE_3D)
	{
		tobj = unit->currentTexture3D;
		tl = tobj->level + level;
		tl->bytes = crTextureSize(format, type, width, height, depth);
	}
	else if (target == GL_PROXY_TEXTURE_3D)
	{
		tobj = &(t->proxy3D);
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
	DIRTY(tobj->imageBit, g->neg_bitid);
	DIRTY(tl->dirty, g->neg_bitid);
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
#ifdef CR_NV_texture_rectangle 
	else if (target == GL_TEXTURE_RECTANGLE_NV)
	{
		tobj = unit->currentTextureRect;
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
