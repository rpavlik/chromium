/* Copyright (c) 2001, Stanford University
* All rights reserved
*
* See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_spu.h"
#include "cr_mem.h"
#include "cr_error.h"
#include "cr_bbox.h"
#include "cr_url.h"
#include <float.h>

#include "binaryswapspu.h"

#define MTRANSFORM(x, y, z, w, m, vx, vy, vz) \
    x = m[0]*vx + m[4]*vy + m[8]*vz  + m[12]; \
    y = m[1]*vx + m[5]*vy + m[9]*vz  + m[13]; \
    z = m[2]*vx + m[6]*vy + m[10]*vz + m[14]; \
    w = m[3]*vx + m[7]*vy + m[11]*vz + m[15]

#define I_TRANSFORM(num, m, vx, vy, vz) \
    MTRANSFORM (x[num], y[num], z[num], w[num], m, vx, vy, vz)

#define CLAMP(a, b, c) \
    if (a < b) a = b; if (a > c) a = c

/****************************************************************
 *
 * These functions below are used to do bounding box calculations
 *
 ****************************************************************/
void clipCoords (GLdouble modl[16], GLdouble proj[16], 
		 GLdouble *x1, GLdouble *y1, GLdouble *z1,
		 GLdouble *x2, GLdouble *y2, GLdouble *z2) 
{
	static GLdouble m[16];
	int i;
	
	GLdouble x[8], y[8], z[8], w[8];
	GLdouble vx1, vy1, vz1;
	GLdouble vx2, vy2, vz2;
	
	GLdouble xmin=DBL_MAX, ymin=DBL_MAX, zmin=DBL_MAX;
	GLdouble xmax=-DBL_MAX, ymax=-DBL_MAX, zmax=-DBL_MAX;
	
	m[0] =  proj[0] * modl[0] + proj[4]  * modl[1]  + 
		proj[8] * modl[2] + proj[12] * modl[3];	
	
	m[1] =  proj[1] * modl[0] + proj[5]  * modl[1]  + 
		proj[9] * modl[2] + proj[13] * modl[3];	
	
	m[2] =  proj[2]  * modl[0] + proj[6]  * modl[1]  + 
		proj[10] * modl[2] + proj[14] * modl[3];	
	
	m[3] =  proj[3]  * modl[0] + proj[7]  * modl[1]  + 
		proj[11] * modl[2] + proj[15] * modl[3];	
	
	m[4] =  proj[0] * modl[4] + proj[4]  * modl[5]  + 
		proj[8] * modl[6] + proj[12] * modl[7];	
	
	m[5] =  proj[1] * modl[4] + proj[5]  * modl[5]  + 
		proj[9] * modl[6] + proj[13] * modl[7];	
	
	m[6] =  proj[2]  * modl[4] + proj[6]  * modl[5]  + 
		proj[10] * modl[6] + proj[14] * modl[7];	
	
	m[7] =  proj[3]  * modl[4] + proj[7]  * modl[5]  + 
		proj[11] * modl[6] + proj[15] * modl[7];	
	
	m[8] =  proj[0] * modl[8]  + proj[4]  * modl[9]  + 
		proj[8] * modl[10] + proj[12] * modl[11];	
	
	m[9] =  proj[1] * modl[8]  + proj[5]  * modl[9]  + 
		proj[9] * modl[10] + proj[13] * modl[11];
	
	m[10] = proj[2]  * modl[8]  + proj[6]  * modl[9]  + 
		proj[10] * modl[10] + proj[14] * modl[11];	
	
	m[11] = proj[3]  * modl[8]  + proj[7]  * modl[9]  + 
		proj[11] * modl[10] + proj[15] * modl[11];	
	
	m[12] = proj[0] * modl[12]  + proj[4]  * modl[13] + 
		proj[8] * modl[14]  + proj[12] * modl[15];	
	
	m[13] = proj[1] * modl[12]  + proj[5]  * modl[13] + 
		proj[9] * modl[14]  + proj[13] * modl[15];	
	
	m[14] = proj[2]  * modl[12] + proj[6]  * modl[13] + 
		proj[10] * modl[14] + proj[14] * modl[15];	
	
	m[15] = proj[3] *  modl[12] + proj[7]  * modl[13] + 
		proj[11] * modl[14] + proj[15] * modl[15]; 
	
	/* Tranform the point by m */
	vx1 = *x1;
	vy1 = *y1;
	vz1 = *z1;
	vx2 = *x2;
	vy2 = *y2;
	vz2 = *z2;
	
	I_TRANSFORM (0 , m, vx1, vy1, vz1);
	I_TRANSFORM (1 , m, vx1, vy1, vz2);
	I_TRANSFORM (2 , m, vx1, vy2, vz1);
	I_TRANSFORM (3 , m, vx1, vy2, vz2);
	I_TRANSFORM (4 , m, vx2, vy1, vz1);
	I_TRANSFORM (5 , m, vx2, vy1, vz2);
	I_TRANSFORM (6 , m, vx2, vy2, vz1);
	I_TRANSFORM (7 , m, vx2, vy2, vz2);
	
	for (i=0; i<8; i++) {   
		x[i] /= w[i];
		y[i] /= w[i];
		z[i] /= w[i];
		
		if (x[i] > xmax) xmax = x[i];
		if (y[i] > ymax) ymax = y[i];
		if (z[i] > zmax) zmax = z[i]; 
		if (x[i] < xmin) xmin = x[i];
		if (y[i] < ymin) ymin = y[i];
		if (z[i] < zmin) zmin = z[i];
	}    
	
	*x1 = xmin;
	*y1 = ymin;
	*z1 = zmin;
	*x2 = xmax;
	*y2 = ymax;
	*z2 = zmax;
}

/*******************************************************
 * Get the clipped window based on the projection and 
 * model matrices and the bounding box supplied by the
 * application.
 *******************************************************/
