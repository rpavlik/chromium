/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_TEXTURE_H
#define CR_STATE_TEXTURE_H

#include "cr_glstate.h"
#include "state/cr_statetypes.h"
#include "state/cr_extensions.h"

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
	
	GLcolorf	             borderColor;
	GLenum		             target;
	GLuint		             name;
	struct __CRTextureObj *next;
	GLenum		             minFilter, magFilter;
	GLenum		             wrapS, wrapT;

	GLbitvalue	           dirty;
	GLbitvalue             paramsBit[CR_MAX_TEXTURE_UNITS];
	GLbitvalue             imageBit[CR_MAX_TEXTURE_UNITS];
	CRTextureObjExtensions extensions;
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
	GLuint allocated;
	CRTextureObj *textures;

	CRTextureObj *firstFree;
	CRTextureObj *currentTexture1D;
	CRTextureObj *currentTexture2D;
	CRTextureObj *currentTexture3D;
	CRTextureObj *currentTextureCubeMap;
	GLuint        currentTexture1DName[CR_MAX_TEXTURE_UNITS];
	GLuint        currentTexture2DName[CR_MAX_TEXTURE_UNITS];
	GLuint        currentTexture3DName[CR_MAX_TEXTURE_UNITS];
	GLuint	      currentTextureCubeMapName[CR_MAX_TEXTURE_UNITS];

	GLint		curTextureUnit;

	GLenum		envMode[CR_MAX_TEXTURE_UNITS];
	GLcolorf	envColor[CR_MAX_TEXTURE_UNITS];
	
	GLboolean	enabled1D[CR_MAX_TEXTURE_UNITS];
	GLboolean	enabled2D[CR_MAX_TEXTURE_UNITS];
	GLboolean	enabled3D[CR_MAX_TEXTURE_UNITS];
	GLboolean	enabledCubeMap[CR_MAX_TEXTURE_UNITS];
	GLtexcoordb	textureGen[CR_MAX_TEXTURE_UNITS];

	CRTextureObj      *mapping[CRTEXTURE_HASHSIZE];
	CRTextureFreeElem *freeList;
	CRTextureID       *hwidhash[CRTEXTURE_HASHSIZE];

	CRTextureObj base1D;
	CRTextureObj base2D;
	CRTextureObj base3D;
	CRTextureObj baseCubeMap;
	
	GLint		maxTextureUnitsARB;
	GLint		maxTextureSize;
	GLint		max3DTextureSize;
	GLint		maxCubeMapTextureSize;
	GLint		maxLevel;
	GLint		max3DLevel;
	GLint		maxCubeMapLevel;
	
	GLboolean	broadcastTextures;

	/* Texture gen mode */
	GLvectorf	objSCoeff[CR_MAX_TEXTURE_UNITS];
	GLvectorf	objTCoeff[CR_MAX_TEXTURE_UNITS];
	GLvectorf	objRCoeff[CR_MAX_TEXTURE_UNITS];
	GLvectorf	objQCoeff[CR_MAX_TEXTURE_UNITS];
	GLvectorf	eyeSCoeff[CR_MAX_TEXTURE_UNITS];
	GLvectorf	eyeTCoeff[CR_MAX_TEXTURE_UNITS];
	GLvectorf	eyeRCoeff[CR_MAX_TEXTURE_UNITS];
	GLvectorf	eyeQCoeff[CR_MAX_TEXTURE_UNITS];
	GLtexcoorde	gen[CR_MAX_TEXTURE_UNITS];

	CRTextureStateExtensions extensions;
} CRTextureState;

void crStateTextureInitBits (CRTextureBits *t);
void crStateTextureInit(CRTextureState *t);

void crStateTextureInitTexture(GLuint name);
CRTextureObj *crStateTextureAllocate(GLuint name);
void crStateTextureDelete(GLuint name);
CRTextureObj *crStateTextureGet(GLuint textureid);
int crStateTextureGetSize(GLenum target, GLenum level);
const GLvoid * crStateTextureGetData(GLenum target, GLenum level);

void crStateTextureDiff(CRTextureBits *bb, GLbitvalue bitID, 
		CRTextureState *from, CRTextureState *to);
void crStateTextureSwitch(CRTextureBits *bb, GLbitvalue bitID, 
		CRTextureState *from, CRTextureState *to);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_TEXTURE_H */
