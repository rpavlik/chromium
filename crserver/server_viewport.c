/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "server_dispatch.h"
#include "server.h"
#include "cr_error.h"
#include "state/cr_statetypes.h"

static const GLmatrix identity_matrix = { 
	(GLdefault) 1.0, (GLdefault) 0.0, (GLdefault) 0.0, (GLdefault) 0.0,
	(GLdefault) 0.0, (GLdefault) 1.0, (GLdefault) 0.0, (GLdefault) 0.0,
	(GLdefault) 0.0, (GLdefault) 0.0, (GLdefault) 1.0, (GLdefault) 0.0,
	(GLdefault) 0.0, (GLdefault) 0.0, (GLdefault) 0.0, (GLdefault) 1.0
};

static void crServerViewportClipToWindow (const GLrecti *imagewindow, GLrecti *q) 
{
	if (q->x1 < imagewindow->x1) q->x1 = imagewindow->x1;
	if (q->x1 > imagewindow->x2) q->x1 = imagewindow->x2;

	if (q->x2 > imagewindow->x2) q->x2 = imagewindow->x2;
	if (q->x2 < imagewindow->x1) q->x2 = imagewindow->x1;
	
	if (q->y1 < imagewindow->y1) q->y1 = imagewindow->y1;
	if (q->y1 > imagewindow->y2) q->y1 = imagewindow->y2;

	if (q->y2 > imagewindow->y2) q->y2 = imagewindow->y2;
	if (q->y2 < imagewindow->y1) q->y2 = imagewindow->y1;
}

static void crServerViewportConvertToOutput (const GLrecti *imagewindow,
							  const GLrecti *outputwindow, GLrecti *q) 
{
	q->x1 = q->x1 - imagewindow->x1 + outputwindow->x1;
	q->x2 = q->x2 - imagewindow->x2 + outputwindow->x2;
	q->y1 = q->y1 - imagewindow->y1 + outputwindow->y1;
	q->y2 = q->y2 - imagewindow->y2 + outputwindow->y2;
}

/*
 * Setup the viewport parameters.  This is called by crServerSetOutputBounds()
 * when we prepare to render a tile.
 * Input:  v - viewport params
 *         outputwindow - tile bounds in server's rendering window
 *         imagespace - whole mural bounds
 *         imagewindow - tile bounds in mural space
 * Output:  clipped_imagespace - imagespace after scissor is applied
 *          clipped_imagewindow - imagewindow after scissor is applied
 */
void crServerSetViewportBounds (CRViewportState *v,
							  const GLrecti *outputwindow,
							  const GLrecti *imagespace,
							  const GLrecti *imagewindow,
							  GLrecti *clipped_imagespace,
							  GLrecti *clipped_imagewindow) 
{
	GLrecti q;

	v->outputDims = *imagewindow;

	v->widthScale = (GLfloat) ( outputwindow->x2 - outputwindow->x1 );
	v->widthScale /= (GLfloat) ( imagespace->x2 - imagespace->x1 );

	v->heightScale = (GLfloat) ( outputwindow->y2 - outputwindow->y1 );
	v->heightScale /= (GLfloat) ( imagespace->y2 - imagespace->y1 );

	v->x_offset = outputwindow->x1;
	v->y_offset = outputwindow->y1;

	/* If the scissor is invalid 
	** set it to the whole output
	** We might as well use the actual scissorTest rather than
	** scissorValid - it never gets reset anyway.
	*/
	if (!v->scissorTest) 
	{
		cr_server.head_spu->dispatch_table.Scissor(
			outputwindow->x1, outputwindow->y1,
			outputwindow->x2 - outputwindow->x1,
			outputwindow->y2 - outputwindow->y1);
	} 
	else 
	{
		q.x1 = v->scissorX;
		q.x2 = v->scissorX + v->scissorW;
		q.y1 = v->scissorY;
		q.y2 = v->scissorY + v->scissorH;

		crServerViewportClipToWindow(imagewindow, &q);
		crServerViewportConvertToOutput(imagewindow, outputwindow, &q);
		cr_server.head_spu->dispatch_table.Scissor(q.x1,  q.y1, 
			q.x2 - q.x1, q.y2 - q.y1);
	}
	
	/* if the viewport is not valid,
	** set it to the entire output.
	*/
	if (!v->viewportValid) 
	{
		if (clipped_imagespace)
		{
			*clipped_imagespace = *imagespace;
		}
		if (clipped_imagewindow)
		{
			*clipped_imagewindow = *imagewindow;
		}

		cr_server.head_spu->dispatch_table.Viewport(outputwindow->x1, outputwindow->y1,
			outputwindow->x2 - outputwindow->x1,
			outputwindow->y2 - outputwindow->y1);

		return;
	}
	
	q.x1 = v->viewportX;
	q.x2 = v->viewportX + v->viewportW;
	q.y1 = v->viewportY;
	q.y2 = v->viewportY + v->viewportH;

	/* This is where the viewport gets clamped to the max size. */
	crServerViewportClipToWindow(imagewindow, &q);

	if (clipped_imagespace) 
	{
		clipped_imagespace->x1 = v->viewportX;
		clipped_imagespace->x2 = v->viewportX + v->viewportW;
		clipped_imagespace->y1 = v->viewportY;
		clipped_imagespace->y2 = v->viewportY + v->viewportH;
	}

	if (clipped_imagewindow)
	{
		*clipped_imagewindow = q;
	}

	crServerViewportConvertToOutput(imagewindow, outputwindow, &q);

	cr_server.head_spu->dispatch_table.Viewport (q.x1,  q.y1, 
		q.x2 - q.x1, q.y2 - q.y1);
}

