/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */
#ifndef CR_SPU_H
#define CR_SPU_H

#ifdef DARWIN
#include <OpenGL/CGLTypes.h>
#endif

#ifdef WINDOWS
#define SPULOAD_APIENTRY __stdcall
#else
#define SPULOAD_APIENTRY
#endif

#include "cr_dll.h"
#include "spu_dispatch_table.h"
#include "cr_net.h"

#define SPU_ENTRY_POINT_NAME "SPULoad"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_THREADS               32      /**< max threads per spu */

typedef struct _SPUSTRUCT SPU;

typedef void (*SPUGenericFunction)(void);

/**
 * SPU Named function descriptor
 */
typedef struct {
	char *name;
	SPUGenericFunction fn;
} SPUNamedFunctionTable;

/**
 * SPU function table descriptor
 */
typedef struct {
	SPUDispatchTable *childCopy;
	void *data;
	SPUNamedFunctionTable *table;
} SPUFunctions;

/** 
 * SPU Option callback
 * \param spu
 * \param response
 */
typedef void (*SPUOptionCB)( void *spu, const char *response );

typedef enum { CR_BOOL, CR_INT, CR_FLOAT, CR_STRING, CR_ENUM } cr_type;

/**
 * SPU Options table
 */
typedef struct {
	const char *option;	/**< Name of the option */
	cr_type type; 		/**< Type of option */
	int numValues;  /**< usually 1 */
	const char *deflt;  /**< comma-separated string of [numValues] defaults */
	const char *min;    /**< comma-separated string of [numValues] minimums */
	const char *max;    /**< comma-separated string of [numValues] maximums */
	const char *description; /**< Textual description of the option */
	SPUOptionCB cb;		/**< Callback function */
} SPUOptions, *SPUOptionsPtr;


/** Init spu */
typedef SPUFunctions *(*SPUInitFuncPtr)(int id, SPU *child,
		SPU *super, unsigned int, unsigned int );
typedef void (*SPUSelfDispatchFuncPtr)(SPUDispatchTable *);
/** Cleanup spu */
typedef int (*SPUCleanupFuncPtr)(void);
/** Load spu */
typedef int (*SPULoadFunction)(char **, char **, void *, void *, void *,
			       SPUOptionsPtr *, int *);


/**
 * masks for spu_flags 
 */
#define SPU_PACKER_MASK           0x1
#define SPU_NO_PACKER             0x0
#define SPU_HAS_PACKER            0x1
#define SPU_TERMINAL_MASK         0x2
#define SPU_NOT_TERMINAL          0x0
#define SPU_IS_TERMINAL           0x2
#define SPU_MAX_SERVERS_MASK      0xc
#define SPU_MAX_SERVERS_ZERO      0x0
#define SPU_MAX_SERVERS_ONE       0x4
#define SPU_MAX_SERVERS_UNLIMITED 0x8


/**
 * SPU descriptor
 */
struct _SPUSTRUCT {
	char *name;			/**< Name of the spu */
	char *super_name;		/**< Name of the super class of the spu */
	int id;				/**< Id num of the spu */
        int spu_flags;			/**< options fags for the SPU */
	struct _SPUSTRUCT *superSPU;	/**< Pointer to the descriptor for the super class */
	CRDLL *dll;			/**< pointer to shared lib for spu */
	SPULoadFunction entry_point;	/**< SPU's entry point (SPULoad()) */
	SPUInitFuncPtr init;		/**< SPU init function */
	SPUSelfDispatchFuncPtr self;	/**< */
	SPUCleanupFuncPtr cleanup;	/**< SPU cleanup func */
	SPUFunctions *function_table;	/**< Function table for spu */
	SPUOptions *options;		/**< Options table */
	SPUDispatchTable dispatch_table;
	void *privatePtr;  		/**< pointer to SPU-private data */
};


/**
 * These are the OpenGL / window system interface functions
 */
#if defined(WINDOWS)
/**
 * Windows/WGL
 */