int getClippedWindow(GLdouble modl[16], GLdouble proj[16], 
		     int *xstart, int* ystart,
		     int* xend, int* yend )
{
	GLfloat viewport[4];
	GLdouble x1, x2, y1, y2, z1, z2;
	int win_height, win_width;
	
	if(binaryswap_spu.bbox != NULL){
		x1=binaryswap_spu.bbox->xmin;
		y1=binaryswap_spu.bbox->ymin;
		z1=binaryswap_spu.bbox->zmin;
		x2=binaryswap_spu.bbox->xmax; 
		y2=binaryswap_spu.bbox->ymax;
		z2=binaryswap_spu.bbox->zmax;
	}
	else{ //no bounding box defined
		return 0;
		crDebug("No BBox");
	}
		
	clipCoords(modl, proj, &x1, &y1, &z1, &x2, &y2, &z2);
	/* Sanity check... */
	if( x2 < x1 || y2 < y1 || z2 < z1){
		crWarning( "Damnit!!!!, we screwed up the clipping somehow..." );
		return 0;
	}
	
	/* adjust depth for alpha composite */
	binaryswap_spu.depth = z2;

	/* can we remove this get to speed things up? */
	binaryswap_spu.super.GetFloatv( GL_VIEWPORT, viewport );
	(*xstart) = (int)((x1+1.0f)*(viewport[2] / 2.0f) + viewport[0]);
	(*ystart) = (int)((y1+1.0f)*(viewport[3] / 2.0f) + viewport[1]);
	(*xend)   = (int)((x2+1.0f)*(viewport[2] / 2.0f) + viewport[0]);
	(*yend)   = (int)((y2+1.0f)*(viewport[3] / 2.0f) + viewport[1]);
	
	win_width  = (int)viewport[2];
	win_height = (int)viewport[3];
	
	CLAMP ((*xstart), 0, win_width);
	CLAMP ((*xend),   0, win_width);
	CLAMP ((*ystart), 0, win_height);
	CLAMP ((*yend),   0, win_height);
	crDebug("x: %d, y: %d, w: %d, h: %d", 
		*xstart, *ystart, 
		*xstart + *xend,
		*ystart + *yend);
	return 1;
}


/*
 * Build swap arrays
 */
static void BuildSwapLimits( WindowInfo *window )
{
	int xdiv = 1, ydiv = 1, i = 0;
	/* clean up old data structures */
	if(window->read_x)
		crFree(window->read_x);
	if(window->read_y)
		crFree(window->read_y);
	if(window->read_width)
		crFree(window->read_width);
	if(window->read_height)
		crFree(window->read_height);
	
	window->read_x      = crAlloc(binaryswap_spu.stages*sizeof(int));
	window->read_y      = crAlloc(binaryswap_spu.stages*sizeof(int));
	window->read_width  = crAlloc(binaryswap_spu.stages*sizeof(int));
	window->read_height = crAlloc(binaryswap_spu.stages*sizeof(int));
	
	/* figure out swap positions */
	/* BUG: Need to deal with odd window dimensions, 
	 * the current code can be off by one along the edge */
	for(i=0; i<binaryswap_spu.stages; i++)
	{
		/* even stage => right/left */
		if(i%2 == 0)
		{
			xdiv *=2;
			/* right */
			if(!binaryswap_spu.highlow[i])
			{
				crDebug("Swap %d: right", i);
				if(i==0)
				{
					window->read_x[i] = window->width/(xdiv);
					window->read_y[i] = 0;
				}
				else
				{
					window->read_x[i] = window->read_x[i-1]+window->width/(xdiv);
					window->read_y[i] = window->height/(ydiv)-window->read_y[i-1];
				}
			}
			/* left */
			else
			{
				crDebug("Swap %d: left", i);
				if(i==0)
				{
					window->read_y[i] = 0;
					window->read_x[i] = 0;
				}
				else
				{
					window->read_x[i] = window->read_x[i-1];
					window->read_y[i] = window->height/(ydiv)-window->read_y[i-1];
				}
			}
		}
		/* odd stage  => top/bottom */
		else
		{
			ydiv *=2;
			/* top */
			if(binaryswap_spu.highlow[i])
			{
				crDebug("Swap %d: top", i);
				window->read_x[i] = window->width/(xdiv)-window->read_x[i-1];
				window->read_y[i] = window->read_y[i-1]+window->height/(ydiv);
			}
			/* bottom */
			else
			{
				crDebug("Swap %d: bottom", i);
				window->read_x[i] = window->width/(xdiv)-window->read_x[i-1];
				window->read_y[i] = window->read_y[i-1];
			}
		}
		window->read_width[i] = window->width/xdiv;
		window->read_height[i] = window->height/ydiv;
		crDebug("Width: %d, Height: %d", window->read_width[i], window->read_height[i]);
		crDebug("x: %d, y: %d", window->read_x[i], window->read_y[i]);
	}
}


/*
 * Allocate the color and depth buffers needed for the glDraw/ReadPixels
 * commands for the given window.
 */
static void AllocBuffers( WindowInfo *window )
{
	CRASSERT(window);
	CRASSERT(window->width >= 0);
	CRASSERT(window->height >= 0);

	if (window->msgBuffer)
		crFree(window->msgBuffer);

	window->msgBuffer = (GLubyte *) crAlloc( sizeof(BinarySwapMsg) + 
						 window->width * window->height
						 * ( (window->bytesPerDepth + 
						      window->bytesPerColor) * sizeof(GLubyte)));
	/* Setup message type to keep network layer happy */
	((BinarySwapMsg*) window->msgBuffer)->header.type = CR_MESSAGE_OOB;
}



/*
 * Determine the size of the given binaryswap SPU window.
 * We may either have to query the super or child SPU window dims.
 * Reallocate the glReadPixels RGBA/depth buffers if the size changes.
 */
