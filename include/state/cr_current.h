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
	CRbitvalue  dirty[CR_MAX_BITARRAY];
	CRbitvalue  enable[CR_MAX_BITARRAY];
	CRbitvalue  color[CR_MAX_BITARRAY];
	CRbitvalue  secondaryColor[CR_MAX_BITARRAY];
	CRbitvalue  index[CR_MAX_BITARRAY];
	CRbitvalue  texCoord[CR_MAX_TEXTURE_UNITS][CR_MAX_BITARRAY];
	CRbitvalue  normal[CR_MAX_BITARRAY];
	CRbitvalue  raster[CR_MAX_BITARRAY];
	CRbitvalue  edgeFlag[CR_MAX_BITARRAY];
#ifdef CR_EXT_fog_coord
	CRbitvalue  fogCoord[CR_MAX_BITARRAY];
#endif
	CRbitvalue  vertexAttrib[CR_MAX_VERTEX_ATTRIBS][CR_MAX_BITARRAY];
} CRCurrentBits;

typedef struct {
	/* Pre-transform values */
	GLvectorf	pos;
	GLcolorf	color;
	GLcolorf	secondaryColor;
	GLtexcoordf	texCoord[CR_MAX_TEXTURE_UNITS];
	GLvectorf	normal;
	GLboolean	edgeFlag;
	GLfloat		index;
#ifdef CR_EXT_fog_coord
	GLfloat		fogCoord;
#endif
	/* Post-transform values */
	GLvectorf	eyePos;
	GLvectorf	clipPos;
	GLvectorf	winPos;
} CRVertex;


/*
 * XXX NV vertex attribs should alias conventional attribs.
 */
typedef struct {
	/* XXX use a CRVertex for this state */
	GLcolorf     color;
	GLcolorf     secondaryColor;
	GLfloat      index;
	GLtexcoordf  texCoord[CR_MAX_TEXTURE_UNITS];
	GLvectorf    normal;
	GLboolean    edgeFlag;
	GLvectorf    vertexAttrib[CR_MAX_VERTEX_ATTRIBS];
#ifdef CR_EXT_fog_coord
	GLfloat      fogCoord;
#endif

	/* XXX use a CRVertex for this state */
	GLcolorf     colorPre;
	GLcolorf     secondaryColorPre;
	GLfloat      indexPre;
	GLtexcoordf  texCoordPre[CR_MAX_TEXTURE_UNITS];
	GLvectorf    normalPre;
	GLboolean    edgeFlagPre;
	GLvectorf    vertexAttribPre[CR_MAX_VERTEX_ATTRIBS];
#ifdef CR_EXT_fog_coord
	GLfloat      fogCoordPre;
#endif

	CRCurrentStatePointers   *current;

	GLvectorf    rasterOrigin;   /* needed for tilesort support */

	/* XXX use a CRVertex for this state */
	GLvectorf    rasterPos;
	GLfloat      rasterDistance; /* aka fog coord */
	GLcolorf     rasterColor;
	GLcolorf     rasterSecondaryColor;
	GLtexcoordf  rasterTexture;
	GLdouble     rasterIndex;
	GLboolean    rasterValid;

	/* glBegin/End state */
	GLboolean    inBeginEnd;
	GLenum       mode;
	GLuint       beginEndMax;
	GLuint       beginEndNum;
	GLuint       flushOnEnd;

} CRCurrentState;

void crStateCurrentInit( CRContext *ctx );

void crStateCurrentRecover( void );

void crStateCurrentDiff(CRCurrentBits *bb, CRbitvalue *bitID,
		CRCurrentState *from, CRCurrentState *to);
void crStateCurrentSwitch(GLuint maxTexUnits,
		CRCurrentBits *bb, CRbitvalue *bitID,
		CRCurrentState *from, CRCurrentState *to);

void crStateRasterPosUpdate(GLfloat x, GLfloat y, GLfloat z, GLfloat w);


#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_CURRENT_H */
