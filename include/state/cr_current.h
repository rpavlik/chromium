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
	GLbitvalue  texCoord;
	GLbitvalue  normal;
	GLbitvalue  raster;
	GLbitvalue  edgeFlag;
} CRCurrentBits; 

typedef struct {
	GLvectorf	pos;
	GLcolorf	color;
	GLtexcoordf	texCoord;
	GLvectorf	normal;
	GLboolean	edgeFlag;
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

#ifdef __cplusplusk
}
#endif

#endif /* CR_STATE_CURRENT_H */
