#include <stdlib.h>
#include <string.h>
#include <OpenGL/OpenGL.h>
#include "cr_error.h"
#include "cr_spu.h"
#include "stub.h"

/*
  Currently only glx apps are supported, so for now we simply
  return an error for every cgl call we get. 
*/

CGLError CGLChoosePixelFormat (const CGLPixelFormatAttribute *attribs,
			       CGLPixelFormatObj *pix,
			       long *nvirt)
{
  (void) attribs;
  (void) pix;
  (void) nvirt;

  return kCGLBadConnection; 
} 

CGLError CGLDestroyPixelFormat(CGLPixelFormatObj pix)
{
  (void) pix;
  return kCGLBadConnection;
}

CGLError CGLDescribePixelFormat(CGLPixelFormatObj pix, long pix_num, CGLPixelFormatAttribute attrib, long *value)
{
  (void) pix;
  (void) pix_num;
  (void) attrib; 
  (void) value;

  return kCGLBadConnection; 
}

CGLError CGLQueryRendererInfo(unsigned long display_mask, CGLRendererInfoObj *rend, long *nrend)
{
  (void) display_mask;
  (void) rend;
  (void) nrend;

  return kCGLBadConnection; 
}

CGLError CGLDestroyRendererInfo(CGLRendererInfoObj rend)
{
  (void) rend;

  return kCGLBadConnection;
}

CGLError CGLDescribeRenderer(CGLRendererInfoObj rend, long rend_num, CGLRendererProperty prop, long *value)
{
  (void) rend;
  (void) rend_num;
  (void) prop;
  (void) value;

  return kCGLBadConnection; 
}

/*
** Context functions
*/
CGLError CGLCreateContext(CGLPixelFormatObj pix, CGLContextObj share, CGLContextObj *ctx)
{
  (void) pix;
  (void) share;
  (void) ctx;

  return kCGLBadConnection; 
}


CGLError CGLDestroyContext(CGLContextObj ctx)
{
  (void) ctx;

  return kCGLBadConnection; 
}

CGLError CGLCopyContext(CGLContextObj src, CGLContextObj dst, unsigned long mask)
{
  (void) src;
  (void) dst;
  (void) mask;

  return kCGLBadConnection; 
}


CGLError CGLSetCurrentContext(CGLContextObj ctx)
{
  (void) ctx;

  return kCGLBadConnection; 
}

CGLContextObj CGLGetCurrentContext(void)
{
  return NULL;
}


/*
** PBuffer functions
*/
CGLError CGLCreatePBuffer(long width, long height, unsigned long target, unsigned long internalFormat, long max_level, CGLPBufferObj *pbuffer) 
{
  (void) width;
  (void) height;
  (void) target;
  (void) internalFormat;
  (void) max_level;
  (void) pbuffer;

  return kCGLBadConnection; 
}

CGLError CGLDestroyPBuffer(CGLPBufferObj pbuffer) 
{
  (void) pbuffer;

  return kCGLBadConnection; 
}

CGLError CGLDescribePBuffer(CGLPBufferObj obj, long *width, long *height, unsigned long *target, unsigned long *internalFormat, long *mipmap) 
{
  (void) obj;
  (void) width;
  (void) height;
  (void) target;
  (void) internalFormat;
  (void) mipmap;

  return kCGLBadConnection; 
}

CGLError CGLTexImagePBuffer(CGLContextObj ctx, CGLPBufferObj pbuffer, unsigned long source) 
{
  (void) ctx;
  (void) pbuffer;
  (void) source;

  return kCGLBadConnection;
}

/*
** Drawable Functions
*/
CGLError CGLSetOffScreen(CGLContextObj ctx, long width, long height, long rowbytes, void *baseaddr)
{
  (void) ctx;
  (void) width;
  (void) height;
  (void) rowbytes;
  (void) baseaddr;

  return kCGLBadConnection; 
}

CGLError CGLGetOffScreen(CGLContextObj ctx, long *width, long *height, long *rowbytes, void **baseaddr)
{
  (void) ctx;
  (void) width;
  (void) height;
  (void) rowbytes;
  (void) baseaddr;

  return kCGLBadConnection;
}

CGLError CGLSetFullScreen(CGLContextObj ctx)
{
  (void) ctx;

  return kCGLBadConnection;
}

CGLError CGLSetPBuffer(CGLContextObj ctx, CGLPBufferObj pbuffer, unsigned long face, long level, long screen) 
{
  (void) ctx;
  (void) pbuffer;
  (void) face;
  (void) level;
  (void) screen;

  return kCGLBadConnection; 
}

CGLError CGLGetPBuffer(CGLContextObj ctx, CGLPBufferObj *pbuffer, unsigned long *face, long *level, long *screen) 
{
  (void) ctx;
  (void) pbuffer;
  (void) face;
  (void) level;
  (void) screen;

  return kCGLBadConnection; 
}

CGLError CGLClearDrawable(CGLContextObj ctx)
{
  (void) ctx;

  return kCGLBadConnection; 
}

CGLError CGLFlushDrawable(CGLContextObj ctx)
{
  (void) ctx;

  return kCGLBadConnection; 
}

/*
** Per context enables and parameters
*/
CGLError CGLEnable(CGLContextObj ctx, CGLContextEnable pname)
{
  (void) ctx;
  (void) pname;

  return kCGLBadConnection;
}

CGLError CGLDisable(CGLContextObj ctx, CGLContextEnable pname)
{
  (void) ctx;
  (void) pname;

  return kCGLBadConnection; 
}

CGLError CGLIsEnabled(CGLContextObj ctx, CGLContextEnable pname, long *enable)
{
  (void) ctx;
  (void) pname;
  (void) enable;

  return kCGLBadConnection;
}

CGLError CGLSetParameter(CGLContextObj ctx, CGLContextParameter pname, const long *params)
{
  (void) ctx;
  (void) pname;
  (void) params;

  return kCGLBadConnection;
}

CGLError CGLGetParameter(CGLContextObj ctx, CGLContextParameter pname, long *params)
{
  (void) ctx;
  (void) pname;
  (void) params;

  return kCGLBadConnection; 
}

/*
** Virtual screen functions
*/
CGLError CGLSetVirtualScreen(CGLContextObj ctx, long screen)
{
  (void) ctx;
  (void) screen;

  return kCGLBadConnection; 
}

CGLError CGLGetVirtualScreen(CGLContextObj ctx, long *screen)
{
  (void) ctx;
  (void) screen;

  return kCGLBadConnection;
}

/*
** Global library options
*/
CGLError CGLSetOption(CGLGlobalOption pname, long param)
{
  (void) pname;
  (void) param;

  return kCGLBadConnection;
}

CGLError CGLGetOption(CGLGlobalOption pname, long *param)
{
  (void) pname;
  (void) param;

  return kCGLBadConnection;
}

/*
** Version numbers
*/
void CGLGetVersion(long *majorvers, long *minorvers)
{
  *majorvers=1;
  *minorvers=4;
}

/*
** Convert an error code to a string
*/
const char *errString = "The operation is not supported by Chromium";

const char *CGLErrorString(CGLError error)
{
  return errString; 
}


