/* Copyright (c) 2001, Stanford University
 * All rights reserved
 * Author: Mike Houston
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*
 * TO DO:
 * 1) BBox code for better use of network transmit
 * 2) Check send/recv code to see if we can not send/recv recv/send
 * 3) Do alpha_depth compositing since depth and alpha work seperately.
 *    This will have lots of limitations, but will allow people to not
 *    have to specify a bounding box or a depth.
 * 4) We don't handle byte swapping correctly because a nice solution
 *    to doing swaped OOB has not been found yet.
 */

/* 
 * Possible bug list:
 *
 * We don't handle byte swapping!!!! (read the above)
 *
 * Alpha compositing:
 * 1) The hacky glHint method of passing info is problematic
 *    especially on systems that are not 32-bit.  What to do?
 * 2) There may be an issue with depth during the swap, should 
 *    we update our depth to the furthest forward of ourselves
 *    and our swap partner?
 *
 * Depth compositing:
 * 1) Is depth maintained in the swap for all drivers?
 * 2) Is using glOrtho with a depth of 1000000.0 deep enough?
 */

#include <stdio.h>
#include "cr_spu.h"
#include "binaryswapspu.h"
#include "cr_error.h"
#include "cr_net.h"
#include "cr_mem.h"

#define MTRANSFORM(x, y, z, w, m, vx, vy, vz) \
    x = m[0]*vx + m[4]*vy + m[8]*vz  + m[12]; \
    y = m[1]*vx + m[5]*vy + m[9]*vz  + m[13]; \
    z = m[2]*vx + m[6]*vy + m[10]*vz + m[14]; \
    w = m[3]*vx + m[7]*vy + m[11]*vz + m[15]

#define I_TRANSFORM(num, m, vx, vy, vz) \
    MTRANSFORM (x[num], y[num], z[num], w[num], m, vx, vy, vz)

#define FLT_MAX 3.402823466e+38f

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
  
  m[0] =  proj[0]*modl[0]  + proj[4]*modl[1]  + proj[8]*modl[2]   + proj[12]*modl[3];	
  m[1] =  proj[1]*modl[0]  + proj[5]*modl[1]  + proj[9]*modl[2]   + proj[13]*modl[3];	
  m[2] =  proj[2]*modl[0]  + proj[6]*modl[1]  + proj[10]*modl[2]  + proj[14]*modl[3];	
  m[3] =  proj[3]*modl[0]  + proj[7]*modl[1]  + proj[11]*modl[2]  + proj[15]*modl[3];	
  m[4] =  proj[0]*modl[4]  + proj[4]*modl[5]  + proj[8]*modl[6]   + proj[12]*modl[7];	
  m[5] =  proj[1]*modl[4]  + proj[5]*modl[5]  + proj[9]*modl[6]   + proj[13]*modl[7];	
  m[6] =  proj[2]*modl[4]  + proj[6]*modl[5]  + proj[10]*modl[6]  + proj[14]*modl[7];	
  m[7] =  proj[3]*modl[4]  + proj[7]*modl[5]  + proj[11]*modl[6]  + proj[15]*modl[7];	
  m[8] =  proj[0]*modl[8]  + proj[4]*modl[9]  + proj[8]*modl[10]  + proj[12]*modl[11];	
  m[9] =  proj[1]*modl[8]  + proj[5]*modl[9]  + proj[9]*modl[10]  + proj[13]*modl[11];	
  m[10] = proj[2]*modl[8]  + proj[6]*modl[9]  + proj[10]*modl[10] + proj[14]*modl[11];	
  m[11] = proj[3]*modl[8]  + proj[7]*modl[9]  + proj[11]*modl[10] + proj[15]*modl[11];	
  m[12] = proj[0]*modl[12] + proj[4]*modl[13] + proj[8]*modl[14]  + proj[12]*modl[15];	
  m[13] = proj[1]*modl[12] + proj[5]*modl[13] + proj[9]*modl[14]  + proj[13]*modl[15];	
  m[14] = proj[2]*modl[12] + proj[6]*modl[13] + proj[10]*modl[14] + proj[14]*modl[15];	
  m[15] = proj[3]*modl[12] + proj[7]*modl[13] + proj[11]*modl[14] + proj[15]*modl[15];     
  
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

