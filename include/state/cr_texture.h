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

	GLbitvalue dirty;
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
	GLbitvalue             paramsBit;
	GLbitvalue             imageBit;
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
	GLbitvalue enable;
	GLbitvalue current;
	GLbitvalue objGen;
	GLbitvalue eyeGen;
	GLbitvalue envBit;
	GLbitvalue gen;
} CRTextureBits;

typedef struct {
	GLuint allocated;
	CRTextureObj *textures;

	CRTextureObj *firstFree;
	CRTextureObj *currentTexture1D;
	CRTextureObj *currentTexture2D;
	CRTextureObj *currentTexture3D;
	GLuint        currentTexture1DName;
	GLuint        currentTexture2DName;
	GLuint        currentTexture3DName;

	GLenum		    envMode;
	GLcolorf	    envColor;
	
	GLboolean	    enabled1D;
	GLboolean	    enabled2D;
	GLboolean	    enabled3D;
	GLtexcoordb	  textureGen;

	CRTextureObj      *mapping[CRTEXTURE_HASHSIZE];
	CRTextureFreeElem *freeList;
	CRTextureID       *hwidhash[CRTEXTURE_HASHSIZE];

	CRTextureObj base1D;
	CRTextureObj base2D;
	CRTextureObj base3D;
	
	GLint		maxTextureSize;
	GLint		max3DTextureSize;
	GLint		maxLevel;
	GLint		max3DLevel;

	GLboolean	broadcastTextures;

	/* Texture gen mode */
	GLvectorf	objSCoeff;
	GLvectorf	objTCoeff;
	GLvectorf	objRCoeff;
	GLvectorf	objQCoeff;
	GLvectorf	eyeSCoeff;
	GLvectorf	eyeTCoeff;
	GLvectorf	eyeRCoeff;
	GLvectorf	eyeQCoeff;
	GLtexcoorde	gen;
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
