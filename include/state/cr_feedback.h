/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_FEEDBACK_H 
#define CR_STATE_FEEDBACK_H 

#include "state/cr_statetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NAME_STACK_DEPTH 64

typedef struct {
	GLbitvalue dirty[CR_MAX_BITARRAY];
} CRFeedbackBits;

typedef struct {
	GLbitvalue dirty[CR_MAX_BITARRAY];
} CRSelectionBits;

typedef struct {
	GLenum	type;
	GLuint	mask;
	GLfloat	*buffer;
	GLuint	bufferSize;
	GLuint	count;

	GLvectorf texcoord;
	GLvectorf oldtexcoord;
	GLfloat color[4];
	GLvectorf vertex;
	GLvectorf rasterpos;
	GLfloat index; /* don't support it Chromium, but.... */

	/* For LineLoops */
	GLuint polygoni;
	GLuint starti;
	GLvectorf startv;
	GLvectorf startt;
} CRFeedbackState;

typedef struct {
   	GLuint *buffer;
   	GLuint bufferSize;
   	GLuint bufferCount;
   	GLuint hits;
   	GLuint nameStackDepth;
   	GLuint nameStack[MAX_NAME_STACK_DEPTH];
   	GLboolean hitFlag;
   	GLfloat hitMinZ, hitMaxZ;
} CRSelectionState;

void crStateFeedbackDiff(CRFeedbackBits *bb, GLbitvalue *bitID, 
		CRFeedbackState *from, CRFeedbackState *to);
void crStateFeedbackSwitch(CRFeedbackBits *bb, GLbitvalue *bitID, 
		CRFeedbackState *from, CRFeedbackState *to);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_FEEDBACK_H */
