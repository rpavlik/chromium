#ifndef CR_STATE_EXTENSIONS_H
#define CR_STATE_EXTENSIONS_H

typedef struct {
	// Cube map is inside the normal texture data.
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