static void CheckWindowSize( WindowInfo *window )
{
	GLint newSize[2];

	newSize[0] = newSize[1] = 0;
	if (binaryswap_spu.resizable)
	{
		/* ask downstream SPU (probably render) for its window size */
		binaryswap_spu.child.GetChromiumParametervCR(GL_WINDOW_SIZE_CR,
							     window->childWindow, 
							     GL_INT, 2, newSize);
		if (newSize[0] == 0 && newSize[1] == 0)
		{
			/* something went wrong - recover - try viewport */
			GLint geometry[4];
			binaryswap_spu.child.GetIntegerv( GL_VIEWPORT, geometry );
			newSize[0] = geometry[2];
			newSize[1] = geometry[3];
		}
		window->childWidth = newSize[0];
		window->childHeight = newSize[1];
	}
	else
	{
		/* not resizable - ask render SPU for its window size */
		binaryswap_spu.super.GetChromiumParametervCR(GL_WINDOW_SIZE_CR,
							     window->renderWindow, 
							     GL_INT, 2, newSize);
	}

	if (newSize[0] != window->width || newSize[1] != window->height)
	{
		/* The window size has changed (or first-time init) */
		window->width = newSize[0];
		window->height = newSize[1];
		
		if (binaryswap_spu.alpha_composite)
			window->bytesPerColor = 4 * sizeof(GLubyte);
		else
			window->bytesPerColor = 3 * sizeof(GLubyte);
		
		if (binaryswap_spu.depth_composite)
		{
			window->bytesPerDepth = 4;
		}
		else
			window->bytesPerDepth = 0;
		
	
		if (binaryswap_spu.resizable)
		{
			/* update super/render SPU window size & viewport */
			CRASSERT(newSize[0] > 0);
			CRASSERT(newSize[1] > 0);
			binaryswap_spu.super.WindowSize( window->renderWindow, 
							 newSize[0], newSize[1] );
			binaryswap_spu.super.Viewport( 0, 0, newSize[0], newSize[1] );
			
			/* set child's viewport too */
			binaryswap_spu.child.Viewport( 0, 0, newSize[0], newSize[1] );
		}	
		AllocBuffers(window);
		BuildSwapLimits(window);			
	}
}


/*
 * This is the guts of the binaryswap operation.  Here, we call glReadPixels
 * to read a region of the color and depth buffers from the parent (render
 * SPU) window.	 We then begin the process of composition using binary swap.
 * Each node does a glReadPixels and sends it's swap region to its swap 
 * partner.  Once we receive our partner's frame region, we use draw pixels to
 * composite against our frame.  This repeats for all swap partners.  Once we
 * have all of the regions composited against the one we are responsible for,
 * we issue another read and send our region to final display by glDrawPixels 
 * (and some other GL functions) on the child/downstream SPU to send our region
 * to the final display node.
 * Input:  window - the window we're processing
 *	   startx, starty - glReadPixels start coordinates
 *	   endx,   endy   - glReadPixels ending coordinates
 */