void crServerClampViewport( int x, int y, unsigned int width, unsigned int height,
		int *server_x, int *server_y, unsigned int *server_width, unsigned int *server_height, int extent )
{
	*server_x = x - cr_server.extents[extent].x1;
	*server_y = y - cr_server.extents[extent].y1;
	*server_width = width;
	*server_height = height;
	if (*server_x < 0)
	{
		*server_x = 0;
	}
	if (*server_y < 0)
	{
		*server_y = 0;
	}
	if (*server_x + width > (unsigned int) (cr_server.extents[extent].x2 - cr_server.extents[extent].x1))
	{
		*server_width = cr_server.extents[extent].x2 - cr_server.extents[extent].x1 - *server_x;
	}
	if (*server_y + height > (unsigned int) (cr_server.extents[extent].y2 - cr_server.extents[extent].y1))
	{
		*server_height = cr_server.extents[extent].y2 - cr_server.extents[extent].y1 - *server_y;
	}
}


/*
 * Pre-multiply the current projection matrix with the current client's
 * base projection.  I.e.  P' = b * P.  Note that OpenGL's glMultMatrix
 * POST-multiplies.
 */
void crServerApplyBaseProjection(void)
{
	cr_server.head_spu->dispatch_table.PushAttrib( GL_TRANSFORM_BIT );
	cr_server.head_spu->dispatch_table.MatrixMode( GL_PROJECTION );

	cr_server.head_spu->dispatch_table.LoadMatrixf( (GLfloat *) &(cr_server.curClient->baseProjection) );
#if 1
	cr_server.head_spu->dispatch_table.MultMatrixf(cr_server.alignment_matrix);
#endif	
	cr_server.head_spu->dispatch_table.MultMatrixf( (GLfloat *) (cr_server.curClient->currentCtx->transform.projection + cr_server.curClient->currentCtx->transform.projectionDepth) );
	cr_server.head_spu->dispatch_table.PopAttrib( );
}


/*
 * Recompute the "base projection" matrix.  We examine the server extent
 * specified by cr_server.curExtent.  I think the base projection matrix
 * maps the extent space into the mural space (a scale and translate).
 * Input: x, y - mural origin (always 0,0?)
 * Input: w, h - mural width, height
 * Output: base - base projection matrix.
 */
void crServerRecomputeBaseProjection(GLmatrix *base, GLint x, GLint y, GLint w, GLint h)
{
	GLfloat xscale, yscale;
	GLfloat xtrans, ytrans;
	GLrectf p;

	/* 
	 * We need to take account of the current viewport parameters,
	 * and they are passed to this function as x, y, w, h.
 	 * In the default case (from main.c) we pass the the
	 * full muralsize of 0, 0, muralWidth, muralHeight
	 */
	p.x1 = ((GLfloat) (cr_server.extents[cr_server.curExtent].x1) - x) / (w);
	p.y1 = ((GLfloat) (cr_server.extents[cr_server.curExtent].y1) - y) / (h);
	p.x2 = ((GLfloat) (cr_server.extents[cr_server.curExtent].x2) - x) / (w);
	p.y2 = ((GLfloat) (cr_server.extents[cr_server.curExtent].y2) - y) / (h);

	/* XXX This gets real kludgy.
	 * It's tricky when viewport's cross server boundaries
	 * and we can only hope for the best.
	 */
	if (p.x1 < 0.0) { 
		p.x1 = 0.0;
		if (p.x2 > 1.0) p.x2 = 1.0;
	}

	if (p.y1 < 0.0) {
		p.y1 = 0.0; 
		if (p.y2 > 1.0) p.y2 = 1.0;
	}

	/* Rescale [0,1] -> [-1,1] */
	p.x1 = p.x1*2.0f - 1.0f;
	p.x2 = p.x2*2.0f - 1.0f;
	p.y1 = p.y1*2.0f - 1.0f;
	p.y2 = p.y2*2.0f - 1.0f;

	xscale = 2.0f / (p.x2 - p.x1);
	yscale = 2.0f / (p.y2 - p.y1);
	xtrans = -(p.x2 + p.x1) / 2.0f;
	ytrans = -(p.y2 + p.y1) / 2.0f;

	*base = identity_matrix;
	base->m00 = xscale;
	base->m11 = yscale;
	base->m30 = xtrans*xscale;
	base->m31 = ytrans*yscale;
}

void SERVER_DISPATCH_APIENTRY crServerDispatchViewport( GLint x, GLint y, GLsizei width, GLsizei height )
{
	/* Note -- If there are tiles, this will be overridden in the 
	 * process of decoding the BoundsInfo packet, so no worries. */
	crStateViewport( x, y, width, height );
	cr_server.head_spu->dispatch_table.Viewport( x, y, width, height );
}
