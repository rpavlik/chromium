/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_ATTRIB_H 
#define CR_STATE_ATTRIB_H 

#include "cr_glwrapper.h"
#include "state/cr_statetypes.h"
#include "state/cr_extensions.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CR_MAX_ATTRIB_STACK_DEPTH 1024

typedef struct {
	GLbitvalue dirty;
} CRAttribBits;

typedef struct {
	GLint attribStackDepth;
	GLint maxAttribStackDepth;

	GLbitvalue pushMaskStack[CR_MAX_ATTRIB_STACK_DEPTH];
} CRAttribState;

void crStateAttribInit(CRAttribState *a);

// No diff!
void crStateAttribSwitch(CRAttribBits *bb, GLbitvalue bitID, 
		CRAttribState *from, CRAttribState *to);

#ifdef __cplusplus
}
#endif

#endif /* CR_STATE_ATTRIB_H */
