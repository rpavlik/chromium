/* Copyright (c) 2001, Stanford University
* All rights reserved
*
* See the file LICENSE.txt for information on redistributing this software.
*/

/*
 * TODO: 
 * 1) Need to support resizable windows once we get CRUT
 * 2) Can we use BBox util library, or is this way faster? 
 *
 */

#include <stdio.h>
#include "cr_spu.h"
#include "cr_net.h"
#include "cr_mem.h"
#include "cr_error.h"
#include "cr_bbox.h"
#include "binaryswapspu.h"

#define MTRANSFORM(x, y, z, w, m, vx, vy, vz) \
    x = m[0]*vx + m[4]*vy + m[8]*vz  + m[12]; \
    y = m[1]*vx + m[5]*vy + m[9]*vz  + m[13]; \
    z = m[2]*vx + m[6]*vy + m[10]*vz + m[14]; \
    w = m[3]*vx + m[7]*vy + m[11]*vz + m[15]

#define I_TRANSFORM(num, m, vx, vy, vz) \
    MTRANSFORM (x[num], y[num], z[num], w[num], m, vx, vy, vz)

#define FLT_MAX 3.402823466e+38f

#define CLAMP(a, b, c) \
if (a < b) a = b; if (a > c) a = c

BinaryswapSPU binaryswap_spu;
GLfloat modl[16];
GLfloat proj[16];

/****************************************************************
 *
 * These functions below are used to do bounding box calculations
 *
 ****************************************************************/
void clipCoords (GLfloat *x1, GLfloat *y1, GLfloat *z1,
		 GLfloat *x2, GLfloat *y2, GLfloat *z2) 
{
  static GLfloat m[16];
  int i;
  
  GLfloat x[8], y[8], z[8], w[8];
  GLfloat vx1, vy1, vz1;
  GLfloat vx2, vy2, vz2;
  
  GLfloat xmin=FLT_MAX, ymin=FLT_MAX, zmin=FLT_MAX;
  GLfloat xmax=-FLT_MAX, ymax=-FLT_MAX, zmax=-FLT_MAX;
  
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

  m[11] = proj[3]  * modl[8]  + proj[7]   * modl[9]  + 
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
  binaryswap_spu.depth = zmax;
}

int getClippedWindow(int *xstart, int* ystart,int* xend, int* yend )
{
  GLfloat viewport[4];
  GLint   window_dim[4];
  GLfloat x1, x2, y1, y2, z1, z2;
  int win_height, win_width;

  if(binaryswap_spu.bounding_box != NULL){
    x1=binaryswap_spu.bounding_box->x1;
    y1=binaryswap_spu.bounding_box->y1;
    z1=binaryswap_spu.bounding_box->z1;
    x2=binaryswap_spu.bounding_box->x2; 
    y2=binaryswap_spu.bounding_box->y2;
    z2=binaryswap_spu.bounding_box->z2;
  }
  else{ //no bounding box defined
    return 0;
  }

  /* get current matrices for clipping */
  binaryswap_spu.super.GetFloatv(GL_MODELVIEW_MATRIX,  modl);
  binaryswap_spu.super.GetFloatv(GL_PROJECTION_MATRIX, proj);

  clipCoords(&x1, &y1, &z1, &x2, &y2, &z2);
  /* Sanity check... */
  if( x2 < x1 || y2 < y1 || z2 < z1){
    crWarning( "Damnit!!!!, we screwed up the clipping somehow..." );
    return 0;
  }
  
  /* can we remove this get to speed things up? */
  binaryswap_spu.super.GetFloatv( GL_VIEWPORT, viewport );
  (*xstart) = (int)((x1+1.0f)*(viewport[2] / 2.0f) + viewport[0]);
  (*ystart) = (int)((y1+1.0f)*(viewport[3] / 2.0f) + viewport[1]);
  (*xend)   = (int)((x2+1.0f)*(viewport[2] / 2.0f) + viewport[0]);
  (*yend)   = (int)((y2+1.0f)*(viewport[3] / 2.0f) + viewport[1]);

  /* can we remove this get to speed things up? */
  binaryswap_spu.super.GetIntegerv( GL_VIEWPORT, window_dim );
  win_width  = window_dim[2];
  win_height = window_dim[3];
  
  CLAMP ((*xstart), 0, win_width);
  CLAMP ((*xend),   0, win_width);
  CLAMP ((*ystart), 0, win_height);
  CLAMP ((*yend),   0, win_height);
  return 1;
}

