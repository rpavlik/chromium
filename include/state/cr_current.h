/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_CURRENT_H
#define CR_STATE_CURRENT_H

#include "state/cr_currentpointers.h"
#include "cr_glwrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLbitvalue  dirty;
	GLbitvalue  enable;
	GLbitvalue  color;
	GLbitvalue	secondaryColor;
	GLbitvalue  index;
	GLbitvalue  texCoord[CR_MAX_TEXTURE_UNITS];
	GLbitvalue  normal;
	GLbitvalue  raster;
	GLbitvalue  edgeFlag;
} CRCurrentBits; 

typedef struct {
	GLvectorf	pos;
	GLcolorf	color;
	GLcolorf	secondaryColor;
	GLtexcoordf	texCoord[CR_MAX_TEXTURE_UNITS];
	GLvectorf	normal;
	GLboolean	edgeFlag;
	GLfloat		index;
} CRVertex;


typedef struct {
	GLcolorf     color;
	GLcolorf     secondaryColor;
	GLfloat      index;
	GLtexcoordf  texCoord[CR_MAX_TEXTURE_UNITS];
	GLvectorf    normal;
	GLboolean    edgeFlag;

	GLcolorf     colorPre;
	GLcolorf     secondaryColorPre;
	GLfloat      indexPre;
	GLtexcoordf  texCoordPre[CR_MAX_TEXTURE_UNITS];
	GLvectorf    normalPre;
	GLboolean    edgeFlagPre;

	CRCurrentStatePointers   *current;

	GLvectorf    rasterPos;
	GLvectorf    rasterPosPre;

	GLfloat      rasterDistance;
	GLcolorf     rasterColor;
	GLcolorf     rasterSecondaryColor;
	GLtexcoordf  rasterTexture;
	GLdouble     rasterIndex;
	GLboolean    rasterValid;

	GLboolean    normalize;

	GLboolean    inBeginEnd;
	GLenum       mode;
	GLuint       beginEndMax;
	GLuint       beginEndNum;
	GLuint       flushOnEnd;

} CRCurrentState;

void crStateCurrentInit( CRCurrentState *current );
void crStateCurrentInitBits( CRCurrentBits *currentbits );

void crStateCurrentRecover( void );

void crStateCurrentDiff(CRCurrentBits *bb, GLbitvalue bitID, 
		CRCurrentState *from, CRCurrentState *to);
void crStateCurrentSwitch(CRCurrentBits *bb, GLbitvalue bitID, 
		CRCurrentState *from, CRCurrentState *to);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_CURRENT_H */
