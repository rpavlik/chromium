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

void crStatePixelInit(CRPixelState *p) { 
	GLcolorf zero_color = {0.0f, 0.0f, 0.0f, 0.0f};
	GLcolorf one_color = {1.0f, 1.0f, 1.0f, 1.0f};

	p->mapColor           = GL_FALSE;
	p->mapStencil         = GL_FALSE;
	p->indexShift         = 0;
	p->indexOffset        = 0;
	p->scale              = one_color;
	p->depthScale         = 1.0f;
	p->bias               = zero_color;
	p->depthBias          = 0.0f;
	p->xZoom              = 1.0f;
	p->yZoom              = 1.0f;

	p->mapStoS[0] = 0;
	p->mapItoI[0] = 0;
	p->mapItoR[0] = 0.0;
	p->mapItoG[0] = 0.0;
	p->mapItoB[0] = 0.0;
	p->mapItoA[0] = 0.0;
	p->mapRtoR[0] = 0.0;
	p->mapGtoG[0] = 0.0;
	p->mapBtoB[0] = 0.0;
	p->mapAtoA[0] = 0.0;

	p->mapItoIsize   = 1;
	p->mapStoSsize   = 1;
	p->mapItoRsize   = 1;
	p->mapItoGsize   = 1;
	p->mapItoBsize   = 1;
	p->mapItoAsize   = 1;
	p->mapRtoRsize   = 1;
	p->mapGtoGsize   = 1;
	p->mapBtoBsize   = 1;
	p->mapAtoAsize   = 1;
}

void STATE_APIENTRY crStatePixelTransferi (GLenum pname, GLint param) {
	crStatePixelTransferf( pname, (GLfloat) param );
}

void STATE_APIENTRY crStatePixelTransferf (GLenum pname, GLfloat param) {
	CRContext *g    = GetCurrentContext();
	CRStateBits *sb = GetCurrentBits();
	CRPixelState *p = &(g->pixel);
	CRPixelBits *pb = &(sb->pixel);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "PixelTransfer{if} called in Begin/End");
		return;
	}

	FLUSH();

	switch( pname )
	{
		case GL_MAP_COLOR:
			p->mapColor = (GLboolean) ((param == 0.0f) ? GL_FALSE : GL_TRUE);
			break;
		case GL_MAP_STENCIL:
			p->mapStencil = (GLboolean) ((param == 0.0f) ? GL_FALSE : GL_TRUE);
			break;
		case GL_INDEX_SHIFT:
			p->indexShift = (GLint) param;
			break;
		case GL_INDEX_OFFSET:
			p->indexOffset = (GLint) param;
			break;
		case GL_RED_SCALE:
			p->scale.r = param;
			break;
		case GL_GREEN_SCALE:
			p->scale.g = param;
			break;
		case GL_BLUE_SCALE:
			p->scale.b = param;
			break;
		case GL_ALPHA_SCALE:
			p->scale.a = param;
			break;
		case GL_DEPTH_SCALE:
			p->depthScale = param;
			break;
		case GL_RED_BIAS:
			p->bias.r = param;
			break;
		case GL_GREEN_BIAS:
			p->bias.g = param;
			break;
		case GL_BLUE_BIAS:
			p->bias.b = param;
			break;
		case GL_ALPHA_BIAS:
			p->bias.a = param;
			break;
		case GL_DEPTH_BIAS:
			p->depthBias = param;
			break;
		default:
			crStateError( __LINE__, __FILE__, GL_INVALID_VALUE, "Unknown glPixelTransfer pname: %d", pname );
			return;
	}
	DIRTY(pb->transfer, g->neg_bitid);
	DIRTY(pb->dirty, g->neg_bitid);
}

void STATE_APIENTRY crStatePixelZoom (GLfloat xfactor, GLfloat yfactor) 
{
	CRContext *g    = GetCurrentContext();
	CRStateBits *sb = GetCurrentBits();
	CRPixelState *p = &(g->pixel);
	CRPixelBits *pb = &(sb->pixel);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "PixelZoom called in Begin/End");
		return;
	}

	FLUSH();

	p->xZoom = xfactor;
	p->yZoom = yfactor;
	DIRTY(pb->zoom, g->neg_bitid);
	DIRTY(pb->dirty, g->neg_bitid);
}