float eyeDist (GLfloat x1, GLfloat y1, GLfloat z1,
	       GLfloat x2, GLfloat y2, GLfloat z2) 
{
  int i;
  
  GLfloat x[8], y[8], z[8], w[8];
  GLfloat vx, vy, vz;
  
  /* Tranform the point by m */
  I_TRANSFORM (0 , modl, x1, y1, z1);
  I_TRANSFORM (1 , modl, x1, y1, z2);
  I_TRANSFORM (2 , modl, x1, y2, z1);
  I_TRANSFORM (3 , modl, x1, y2, z2);
  I_TRANSFORM (4 , modl, x2, y1, z1);
  I_TRANSFORM (5 , modl, x2, y1, z2);
  I_TRANSFORM (6 , modl, x2, y2, z1);
  I_TRANSFORM (7 , modl, x2, y2, z2);
  
  vx = 0.0f;
  vy = 0.0f;
  vz = 0.0f;
  
  for (i=0; i<8; i++) {
    x[i] /= w[i];
    y[i] /= w[i];
    z[i] /= w[i];
    
    vx += x[i];
    vy += y[i];
    vz += z[i];
  }    
  
  vx /= 8.0f;
  vy /= 8.0f;
  vz /= 8.0f;
  
  return vx*vx + vy*vy + vz*vz;
}


/**************************************************************
 *
 * For now, this just strips the depth out of the box.  We should
 * also do a screen bounding box to lighten network traffic.
 *
 **************************************************************/
void setupBBox(void)
{
  glGetFloatv (GL_MODELVIEW_MATRIX,  modl);
  glGetFloatv (GL_PROJECTION_MATRIX, proj);
  
  binaryswap_spu.depth = eyeDist(binaryswap_spu.bounding_box->x1, 
				 binaryswap_spu.bounding_box->y1, 
				 binaryswap_spu.bounding_box->z1,
				 binaryswap_spu.bounding_box->x2, 
				 binaryswap_spu.bounding_box->y2, 
				 binaryswap_spu.bounding_box->z2);
}


/**************************************************************************
 *
 * Right now this is the only nice way to generically pass information into
 * Chromium.  Yes this is hacky...
 *
 **************************************************************************/
void BINARYSWAPSPU_APIENTRY binaryswapHint( GLenum target,
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
    /* figure out all of the info we need */
    setupBBox();
  }
  /* Pass on all other hints to child */
  else{
    binaryswap_spu.child.Hint( target, mode );
  }
}

/********************************************************
 *
 * The meat of the spu.
 *
 ********************************************************/