/**************************************************************************
 *
 * Right now this is the only nice way to generically pass information into
 * Chromium without linking to Chromium.  Yes this is hacky...
 *
 **************************************************************************/
void BINARYSWAPSPU_APIENTRY binaryswapspuHint( GLenum target,
					    GLenum mode )
{
  /* intercept Z info */
  if( target == 0x4200 ){
    /* mode is really a float, cast it and grab info */
    binaryswap_spu.depth = (float)mode;
  } 
  if( target == 0x4201 ){
    /* mode is really a pointer, cast it and grab info */
    binaryswap_spu.bounding_box = (BBox*)mode;
  }
  /* Pass on all other hints to child */
  else{
    binaryswap_spu.child.Hint( target, mode );
  }
}


/*
 * Allocate a new ThreadInfo structure and bind this to the calling thread
 * with crSetTSD().
 */
#ifdef CHROMIUM_THREADSAFE
static ThreadInfo *binaryswapspuNewThread( unsigned long id )
{
  ThreadInfo *thread = crCalloc(sizeof(ThreadInfo));
  if (thread) {
    crSetTSD(&_BinaryswapTSD, thread);
    thread->id = id;
    thread->currentContext = -1;
    thread->currentWindow = -1;
  }
  return thread;
}
#endif

static void DoFlush( WindowInfo *window )
{ 

  /* Things we setup once since the setup is expensive */
  static int geometry[4];
  static int stages;
  static int first_time = 1;
  static int* read_x;
  static int* read_y;
  static int* width;
  static int* height;

  int i, xdiv, ydiv;
  float other_depth;
  int draw_x = 0, draw_y = 0;
  int draw_width = 0, draw_height = 0;
  int start_clipped_x = 0, start_clipped_y = 0;
  int end_clipped_x = 0, end_clipped_y = 0;
  int read_start_x, read_start_y, read_width, read_height;
  GLubyte* incoming_color;
  GLfloat* incoming_depth;
  CRMessage *incoming_msg;
  BinarySwapMsg *render_info;
  GLint packAlignment;
  
  if (first_time){
    binaryswap_spu.child.BarrierCreateCR( BINARYSWAP_CLEARBARRIER, 0 );
    binaryswap_spu.child.BarrierCreateCR( BINARYSWAP_SWAPBARRIER, 0 );
    binaryswap_spu.child.SemaphoreCreateCR( MUTEX_SEMAPHORE, 1 );
    binaryswap_spu.child.LoadIdentity();
    binaryswap_spu.child.Ortho( 0, window->width  - 1,	
				0, window->height - 1,
				-10000, 10000 );

    binaryswap_spu.super.GetIntegerv( GL_VIEWPORT, geometry );
    if(binaryswap_spu.alpha_composite){
      binaryswap_spu.outgoing_msg = 
	(GLubyte *) crAlloc( sizeof(BinarySwapMsg) + 
			     (geometry[2] * geometry[3] * 
			      (4 * sizeof(GLubyte))));
    }
    else{
      binaryswap_spu.outgoing_msg = 
	(GLubyte *) crAlloc( sizeof(BinarySwapMsg) + 
			     (geometry[2] * geometry[3] * 
			      (3 * sizeof(GLubyte) + sizeof(GLfloat)))); 
    }
    
    binaryswap_spu.offset = sizeof( BinarySwapMsg );
    ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->header.type = CR_MESSAGE_OOB;

    stages = binaryswap_spu.stages;
    read_x = crAlloc(stages*sizeof(int));
    read_y = crAlloc(stages*sizeof(int));
    width  = crAlloc(stages*sizeof(int));
    height = crAlloc(stages*sizeof(int));

    /* figure out swap positions */
    xdiv = ydiv = 1;
    for(i=0; i<stages; i++){
      /* even stage => right/left */
      if(i%2 == 0){
	xdiv *=2;
	/* right */
	if(!binaryswap_spu.highlow[i]){
	  crDebug("Swap %d: right", i);
	  if(i==0){
	    read_x[i] = geometry[2]/(xdiv);
	    read_y[i] = 0;
	  }
	  else{
	    read_x[i] = read_x[i-1]+geometry[2]/(xdiv);
	    read_y[i] = geometry[3]/(ydiv)-read_y[i-1];
	  }
	}
	/* left */
	else{
	  crDebug("Swap %d: left", i);
	  if(i==0){
	    read_y[i] = 0;
	    read_x[i] = 0;
	  }
	  else{
	    read_x[i] = read_x[i-1];
	    read_y[i] = geometry[3]/(ydiv)-read_y[i-1];
	  }
	}
      }
      /* odd stage  => top/bottom */
      else{
	ydiv *=2;
	/* top */
	if(binaryswap_spu.highlow[i]){
	  crDebug("Swap %d: top", i);
	    read_x[i] = geometry[2]/(xdiv)-read_x[i-1];
	    read_y[i] = read_y[i-1]+geometry[3]/(ydiv);
	}
	/* bottom */
	else{
	  crDebug("Swap %d: bottom", i);
	  read_x[i] = geometry[2]/(xdiv)-read_x[i-1];
	  read_y[i] = read_y[i-1];
	}
      }
      width[i] = geometry[2]/xdiv;
      height[i] = geometry[3]/ydiv;
      crDebug("Width: %d, Height: %d", width[i], height[i]);
      crDebug("x: %d, y: %d", read_x[i], read_y[i]);
    }
    first_time = 0;
  }

  if(binaryswap_spu.clipped_window){
    start_clipped_x = 0;
    start_clipped_y = 0;
    end_clipped_x = 0;
    end_clipped_y = 0;
    
    if(!getClippedWindow(&start_clipped_x, &start_clipped_y, 
			 &end_clipped_x, &end_clipped_y )){
      crWarning("Bounding box was null, I'll no longer attempt to clip");
      binaryswap_spu.clipped_window = !binaryswap_spu.clipped_window;
    }    
  }
  
  /* setup positions so we can do a draw pixels correctly! */
  binaryswap_spu.super.MatrixMode(GL_PROJECTION);
  binaryswap_spu.super.LoadIdentity();
  binaryswap_spu.super.Ortho(0.0, (GLdouble) geometry[2], 0.0, 
			     (GLdouble) geometry[3], -10.0, 1000000.0);
  binaryswap_spu.super.MatrixMode(GL_MODELVIEW);
  binaryswap_spu.super.LoadIdentity();  
  
  /* fix pixel alignment */
  binaryswap_spu.super.GetIntegerv( GL_PACK_ALIGNMENT, &packAlignment );
  binaryswap_spu.super.PixelStorei( GL_PACK_ALIGNMENT, 1 );

  /* blend other guy's stuff with ours */
  binaryswap_spu.super.Enable( GL_BLEND );

  /* make sure everyone is up to speed */
  /* figure out our portion for each stage */
  for(i=0; i<stages; i++){
    /* set up message header */
    ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->start_x         = read_x[i];
    ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->start_y         = read_y[i];
    ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->width           = width[i];
    ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->height          = height[i];
    ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->clipped_x       = 0;
    ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->clipped_y       = 0;
    ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->clipped_width   = 0;
    ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->clipped_height  = 0;
    ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->depth           = binaryswap_spu.depth;

    if(binaryswap_spu.clipped_window){
      if(start_clipped_x < read_x[i])
	((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->clipped_x = read_x[i];
      else
	((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->clipped_x = start_clipped_x;
      
      if(start_clipped_y < read_y[i])
	((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->clipped_y = read_y[i];
      else
	((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->clipped_y = start_clipped_y;
      
      if(end_clipped_x > (read_x[i]+width[i]))
	((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->clipped_width  = 
	  (read_x[i]+width[i]) - ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->clipped_x+1;
      else
	((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->clipped_width  = 
	  end_clipped_x - ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->clipped_x+1;
      
      if(end_clipped_y > (read_y[i]+height[i]))
	((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->clipped_height = 
	  (read_y[i]+height[i]) - ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->clipped_y+1;
      else
	((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->clipped_height = 
	  end_clipped_y - ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->clipped_y+1;
      
      if(((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->clipped_width < 0)
	((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->clipped_width = 0; 
      if(((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->clipped_height < 0)
	((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->clipped_height = 0;
      
      read_start_x = ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->clipped_x;
      read_start_y = ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->clipped_y;
      read_width   = ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->clipped_width;
      read_height  = ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->clipped_height;
    }
    else{
      read_start_x = ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->start_x;
      read_start_y = ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->start_y;
      read_width   = ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->width;
      read_height  = ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->height;
    }
    
    /* read our portion for this pass */
    /* figure out which mode to use, depth or alpha */
    if(binaryswap_spu.alpha_composite){
      binaryswap_spu.super.ReadPixels( read_start_x, read_start_y, read_width, read_height, 
				       GL_BGRA_EXT, GL_UNSIGNED_BYTE, 
				       (unsigned char*)binaryswap_spu.outgoing_msg +
				       binaryswap_spu.offset ); 

      /* lower of pair => recv,send */
      if(binaryswap_spu.highlow[i]){
	crNetGetMessage( binaryswap_spu.peer_recv[i], &incoming_msg);
	crNetSend( binaryswap_spu.peer_send[i], NULL, binaryswap_spu.outgoing_msg, 
		   (read_width * read_height * 4) + binaryswap_spu.offset);
	if (binaryswap_spu.mtu > binaryswap_spu.peer_send[i]->mtu)
		binaryswap_spu.mtu = binaryswap_spu.peer_send[i]->mtu;
      }
      /* higher of pair => send,recv */
      else{
	crNetSend( binaryswap_spu.peer_send[i], NULL, binaryswap_spu.outgoing_msg, 
		   (read_width * read_height * 4) + binaryswap_spu.offset);
	if (binaryswap_spu.mtu > binaryswap_spu.peer_send[i]->mtu)
		binaryswap_spu.mtu = binaryswap_spu.peer_send[i]->mtu;
	crNetGetMessage( binaryswap_spu.peer_recv[i], &incoming_msg);
      }
      
      /* get render info from other node */
      render_info = (BinarySwapMsg*)incoming_msg;
      if(binaryswap_spu.clipped_window){
	draw_x = render_info->clipped_x;
	draw_y = render_info->clipped_y;
	draw_width = render_info->clipped_width;
	draw_height = render_info->clipped_height;
      }
      else{
	draw_x = render_info->start_x;
	draw_y = render_info->start_y;
	draw_width = render_info->width;
	draw_height = render_info->height;
      }
      other_depth = render_info->depth;

      if(binaryswap_spu.depth < other_depth){
	binaryswap_spu.depth = other_depth;
      }
      
      /* get incoming fb */
      incoming_color = (GLubyte*)((unsigned char*)incoming_msg + binaryswap_spu.offset);
      
      /* figure out blend function based on z */
      /* Other image is on top of ours! */
      if(binaryswap_spu.depth > other_depth){
	/* over operator */
	binaryswap_spu.super.BlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA ); 
      }
      /* other image is under ours */
      else{  
	/* under operator */
	binaryswap_spu.super.BlendFunc( GL_ONE_MINUS_DST_ALPHA, GL_ONE );
      }
      binaryswap_spu.super.RasterPos2i( draw_x, draw_y );
      binaryswap_spu.super.DrawPixels( draw_width, draw_height, 
				       GL_BGRA_EXT, GL_UNSIGNED_BYTE, incoming_color ); 
      
    }
    /* depth composite */
    else{ 
      binaryswap_spu.super.ReadPixels( read_start_x, read_start_y, read_width, read_height, 
				       GL_BGR_EXT, GL_UNSIGNED_BYTE, 
				       (unsigned char*)binaryswap_spu.outgoing_msg +
				       binaryswap_spu.offset ); 
      binaryswap_spu.super.ReadPixels( read_start_x, read_start_y, read_width, read_height, 
				       GL_DEPTH_COMPONENT, GL_FLOAT, 
				       (unsigned char*)binaryswap_spu.outgoing_msg + // base address
				       (width[i] * height[i] * 3) + // color information
				       binaryswap_spu.offset );  // message header

      /* lower of pair => recv,send */
      if(binaryswap_spu.highlow[i]){
	crNetGetMessage( binaryswap_spu.peer_recv[i], &incoming_msg);
	crNetSend( binaryswap_spu.peer_send[i], NULL, binaryswap_spu.outgoing_msg,  
		   read_width*read_height*(3+4) + binaryswap_spu.offset);
	if (binaryswap_spu.mtu > binaryswap_spu.peer_send[i]->mtu)
		binaryswap_spu.mtu = binaryswap_spu.peer_send[i]->mtu;
	
      }
      /* higher of pair => send,recv */
      else{
	crNetSend( binaryswap_spu.peer_send[i], NULL, binaryswap_spu.outgoing_msg, 
		  read_width*read_height*(3+4) + binaryswap_spu.offset);
	if (binaryswap_spu.mtu > binaryswap_spu.peer_send[i]->mtu)
		binaryswap_spu.mtu = binaryswap_spu.peer_send[i]->mtu;
	crNetGetMessage( binaryswap_spu.peer_recv[i], &incoming_msg);
      }
      
      /* get render info from other node */
      render_info = (BinarySwapMsg*)incoming_msg;
      draw_x = render_info->start_x;
      draw_y = render_info->start_y;
      draw_width = render_info->width;
      draw_height = render_info->height;
      other_depth = render_info->depth;
      
      /* get incoming fb */
      incoming_color = (GLubyte*)((unsigned char*)incoming_msg + binaryswap_spu.offset);
      incoming_depth = (GLfloat*)(incoming_color + draw_width*draw_height*3);

      /* stupid stecil buffer tricks */

      /* mask portion to draw */
      binaryswap_spu.super.ColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
      binaryswap_spu.super.Enable( GL_STENCIL_TEST );
      binaryswap_spu.super.Enable( GL_DEPTH_TEST );
      binaryswap_spu.super.ClearStencil( 0 );
      binaryswap_spu.super.Clear( GL_STENCIL_BUFFER_BIT );
      binaryswap_spu.super.DepthFunc( GL_LESS );
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
    
    /* make sure everything got drawn for next pass */
    binaryswap_spu.super.Flush();

    /* more fun with clipped windowing... */
    if(binaryswap_spu.clipped_window){
      int end_x, end_y;
      int start_x, start_y;
      int temp;
      
      /* find optimal starting point */
      start_x = render_info->start_x;
      if(start_clipped_x < render_info->clipped_x){
	temp = start_clipped_x;
      }
      else{
	temp = render_info->clipped_x;
	if(render_info->clipped_width > 0 && render_info->clipped_height > 0)
	  start_clipped_x = temp;
      }
      if(temp > start_x){
	start_x = temp;
      }

      start_y = render_info->start_y;
      if(start_clipped_y < render_info->clipped_y){
	temp = start_clipped_y;
      }
      else{
	temp = render_info->clipped_y;
	if(render_info->clipped_width > 0 && render_info->clipped_height > 0)
	  start_clipped_y = temp;
      }
      if(temp > start_y){
	start_y = temp;
      }
           
      /* find optimal ending point */
      end_x = render_info->start_x + render_info->width - 1;
      if(end_clipped_x > (render_info->clipped_x + render_info->clipped_width - 1)){
	temp = end_clipped_x;
      }
      else{
	temp = (render_info->clipped_x + render_info->clipped_width - 1);
	if(render_info->clipped_width > 0 && render_info->clipped_height > 0)
	  end_clipped_x = temp;
      }
      if(end_x > temp){
	end_x = temp;
      }
     
      end_y = render_info->start_y + render_info->height - 1;
      if(end_clipped_y > (render_info->clipped_y + render_info->clipped_height - 1)){
	temp = end_clipped_y;
      }
      else{
	temp = (render_info->clipped_y + render_info->clipped_height - 1);
	if(render_info->clipped_width > 0 && render_info->clipped_height > 0)
	  end_clipped_y = temp;
      }
      if(end_y > temp){
	end_y = temp;
      }
      
      if((i+1) >= stages){
	/* setup final info */
	draw_x = start_x;
	draw_y = start_y;
	draw_width = end_x - start_x + 1;
	draw_height = end_y - start_y + 1;
      }
    }
    
    /* clean up the memory allocated for the recv */
    crNetFree( binaryswap_spu.peer_recv[i], incoming_msg );
  }
  
  if(binaryswap_spu.alpha_composite){
    binaryswap_spu.super.Disable( GL_BLEND );
  }

  /* send our final portion off to child */
  binaryswap_spu.super.ReadPixels( draw_x, draw_y, draw_width, draw_height, 
				   GL_RGB, GL_UNSIGNED_BYTE, 
				   binaryswap_spu.outgoing_msg + 
				   binaryswap_spu.offset ); 
  binaryswap_spu.child.Clear( GL_COLOR_BUFFER_BIT );
  binaryswap_spu.child.PixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  
  /* 
   * Make sure everyone has issued a clear to the child,
   * if not, then we'll clear what we are trying to draw... 
   */
  binaryswap_spu.child.BarrierExecCR( BINARYSWAP_CLEARBARRIER );
  
  binaryswap_spu.child.SemaphorePCR( MUTEX_SEMAPHORE );
  binaryswap_spu.child.MatrixMode(GL_PROJECTION);
  binaryswap_spu.child.LoadIdentity();
  binaryswap_spu.child.Ortho(0.0, (GLdouble) geometry[2], 0.0, 
			     (GLdouble) geometry[3], 10.0, -10.0);
  binaryswap_spu.child.MatrixMode(GL_MODELVIEW);
  binaryswap_spu.child.LoadIdentity();
  if(draw_width > 0 && draw_height > 0){
    binaryswap_spu.child.RasterPos2i( draw_x, draw_y );    
    binaryswap_spu.child.DrawPixels( draw_width, draw_height, 
				     GL_RGB, GL_UNSIGNED_BYTE, 
				     binaryswap_spu.outgoing_msg + 
				     binaryswap_spu.offset );
  }  
  binaryswap_spu.child.SemaphoreVCR( MUTEX_SEMAPHORE );
  binaryswap_spu.super.PixelStorei(GL_PACK_ALIGNMENT, packAlignment);
}


static void BINARYSWAPSPU_APIENTRY binaryswapspuSwapBuffers( GLint window, GLint flags )
{
  /* DoFlush will do the clear and the first barrier
   * if necessary. */
  
  DoFlush( &(binaryswap_spu.windows[window]) );
  
  binaryswap_spu.child.BarrierExecCR( BINARYSWAP_SWAPBARRIER );
   
  if(binaryswap_spu.node_num == 0){
    binaryswap_spu.child.SwapBuffers( binaryswap_spu.windows[window].childWindow, 0 );
    binaryswap_spu.child.Finish();
  }

  binaryswap_spu.cleared_this_frame = 0;
  
  (void) flags;
}


static GLint BINARYSWAPSPU_APIENTRY binaryswapspuCreateContext( const char *dpyName, 
								GLint visual)
{
  int i;
  
  CRASSERT(binaryswap_spu.child.BarrierCreateCR);
  
  /* find empty slot in contexts[] array */
  for (i = 0; i < MAX_CONTEXTS; i++) {
    if (!binaryswap_spu.contexts[i].inUse)
      break;
  }
  if (i == MAX_CONTEXTS) {
    crWarning("ran out of contexts in binaryswapspuCreateContext");
    return -1;
  }
   
  binaryswap_spu.contexts[i].inUse = GL_TRUE;
  binaryswap_spu.contexts[i].renderContext = binaryswap_spu.super.CreateContext(dpyName, 
										visual);
  binaryswap_spu.contexts[i].childContext  = binaryswap_spu.child.CreateContext(dpyName, 
										visual);
  
  /* create a state tracker (to record matrix operations) for this context */
  binaryswap_spu.contexts[i].tracker = crStateCreateContext( NULL );
  
  return i;
}


static void BINARYSWAPSPU_APIENTRY binaryswapspuDestroyContext( GLint ctx )
{
  /*	binaryswap_spu.child.BarrierCreateCR( DESTROY_CONTEXT_BARRIER, 0 );*/
  CRASSERT(ctx >= 0);
  CRASSERT(ctx < MAX_CONTEXTS);
  binaryswap_spu.super.DestroyContext(binaryswap_spu.contexts[ctx].renderContext);
  binaryswap_spu.contexts[ctx].inUse = GL_FALSE;
  crStateDestroyContext( binaryswap_spu.contexts[ctx].tracker );
}


static void BINARYSWAPSPU_APIENTRY binaryswapspuMakeCurrent(GLint window, 
							    GLint nativeWindow, 
							    GLint ctx)
{
  GET_THREAD(thread);
  
#ifdef CHROMIUM_THREADSAFE
  if (!thread) {
    thread = binaryswapspuNewThread( crThreadID() );
  }
#endif
  
  CRASSERT(thread);
  
  if (window >= 0 && ctx >= 0) {
    thread->currentWindow = window;
    thread->currentContext = ctx;
    CRASSERT(binaryswap_spu.windows[window].inUse);
    CRASSERT(binaryswap_spu.contexts[ctx].inUse);
    binaryswap_spu.super.MakeCurrent(binaryswap_spu.windows[window].renderWindow,
				     nativeWindow,
				     binaryswap_spu.contexts[ctx].renderContext);
    binaryswap_spu.child.MakeCurrent(binaryswap_spu.windows[window].childWindow,
				     nativeWindow,
				     binaryswap_spu.contexts[ctx].childContext);
    
    /* state tracker (for matrices) */
    crStateMakeCurrent( binaryswap_spu.contexts[ctx].tracker );
  }
  else {
    thread->currentWindow = -1;
    thread->currentContext = -1;
  }
}


static GLint BINARYSWAPSPU_APIENTRY binaryswapspuCreateWindow( const char *dpyName, 
							       GLint visBits )
{
  int i;
  
  /* find empty slot in windows[] array */
  for (i = 0; i < MAX_WINDOWS; i++) {
    if (!binaryswap_spu.windows[i].inUse)
      break;
  }
  if (i == MAX_WINDOWS) {
    crWarning("Ran out of windows in binaryswapspuCreateWindow");
    return -1;
  }
    
  binaryswap_spu.windows[i].inUse = GL_TRUE;
  binaryswap_spu.windows[i].renderWindow = binaryswap_spu.super.crCreateWindow( dpyName, 
										visBits );
  binaryswap_spu.windows[i].childWindow = 0;
  binaryswap_spu.windows[i].width = -1; /* unknown */
  binaryswap_spu.windows[i].height = -1; /* unknown */
  binaryswap_spu.windows[i].colorBuffer = NULL;
  binaryswap_spu.windows[i].depthBuffer = NULL;
  
  return i;
}

static void BINARYSWAPSPU_APIENTRY binaryswapspuDestroyWindow( GLint window )
{
  CRASSERT(window >= 0);
  CRASSERT(window < MAX_WINDOWS);
  binaryswap_spu.super.DestroyWindow( binaryswap_spu.windows[window].renderWindow );
  binaryswap_spu.windows[window].inUse = GL_FALSE;
}

static void BINARYSWAPSPU_APIENTRY binaryswapspuWindowSize( GLint window, 
							    GLint w, 
							    GLint h )
{
  CRASSERT(window == binaryswap_spu.renderWindow);
  binaryswap_spu.super.WindowSize( binaryswap_spu.renderWindow, w, h );
  binaryswap_spu.child.WindowSize( binaryswap_spu.childWindow, w, h );
}

/* don't implement WindowPosition() */


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
  { "Hint", (SPUGenericFunction) binaryswapspuHint },
  { "CreateContext", (SPUGenericFunction) binaryswapspuCreateContext },
  { "DestroyContext", (SPUGenericFunction) binaryswapspuDestroyContext },
  { "MakeCurrent", (SPUGenericFunction) binaryswapspuMakeCurrent },
  { "crCreateWindow", (SPUGenericFunction) binaryswapspuCreateWindow },
  { "DestroyWindow", (SPUGenericFunction) binaryswapspuDestroyWindow },
  { "WindowSize", (SPUGenericFunction) binaryswapspuWindowSize },
  { "ChromiumParametervCR", (SPUGenericFunction) binaryswapspuChromiumParametervCR },
  { NULL, NULL }
};