static void CompositeNode( WindowInfo *window, 
			   int startx, int starty,
			   int endx, int endy)
{
	int i = 0;
	int read_start_x = 0, read_start_y = 0;
	int read_width = 0, read_height = 0;
	CRMessage *incoming_msg = NULL;
	GLubyte* incoming_color = NULL;
	GLfloat* incoming_depth = NULL;
	BinarySwapMsg *render_info = NULL;
	int draw_x = 0, draw_y = 0;
	int draw_width = 0, draw_height = 0;
	double other_depth = 0.0;   

	int recalc_end_x = 0, recalc_end_y = 0;
	int recalc_start_x = 0, recalc_start_y = 0;
	int recalc_temp;
	
	CRASSERT(window->width > 0);
	CRASSERT(window->height > 0);
	/* figure out our portion for each stage */
	for(i=0; i<binaryswap_spu.stages; i++)
	{
		/* set up message header */
		((BinarySwapMsg*) window->msgBuffer)->start_x        = window->read_x[i];
		((BinarySwapMsg*) window->msgBuffer)->start_y        = window->read_y[i];
		((BinarySwapMsg*) window->msgBuffer)->width          = window->read_width[i];
		((BinarySwapMsg*) window->msgBuffer)->height         = window->read_height[i];
		((BinarySwapMsg*) window->msgBuffer)->depth          = binaryswap_spu.depth;
		
		if(startx < window->read_x[i])
			((BinarySwapMsg*) window->msgBuffer)->clipped_x = window->read_x[i];
		else
			((BinarySwapMsg*) window->msgBuffer)->clipped_x = startx;
		
		if(starty < window->read_y[i])
			((BinarySwapMsg*) window->msgBuffer)->clipped_y = window->read_y[i];
		else
			((BinarySwapMsg*) window->msgBuffer)->clipped_y = starty;
		
		if(endx > (window->read_x[i]+window->read_width[i]))
			((BinarySwapMsg*) window->msgBuffer)->clipped_width  = 
				(window->read_x[i]+window->read_width[i]) - 
				((BinarySwapMsg*) window->msgBuffer)->clipped_x+1;
		else
			((BinarySwapMsg*) window->msgBuffer)->clipped_width  = 
				endx - ((BinarySwapMsg*) window->msgBuffer)->clipped_x+1;
		
		if(endy > (window->read_y[i]+window->read_height[i]))
			((BinarySwapMsg*) window->msgBuffer)->clipped_height = 
				(window->read_y[i]+window->read_height[i]) - 
				((BinarySwapMsg*) window->msgBuffer)->clipped_y+1;
		else
			((BinarySwapMsg*) window->msgBuffer)->clipped_height = 
				endy - ((BinarySwapMsg*) window->msgBuffer)->clipped_y+1;
		
		if(((BinarySwapMsg*) window->msgBuffer)->clipped_width < 0)
			((BinarySwapMsg*) window->msgBuffer)->clipped_width = 0; 
		if(((BinarySwapMsg*) window->msgBuffer)->clipped_height < 0)
			((BinarySwapMsg*) window->msgBuffer)->clipped_height = 0;
		
		read_start_x = ((BinarySwapMsg*) window->msgBuffer)->clipped_x;
		read_start_y = ((BinarySwapMsg*) window->msgBuffer)->clipped_y;
		read_width   = ((BinarySwapMsg*) window->msgBuffer)->clipped_width;
		read_height  = ((BinarySwapMsg*) window->msgBuffer)->clipped_height;
		
		/* read our portion for this pass */
		/* figure out which mode to use, depth or alpha */
		if(binaryswap_spu.alpha_composite)
		{
			if(read_width > 0 && read_height > 0)
				binaryswap_spu.super.ReadPixels( read_start_x, read_start_y, 
								 read_width, read_height, 
								 GL_ABGR_EXT, GL_UNSIGNED_BYTE, 
								 (GLubyte*)window->msgBuffer +
								 binaryswap_spu.offset ); 
			
			/* lower of pair => recv,send */
			if(binaryswap_spu.highlow[i])
			{
				crNetGetMessage( binaryswap_spu.peer_recv[i], &incoming_msg);
				crNetSend( binaryswap_spu.peer_send[i], NULL, window->msgBuffer, 
					   (read_width * read_height * 4) + binaryswap_spu.offset);
				if (binaryswap_spu.mtu > binaryswap_spu.peer_send[i]->mtu)
					binaryswap_spu.mtu = binaryswap_spu.peer_send[i]->mtu;
			}
			/* higher of pair => send,recv */
			else
			{
				crNetSend( binaryswap_spu.peer_send[i], NULL, window->msgBuffer, 
					   (read_width * read_height * 4) + binaryswap_spu.offset);
				if (binaryswap_spu.mtu > binaryswap_spu.peer_send[i]->mtu)
					binaryswap_spu.mtu = binaryswap_spu.peer_send[i]->mtu;
				crNetGetMessage( binaryswap_spu.peer_recv[i], &incoming_msg);
			}
			
			/* get render info from other node */
			render_info = (BinarySwapMsg*)incoming_msg;
			draw_x      = render_info->clipped_x;
			draw_y      = render_info->clipped_y;
			draw_width  = render_info->clipped_width;
			draw_height = render_info->clipped_height;
			other_depth = render_info->depth;
			
			/* get incoming fb */
			if(draw_width > 0 && draw_width > 0){
				incoming_color = (GLubyte*)((GLubyte*)incoming_msg + binaryswap_spu.offset);
				binaryswap_spu.super.Enable(GL_BLEND);
				/* figure out blend function based on z */
				/* Other image is on top of ours! */
				if(binaryswap_spu.depth > other_depth)
				{
					/* over operator */
					binaryswap_spu.super.BlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA ); 
				}
				/* other image is under ours */
				else
				{  
					/* under operator */
					binaryswap_spu.super.BlendFunc( GL_ONE_MINUS_DST_ALPHA, GL_ONE );
				}	
				
				if(binaryswap_spu.depth < other_depth)
				{
					binaryswap_spu.depth = other_depth;
				}
				
				binaryswap_spu.super.RasterPos2i( draw_x, draw_y );
				binaryswap_spu.super.DrawPixels( draw_width, draw_height, 
								 GL_ABGR_EXT, GL_UNSIGNED_BYTE, incoming_color ); 
			}
		}
		/* depth composite */
		else
		{ 	
			if(read_width > 0 && read_height > 0){
				binaryswap_spu.super.ReadPixels( read_start_x, read_start_y, 
								 read_width, read_height, 
								 GL_BGR_EXT, GL_UNSIGNED_BYTE, 
								 (GLubyte*)window->msgBuffer +
								 binaryswap_spu.offset ); 
				binaryswap_spu.super.ReadPixels( read_start_x, read_start_y, 
								 read_width, read_height, 
								 GL_DEPTH_COMPONENT, GL_FLOAT, 
								 (GLubyte*)window->msgBuffer + // base address
								 (read_width * read_height * 3) + // color information
								 binaryswap_spu.offset );  // message header
			}
			
			/* lower of pair => recv,send */
			if(binaryswap_spu.highlow[i])
			{
				crNetGetMessage( binaryswap_spu.peer_recv[i], &incoming_msg);
				crNetSend( binaryswap_spu.peer_send[i], NULL, window->msgBuffer,  
					   read_width*read_height*(3+4) + binaryswap_spu.offset);
				if (binaryswap_spu.mtu > binaryswap_spu.peer_send[i]->mtu)
					binaryswap_spu.mtu = binaryswap_spu.peer_send[i]->mtu;
			}
			/* higher of pair => send,recv */
			else
			{
				crNetSend( binaryswap_spu.peer_send[i], NULL, window->msgBuffer, 
					   read_width*read_height*(3+4) + binaryswap_spu.offset);
				if (binaryswap_spu.mtu > binaryswap_spu.peer_send[i]->mtu)
					binaryswap_spu.mtu = binaryswap_spu.peer_send[i]->mtu;
				crNetGetMessage( binaryswap_spu.peer_recv[i], &incoming_msg);
			}
			
			/* get render info from other node */
			render_info = (BinarySwapMsg*)incoming_msg;
			draw_x      = render_info->clipped_x;
			draw_y      = render_info->clipped_y;
			draw_width  = render_info->clipped_width;
			draw_height = render_info->clipped_height;
			
			if(draw_width > 0 && draw_width > 0){
				/* get incoming fb */
				incoming_color = (GLubyte*)((GLubyte*)incoming_msg + binaryswap_spu.offset);
				incoming_depth = (GLfloat*)(incoming_color + draw_width*draw_height*3);
				
				/* stupid stecil buffer tricks */
				/* mask portion to draw */
				binaryswap_spu.super.ColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
				binaryswap_spu.super.Enable( GL_STENCIL_TEST );
				binaryswap_spu.super.Enable( GL_DEPTH_TEST );
				binaryswap_spu.super.ClearStencil( 0 );
				binaryswap_spu.super.Clear( GL_STENCIL_BUFFER_BIT );
				binaryswap_spu.super.DepthFunc( GL_LEQUAL );
				binaryswap_spu.super.StencilFunc( GL_ALWAYS, 0x1, 0x1 );
				binaryswap_spu.super.StencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );
				binaryswap_spu.super.RasterPos2i( draw_x, draw_y );
				binaryswap_spu.super.DrawPixels( draw_width, draw_height, 
								 GL_DEPTH_COMPONENT, 
								 GL_FLOAT, incoming_depth );
				binaryswap_spu.super.Disable( GL_DEPTH_TEST );
				
				/* draw where depth worked */
				binaryswap_spu.super.StencilFunc( GL_EQUAL, 1, 1 );
				binaryswap_spu.super.StencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
				binaryswap_spu.super.ColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
				binaryswap_spu.super.DrawPixels( draw_width, draw_height, GL_BGR_EXT, 
								 GL_UNSIGNED_BYTE, incoming_color );
				binaryswap_spu.super.Disable( GL_STENCIL_TEST );
				binaryswap_spu.super.Enable( GL_DEPTH_TEST ); 
			}
		}
		
		/* make sure everything got drawn for next pass */
		binaryswap_spu.super.Flush();
		
		/* find optimal starting point for readback based on 
		   this nodes region and the partner's region */
		recalc_start_x = render_info->start_x;
		if(startx < render_info->clipped_x){
			recalc_temp = startx;
		}
		else{
			recalc_temp = render_info->clipped_x;
			if(render_info->clipped_width > 0 && render_info->clipped_height > 0)
				startx = recalc_temp;
		}
		if(recalc_temp > recalc_start_x){
			recalc_start_x = recalc_temp;
		}
		
		recalc_start_y = render_info->start_y;
		if(starty < render_info->clipped_y){
			recalc_temp = starty;
		}
		else{
			recalc_temp = render_info->clipped_y;
			if(render_info->clipped_width > 0 && render_info->clipped_height > 0)
				starty = recalc_temp;
		}
		if(recalc_temp > recalc_start_y){
			recalc_start_y = recalc_temp;
		}
		
		/* find optimal ending point for readback based on 
		   this nodes region and the partner's region */
		recalc_end_x = render_info->start_x + render_info->width - 1;
		if(endx > (render_info->clipped_x + render_info->clipped_width - 1)){
			recalc_temp = endx;
		}
		else{
			recalc_temp = (render_info->clipped_x + render_info->clipped_width - 1);
			if(render_info->clipped_width > 0 && render_info->clipped_height > 0)
				endx = recalc_temp;
		}
		if(recalc_end_x > recalc_temp){
			recalc_end_x = recalc_temp;
		}
		
		recalc_end_y = render_info->start_y + render_info->height - 1;
		if(endy > (render_info->clipped_y + render_info->clipped_height - 1)){
			recalc_temp = endy;
		}
		else{
			recalc_temp = (render_info->clipped_y + render_info->clipped_height - 1);
			if(render_info->clipped_width > 0 && render_info->clipped_height > 0)
				endy = recalc_temp;
		}
		if(recalc_end_y > recalc_temp){
			recalc_end_y = recalc_temp;
		}
		
		/* clean up the memory allocated for the recv */
		crNetFree( binaryswap_spu.peer_recv[i], incoming_msg );
	}

	draw_x = recalc_start_x;
	draw_y = recalc_start_y;
	draw_width = recalc_end_x - recalc_start_x + 1;
	draw_height = recalc_end_y - recalc_start_y + 1;

	if(draw_width > 0 && draw_height > 0){
		/* send our final portion off to child */
		binaryswap_spu.super.ReadPixels( draw_x, draw_y, draw_width, draw_height, 
						 GL_BGR_EXT, GL_UNSIGNED_BYTE, 
						 window->msgBuffer + 
						 binaryswap_spu.offset ); 
		
		/*
		 * Set the downstream viewport.	 If we don't do this, and the
		 * downstream window is resized, the glRasterPos command doesn't
		 * seem to be reliable.	 This is a problem both with Mesa and the
		 * NVIDIA drivers.  Technically, this may not be a driver bug at
		 * all since we're doing funny stuff.  Anyway, this fixes the problem.
		 * Note that the width and height are arbitrary since we only care
		 * about getting the origin right.  glDrawPixels, glClear, etc don't
		 * care what the viewport size is.  (BrianP)
		 */	
		CRASSERT(window->width > 0);
		CRASSERT(window->height > 0);	
		
		binaryswap_spu.child.SemaphorePCR( MUTEX_SEMAPHORE );
		binaryswap_spu.child.Viewport( 0, 0, window->width, window->height );	
		
		binaryswap_spu.child.RasterPos2i( 0, 0 );
		binaryswap_spu.child.Bitmap( 0, 0, 0, 0, (GLfloat)draw_x, (GLfloat)draw_y, NULL); 
		binaryswap_spu.child.DrawPixels( draw_width, draw_height, 
						 GL_BGR_EXT, GL_UNSIGNED_BYTE, 
						 window->msgBuffer + 
						 binaryswap_spu.offset );
		
		binaryswap_spu.child.SemaphoreVCR( MUTEX_SEMAPHORE );	
	}
}


