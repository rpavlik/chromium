/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_CURRENT_H
#define CR_STATE_CURRENT_H

#include "state/cr_currentpointers.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLbitvalue  dirty[CR_MAX_BITARRAY];
	GLbitvalue  enable[CR_MAX_BITARRAY];
	GLbitvalue  color[CR_MAX_BITARRAY];
	GLbitvalue	secondaryColor[CR_MAX_BITARRAY];
	GLbitvalue  index[CR_MAX_BITARRAY];
	GLbitvalue  texCoord[CR_MAX_TEXTURE_UNITS][CR_MAX_BITARRAY];
	GLbitvalue  normal[CR_MAX_BITARRAY];
	GLbitvalue  raster[CR_MAX_BITARRAY];
	GLbitvalue  edgeFlag[CR_MAX_BITARRAY];
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

	GLboolean    inBeginEnd;
	GLenum       mode;
	GLuint       beginEndMax;
	GLuint       beginEndNum;
	GLuint       flushOnEnd;

} CRCurrentState;

void crStateCurrentInit( CRLimitsState *limits, CRCurrentState *current );
void crStateCurrentInitBits( CRCurrentBits *currentbits );

void crStateCurrentRecover( void );

void crStateCurrentDiff(CRCurrentBits *bb, GLbitvalue *bitID,
		CRCurrentState *from, CRCurrentState *to);
void crStateCurrentSwitch(GLuint maxTexUnits,
		CRCurrentBits *bb, GLbitvalue *bitID,
		CRCurrentState *from, CRCurrentState *to);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_CURRENT_H */