/*@{*/
typedef HGLRC (WGL_APIENTRY *wglCreateContextFunc_t)(HDC);
typedef void (WGL_APIENTRY *wglDeleteContextFunc_t)(HGLRC);
typedef BOOL (WGL_APIENTRY *wglMakeCurrentFunc_t)(HDC,HGLRC);
typedef BOOL (WGL_APIENTRY *wglSwapBuffersFunc_t)(HDC);
typedef int (WGL_APIENTRY *wglChoosePixelFormatFunc_t)(HDC, CONST PIXELFORMATDESCRIPTOR *);
typedef int (WGL_APIENTRY *wglDescribePixelFormatFunc_t)(HDC, int, UINT, CONST PIXELFORMATDESCRIPTOR *);
typedef int (WGL_APIENTRY *wglSetPixelFormatFunc_t)(HDC, int, CONST PIXELFORMATDESCRIPTOR *);
typedef HGLRC (WGL_APIENTRY *wglGetCurrentContextFunc_t)();
typedef PROC (WGL_APIENTRY *wglGetProcAddressFunc_t)();
typedef BOOL (WGL_APIENTRY *wglChoosePixelFormatEXTFunc_t)(HDC, const int *, const FLOAT *, UINT, int *, UINT *);
typedef BOOL (WGL_APIENTRY *wglGetPixelFormatAttribivEXTFunc_t)(HDC, int, int, UINT, int *, int *);
typedef BOOL (WGL_APIENTRY *wglGetPixelFormatAttribfvEXTFunc_t)(HDC, int, int, UINT, int *, int *);
typedef const GLubyte *(WGL_APIENTRY *glGetStringFunc_t)( GLenum );
typedef const GLubyte *(WGL_APIENTRY *wglGetExtensionsStringEXTFunc_t)( HDC );
/*@}*/
#else
/**
 * X11/GLX
 */
/*@{*/
typedef int (*glXGetConfigFunc_t)( Display *, XVisualInfo *, int, int * );
typedef Bool (*glXQueryExtensionFunc_t) (Display *, int *, int * );
typedef const char *(*glXQueryExtensionsStringFunc_t) (Display *, int );
typedef XVisualInfo *(*glXChooseVisualFunc_t)( Display *, int, int * );
typedef GLXContext (*glXCreateContextFunc_t)( Display *, XVisualInfo *, GLXContext, Bool );
typedef void (*glXUseXFontFunc_t)(Font font, int first, int count, int listBase);
typedef void (*glXDestroyContextFunc_t)( Display *, GLXContext );
typedef Bool (*glXIsDirectFunc_t)( Display *, GLXContext );
typedef Bool (*glXMakeCurrentFunc_t)( Display *, GLXDrawable, GLXContext );
typedef void (*glXSwapBuffersFunc_t)( Display *, GLXDrawable );
typedef CR_GLXFuncPtr (*glXGetProcAddressARBFunc_t)( const GLubyte *name );
typedef Display *(*glXGetCurrentDisplayFunc_t)( void );
typedef const GLubyte *(*glGetStringFunc_t)( GLenum );
typedef Bool (*glXJoinSwapGroupNVFunc_t)(Display *dpy, GLXDrawable drawable, GLuint group);
typedef Bool (*glXBindSwapBarrierNVFunc_t)(Display *dpy, GLuint group, GLuint barrier);
typedef Bool (*glXQuerySwapGroupNVFunc_t)(Display *dpy, GLXDrawable drawable, GLuint *group, GLuint *barrier);
typedef Bool (*glXQueryMaxSwapGroupsNVFunc_t)(Display *dpy, int screen, GLuint *maxGroups, GLuint *maxBarriers);
typedef Bool (*glXQueryFrameCountNVFunc_t)(Display *dpy, int screen, GLuint *count);
typedef Bool (*glXResetFrameCountNVFunc_t)(Display *dpy, int screen);
/*@}*/
#endif


