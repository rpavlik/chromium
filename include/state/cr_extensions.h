/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_EXTENSIONS_H
#define CR_STATE_EXTENSIONS_H

#include "cr_glwrapper.h"
#include "state/cr_statetypes.h"

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