void STATE_APIENTRY crStateBitmap( GLsizei width, GLsizei height, 
		GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, 
		const GLubyte *bitmap)
{
	CRContext *g = GetCurrentContext();
	CRCurrentState *c = &(g->current);
	CRStateBits *s = GetCurrentBits();
	CRCurrentBits *cb = &(s->current);

	(void) xorig;
	(void) yorig;
	(void) bitmap;
	
	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, 
			"Bitmap called in begin/end");
		return;
	}

	if (width < 0 || height < 0)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
			"Bitmap called with neg dims: %dx%d", width, height);
		return;
	}

	if (!c->rasterValid)
	{
		return;
	}

	c->rasterPos.x += xmove;
	c->rasterPos.y += ymove;
	DIRTY(cb->raster, g->neg_bitid);
	DIRTY(cb->dirty, g->neg_bitid);

	c->rasterPosPre.x += xmove;
	c->rasterPosPre.y += ymove;
}
 

#define UNUSED(x) ((void)(x))

#define CLAMP(x, min, max)  ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

void STATE_APIENTRY crStatePixelMapfv (GLenum map, GLint mapsize, const GLfloat * values)
{
	CRContext *g = GetCurrentContext();
	CRStateBits *sb = GetCurrentBits();
	CRPixelState *p = &(g->pixel);
	CRPixelBits *pb = &(sb->pixel);
	GLint i;

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "PixelMap called in Begin/End");
		return;
	}

	FLUSH();

	if (mapsize < 0 || mapsize > CR_MAX_PIXEL_MAP_TABLE) {
	   crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "PixelMap(mapsize)");
	   return;
	}

	if (map >= GL_PIXEL_MAP_S_TO_S && map <= GL_PIXEL_MAP_I_TO_A) {
	   /* XXX check that mapsize is a power of two */
	}

	switch (map) {
	case GL_PIXEL_MAP_S_TO_S:
		p->mapStoSsize = mapsize;
		for (i=0;i<mapsize;i++) {
			p->mapStoS[i] = (GLint) values[i];
		}
		break;
	case GL_PIXEL_MAP_I_TO_I:
		p->mapItoIsize = mapsize;
		for (i=0;i<mapsize;i++) {
			p->mapItoI[i] = (GLint) values[i];
		}
		break;
	case GL_PIXEL_MAP_I_TO_R:
		p->mapItoRsize = mapsize;
		for (i=0;i<mapsize;i++) {
			GLfloat val = CLAMP( values[i], 0.0F, 1.0F );
			p->mapItoR[i] = val;
		}
		break;
	case GL_PIXEL_MAP_I_TO_G:
		p->mapItoGsize = mapsize;
		for (i=0;i<mapsize;i++) {
			GLfloat val = CLAMP( values[i], 0.0F, 1.0F );
			p->mapItoG[i] = val;
		}
		break;
	case GL_PIXEL_MAP_I_TO_B:
		p->mapItoBsize = mapsize;
		for (i=0;i<mapsize;i++) {
            GLfloat val = CLAMP( values[i], 0.0F, 1.0F );
			p->mapItoB[i] = val;
		}
		break;
	case GL_PIXEL_MAP_I_TO_A:
		p->mapItoAsize = mapsize;
		for (i=0;i<mapsize;i++) {
            GLfloat val = CLAMP( values[i], 0.0F, 1.0F );
			p->mapItoA[i] = val;
		}
		break;
	case GL_PIXEL_MAP_R_TO_R:
		p->mapRtoRsize = mapsize;
		for (i=0;i<mapsize;i++) {
			p->mapRtoR[i] = CLAMP( values[i], 0.0F, 1.0F );
		}
		break;
	case GL_PIXEL_MAP_G_TO_G:
		p->mapGtoGsize = mapsize;
		for (i=0;i<mapsize;i++) {
			p->mapGtoG[i] = CLAMP( values[i], 0.0F, 1.0F );
		}
		break;
	case GL_PIXEL_MAP_B_TO_B:
		p->mapBtoBsize = mapsize;
		for (i=0;i<mapsize;i++) {
			p->mapBtoB[i] = CLAMP( values[i], 0.0F, 1.0F );
		}
		break;
	case GL_PIXEL_MAP_A_TO_A:
		p->mapAtoAsize = mapsize;
		for (i=0;i<mapsize;i++) {
			p->mapAtoA[i] = CLAMP( values[i], 0.0F, 1.0F );
		}
		break;
	default:
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "PixelMap(map)");
		return;
	}

	DIRTY(pb->maps, g->neg_bitid);
	DIRTY(pb->dirty, g->neg_bitid);
}