/*
 * Do binaryswap/composite for a window.
 * This involves:
 *   - computing the image regions (tiles) to process
 *   - bounding box testing (only if running on the faker / whole window)
 *   - doing glClear
 *   - call CompositeNode() for each region
 */
static void ProcessNode( WindowInfo *window, GLdouble modl[16], GLdouble proj[16] )
{
	int read_start_x = 0;
	int read_start_y = 0;	
	int read_end_x = window->width;
	int read_end_y = window->height;

	/* deal with clipping */
	getClippedWindow( modl, proj, 
			  &read_start_x, &read_start_y, 
			  &read_end_x, &read_end_y);
	
	/* One will typically use serverNode.Conf('only_swap_once', 1) to
	 * prevent extraneous glClear and SwapBuffer calls on the server.
	 */
	if (binaryswap_spu.depth_composite) 
		binaryswap_spu.child.Clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	else 
		binaryswap_spu.child.Clear( GL_COLOR_BUFFER_BIT );
	
	/* wait for everyone to finish clearing */
	binaryswap_spu.child.BarrierExecCR( CLEAR_BARRIER );

	CompositeNode(window, read_start_x, read_start_y, read_end_x, read_end_y);
}

/************************************************************
 * Deals with putting back settings we muck with and checking
 * for possible window resize.
 ************************************************************/ 