#ifdef DARWIN
  typedef CGLError (*CGLChoosePixelFormatFunc_t)  (const CGLPixelFormatAttribute *attribs,
					       CGLPixelFormatObj *pix, long *npix);
  typedef CGLError (*CGLDestroyPixelFormatFunc_t)  (CGLPixelFormatObj pix);
  typedef CGLError (*CGLDescribePixelFormatFunc_t) (CGLPixelFormatObj pix, long pix_num, CGLPixelFormatAttribute attrib, long *value);
  typedef CGLError (*CGLQueryRendererInfoFunc_t) (unsigned long display_mask, CGLRendererInfoObj *rend, long *nrend);
  typedef CGLError (*CGLDestroyRendererInfoFunc_t) (CGLRendererInfoObj rend);
  typedef CGLError (*CGLDescribeRendererFunc_t) (CGLRendererInfoObj rend, long rend_num, CGLRendererProperty prop, long *value);
  typedef CGLError (*CGLCreateContextFunc_t) (CGLPixelFormatObj pix, CGLContextObj share, CGLContextObj *ctx);
  typedef CGLError (*CGLDestroyContextFunc_t) (CGLContextObj ctx);
  typedef CGLError (*CGLCopyContextFunc_t) (CGLContextObj src, CGLContextObj dst, unsigned long mask);
  typedef CGLError (*CGLSetCurrentContextFunc_t)(CGLContextObj ctx);
  typedef CGLContextObj (*CGLGetCurrentContextFunc_t)(void);
  typedef CGLError (*CGLCreatePBufferFunc_t) (long width, long height, unsigned long target, unsigned long internalFormat, long max_level, CGLPBufferObj *pbuffer);
  typedef CGLError (*CGLDestroyPBufferFunc_t) (CGLPBufferObj pbuffer);
  typedef CGLError (*CGLDescribePBufferFunc_t) (CGLPBufferObj obj, long *width, long *height, unsigned long *target, unsigned long *internalFormat, long *mipmap) ;
  typedef CGLError (*CGLTexImagePBufferFunc_t) (CGLContextObj ctx, CGLPBufferObj pbuffer, unsigned long source) ;
  typedef CGLError (*CGLSetOffScreenFunc_t) (CGLContextObj ctx, long width, long height, long rowbytes, void *baseaddr);
  typedef CGLError (*CGLGetOffScreenFunc_t)(CGLContextObj ctx, long *width, long *height, long *rowbytes, void **baseaddr);
  typedef CGLError (*CGLSetFullScreenFunc_t)(CGLContextObj ctx);
  typedef CGLError (*CGLSetPBufferFunc_t) (CGLContextObj ctx, CGLPBufferObj pbuffer, unsigned long face, long level, long screen) ;
  typedef CGLError (*CGLGetPBufferFunc_t) (CGLContextObj ctx, CGLPBufferObj *pbuffer, unsigned long *face, long *level, long *screen) ;
  typedef CGLError (*CGLClearDrawableFunc_t)(CGLContextObj ctx);
  typedef CGLError (*CGLFlushDrawableFunc_t)(CGLContextObj ctx);
  typedef CGLError (*CGLEnableFunc_t) (CGLContextObj ctx, CGLContextEnable pname);
  typedef CGLError (*CGLDisableFunc_t) (CGLContextObj ctx, CGLContextEnable pname);
  typedef CGLError (*CGLIsEnabledFunc_t) (CGLContextObj ctx, CGLContextEnable pname, long *enable);
  typedef CGLError (*CGLSetParameterFunc_t) (CGLContextObj ctx, CGLContextParameter pname, const long *params);
  typedef CGLError (*CGLGetParameterFunc_t) (CGLContextObj ctx, CGLContextParameter pname, long *params);
  typedef CGLError (*CGLSetVirtualScreenFunc_t) (CGLContextObj ctx, long screen);
  typedef CGLError (*CGLGetVirtualScreenFunc_t)(CGLContextObj ctx, long *screen);
  typedef CGLError (*CGLSetOptionFunc_t) (CGLGlobalOption pname, long param);
  typedef CGLError (*CGLGetOptionFunc_t) (CGLGlobalOption pname, long *param);
  typedef void (*CGLGetVersionFunc_t)(long *majorvers, long *minorvers);
  typedef char *(*CGLErrorStringFunc_t)(CGLError error);
#endif


/**
 * Package up the WGL/GLX function pointers into a struct.  We use
 * this in a few different places.
 */
