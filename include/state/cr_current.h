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
	GLbitvalue  index;
	GLbitvalue  texcoord;
	GLbitvalue  normal;
	GLbitvalue  raster;
	GLbitvalue  edgeflag;
} CRCurrentBits; 

typedef struct {
	GLvectorf	pos;
	GLcolorf	color;
	GLtexcoordf	texcoord;
	GLvectorf	normal;
	GLboolean	edgeflag;
	GLfloat		index;
} CRVertex;


typedef struct {
	GLcolorf     color;
	GLfloat      index;
	GLtexcoordf  texCoord;
	GLvectorf    normal;
	GLboolean    edgeFlag;

	GLcolorf     colorPre;
	GLfloat      indexPre;
	GLtexcoordf  texCoordPre;
	GLvectorf    normalPre;
	GLboolean    edgeFlagPre;

	CRCurrentStatePointers   *current;

	GLvectorf    rasterPos;
	GLvectorf    rasterPosPre;

	GLfloat      rasterDistance;
	GLcolorf     rasterColor;
	GLtexcoordf  rasterTexture;
	GLdouble     rasterIndex;
	GLboolean    rasterValid;

	GLboolean    normalize;

	GLboolean    inBeginEnd;
	GLenum       mode;
	GLboolean    isLoop;

	GLuint       beginEndMax;
	GLuint       beginEndNum;
	GLuint       flushOnEnd;

	GLint        numRestore;
	GLint        wind;
	CRVertex     vtx[3];
} CRCurrentState;

void crStateCurrentInit( CRCurrentState *current );
void crStateCurrentInitBits( CRCurrentBits *currentbits );

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_CURRENT_H */