void BINARYSWAPSPU_APIENTRY binaryswapSwapBuffers( void )
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
  GLubyte* incoming_color;
  GLfloat* incoming_depth;
  CRMessage *incoming_msg;
  BinarySwapMsg *render_info;

  /* yes this is a little hacky.... */
  if(first_time){
    first_time = 0;
    binaryswap_spu.super.GetIntegerv( GL_VIEWPORT, geometry );
    if(binaryswap_spu.alpha_composite){
      binaryswap_spu.outgoing_msg = (GLubyte *) crAlloc( (geometry[2] * geometry[3] * 4)/2 
							 + sizeof(BinarySwapMsg));
    }
    else{
      binaryswap_spu.outgoing_msg = 
	(GLubyte *) crAlloc( sizeof(BinarySwapMsg) +
			     (geometry[2] * geometry[3] * 
			      (3 * sizeof(GLubyte) + sizeof(GLfloat))/2)); 
    }
    
    binaryswap_spu.offset = sizeof( BinarySwapMsg );
    ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->header.type = CR_MESSAGE_OOB;
    
    binaryswap_spu.child.BarrierCreate( BSWAP_BARRIER, 0 );

    stages = binaryswap_spu.stages;
    read_x = crAlloc(stages*sizeof(int));
    read_y = crAlloc(stages*sizeof(int));
    width = crAlloc(stages*sizeof(int));
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
  }

  /* setup positions so we can do a draw pixels correctly! */
  binaryswap_spu.super.MatrixMode(GL_PROJECTION);
  binaryswap_spu.super.LoadIdentity();
  binaryswap_spu.super.Ortho(0.0, (GLdouble) geometry[2], 0.0, 
			     (GLdouble) geometry[3], -10.0, 1000000.0);
  binaryswap_spu.super.MatrixMode(GL_MODELVIEW);
  binaryswap_spu.super.LoadIdentity();  

  /* blend other guy's stuff with ours */
  binaryswap_spu.super.Enable( GL_BLEND );

  /* make sure everyone is up to speed */
  binaryswap_spu.child.BarrierExec( BSWAP_BARRIER );
  /* figure out our portion for each stage */
  for(i=0; i<stages; i++){
    /* set up message header */
    ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->start_x = read_x[i];
    ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->start_y = read_y[i];
    ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->width   = width[i];
    ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->height  = height[i];
    ((BinarySwapMsg*) binaryswap_spu.outgoing_msg)->depth   = binaryswap_spu.depth;

    /* read our portion for this pass */
    /* figure out which mode to use, depth or alpha */
    if(binaryswap_spu.alpha_composite){
      binaryswap_spu.super.ReadPixels( read_x[i], read_y[i], width[i], height[i], 
				       GL_BGRA_EXT, GL_UNSIGNED_BYTE, 
				       (unsigned char*)binaryswap_spu.outgoing_msg +
				       binaryswap_spu.offset ); 

      /* lower of pair => recv,send */
      if(binaryswap_spu.highlow[i]){
	crNetGetMessage( binaryswap_spu.peer_recv[i], &incoming_msg);
	crNetSend( binaryswap_spu.peer_send[i], NULL, binaryswap_spu.outgoing_msg, 
		   (width[i]*height[i]*4) + binaryswap_spu.offset);
      }
      /* higher of pair => send,recv */
      else{
	crNetSend( binaryswap_spu.peer_send[i], NULL, binaryswap_spu.outgoing_msg, 
		   (width[i]*height[i]*4) + binaryswap_spu.offset);
	crNetGetMessage( binaryswap_spu.peer_recv[i], &incoming_msg);
      }
      
      /* get render info from other node */
      render_info = (BinarySwapMsg*)incoming_msg;
      draw_x = render_info->start_x;
      draw_y = render_info->start_y;
      draw_width = render_info->width;
      draw_height = render_info->height;
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
      binaryswap_spu.super.ReadPixels( read_x[i], read_y[i], width[i], height[i], 
				       GL_BGR_EXT, GL_UNSIGNED_BYTE, 
				       (unsigned char*)binaryswap_spu.outgoing_msg +
				       binaryswap_spu.offset ); 
      binaryswap_spu.super.ReadPixels( read_x[i], read_y[i], width[i], height[i], 
				       GL_DEPTH_COMPONENT, GL_FLOAT, 
				       (unsigned char*)binaryswap_spu.outgoing_msg + // base address
				       (width[i] * height[i] * 3) + // color information
				       binaryswap_spu.offset );  // message header

      /* lower of pair => recv,send */
      if(binaryswap_spu.highlow[i]){
	crNetGetMessage( binaryswap_spu.peer_recv[i], &incoming_msg);
	crNetSend( binaryswap_spu.peer_send[i], NULL, binaryswap_spu.outgoing_msg,  
		   width[i]*height[i]*(3+4) + binaryswap_spu.offset);
	
      }
      /* higher of pair => send,recv */
      else{
	crNetSend( binaryswap_spu.peer_send[i], NULL, binaryswap_spu.outgoing_msg, 
		  width[i]*height[i]*(3+4) + binaryswap_spu.offset);
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
  
  /* 
   * Make sure everyone has issued a clear to the child,
   * if not, then we'll clear what we are trying to draw... 
   */
  binaryswap_spu.child.BarrierExec( BSWAP_BARRIER );
  binaryswap_spu.child.MatrixMode(GL_PROJECTION);
  binaryswap_spu.child.LoadIdentity();
  binaryswap_spu.child.Ortho(0.0, (GLdouble) geometry[2], 0.0, 
			     (GLdouble) geometry[3], 10.0, -10.0);
  binaryswap_spu.child.MatrixMode(GL_MODELVIEW);
  binaryswap_spu.child.LoadIdentity();
  binaryswap_spu.child.RasterPos2i( draw_x, draw_y );
  
  binaryswap_spu.child.DrawPixels( draw_width, draw_height, 
				   GL_RGB, GL_UNSIGNED_BYTE, 
				   binaryswap_spu.outgoing_msg + 
				   binaryswap_spu.offset );
  
  
  /* Make sure everyone has drawn their pixels before we swap buffers */
  binaryswap_spu.child.BarrierExec( BSWAP_BARRIER );
  
  /* actually do the swap buffers */
  if(binaryswap_spu.node_num == 0){
    binaryswap_spu.child.SwapBuffers();
  }
  
  binaryswap_spu.super.SwapBuffers();
}

/**************************************************************
 *
 * Setup all of the functions that this SPU can handle
 *
 **************************************************************/
SPUNamedFunctionTable binaryswap_table[] = {
  { "SwapBuffers", (SPUGenericFunction) binaryswapSwapBuffers },
  { "Hint", (SPUGenericFunction) binaryswapHint },
  { NULL, NULL }
};