typedef struct {
#if defined(WINDOWS)
	wglGetProcAddressFunc_t wglGetProcAddress;
	wglCreateContextFunc_t wglCreateContext;
	wglDeleteContextFunc_t wglDeleteContext;
	wglMakeCurrentFunc_t wglMakeCurrent;
	wglSwapBuffersFunc_t wglSwapBuffers;
	wglGetCurrentContextFunc_t wglGetCurrentContext;
	wglChoosePixelFormatFunc_t wglChoosePixelFormat;
	wglDescribePixelFormatFunc_t wglDescribePixelFormat;
	wglSetPixelFormatFunc_t wglSetPixelFormat;
	wglChoosePixelFormatEXTFunc_t wglChoosePixelFormatEXT;
	wglGetPixelFormatAttribivEXTFunc_t wglGetPixelFormatAttribivEXT;
	wglGetPixelFormatAttribfvEXTFunc_t wglGetPixelFormatAttribfvEXT;
	wglGetExtensionsStringEXTFunc_t wglGetExtensionsStringEXT;
	aglSwapBuffersFunc_t aglSwapBuffers;
#else
	glXGetConfigFunc_t  glXGetConfig;
	glXQueryExtensionFunc_t glXQueryExtension;
	glXQueryExtensionsStringFunc_t glXQueryExtensionsString;
	glXChooseVisualFunc_t glXChooseVisual;
	glXCreateContextFunc_t glXCreateContext;
	glXDestroyContextFunc_t glXDestroyContext;
	glXUseXFontFunc_t glXUseXFont;
	glXIsDirectFunc_t glXIsDirect;
	glXMakeCurrentFunc_t glXMakeCurrent;
	glXSwapBuffersFunc_t glXSwapBuffers;
	glXGetProcAddressARBFunc_t glXGetProcAddressARB;
	glXGetCurrentDisplayFunc_t glXGetCurrentDisplay;
	glXJoinSwapGroupNVFunc_t glXJoinSwapGroupNV;
	glXBindSwapBarrierNVFunc_t glXBindSwapBarrierNV;
	glXQuerySwapGroupNVFunc_t glXQuerySwapGroupNV;
	glXQueryMaxSwapGroupsNVFunc_t glXQueryMaxSwapGroupsNV;
	glXQueryFrameCountNVFunc_t glXQueryFrameCountNV;
	glXResetFrameCountNVFunc_t glXResetFrameCountNV;
#endif
#ifdef DARWIN
  CGLChoosePixelFormatFunc_t CGLChoosePixelFormat;
  CGLDestroyPixelFormatFunc_t CGLDestroyPixelFormat;
  CGLDescribePixelFormatFunc_t CGLDescribePixelFormat;
  CGLQueryRendererInfoFunc_t CGLQueryRendererInfo;
  CGLDestroyRendererInfoFunc_t CGLDestroyRendererInfo;
  CGLDescribeRendererFunc_t CGLDescribeRenderer;
  CGLCreateContextFunc_t CGLCreateContext;
  CGLDestroyContextFunc_t CGLDestroyContext;
  CGLSetCurrentContextFunc_t CGLSetCurrentContext;
  CGLCopyContextFunc_t CGLCopyContext;
  CGLGetCurrentContextFunc_t CGLGetCurrentContext;
  CGLCreatePBufferFunc_t CGLCreatePBuffer;
  CGLDestroyPBufferFunc_t CGLDestroyPBuffer;
  CGLDescribePBufferFunc_t CGLDescribePBuffer;
  CGLTexImagePBufferFunc_t CGLTexImagePBuffer;
  CGLSetPBufferFunc_t CGLSetPBuffer;
  CGLGetPBufferFunc_t CGLGetPBuffer;
  CGLEnableFunc_t CGLEnable;
  CGLDisableFunc_t CGLDisable;
  CGLIsEnabledFunc_t CGLIsEnabled;
  CGLSetParameterFunc_t CGLSetParameter;
  CGLGetParameterFunc_t CGLGetParameter;
  CGLSetOffScreenFunc_t CGLSetOffScreen;
  CGLGetOffScreenFunc_t CGLGetOffScreen;
  CGLSetFullScreenFunc_t CGLSetFullScreen;
  CGLClearDrawableFunc_t CGLClearDrawable;
  CGLFlushDrawableFunc_t CGLFlushDrawable;
  CGLSetVirtualScreenFunc_t CGLSetVirtualScreen;
  CGLGetVirtualScreenFunc_t CGLGetVirtualScreen;
  CGLSetOptionFunc_t CGLSetOption;
  CGLGetOptionFunc_t CGLGetOption;
  CGLGetVersionFunc_t CGLGetVersion;
  CGLErrorStringFunc_t CGLErrorString;
#endif
	glGetStringFunc_t glGetString;
} crOpenGLInterface;


/** This is the one required function in _all_ SPUs */
int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options, int *flags );

SPU *crSPULoad( SPU *child, int id, char *name, char *dir, void *server);
SPU *crSPULoadChain( int count, int *ids, char **names, char *dir, void *server );
void crSPUInitDispatchTable( SPUDispatchTable *table );
void crSPUCopyDispatchTable( SPUDispatchTable *dst, SPUDispatchTable *src );
void crSPUChangeInterface( SPUDispatchTable *table, void *origFunc, void *newFunc );


void crSPUSetDefaultParams( void *spu, SPUOptions *options );
void crSPUGetMothershipParams( CRConnection *conn, void *spu, SPUOptions *options );


SPUGenericFunction crSPUFindFunction( const SPUNamedFunctionTable *table, const char *fname );
void crSPUInitDispatch( SPUDispatchTable *dispatch, const SPUNamedFunctionTable *table );
void crSPUInitDispatchNops(SPUDispatchTable *table);

int crLoadOpenGL( crOpenGLInterface *crInterface, SPUNamedFunctionTable table[] );
void crUnloadOpenGL( void );
int crLoadOpenGLExtensions( const crOpenGLInterface *crInterface, SPUNamedFunctionTable table[] );

#ifdef USE_OSMESA
int crLoadOSMesa( OSMesaContext (**createContext)( GLenum format, OSMesaContext sharelist ), 
		  GLboolean (**makeCurrent)( OSMesaContext ctx, GLubyte *buffer, 
					     GLenum type, GLsizei width, GLsizei height ),
		  void (**destroyContext)( OSMesaContext ctx ));
#endif

#ifdef __cplusplus
}
#endif

#endif /* CR_SPU_H */
