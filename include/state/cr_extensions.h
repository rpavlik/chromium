#ifndef CR_STATE_EXTENSIONS_H
#define CR_STATE_EXTENSIONS_H

typedef struct {
	GLfloat maxTextureMaxAnisotropy;
} CRTextureStateExtensions;

typedef struct {
	GLfloat maxAnisotropy;
} CRTextureObjExtensions;

typedef struct {
	GLenum fogDistanceMode;
} CRFogStateExtensions;

typedef struct {
	GLcolorf blendColor;
	GLenum blendEquation;
} CRBufferStateExtensions;

#endif /* CR_STATE_EXTENSIONS_H */
