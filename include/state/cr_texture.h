/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_TEXTURE_H
#define CR_STATE_TEXTURE_H

#include "state/cr_statetypes.h"
#include "state/cr_limits.h"

#define CRTEXTURE_HASHSIZE 1047
#define CRTEXTURE_NAMEOFFSET 4

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __CRTextureID {
	GLuint name;
	GLuint hwid;
	struct __CRTextureID *next;
} CRTextureID;

struct CRTextureFormat {
	GLubyte redbits;
	GLubyte greenbits;
	GLubyte bluebits;
	GLubyte alphabits;
	GLubyte luminancebits;
	GLubyte intensitybits;
	GLubyte indexbits;
};

typedef struct {
	GLubyte *img;
	int bytes;
	GLint width;
	GLint height;
	GLint depth;
	GLint internalFormat;
	GLint border;
	GLenum format;
	GLenum type;
	int	bytesPerPixel;

	const struct CRTextureFormat *texFormat;

	GLbitvalue dirty[CR_MAX_TEXTURE_UNITS][CR_MAX_BITARRAY];
} CRTextureLevel;

typedef struct __CRTextureObj {

	CRTextureLevel        *level;
#ifdef CR_ARB_texture_cube_map
	CRTextureLevel        *negativeXlevel;
	CRTextureLevel        *positiveYlevel;
	CRTextureLevel        *negativeYlevel;
	CRTextureLevel        *positiveZlevel;
	CRTextureLevel        *negativeZlevel;
#endif

	GLcolorf               borderColor;
	GLenum                 target;
	GLuint                 name;
	struct __CRTextureObj *next;
	GLenum                 minFilter, magFilter;
	GLenum                 wrapS, wrapT;
#ifdef CR_OPENGL_VERSION_1_2
	GLenum                 wrapR;
	GLfloat                priority;
	GLfloat                minLod;
	GLfloat                maxLod;
	GLint                  baseLevel;
	GLint                  maxLevel;
#endif

	GLbitvalue	           dirty[CR_MAX_BITARRAY];
	GLbitvalue             paramsBit[CR_MAX_TEXTURE_UNITS][CR_MAX_BITARRAY];
	GLbitvalue             imageBit[CR_MAX_TEXTURE_UNITS][CR_MAX_BITARRAY];
#ifdef CR_EXT_texture_filter_anisotropic
	GLfloat maxAnisotropy;
#endif
} CRTextureObj;

typedef struct __CRTextureFreeElem {
	GLuint min;
	GLuint max;
	struct __CRTextureFreeElem *next;
	struct __CRTextureFreeElem *prev;
} CRTextureFreeElem;

typedef struct {
	GLbitvalue dirty[CR_MAX_BITARRAY];
	GLbitvalue enable[CR_MAX_TEXTURE_UNITS][CR_MAX_BITARRAY];
	GLbitvalue current[CR_MAX_TEXTURE_UNITS][CR_MAX_BITARRAY];
	GLbitvalue objGen[CR_MAX_TEXTURE_UNITS][CR_MAX_BITARRAY];
	GLbitvalue eyeGen[CR_MAX_TEXTURE_UNITS][CR_MAX_BITARRAY];
	GLbitvalue envBit[CR_MAX_TEXTURE_UNITS][CR_MAX_BITARRAY];
	GLbitvalue gen[CR_MAX_TEXTURE_UNITS][CR_MAX_BITARRAY];
} CRTextureBits;

typedef struct {
	/* Current texture objects (in terms of glBindTexture and glActiveTexture) */
	CRTextureObj *currentTexture1D;
	CRTextureObj *currentTexture2D;
	CRTextureObj *currentTexture3D;
#ifdef CR_ARB_texture_cube_map
	CRTextureObj *currentTextureCubeMap;
#endif

	GLboolean	enabled1D;
	GLboolean	enabled2D;
	GLboolean	enabled3D;
#ifdef CR_ARB_texture_cube_map
	GLboolean	enabledCubeMap;
#endif

	GLenum		envMode;
	GLcolorf	envColor;
	
	GLtexcoordb	textureGen;
	GLvectorf	objSCoeff;
	GLvectorf	objTCoeff;
	GLvectorf	objRCoeff;
	GLvectorf	objQCoeff;
	GLvectorf	eyeSCoeff;
	GLvectorf	eyeTCoeff;
	GLvectorf	eyeRCoeff;
	GLvectorf	eyeQCoeff;
	GLtexcoorde	gen;

	/* These are only used for glPush/PopAttrib */
	CRTextureObj Saved1D;
	CRTextureObj Saved2D;
	CRTextureObj Saved3D;
#ifdef CR_ARB_texture_cube_map
	CRTextureObj SavedCubeMap;
#endif
} CRTextureUnit;

typedef struct {
	GLuint allocated;
	CRTextureObj *textures;
	CRTextureObj *firstFree;

	/* FIXME: these should be moved into a shared-context structure */
	CRTextureObj      *mapping[CRTEXTURE_HASHSIZE];
	CRTextureFreeElem *freeList;
	CRTextureID       *hwidhash[CRTEXTURE_HASHSIZE];

	/* Default texture objects (name = 0) */
	CRTextureObj base1D;
	CRTextureObj base2D;
	CRTextureObj base3D;
#ifdef CR_ARB_texture_cube_map
	CRTextureObj baseCubeMap;
#endif
	
	/* Proxy texture objects */
	CRTextureObj proxy1D;
	CRTextureObj proxy2D;
	CRTextureObj proxy3D;
#ifdef CR_ARB_texture_cube_map
	CRTextureObj proxyCubeMap;
#endif

	GLint		curTextureUnit;

	GLint		maxLevel;
	GLint		max3DLevel;
	GLint		maxCubeMapLevel;
	
	GLboolean	broadcastTextures;

	/* Per-texture unit state: */
	CRTextureUnit	unit[CR_MAX_TEXTURE_UNITS];
} CRTextureState;

void crStateTextureInitBits (CRTextureBits *t);
void crStateTextureInit(const CRLimitsState *limits, CRTextureState *t);

void crStateTextureInitTexture(GLuint name);
CRTextureObj *crStateTextureAllocate(GLuint name);
void crStateTextureDelete(GLuint name);
CRTextureObj *crStateTextureGet(GLenum target, GLuint textureid);
int crStateTextureGetSize(GLenum target, GLenum level);
const GLvoid * crStateTextureGetData(GLenum target, GLenum level);

void crStateTextureDiff(CRContext *g, CRTextureBits *bb, GLbitvalue *bitID, 
		CRTextureState *from, CRTextureState *to);
void crStateTextureSwitch(CRContext *g, CRTextureBits *bb, GLbitvalue *bitID, 
		CRTextureState *from, CRTextureState *to);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_TEXTURE_H */
