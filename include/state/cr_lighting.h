/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_LIGHTING_H
#define CR_STATE_LIGHTING_H

#include "cr_glwrapper.h"
#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CR_NUM_LIGHTS 8

typedef struct {
	GLbitvalue dirty;
	GLbitvalue enable;
	GLbitvalue ambient;
	GLbitvalue diffuse;
	GLbitvalue specular;
	GLbitvalue position;
	GLbitvalue attenuation;
	GLbitvalue spot;
} CRLightBits;

typedef struct {
	GLbitvalue dirty;
	GLbitvalue shadeModel;
	GLbitvalue colorMaterial;
	GLbitvalue lightModel;
	GLbitvalue material;
	GLbitvalue enable;
	CRLightBits *light;
} CRLightingBits;

typedef struct {
	GLboolean	enable;
	GLcolorf	ambient;
	GLcolorf	diffuse;
	GLcolorf	specular;
	GLvectorf	position;
	GLvectorf	objPosition;
	GLfloat		constantAttenuation;
	GLfloat		linearAttenuation;
	GLfloat		quadraticAttenuation;
	GLvectorf	spotDirection;
	GLfloat 	spotExponent;
	GLfloat		spotCutoff;
} CRLight;

typedef struct {
	GLboolean	lighting;
	GLboolean	colorMaterial;
	GLenum		shadeModel;
	GLenum		colorMaterialMode;
	GLenum		colorMaterialFace;
	GLcolorf	ambient[2];
	GLcolorf	diffuse[2];
	GLcolorf	specular[2];
	GLcolorf	emission[2];
	GLfloat		shininess[2];
	GLcolorf	lightModelAmbient;
	GLboolean	lightModelLocalViewer;
	GLboolean	lightModelTwoSide;
	CRLight		*light;
} CRLightingState;

void crStateLightingInitBits (CRLightingBits *l);
void crStateLightingInit (CRLightingState *l);

void crStateLightingDiff(CRLightingBits *bb, GLbitvalue bitID, 
		CRLightingState *from, CRLightingState *to);
void crStateLightingSwitch(CRLightingBits *bb, GLbitvalue bitID, 
		CRLightingState *from, CRLightingState *to);

void crStateColorMaterialRecover( void );

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_LIGHTING_H */
