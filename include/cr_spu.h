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
#include "state/cr_limits.h"
#include "cr_net.h"

#define SPU_ENTRY_POINT_NAME "SPULoad"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_THREADS               32      /* max threads per spu */

typedef struct _SPUSTRUCT SPU;

typedef void (*SPUGenericFunction)(void);

typedef struct {
	char *name;
	SPUGenericFunction fn;
} SPUNamedFunctionTable;

typedef struct {
	SPUDispatchTable *childCopy;
	void *data;
	SPUNamedFunctionTable *table;
} SPUFunctions;

typedef void (*SPUOptionCB)( void *spu, const char *response );

typedef struct {
	const char *option;
	enum { CR_BOOL, CR_INT, CR_FLOAT, CR_STRING, CR_ENUM } type; 
	int numValues;  /* usually 1 */
	const char *deflt;  /* comma-separated string of [numValues] defaults */
	const char *min;    /* comma-separated string of [numValues] minimums */
	const char *max;    /* comma-separated string of [numValues] maximums */
	const char *description;
	SPUOptionCB cb;
} SPUOptions, *SPUOptionsPtr;


typedef SPUFunctions *(*SPUInitFuncPtr)(int id, SPU *child,
		SPU *super, unsigned int, unsigned int );
typedef void (*SPUSelfDispatchFuncPtr)(SPUDispatchTable *);
typedef int (*SPUCleanupFuncPtr)(void);
typedef int (*SPULoadFunction)(char **, char **, void *, void *, void *,
			       SPUOptionsPtr *, int *);


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


struct _SPUSTRUCT {
	char *name;
	char *super_name;
	int id;
        int spu_flags;
	struct _SPUSTRUCT *superSPU;
	CRDLL *dll;
	SPULoadFunction entry_point;
	SPUInitFuncPtr init;
	SPUSelfDispatchFuncPtr self;
	SPUCleanupFuncPtr cleanup;
	SPUFunctions *function_table;
	SPUOptions *options;
	SPUDispatchTable dispatch_table;
	void *privatePtr;  /* pointer to SPU-private data */
};


/*
 * These are the OpenGL / window system interface functions
 */
#ifdef WINDOWS
typedef HGLRC (WGL_APIENTRY *wglCreateContextFunc_t)(HDC);
typedef void (WGL_APIENTRY *wglDeleteContextFunc_t)(HGLRC);
typedef BOOL (WGL_APIENTRY *wglMakeCurrentFunc_t)(HDC,HGLRC);
typedef BOOL (WGL_APIENTRY *wglSwapBuffersFunc_t)(HDC);
typedef int (WGL_APIENTRY *wglChoosePixelFormatFunc_t)(HDC, CONST PIXELFORMATDESCRIPTOR *);
typedef int (WGL_APIENTRY *wglDescribePixelFormatFunc_t)(HDC, int, UINT, CONST PIXELFORMATDESCRIPTOR *);
typedef int (WGL_APIENTRY *wglSetPixelFormatFunc_t)(HDC, int, CONST PIXELFORMATDESCRIPTOR *);
typedef HGLRC (WGL_APIENTRY *wglGetCurrentContextFunc_t)();
typedef PROC (WGL_APIENTRY *wglGetProcAddressFunc_t)();
typedef const GLubyte *(WGL_APIENTRY *glGetStringFunc_t)( GLenum );
#else
typedef int (*glXGetConfigFunc_t)( Display *, XVisualInfo *, int, int * );
typedef Bool (*glXQueryExtensionFunc_t) (Display *, int *, int * );
typedef XVisualInfo *(*glXChooseVisualFunc_t)( Display *, int, int * );
typedef GLXContext (*glXCreateContextFunc_t)( Display *, XVisualInfo *, GLXContext, Bool );
typedef void (*glXDestroyContextFunc_t)( Display *, GLXContext );
typedef Bool (*glXIsDirectFunc_t)( Display *, GLXContext );
typedef Bool (*glXMakeCurrentFunc_t)( Display *, GLXDrawable, GLXContext );
typedef void (*glXSwapBuffersFunc_t)( Display *, GLXDrawable );
typedef CR_GLXFuncPtr (*glXGetProcAddressARBFunc_t)( const GLubyte *name );
typedef Display *(*glXGetCurrentDisplayFunc_t)( void );
typedef const GLubyte *(*glGetStringFunc_t)( GLenum );
#endif

/*
 * Package up the WGL/GLX function pointers into a struct.  We use
 * this in a few different places.
 */
typedef struct {
#ifdef WINDOWS
	wglGetProcAddressFunc_t wglGetProcAddress;
	wglCreateContextFunc_t wglCreateContext;
	wglDeleteContextFunc_t wglDeleteContext;
	wglMakeCurrentFunc_t wglMakeCurrent;
	wglSwapBuffersFunc_t wglSwapBuffers;
	wglGetCurrentContextFunc_t wglGetCurrentContext;
	wglChoosePixelFormatFunc_t wglChoosePixelFormat;
	wglDescribePixelFormatFunc_t wglDescribePixelFormat;
	wglSetPixelFormatFunc_t wglSetPixelFormat;
#else
	glXGetConfigFunc_t  glXGetConfig;
	glXQueryExtensionFunc_t glXQueryExtension;
	glXChooseVisualFunc_t glXChooseVisual;
	glXCreateContextFunc_t glXCreateContext;
	glXDestroyContextFunc_t glXDestroyContext;
	glXIsDirectFunc_t glXIsDirect;
	glXMakeCurrentFunc_t glXMakeCurrent;
	glXSwapBuffersFunc_t glXSwapBuffers;
	glXGetProcAddressARBFunc_t glXGetProcAddressARB;
	glXGetCurrentDisplayFunc_t glXGetCurrentDisplay;
#endif
	glGetStringFunc_t glGetString;
} crOpenGLInterface;



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

void crSPUInitGLLimits( CRLimitsState *limits );
void crSPUCopyGLLimits( CRLimitsState *dest, const CRLimitsState *src );
void crSPUQueryGLLimits( CRConnection *conn, int spu_id, CRLimitsState *limits );
void crSPUReportGLLimits( const CRLimitsState *limits, int spu_id );
void crSPUGetGLLimits( const SPUNamedFunctionTable *table, CRLimitsState *limits );
void crSPUMergeGLLimits( int n, const CRLimitsState *limits, CRLimitsState *merged );
void crSPUPropogateGLLimits( CRConnection *conn, int spu_id, const SPU *child_spu, CRLimitsState *limitsResult );

int crLoadOpenGL( crOpenGLInterface *crInterface, SPUNamedFunctionTable table[] );
int crLoadOpenGLExtensions( const crOpenGLInterface *crInterface, SPUNamedFunctionTable table[] );

#ifdef __cplusplus
}
#endif

#endif /* CR_SPU_H */