void STATE_APIENTRY crStatePixelMapuiv (GLenum map, GLint mapsize, const GLuint * values)
{
   GLfloat fvalues[CR_MAX_PIXEL_MAP_TABLE];
   GLint i;
   if (map==GL_PIXEL_MAP_I_TO_I || map==GL_PIXEL_MAP_S_TO_S) {
      for (i=0;i<mapsize;i++) {
         fvalues[i] = (GLfloat) values[i];
      }
   }
   else {
      for (i=0;i<mapsize;i++) {
         fvalues[i] = values[i] / 4294967295.0F;
      }
   }
   crStatePixelMapfv(map, mapsize, fvalues);
}
 
void STATE_APIENTRY crStatePixelMapusv (GLenum map, GLint mapsize, const GLushort * values)
{
   GLfloat fvalues[CR_MAX_PIXEL_MAP_TABLE];
   GLint i;
   if (map==GL_PIXEL_MAP_I_TO_I || map==GL_PIXEL_MAP_S_TO_S) {
      for (i=0;i<mapsize;i++) {
         fvalues[i] = (GLfloat) values[i];
      }
   }
   else {
      for (i=0;i<mapsize;i++) {
         fvalues[i] = values[i] / 65535.0F;
      }
   }
   crStatePixelMapfv(map, mapsize, fvalues);
}

 
void STATE_APIENTRY crStateGetPixelMapfv (GLenum map, GLfloat * values)
{
	CRContext *g = GetCurrentContext();
	CRPixelState *p = &(g->pixel);
	GLint i;

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					 "GetPixelMapfv called in Begin/End");
		return;
	}

	switch (map) {
	case GL_PIXEL_MAP_S_TO_S:
		for (i = 0; i < p->mapStoSsize; i++) {
			values[i] = (GLfloat) p->mapStoS[i];
		}
		break;
	case GL_PIXEL_MAP_I_TO_I:
		for (i = 0; i < p->mapItoIsize; i++) {
			values[i] = (GLfloat) p->mapItoI[i];
		}
		break;
	case GL_PIXEL_MAP_I_TO_R:
		memcpy(values, p->mapItoR, p->mapItoRsize * sizeof(GLfloat));
		break;
	case GL_PIXEL_MAP_I_TO_G:
		memcpy(values, p->mapItoG, p->mapItoGsize * sizeof(GLfloat));
		break;
	case GL_PIXEL_MAP_I_TO_B:
		memcpy(values, p->mapItoB, p->mapItoBsize * sizeof(GLfloat));
		break;
	case GL_PIXEL_MAP_I_TO_A:
		memcpy(values, p->mapItoA, p->mapItoAsize * sizeof(GLfloat));
		break;
	case GL_PIXEL_MAP_R_TO_R:
		memcpy(values, p->mapRtoR, p->mapRtoRsize * sizeof(GLfloat));
		break;
	case GL_PIXEL_MAP_G_TO_G:
		memcpy(values, p->mapGtoG, p->mapGtoGsize * sizeof(GLfloat));
		break;
	case GL_PIXEL_MAP_B_TO_B:
		memcpy(values, p->mapBtoB, p->mapBtoBsize * sizeof(GLfloat));
		break;
	case GL_PIXEL_MAP_A_TO_A:
		memcpy(values, p->mapAtoA, p->mapAtoAsize * sizeof(GLfloat));
		break;
	default:
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "GetPixelMap(map)");
		return;
	}
}
 