static void DoBinaryswap( WindowInfo *window )
{
	static int first_time = 1;	
	
	/* values used to restore state we change */
	GLint super_packAlignment, super_unpackAlignment;
	GLint child_unpackAlignment;
	GLint super_viewport[4];
	GLdouble super_proj_matrix[16];
	GLdouble super_modl_matrix[16];
	GLboolean super_blend = GL_FALSE;
	GLint super_blend_dst = 0, super_blend_src = 0;
	GLboolean super_color_writemask[4];
	GLint super_depth_func = 0, super_stencil_func = 0, super_stencil_ref = 0; 
	GLint super_stencil_value_mask = 0, super_stencil_fail = 0;
	GLint super_stencil_pass_depth_fail = 0, super_stencil_pass_depth_pass = 0;
	GLboolean super_stencil_test = GL_FALSE, super_depth_test = GL_FALSE;

	if (first_time || window->width < 1 || window->height < 1)
	{
		CheckWindowSize( window );
	}

	if (first_time)
	{
		/* one-time initializations */
		binaryswap_spu.child.BarrierCreateCR(CLEAR_BARRIER, 0);
		binaryswap_spu.child.BarrierCreateCR(SWAP_BARRIER, 0);
		binaryswap_spu.child.SemaphoreCreateCR(MUTEX_SEMAPHORE, 1);
		((BinarySwapMsg*) window->msgBuffer)->header.type = CR_MESSAGE_OOB;
		binaryswap_spu.offset = sizeof( BinarySwapMsg );
		first_time = 0;
	}
	else if (binaryswap_spu.resizable)
	{
		/* check if window size changed, reallocate buffers if needed */
		CheckWindowSize( window );
	}

	/*
	 * Save pack/unpack alignments, and set to one.
	 */
	binaryswap_spu.super.GetIntegerv(GL_PACK_ALIGNMENT, &super_packAlignment);
	binaryswap_spu.super.GetIntegerv(GL_UNPACK_ALIGNMENT, &super_unpackAlignment);
	binaryswap_spu.child.GetIntegerv(GL_UNPACK_ALIGNMENT, &child_unpackAlignment);
	binaryswap_spu.super.PixelStorei(GL_PACK_ALIGNMENT, 1);	
	binaryswap_spu.super.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
	binaryswap_spu.child.PixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/* things we are going to change that we need to put back */
	/* fix things up for reading and drawing pixels */
	binaryswap_spu.super.GetIntegerv( GL_VIEWPORT, super_viewport );
	binaryswap_spu.super.GetDoublev ( GL_PROJECTION_MATRIX, super_proj_matrix);
	binaryswap_spu.super.GetDoublev ( GL_MODELVIEW_MATRIX, super_modl_matrix);

	/* Things alpha compositing mucks with */
	if(binaryswap_spu.alpha_composite)
	{
		super_blend = binaryswap_spu.super.IsEnabled( GL_BLEND );
		binaryswap_spu.super.GetIntegerv( GL_BLEND_DST, &super_blend_dst);
		binaryswap_spu.super.GetIntegerv( GL_BLEND_SRC, &super_blend_src);
	}
	if(binaryswap_spu.depth_composite)
	{
		/* Things depth compositing mucks with */
		binaryswap_spu.super.GetBooleanv( GL_COLOR_WRITEMASK, super_color_writemask);
		binaryswap_spu.super.GetIntegerv( GL_DEPTH_FUNC, &super_depth_func);
		binaryswap_spu.super.GetIntegerv( GL_STENCIL_FUNC, &super_stencil_func);
		binaryswap_spu.super.GetIntegerv( GL_STENCIL_REF, &super_stencil_ref);
		binaryswap_spu.super.GetIntegerv( GL_STENCIL_VALUE_MASK, 
						  &super_stencil_value_mask);
		binaryswap_spu.super.GetIntegerv( GL_STENCIL_FAIL, &super_stencil_fail);
		binaryswap_spu.super.GetIntegerv( GL_STENCIL_PASS_DEPTH_FAIL, 
						  &super_stencil_pass_depth_fail);
		binaryswap_spu.super.GetIntegerv( GL_STENCIL_PASS_DEPTH_PASS, 
						  &super_stencil_pass_depth_pass);
		super_stencil_test = binaryswap_spu.super.IsEnabled( GL_STENCIL_TEST );
		super_depth_test = binaryswap_spu.super.IsEnabled( GL_DEPTH_TEST );
	}

	/* fix viewport and projection for read/draw pixels */
	binaryswap_spu.super.Viewport( 0, 0, window->width, window->height );
	binaryswap_spu.super.MatrixMode(GL_PROJECTION);
	binaryswap_spu.super.LoadIdentity();
	binaryswap_spu.super.Ortho(0.0, (GLdouble) window->width, 0.0, 
				   (GLdouble) window->height, -1., 1.);
	binaryswap_spu.super.MatrixMode(GL_MODELVIEW);
	binaryswap_spu.super.LoadIdentity();

	ProcessNode(window, super_modl_matrix, super_proj_matrix);

	/*
	 * Restore pack/unpack alignments
	 */
	binaryswap_spu.super.PixelStorei(GL_PACK_ALIGNMENT, super_packAlignment);
	binaryswap_spu.super.PixelStorei(GL_UNPACK_ALIGNMENT, super_unpackAlignment);
	binaryswap_spu.child.PixelStorei(GL_UNPACK_ALIGNMENT, child_unpackAlignment);

	/* Fix what we mucked with */
	binaryswap_spu.super.Viewport( super_viewport[0], 
				       super_viewport[1], 
				       super_viewport[2], 
				       super_viewport[3] );
	binaryswap_spu.super.MatrixMode(GL_PROJECTION);
	binaryswap_spu.super.LoadMatrixd(super_proj_matrix);
	binaryswap_spu.super.MatrixMode(GL_MODELVIEW);
	binaryswap_spu.super.LoadMatrixd(super_modl_matrix);

	if(binaryswap_spu.alpha_composite)
	{
		if(super_blend)
			binaryswap_spu.super.Enable(GL_BLEND);
		else
			binaryswap_spu.super.Disable(GL_BLEND);

		binaryswap_spu.super.BlendFunc( super_blend_src, super_blend_dst );
	}
	if(binaryswap_spu.depth_composite)
	{
		if(super_depth_test)
			binaryswap_spu.super.Enable(GL_DEPTH_TEST);
		else
			binaryswap_spu.super.Disable(GL_DEPTH_TEST);
		
		if(super_stencil_test)
			binaryswap_spu.super.Enable(GL_STENCIL_TEST);
		else
			binaryswap_spu.super.Disable(GL_STENCIL_TEST);
		
		binaryswap_spu.super.ColorMask( super_color_writemask[0],
						super_color_writemask[1],
						super_color_writemask[2],
						super_color_writemask[3] );
		binaryswap_spu.super.DepthFunc( super_depth_func );
		binaryswap_spu.super.StencilFunc( super_stencil_func, 
						  super_stencil_ref, 
						  super_stencil_value_mask );
		binaryswap_spu.super.StencilOp( super_stencil_fail,
						super_stencil_pass_depth_fail,
						super_stencil_pass_depth_pass );
	}
}

