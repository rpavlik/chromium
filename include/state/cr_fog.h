/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_FOG_H 
#define CR_STATE_FOG_H 

#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	GLbitvalue dirty[CR_MAX_BITARRAY];
	GLbitvalue color[CR_MAX_BITARRAY];
	GLbitvalue index[CR_MAX_BITARRAY];
	GLbitvalue density[CR_MAX_BITARRAY];
	GLbitvalue start[CR_MAX_BITARRAY];
	GLbitvalue end[CR_MAX_BITARRAY];
	GLbitvalue mode[CR_MAX_BITARRAY];
	GLbitvalue enable[CR_MAX_BITARRAY];
#ifdef CR_NV_fog_distance
	GLbitvalue fogDistanceMode[CR_MAX_BITARRAY];
#endif
#ifdef CR_EXT_fog_coord
	GLbitvalue fogCoordinateSource[CR_MAX_BITARRAY];
#endif
} CRFogBits;

typedef struct {
	GLcolorf  color;
	GLint     index;
	GLfloat   density;
	GLfloat   start;
	GLfloat   end;
	GLint     mode;
	GLboolean enable;
#ifdef CR_NV_fog_distance
	GLenum fogDistanceMode;
#endif
#ifdef CR_EXT_fog_coord
	GLenum fogCoordinateSource;
#endif
} CRFogState;

void crStateFogInitBits (CRFogBits *fb);
void crStateFogInit(CRFogState *f);

void crStateFogDiff(CRFogBits *bb, GLbitvalue *bitID, 
		CRFogState *from, CRFogState *to);
void crStateFogSwitch(CRFogBits *bb, GLbitvalue *bitID, 
		CRFogState *from, CRFogState *to);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_FOG_H */
