/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "cr_glstate.h"
#include "state/cr_statetypes.h"
#include "state_internals.h"
#include "state_extensionfuncs.h"

void crStateFogInit (CRFogState *f) 
{
	GLcolorf black = {0.0f, 0.0f, 0.0f, 0.0f};
	f->color = black;

	f->density = 1.0f;
	f->end = 1.0f;
	f->start = 0.0f;
	f->mode = GL_EXP;
	f->index = 0;
	f->enable = GL_FALSE;

	crStateFogInitExtensions( f );
}

void STATE_APIENTRY crStateFogf(GLenum pname, GLfloat param) 
{
	crStateFogfv( pname, &param );
}

void STATE_APIENTRY crStateFogi(GLenum pname, GLint param) 
{
	GLfloat f_param = (GLfloat) param;
	crStateFogfv( pname, &f_param );
}

void STATE_APIENTRY crStateFogiv(GLenum pname, const GLint *param) 
{
	GLcolor f_color;
	GLfloat f_param;
	switch (pname) 
	{
		case GL_FOG_MODE:
		case GL_FOG_DENSITY:
		case GL_FOG_START:
		case GL_FOG_END:
		case GL_FOG_INDEX:
			f_param = (GLfloat) (*param);
			crStateFogfv( pname, &f_param );
			break;
		case GL_FOG_COLOR:
			f_color.r = ((GLfloat) param[0]) / ((GLfloat) GL_MAXINT);
			f_color.g = ((GLfloat) param[1]) / ((GLfloat) GL_MAXINT);
			f_color.b = ((GLfloat) param[2]) / ((GLfloat) GL_MAXINT);
			f_color.a = ((GLfloat) param[3]) / ((GLfloat) GL_MAXINT);
			crStateFogfv( pname, (GLfloat *) &f_color );
			break;
		default:
			if (!crStateFogivExtensions( pname, param ))
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Invalid glFog Param: %d", param);
			}
			return;
	}
}

void STATE_APIENTRY crStateFogfv(GLenum pname, const GLfloat *param) 
{
	CRContext *g = GetCurrentContext();
	CRFogState *f = &(g->fog);
	CRStateBits *sb = GetCurrentBits();
	CRFogBits *fb = &(sb->fog);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glFogfv called in Begin/End");
		return;
	}

	FLUSH();

	switch (pname) 
	{
		case GL_FOG_MODE:
			{
				GLenum e = (GLenum) *param;
				if (e != GL_LINEAR && e != GL_EXP && e != GL_EXP2)
				{
					crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "Invalid param for glFog: %d", e);
					return;
				}
				f->mode = e;
				fb->mode = g->neg_bitid;
			}
			break;
		case GL_FOG_DENSITY:
			f->density = *param;
			if (f->density < 0.0f)
			{
				f->density = 0.0f;
			}
			fb->density = g->neg_bitid;
			break;
		case GL_FOG_START:
			f->start = *param;
			fb->start = g->neg_bitid;
			break;
		case GL_FOG_END:
			f->end = *param;
			fb->end = g->neg_bitid;
			break;
		case GL_FOG_INDEX:
			f->index = (GLint) *param;
			fb->index = g->neg_bitid;
			break;
		case GL_FOG_COLOR:
			f->color.r = param[0];
			f->color.g = param[1];
			f->color.b = param[2];
			f->color.a = param[3];
			fb->color = g->neg_bitid;
			break;
		default:
			if (!crStateFogfvExtensions( f, pname, param ))
			{
				crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Invalid glFog Param: %d", param);
			}
			fb->extensions = g->neg_bitid;
			break;
	}
	fb->dirty = g->neg_bitid;
}
