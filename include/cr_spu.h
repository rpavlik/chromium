/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_SPU_H
#define CR_SPU_H

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
#elif defined(DARWIN)
/**
 * Apple/AGL
 */
/*@{*/
typedef AGLPixelFormat (*aglChoosePixelFormatFunc_t) (const AGLDevice *, GLint, const GLint *);
// typedef const char *(*glXQueryExtensionsStringFunc_t) (Display *, int );
typedef AGLContext (*aglCreateContextFunc_t)( AGLPixelFormat, AGLContext );
typedef GLboolean (*aglUseFontFunc_t)(AGLContext, GLint, Style, GLint, GLint, GLint, GLint);
typedef GLboolean (*aglDestroyContextFunc_t)( AGLContext );
typedef GLboolean (*aglSetCurrentContextFunc_t)( AGLContext );
typedef void (*aglSwapBuffersFunc_t)( AGLContext );
// typedef CR_GLXFuncPtr (*glXGetProcAddressARBFunc_t)( const GLubyte *name );
// typedef Display *(*glXGetCurrentDisplayFunc_t)( void );
typedef const GLubyte *(*glGetStringFunc_t)( GLenum );
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
#elif defined(DARWIN)
	aglChoosePixelFormatFunc_t aglChoosePixelFormat;
	aglCreateContextFunc_t aglCreateContext;
	aglUseFontFunc_t aglUseFont;
	aglDestroyContextFunc_t aglDestroyContext;
	aglSetCurrentContextFunc_t aglSetCurrentContext;
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
