/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdlib.h>
#include <stdio.h>
#include "state.h"
#include "state/cr_statetypes.h"
#include "state_internals.h"

void crStatePointInit (CRPointState *l) {
	l->pointSmooth = GL_FALSE;
	l->pointSize = 1.0f;
#ifdef CR_ARB_point_parameters
	l->minSize = 0.0f;
	l->maxSize = CR_ALIASED_POINT_SIZE_MAX;
	l->fadeThresholdSize = 1.0f;
	l->distanceAttenuation[0] = 1.0f;
	l->distanceAttenuation[1] = 0.0f;
	l->distanceAttenuation[2] = 0.0f;
#endif

	/*
	 *l->aliasedpointsizerange_min = c->aliasedpointsizerange_min; 
	 *l->aliasedpointsizerange_max = c->aliasedpointsizerange_max; 
	 *l->aliasedpointsizegranularity = c->aliasedpointsizegranularity; 
	 *l->smoothpointsizerange_min = c->smoothpointsizerange_min; 
	 *l->smoothpointsizerange_max = c->smoothpointsizerange_max; 
	 *l->smoothpointgranularity = c->smoothpointgranularity;
	 */
}

void STATE_APIENTRY crStatePointSize(GLfloat size) 
{
	CRContext *g = GetCurrentContext();
	CRPointState *l = &(g->point);
	CRStateBits *sb = GetCurrentBits();
	CRPointBits *lb = &(sb->point);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glPointSize called in begin/end");
		return;
	}

	FLUSH();

	if (size <= 0.0f) 
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "glPointSize called with size <= 0.0: %f", size);
		return;
	}
		
	l->pointSize = size;
	DIRTY(lb->size, g->neg_bitid);
	DIRTY(lb->dirty, g->neg_bitid);
}


#ifdef CR_ARB_point_parameters
void STATE_APIENTRY crStatePointParameterfARB(GLenum pname, GLfloat param)
{


}


void STATE_APIENTRY crStatePointParameterfvARB(GLenum pname, const GLfloat *params)
{

}
#endif