static void BINARYSWAPSPU_APIENTRY binaryswapspuFlush( void )
{
	WindowInfo *window;
	GET_CONTEXT(context);
	CRASSERT(context); /* we shouldn't be flushing without a context */
	window = context->currentWindow;
	if (!window)
			return;

	DoBinaryswap( window );

	/*
	 * XXX I'm not sure we need to sync on glFlush, but let's be safe for now.
	 */
	binaryswap_spu.child.BarrierExecCR( SWAP_BARRIER );
}



static void BINARYSWAPSPU_APIENTRY binaryswapspuSwapBuffers( GLint win, GLint flags )
{
	WindowInfo *window;
	
	window = (WindowInfo *) crHashtableSearch(binaryswap_spu.windowTable, win);
	CRASSERT(window);

	DoBinaryswap( window );
	
	/*
	 * Everyone syncs up here before calling SwapBuffers().
	 */
	binaryswap_spu.child.BarrierExecCR( SWAP_BARRIER );

	binaryswap_spu.child.SwapBuffers( window->childWindow, 
					  flags & ~CR_SUPPRESS_SWAP_BIT );
	binaryswap_spu.child.Finish();
		
	if (binaryswap_spu.local_visualization)
	{
		binaryswap_spu.super.SwapBuffers( window->renderWindow, 0 );
	}
}

static GLint BINARYSWAPSPU_APIENTRY binaryswapspuCreateContext( const char *dpyName, GLint visBits)
{
	static GLint freeID = 0;
	ContextInfo *context;

	CRASSERT(binaryswap_spu.child.BarrierCreateCR);

	if(freeID != 0)
	{
		crError("Binaryswap cannot support multiple contexts");
		return 0;
	}

	context = (ContextInfo *) crCalloc(sizeof(ContextInfo));
	if (!context)
	{
		crWarning("binaryswap SPU: create context failed.");
		return -1;
	}

	/* If doing z-compositing, need stencil buffer */
	if (binaryswap_spu.depth_composite)
		visBits |= CR_STENCIL_BIT;
	else if (binaryswap_spu.alpha_composite)
		visBits |= CR_ALPHA_BIT;

	context->renderContext = binaryswap_spu.super.CreateContext(dpyName, visBits);
	context->childContext = binaryswap_spu.child.CreateContext(dpyName, visBits);

	/* put into hash table */
	crHashtableAdd(binaryswap_spu.contextTable, freeID, context);
	freeID++;
	return freeID - 1;
}


static void BINARYSWAPSPU_APIENTRY binaryswapspuDestroyContext( GLint ctx )
{
	ContextInfo *context;
	context = (ContextInfo *) crHashtableSearch(binaryswap_spu.contextTable, ctx);
	CRASSERT(context);
	binaryswap_spu.super.DestroyContext(context->renderContext);
	crHashtableDelete(binaryswap_spu.contextTable, ctx);
}


static void BINARYSWAPSPU_APIENTRY binaryswapspuMakeCurrent(GLint win, GLint nativeWindow, GLint ctx)
{
	ContextInfo *context;
	WindowInfo *window;

	context = (ContextInfo *) crHashtableSearch(binaryswap_spu.contextTable, ctx);
	window = (WindowInfo *) crHashtableSearch(binaryswap_spu.windowTable, win);

	if (context && window)
	{
#ifdef CHROMIUM_THREADSAFE
		crSetTSD(&_BinaryswapTSD, context);
#else
		binaryswap_spu.currentContext = context;
#endif
		CRASSERT(window);
		context->currentWindow = window;
		binaryswap_spu.super.MakeCurrent(window->renderWindow,
						 nativeWindow, context->renderContext);
		binaryswap_spu.child.MakeCurrent(window->childWindow,
						 nativeWindow, context->childContext);
		/* Initialize child's projection matrix so that glRasterPos2i(0,0)
		 * corresponds to window coordinate (0,0).
		 */
		binaryswap_spu.child.MatrixMode(GL_PROJECTION);
		binaryswap_spu.child.LoadIdentity();
		binaryswap_spu.child.Ortho(0.0, 1.0, 0.0, 1.0, -1., 1.);
	}
	else {
#ifdef CHROMIUM_THREADSAFE
		crSetTSD(&_BinaryswapTSD, NULL);
#else
		binaryswap_spu.currentContext = NULL;
#endif
	}
}