void STATE_APIENTRY crStateGetPixelMapuiv (GLenum map, GLuint * values)
{
	const GLfloat maxUint = 4294967295.0F;
	CRContext *g = GetCurrentContext();
	CRPixelState *p = &(g->pixel);
	GLint i;

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					 "GetPixelMapuiv called in Begin/End");
		return;
	}

	switch (map) {
	case GL_PIXEL_MAP_S_TO_S:
		for (i = 0; i < p->mapStoSsize; i++) {
			values[i] = p->mapStoS[i];
		}
		break;
	case GL_PIXEL_MAP_I_TO_I:
		for (i = 0; i < p->mapItoIsize; i++) {
			values[i] = p->mapItoI[i];
		}
		break;
	case GL_PIXEL_MAP_I_TO_R:
		for (i = 0; i < p->mapItoRsize; i++) {
			values[i] = (GLuint) (p->mapItoR[i] * maxUint);
		}
		break;
	case GL_PIXEL_MAP_I_TO_G:
		for (i = 0; i < p->mapItoGsize; i++) {
			values[i] = (GLuint) (p->mapItoG[i] * maxUint);
		}
		break;
	case GL_PIXEL_MAP_I_TO_B:
		for (i = 0; i < p->mapItoBsize; i++) {
			values[i] = (GLuint) (p->mapItoB[i] * maxUint);
		}
		break;
	case GL_PIXEL_MAP_I_TO_A:
		for (i = 0; i < p->mapItoAsize; i++) {
			values[i] = (GLuint) (p->mapItoA[i] * maxUint);
		}
		break;
	case GL_PIXEL_MAP_R_TO_R:
		for (i = 0; i < p->mapRtoRsize; i++) {
			values[i] = (GLuint) (p->mapRtoR[i] * maxUint);
		}
		break;
	case GL_PIXEL_MAP_G_TO_G:
		for (i = 0; i < p->mapGtoGsize; i++) {
			values[i] = (GLuint) (p->mapGtoG[i] * maxUint);
		}
		break;
	case GL_PIXEL_MAP_B_TO_B:
		for (i = 0; i < p->mapBtoBsize; i++) {
			values[i] = (GLuint) (p->mapBtoB[i] * maxUint);
		}
		break;
	case GL_PIXEL_MAP_A_TO_A:
		for (i = 0; i < p->mapAtoAsize; i++) {
			values[i] = (GLuint) (p->mapAtoA[i] * maxUint);
		}
		break;
	default:
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "GetPixelMapuiv(map)");
		return;
	}
}
 
void STATE_APIENTRY crStateGetPixelMapusv (GLenum map, GLushort * values)
{
	const GLfloat maxUshort = 65535.0F;
	CRContext *g = GetCurrentContext();
	CRPixelState *p = &(g->pixel);
	GLint i;

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					 "GetPixelMapusv called in Begin/End");
		return;
	}

	switch (map) {
	case GL_PIXEL_MAP_S_TO_S:
		for (i = 0; i < p->mapStoSsize; i++) {
			values[i] = p->mapStoS[i];
		}
		break;
	case GL_PIXEL_MAP_I_TO_I:
		for (i = 0; i < p->mapItoIsize; i++) {
			values[i] = p->mapItoI[i];
		}
		break;
	case GL_PIXEL_MAP_I_TO_R:
		for (i = 0; i < p->mapItoRsize; i++) {
			values[i] = (GLushort) (p->mapItoR[i] * maxUshort);
		}
		break;
	case GL_PIXEL_MAP_I_TO_G:
		for (i = 0; i < p->mapItoGsize; i++) {
			values[i] = (GLushort) (p->mapItoG[i] * maxUshort);
		}
		break;
	case GL_PIXEL_MAP_I_TO_B:
		for (i = 0; i < p->mapItoBsize; i++) {
			values[i] = (GLushort) (p->mapItoB[i] * maxUshort);
		}
		break;
	case GL_PIXEL_MAP_I_TO_A:
		for (i = 0; i < p->mapItoAsize; i++) {
			values[i] = (GLushort) (p->mapItoA[i] * maxUshort);
		}
		break;
	case GL_PIXEL_MAP_R_TO_R:
		for (i = 0; i < p->mapRtoRsize; i++) {
			values[i] = (GLushort) (p->mapRtoR[i] * maxUshort);
		}
		break;
	case GL_PIXEL_MAP_G_TO_G:
		for (i = 0; i < p->mapGtoGsize; i++) {
			values[i] = (GLushort) (p->mapGtoG[i] * maxUshort);
		}
		break;
	case GL_PIXEL_MAP_B_TO_B:
		for (i = 0; i < p->mapBtoBsize; i++) {
			values[i] = (GLushort) (p->mapBtoB[i] * maxUshort);
		}
		break;
	case GL_PIXEL_MAP_A_TO_A:
		for (i = 0; i < p->mapAtoAsize; i++) {
			values[i] = (GLushort) (p->mapAtoA[i] * maxUshort);
		}
		break;
	default:
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "GetPixelMapusv(map)");
		return;
	}
}
