/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/* Chromium sources include this file instead of including
 * the GL/gl.h and GL/glext.h headers directly.
 */

#ifndef CR_GLWRAPPER_H
#define CR_GLWRAPPER_H

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#define WGL_APIENTRY __stdcall
#include <windows.h>
#else
#include <GL/glx.h>
#endif

#include <GL/gl.h>
#include <GL/glext.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef WINDOWS

typedef void (*CR_GLXFuncPtr)();
CR_GLXFuncPtr glXGetProcAddressARB( const GLubyte *name );

#endif


/*
 * Define some OpenGL 1.2 tokens in case we're using an old gl.h header.
 */
#ifndef GL_SMOOTH_POINT_SIZE_RANGE
#define GL_SMOOTH_POINT_SIZE_RANGE		0x0B12
#endif

#ifndef GL_SMOOTH_POINT_SIZE_GRANULARITY
#define GL_SMOOTH_POINT_SIZE_GRANULARITY	0x0B13
#endif

#ifndef GL_SMOOTH_LINE_WIDTH_RANGE
#define GL_SMOOTH_LINE_WIDTH_RANGE		0x0B22
#endif

#ifndef GL_SMOOTH_LINE_WIDTH_GRANULARITY
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY	0x0B23
#endif

#ifndef GL_ALIASED_POINT_SIZE_RANGE
#define GL_ALIASED_POINT_SIZE_RANGE		0x846D
#endif

#ifndef GL_ALIASED_LINE_WIDTH_RANGE
#define GL_ALIASED_LINE_WIDTH_RANGE		0x846E
#endif

#ifndef GL_COLOR_MATRIX_STACK_DEPTH
#define GL_COLOR_MATRIX_STACK_DEPTH		0x80B2
#endif

#ifndef GL_COLOR_MATRIX
#define GL_COLOR_MATRIX				0x80B1
#endif

#ifndef GL_TEXTURE_3D
#define GL_TEXTURE_3D				0x806F
#endif

#ifndef GL_MAX_3D_TEXTURE_SIZE
#define GL_MAX_3D_TEXTURE_SIZE			0x8073
#endif

#ifndef GL_PACK_SKIP_IMAGES
#define GL_PACK_SKIP_IMAGES			0x806B
#endif

#ifndef GL_PACK_IMAGE_HEIGHT
#define GL_PACK_IMAGE_HEIGHT			0x806C
#endif

#ifndef GL_UNPACK_SKIP_IMAGES
#define GL_UNPACK_SKIP_IMAGES			0x806D
#endif

#ifndef GL_UNPACK_IMAGE_HEIGHT
#define GL_UNPACK_IMAGE_HEIGHT			0x806E
#endif

#ifndef GL_PROXY_TEXTURE_3D
#define GL_PROXY_TEXTURE_3D			0x8070
#endif

#ifndef GL_TEXTURE_DEPTH
#define GL_TEXTURE_DEPTH			0x8071
#endif

#ifndef GL_TEXTURE_WRAP_R
#define GL_TEXTURE_WRAP_R			0x8072
#endif

#ifndef GL_TEXTURE_BINDING_3D
#define GL_TEXTURE_BINDING_3D			0x806A
#endif

#ifndef GL_MAX_ELEMENTS_VERTICES
#define GL_MAX_ELEMENTS_VERTICES		0x80E8
#endif

#ifndef GL_MAX_ELEMENTS_INDICES
#define GL_MAX_ELEMENTS_INDICES			0x80E9
#endif


/*
 * Tokens for OpenGL 1.2's ARB_imaging subset
 */

#ifndef GL_BLEND_EQUATION
#define GL_BLEND_EQUATION			0x8009
#endif

#ifndef GL_MIN
#define GL_MIN					0x8007
#endif

#ifndef GL_MAX
#define GL_MAX					0x8008
#endif

#ifndef GL_FUNC_ADD
#define GL_FUNC_ADD				0x8006
#endif

#ifndef GL_FUNC_SUBTRACT
#define GL_FUNC_SUBTRACT			0x800A
#endif

#ifndef GL_FUNC_REVERSE_SUBTRACT
#define GL_FUNC_REVERSE_SUBTRACT		0x800B
#endif

#ifndef GL_BLEND_COLOR
#define GL_BLEND_COLOR				0x8005
#endif

#ifndef GL_PER_STAGE_CONSTANTS_NV
#define GL_PER_STAGE_CONSTANTS_NV 		0x8535
#endif


/*
 * Chromium extensions
 */

#ifndef GL_CR_state_parameter
#define GL_CR_state_parameter 1

extern void glChromiumParameteriCR(GLenum target, GLint value);
extern void glChromiumParameterfCR(GLenum target, GLfloat value);
extern void glChromiumParametervCR(GLenum target, GLenum type, GLsizei count, const GLvoid *values);
extern void glGetChromiumParametervCR(GLenum target, GLuint index, GLenum type, GLsizei count, GLvoid *values);

#endif /* GL_CR_state_parameter */


#ifndef GL_CR_cursor_position
#define GL_CR_cursor_position 1

#define GL_CURSOR_POSITION_CR  0x9900  /* unofficial! */

#endif /* GL_CR_cursor_position */


#ifndef GL_CR_bounding_box
#define GL_CR_bounding_box 1

#define GL_DEFAULT_BBOX_CR	0x9901 /* unofficial! */
#define GL_SCREEN_BBOX_CR	0x9902 /* unofficial! */
#define GL_OBJECT_BBOX_CR	0x9903 /* unofficial! */

#endif /* GL_CR_bounding_box */


