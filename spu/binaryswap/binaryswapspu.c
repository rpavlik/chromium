/* Copyright (c) 2001, Stanford University
* All rights reserved
*
* See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_spu.h"
#include "cr_net.h"
#include "cr_mem.h"
#include "cr_error.h"
#include "cr_bbox.h"
#include "cr_applications.h"
#include "binaryswapspu.h"

#define WINDOW_MAGIC 7000
#define CONTEXT_MAGIC 8000

/*
 * Allocate the color and depth buffers needed for the glDraw/ReadPixels
 * commands for the given window.
 */
static void AllocBuffers( WindowInfo *window )
{
  CRASSERT(window);
  CRASSERT(window->width >= 0);
  CRASSERT(window->height >= 0);
  
  if (window->colorBuffer)
    crFree(window->colorBuffer);
  
  window->colorBuffer = (GLubyte *) crAlloc( window->width * window->height * 
					     4 * sizeof(GLubyte) );
  if (binaryswap_spu.extract_depth){
    GLint depthBytes;
    
    if (window->depthBuffer)
      crFree(window->depthBuffer);
    
    if (!window->depthType){
      /* Determine best type for the depth buffer image */
      GLint zBits;
      binaryswap_spu.super.GetIntegerv( GL_DEPTH_BITS, &zBits );
      if (zBits <= 16)
	window->depthType = GL_UNSIGNED_SHORT;
      else
	window->depthType = GL_FLOAT;
    }
    
    if (window->depthType == GL_UNSIGNED_SHORT){
      depthBytes = sizeof(GLushort);
    }
    else{
      CRASSERT(window->depthType == GL_FLOAT);
      depthBytes = sizeof(GLfloat);
    }
    
    window->depthBuffer = (GLfloat *) crAlloc( window->width * window->height
					       * depthBytes );
  }
}


static void CheckWindowSize( WindowInfo *window )
{
  GLint geometry[4];
  
  GLint w = window - binaryswap_spu.windows; /* pointer hack */
  CRASSERT(w >= 0);
  CRASSERT(w < MAX_WINDOWS);
  
  if ((binaryswap_spu.server) && (binaryswap_spu.server->numExtents)){
    /* no sense in reading the whole window if the tile 
     * only convers part of it..
     */
    geometry[2] = binaryswap_spu.server->extents[0].x2 - 
      binaryswap_spu.server->extents[0].x1;
    geometry[3] = binaryswap_spu.server->extents[0].y2 - 
      binaryswap_spu.server->extents[0].y1;
  }
  else{
    /* if the server is null, we are running on the 
     * app node, not a network node, so just readout
     * the whole shebang. if we dont have tiles, we're 
     * likely not doing sort-first., so do it all 
     */
    if (binaryswap_spu.resizable){
      /* ask downstream SPU (probably render) for its window size */
      GLint size[2];
      size[0] = size[1] = 0;
      binaryswap_spu.child.GetChromiumParametervCR(GL_WINDOW_SIZE_CR,
						   w, GL_INT, 2, size);
      if (size[0] == 0){
	/* something went wrong - recover */
	binaryswap_spu.super.GetIntegerv( GL_VIEWPORT, geometry );
      }
      else{
	geometry[2] = size[0];
	geometry[3] = size[1];
      }
    }
    else{
      /* not resizable - ask render SPU for viewport size */
      binaryswap_spu.super.GetIntegerv( GL_VIEWPORT, geometry );
    }
  }
  
  if (geometry[2] != window->width || geometry[3] != window->height){
    if (binaryswap_spu.resizable){
      /* update super/render SPU window size & viewport */
      binaryswap_spu.super.WindowSize( w, geometry[2], geometry[3] );
      binaryswap_spu.super.Viewport( 0, 0, geometry[2], geometry[3] );
      /* set child's viewport too */
      binaryswap_spu.child.Viewport( 0, 0, geometry[2], geometry[3] );
    }
    window->width = geometry[2];
    window->height = geometry[3];
    AllocBuffers(window);
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
  GLubyte* incoming_color;
  GLfloat* incoming_depth;
  CRMessage *incoming_msg;
  BinarySwapMsg *render_info;

  //GLfloat xmax = 0, xmin = 0, ymax = 0, ymin = 0;
  //int x, y, w, h;
  //GLint packAlignment, unpackAlignment;
  
  crDebug("############ swap");

  if (binaryswap_spu.resizable){
    /* check if window size changed, reallocate buffers if needed */
    CheckWindowSize( window );
  }
  
  if (first_time){
    CheckWindowSize( window );
    
    binaryswap_spu.child.BarrierCreate( BINARYSWAP_BARRIER, 0 );
    binaryswap_spu.child.LoadIdentity();
    binaryswap_spu.child.Ortho( 0, window->width - 1,	
				0, window->height - 1,
				-10000, 10000 );

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
  //binaryswap_spu.child.BarrierExec( BINARYSWAP_BARRIER );
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
  binaryswap_spu.child.BarrierExec( BINARYSWAP_BARRIER );
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
  
}


static void BINARYSWAPSPU_APIENTRY binaryswapspuSwapBuffers( GLint window, GLint flags )
{
  /* DoFlush will do the clear and the first barrier
   * if necessary. */
  
  DoFlush( &(binaryswap_spu.windows[window]) );
  
  binaryswap_spu.child.BarrierExec( BINARYSWAP_BARRIER );
   
  binaryswap_spu.child.SwapBuffers( binaryswap_spu.windows[window].childWindow, 0 );

  binaryswap_spu.child.Finish();

  if (binaryswap_spu.local_visualization){
	  binaryswap_spu.super.SwapBuffers( binaryswap_spu.windows[window].renderWindow, 0 );
  }
  binaryswap_spu.cleared_this_frame = 0;
  (void) flags;
}


static GLint BINARYSWAPSPU_APIENTRY binaryswapspuCreateContext( const char *dpyName, 
								GLint visual)
{
  int i;
  
  CRASSERT(binaryswap_spu.child.BarrierCreate);
  
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
  /*	binaryswap_spu.child.BarrierCreate( DESTROY_CONTEXT_BARRIER, 0 );*/
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
  GLint childVisual = visBits;
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
  
  /* If doing z-compositing, need stencil buffer */
  if (binaryswap_spu.extract_depth)
    childVisual |= CR_STENCIL_BIT;
  if (binaryswap_spu.extract_alpha)
    childVisual |= CR_ALPHA_BIT;
  
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
  { "CreateContext", (SPUGenericFunction) binaryswapspuCreateContext },
  { "DestroyContext", (SPUGenericFunction) binaryswapspuDestroyContext },
  { "MakeCurrent", (SPUGenericFunction) binaryswapspuMakeCurrent },
  { "crCreateWindow", (SPUGenericFunction) binaryswapspuCreateWindow },
  { "DestroyWindow", (SPUGenericFunction) binaryswapspuDestroyWindow },
  { "WindowSize", (SPUGenericFunction) binaryswapspuWindowSize },
  { "ChromiumParametervCR", (SPUGenericFunction) binaryswapspuChromiumParametervCR },
  { NULL, NULL }
};