static GLint BINARYSWAPSPU_APIENTRY binaryswapspuWindowCreate( const char *dpyName, GLint visBits )
{
	WindowInfo *window;
	static GLint freeID = 1;  /* skip default window 0 */

	/* Error out on second window */
	if(freeID != 1)
	{
		crError("Binaryswap can't deal with multiple windows!");
		return 0;
	}

	/* If doing z-compositing, need stencil buffer */
	if (binaryswap_spu.depth_composite)
		visBits |= CR_STENCIL_BIT;
	else if (binaryswap_spu.alpha_composite)
		visBits |= CR_ALPHA_BIT;

	/* allocate window */
	window = (WindowInfo *) crCalloc(sizeof(WindowInfo));
	if (!window)
	{
		crWarning("binaryswap SPU: unable to allocate window.");
		return -1;
	}

	/* init window */
	window->index = freeID;
	window->renderWindow = binaryswap_spu.super.WindowCreate(dpyName, visBits);
	window->childWindow = 0; /* the default down-stream window */
	window->width = -1; /* unknown */
	window->height = -1; /* unknown */
	window->msgBuffer = NULL;

	/* put into hash table */
	crHashtableAdd(binaryswap_spu.windowTable, window->index, window);
	freeID++;

	return freeID - 1;
}

static void BINARYSWAPSPU_APIENTRY binaryswapspuWindowDestroy( GLint win )
{
	WindowInfo *window;
	window = (WindowInfo *) crHashtableSearch(binaryswap_spu.windowTable, win);
	CRASSERT(window);
	binaryswap_spu.super.WindowDestroy(window->renderWindow);
	crHashtableDelete(binaryswap_spu.windowTable, win);
}

static void BINARYSWAPSPU_APIENTRY binaryswapspuWindowSize( GLint win, GLint w, GLint h )
{
	WindowInfo *window;
	window = (WindowInfo *) crHashtableSearch(binaryswap_spu.windowTable, win);
	CRASSERT(window);
	binaryswap_spu.super.WindowSize( window->renderWindow, w, h );
	binaryswap_spu.child.WindowSize( window->childWindow, w, h );
}

/* don't implement WindowPosition() */


/* I'M REALLY NOT HAPPY ABOUT DROPPING BARRIERS ON THE FLOOR, BUT WITHOUT THIS
 * SOME APPS LIKE PSUBMIT FAIL TO WORK.  BUT, WHAT IF WE ARE NOT AT THE END OF THE CHAIN?
 * FOR EXAMPLE, WHAT IF I WANT TO DO SOME OTHER TYPE OF COMPOSITE BEFORE BINARYSWAP THAT
 * NEEDS BARRIERS?
 */
static void BINARYSWAPSPU_APIENTRY binaryswapspuBarrierCreateCR( GLuint name, GLuint count )
{
	(void) name;
	/* no-op */
}

static void BINARYSWAPSPU_APIENTRY binaryswapspuBarrierDestroyCR( GLuint name )
{
	(void) name;
	/* no-op */
}

static void BINARYSWAPSPU_APIENTRY binaryswapspuBarrierExecCR( GLuint name )
{
	(void) name;
	/* no-op */
}

static void BINARYSWAPSPU_APIENTRY binaryswapspuViewport( GLint x,
							  GLint y, GLint w, GLint h )
{
	binaryswap_spu.super.Viewport( x, y, w, h );
}

static void BINARYSWAPSPU_APIENTRY binaryswapspuChromiumParametervCR(GLenum target, 
								     GLenum type, 
								     GLsizei count, 
								     GLvoid *values)
{
	switch( target )
	{
		case GL_OBJECT_BBOX_CR:
			binaryswap_spu.bbox = values;
			break;
		default:
			binaryswap_spu.child.ChromiumParametervCR( target, type, count, values );
			break;
	}
}


SPUNamedFunctionTable binaryswap_table[] = {
	{ "SwapBuffers", (SPUGenericFunction) binaryswapspuSwapBuffers },
	{ "CreateContext", (SPUGenericFunction) binaryswapspuCreateContext },
	{ "DestroyContext", (SPUGenericFunction) binaryswapspuDestroyContext },
	{ "MakeCurrent", (SPUGenericFunction) binaryswapspuMakeCurrent },
	{ "WindowCreate", (SPUGenericFunction) binaryswapspuWindowCreate },
	{ "WindowDestroy", (SPUGenericFunction) binaryswapspuWindowDestroy },
	{ "WindowSize", (SPUGenericFunction) binaryswapspuWindowSize },
	{ "BarrierCreateCR", (SPUGenericFunction) binaryswapspuBarrierCreateCR },
	{ "BarrierDestroyCR", (SPUGenericFunction) binaryswapspuBarrierDestroyCR },
	{ "BarrierExecCR", (SPUGenericFunction) binaryswapspuBarrierExecCR },
 	{ "Viewport", (SPUGenericFunction) binaryswapspuViewport }, 
	{ "Flush", (SPUGenericFunction) binaryswapspuFlush },
	{ "ChromiumParametervCR", (SPUGenericFunction) binaryswapspuChromiumParametervCR },
	{ NULL, NULL }
};