#ifndef GL_CR_print_string
#define GL_CR_print_string 1

#define GL_PRINT_STRING_CR	0x9904 /* unofficial! */

#endif /* GL_CR_print_string */


#ifndef GL_CR_tilesort_info
#define GL_CR_tilesort_info 1

#define GL_MURAL_SIZE_CR             0x9905 /* unofficial! */
#define GL_NUM_SERVERS_CR            0x9906 /* unofficial! */
#define GL_NUM_TILES_CR              0x9907 /* unofficial! */
#define GL_TILE_BOUNDS_CR            0x9908 /* unofficial! */
#define GL_VERTEX_COUNTS_CR          0x9909 /* unofficial! */
#define GL_RESET_VERTEX_COUNTERS_CR  0x990A /* unofficial! */
#define GL_SET_MAX_VIEWPORT_CR       0x990B /* unofficial! */

#endif /* GL_CR_tilesort_info */


/*
 * These are the OpenGL / window system interface functions
 */
#ifdef WINDOWS
typedef HGLRC (WGL_APIENTRY *wglCreateContextFunc_t)(HDC);
typedef void (WGL_APIENTRY *wglDeleteContextFunc_t)(HGLRC);
typedef BOOL (WGL_APIENTRY *wglMakeCurrentFunc_t)(HDC,HGLRC);
typedef BOOL (WGL_APIENTRY *wglSwapBuffersFunc_t)(HDC);
typedef int (WGL_APIENTRY *wglChoosePixelFormatFunc_t)(HDC, CONST PIXELFORMATDESCRIPTOR *);
typedef int (WGL_APIENTRY *wglSetPixelFormatFunc_t)(HDC, int, CONST PIXELFORMATDESCRIPTOR *);
typedef HGLRC (WGL_APIENTRY *wglGetCurrentContextFunc_t)();
typedef PROC (WGL_APIENTRY *wglGetProcAddressFunc_t)();
#else
typedef int (*glXGetConfigFunc_t)( Display *, XVisualInfo *, int, int * );
typedef Bool (*glXQueryExtensionFunc_t) (Display *, int *, int * );
typedef XVisualInfo *(*glXChooseVisualFunc_t)( Display *, int, int * );
typedef GLXContext (*glXCreateContextFunc_t)( Display *, XVisualInfo *, GLXContext, Bool );
typedef void (*glXDestroyContextFunc_t)( Display *, GLXContext );
typedef Bool (*glXIsDirectFunc_t)( Display *, GLXContext );
typedef Bool (*glXMakeCurrentFunc_t)( Display *, GLXDrawable, GLXContext );
typedef const GLubyte *(*glGetStringFunc_t)( GLenum );
typedef void (*glXSwapBuffersFunc_t)( Display *, GLXDrawable );
typedef CR_GLXFuncPtr (*glXGetProcAddressARBFunc_t)( const GLubyte *name );
typedef Display *(*glXGetCurrentDisplayFunc_t)( void );
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
	wglSetPixelFormatFunc_t wglSetPixelFormat;
#else
	glXGetConfigFunc_t  glXGetConfig;
	glXQueryExtensionFunc_t glXQueryExtension;
	glXChooseVisualFunc_t glXChooseVisual;
	glXCreateContextFunc_t glXCreateContext;
	glXDestroyContextFunc_t glXDestroyContext;
	glXIsDirectFunc_t glXIsDirect;
	glXMakeCurrentFunc_t glXMakeCurrent;
	glGetStringFunc_t glGetString;
	glXSwapBuffersFunc_t glXSwapBuffers;
	glXGetProcAddressARBFunc_t glXGetProcAddressARB;
	glXGetCurrentDisplayFunc_t glXGetCurrentDisplay;
#endif
} crOpenGLInterface;


/* Used to communicate visual attributes throughout Chromium */
#define CR_RGB_BIT          0x1
#define CR_ALPHA_BIT        0x2
#define CR_DEPTH_BIT        0x4
#define CR_STENCIL_BIT      0x8
#define CR_ACCUM_BIT        0x10
#define CR_DOUBLE_BIT       0x20
#define CR_STEREO_BIT       0x40
#define CR_MULTISAMPLE_BIT  0x80

#define CR_MAX_CONTEXTS      512
#define CR_MAX_BITARRAY      (CR_MAX_CONTEXTS / 32) /* 32 contexts per uint */

#define MAX_THREADS		32	/* max threads per spu */


/* Function inlining */
#if defined(__GNUC__)
#  define INLINE __inline__
#elif defined(__MSC__)
#  define INLINE __inline
#elif defined(_MSC_VER)
#  define INLINE __inline
#elif defined(__ICL)
#  define INLINE __inline
#else
#  define INLINE
#endif

static INLINE void DIRTY( unsigned int *b, const unsigned int *d )
{
	int j;
	for (j=0;j<CR_MAX_BITARRAY;j++)
		b[j] = d[j];
}
static INLINE void FILLDIRTY( unsigned int *b )
{
	int j;
	for (j=0;j<CR_MAX_BITARRAY;j++)
		b[j] = 0xffffffff;
}
static INLINE void INVERTDIRTY( unsigned int *b, const unsigned int *d )
{
	int j;
	for (j=0;j<CR_MAX_BITARRAY;j++)
		b[j] &= d[j];
}
static INLINE int CHECKDIRTY( const unsigned int *b, const unsigned int *d )
{
	int j;

	for (j=0;j<CR_MAX_BITARRAY;j++)
		if (b[j] & d[j])
			return 1;

	return 0;
}


#ifdef __cplusplus
}
#endif


#endif /* CR_GLWRAPPER_H */
