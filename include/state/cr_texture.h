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

typedef struct {
	GLubyte *img;
	int bytes;
	GLint width;
	GLint height;
	GLint depth;
	GLint components;
	GLint border;
	GLenum format;
	GLenum type;
	int	bytesPerPixel;

	GLbitvalue dirty[CR_MAX_TEXTURE_UNITS];
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
	
	GLcolorf	             borderColor;
	GLenum		             target;
	GLuint		             name;
	struct __CRTextureObj *next;
	GLenum		             minFilter, magFilter;
	GLenum		             wrapS, wrapT;
#ifdef CR_OPENGL_VERSION_1_2
	GLenum		             wrapR;
#endif

	GLbitvalue	           dirty;
	GLbitvalue             paramsBit[CR_MAX_TEXTURE_UNITS];
	GLbitvalue             imageBit[CR_MAX_TEXTURE_UNITS];
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
	GLbitvalue dirty;
	GLbitvalue enable[CR_MAX_TEXTURE_UNITS];
	GLbitvalue current[CR_MAX_TEXTURE_UNITS];
	GLbitvalue objGen[CR_MAX_TEXTURE_UNITS];
	GLbitvalue eyeGen[CR_MAX_TEXTURE_UNITS];
	GLbitvalue envBit[CR_MAX_TEXTURE_UNITS];
	GLbitvalue gen[CR_MAX_TEXTURE_UNITS];
} CRTextureBits;

typedef struct {
	GLuint        currentTexture1DName;
	GLuint        currentTexture2DName;
	GLuint        currentTexture3DName;
#ifdef CR_ARB_texture_cube_map
	GLuint	      currentTextureCubeMapName;
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
} CRTextureUnit;

typedef struct {
	GLuint allocated;
	CRTextureObj *textures;
	CRTextureObj *firstFree;

	// FIXME: these should be moved into a shared-context structure
	CRTextureObj      *mapping[CRTEXTURE_HASHSIZE];
	CRTextureFreeElem *freeList;
	CRTextureID       *hwidhash[CRTEXTURE_HASHSIZE];

	// Current texture objects (in terms of glBindTexture and glActiveTexture)
	CRTextureObj *currentTexture1D;
	CRTextureObj *currentTexture2D;
	CRTextureObj *currentTexture3D;
#ifdef CR_ARB_texture_cube_map
	CRTextureObj *currentTextureCubeMap;
#endif

	// Default texture objects (name = 0)
	CRTextureObj base1D;
	CRTextureObj base2D;
	CRTextureObj base3D;
#ifdef CR_ARB_texture_cube_map
	CRTextureObj baseCubeMap;
#endif
	
	// Proxy texture objects
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

	// Per-texture unit state:
	CRTextureUnit	unit[CR_MAX_TEXTURE_UNITS];
} CRTextureState;

void crStateTextureInitBits (CRTextureBits *t);
void crStateTextureInit(const CRLimitsState *limits, CRTextureState *t);

void crStateTextureInitTexture(GLuint name);
CRTextureObj *crStateTextureAllocate(GLuint name);
void crStateTextureDelete(GLuint name);
CRTextureObj *crStateTextureGet(GLuint textureid);
int crStateTextureGetSize(GLenum target, GLenum level);
const GLvoid * crStateTextureGetData(GLenum target, GLenum level);

void crStateTextureDiff(CRContext *g, CRTextureBits *bb, GLbitvalue bitID, 
		CRTextureState *from, CRTextureState *to);
void crStateTextureSwitch(CRContext *g, CRTextureBits *bb, GLbitvalue bitID, 
		CRTextureState *from, CRTextureState *to);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_TEXTURE_H */
