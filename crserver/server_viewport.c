/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "server_dispatch.h"
#include "server.h"
#include "cr_error.h"
#include "cr_glstate.h"
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
	*/
	if (!v->scissorValid) 
	{
		cr_server.head_spu->dispatch_table.Scissor (
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
	*server_x = x - cr_server.x1[extent];
	*server_y = y - cr_server.y1[extent];
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
	if (*server_x + width > (unsigned int) (cr_server.x2[extent] - cr_server.x1[extent]))
	{
		*server_width = cr_server.x2[extent] - cr_server.x1[extent] - *server_x;
	}
	if (*server_y + height > (unsigned int) (cr_server.y2[extent] - cr_server.y1[extent]))
	{
		*server_height = cr_server.y2[extent] - cr_server.y1[extent] - *server_y;
	}
}

void crServerApplyBaseProjection(void)
{
	cr_server.head_spu->dispatch_table.PushAttrib( GL_TRANSFORM_BIT );
	cr_server.head_spu->dispatch_table.MatrixMode( GL_PROJECTION );
	cr_server.head_spu->dispatch_table.LoadMatrixf( (GLfloat *) &(cr_server.curClient->baseProjection) );
	cr_server.head_spu->dispatch_table.MultMatrixf( (GLfloat *) (cr_server.curClient->ctx->transform.projection + cr_server.curClient->ctx->transform.projectionDepth) );
	cr_server.head_spu->dispatch_table.PopAttrib( );
}

void crServerRecomputeBaseProjection(GLmatrix *base) 
{
	GLfloat xscale, yscale;
	GLfloat xtrans, ytrans;

	GLrectf p;

	p.x1 = ((GLfloat) (cr_server.x1[cr_server.curExtent])) / cr_server.muralWidth;
	p.x2 = ((GLfloat) (cr_server.x2[cr_server.curExtent])) / cr_server.muralWidth;
	p.y1 = ((GLfloat) (cr_server.y1[cr_server.curExtent])) / cr_server.muralHeight;
	p.y2 = ((GLfloat) (cr_server.y2[cr_server.curExtent])) / cr_server.muralHeight;
	
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
