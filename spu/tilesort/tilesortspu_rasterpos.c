/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "tilesortspu.h"
#include "tilesortspu_proto.h"
#include "cr_glstate.h"


static void
rasterpos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w) 
{
	GET_CONTEXT(g);

	GLboolean haveRasterPosClip = g->extensions.IBM_rasterpos_clip;

	if (haveRasterPosClip) {
		int save;
		GET_THREAD(thread);

		tilesortspuFlush( thread );

		save = thread->geometry_buffer.geometry_only;
		thread->geometry_buffer.geometry_only = GL_FALSE;

		/* Broadcast the raster pos update */
		if (tilesort_spu.swap)
		{
			crPackEnableSWAP(GL_RASTER_POSITION_UNCLIPPED_IBM);
			crPackRasterPos4fSWAP( x, y, z, w );
			crPackDisableSWAP(GL_RASTER_POSITION_UNCLIPPED_IBM);
		}
		else
		{
			crPackEnable(GL_RASTER_POSITION_UNCLIPPED_IBM);
			crPackRasterPos4f( x, y, z, w );
			crPackDisable(GL_RASTER_POSITION_UNCLIPPED_IBM);
		}
		tilesortspuBroadcastGeom(1);

		thread->geometry_buffer.geometry_only = save;

		/* Need to update state tracker's current raster info, but don't set
		 * dirty bits!
		 */
		crStateRasterPosUpdate(x, y, z, w);
	}
	else {
		/* We can't disable raster pos clipping on the back-end.
		 * Use the old window-pos/bitmap method.
		 */
		crStateRasterPos4f(x, y, z, w); /* DO set dirty state */
	}
}


void TILESORTSPU_APIENTRY tilesortspu_RasterPos2d(GLdouble x, GLdouble y)
{
	rasterpos4f((GLfloat) x, (GLfloat) y, 0.0f, 1.0f);
}

void TILESORTSPU_APIENTRY tilesortspu_RasterPos2f(GLfloat x, GLfloat y)
{
	rasterpos4f(x, y, 0.0f, 1.0f);
}

void TILESORTSPU_APIENTRY tilesortspu_RasterPos2i(GLint x, GLint y)
{
	rasterpos4f((GLfloat) x, (GLfloat) y, 0.0f, 1.0f);
}

void TILESORTSPU_APIENTRY tilesortspu_RasterPos2s(GLshort x, GLshort y)
{
	rasterpos4f((GLfloat) x, (GLfloat) y, 0.0f, 1.0f);
}

void TILESORTSPU_APIENTRY tilesortspu_RasterPos3d(GLdouble x, GLdouble y, GLdouble z)
{
	rasterpos4f((GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0f);
}

void TILESORTSPU_APIENTRY tilesortspu_RasterPos3f(GLfloat x, GLfloat y, GLfloat z)
{
	rasterpos4f(x, y, z, 1.0f);
}

void TILESORTSPU_APIENTRY tilesortspu_RasterPos3i(GLint x, GLint y, GLint z)
{
	rasterpos4f((GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0f);
}

void TILESORTSPU_APIENTRY tilesortspu_RasterPos3s(GLshort x, GLshort y, GLshort z)
{
	rasterpos4f((GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0f);
}

void TILESORTSPU_APIENTRY tilesortspu_RasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	rasterpos4f((GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w);
}

void TILESORTSPU_APIENTRY tilesortspu_RasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	rasterpos4f(x, y, z, w);
}

void TILESORTSPU_APIENTRY tilesortspu_RasterPos4i(GLint x, GLint y, GLint z, GLint w)
{
	rasterpos4f((GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w);
}

void TILESORTSPU_APIENTRY tilesortspu_RasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
	rasterpos4f((GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w);
}

void TILESORTSPU_APIENTRY tilesortspu_RasterPos2dv(const GLdouble *v)
{
	rasterpos4f((GLfloat) v[0], (GLfloat) v[1], 0.0f, 1.0f);
}

void TILESORTSPU_APIENTRY tilesortspu_RasterPos2fv(const GLfloat *v)
{
	rasterpos4f(v[0], v[1], 0.0f, 1.0f);
}

void TILESORTSPU_APIENTRY tilesortspu_RasterPos2iv(const GLint *v)
{
	rasterpos4f((GLfloat) v[0], (GLfloat) v[1], 0.0f, 1.0f);
}

void TILESORTSPU_APIENTRY tilesortspu_RasterPos2sv(const GLshort *v)
{
	rasterpos4f((GLfloat) v[0], (GLfloat) v[1], 0.0f, 1.0f);
}

void TILESORTSPU_APIENTRY tilesortspu_RasterPos3dv(const GLdouble *v)
{
	rasterpos4f((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], 1.0f);
}

void TILESORTSPU_APIENTRY tilesortspu_RasterPos3fv(const GLfloat *v)
{
	rasterpos4f(v[0], v[1], v[2], 1.0f);
}

void TILESORTSPU_APIENTRY tilesortspu_RasterPos3iv(const GLint *v)
{
	rasterpos4f((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], 1.0f);
}

void TILESORTSPU_APIENTRY tilesortspu_RasterPos3sv(const GLshort *v)
{
	rasterpos4f((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], 1.0f);
}

void TILESORTSPU_APIENTRY tilesortspu_RasterPos4dv(const GLdouble *v)
{
	rasterpos4f((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) v[3]);
}

void TILESORTSPU_APIENTRY tilesortspu_RasterPos4fv(const GLfloat *v)
{
	rasterpos4f(v[0], v[1], v[2], v[3]);
}

void TILESORTSPU_APIENTRY tilesortspu_RasterPos4iv(const GLint *v)
{
	rasterpos4f((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) v[3]);
}

void TILESORTSPU_APIENTRY tilesortspu_RasterPos4sv(const GLshort *v)
{
	rasterpos4f((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) v[3]);
}
