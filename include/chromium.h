/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/* Public Chromium exports.
 * Applications are free to include this header file.
 */

#ifndef __CHROMIUM_H__
#define __CHROMIUM_H__


/**********************************************************************/
/*****             System includes and other cruft                *****/
/**********************************************************************/

/*
 * We effectively wrap gl.h, glu.h, etc, just like GLUT
 */

#define GL_GLEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#define WGL_APIENTRY __stdcall
#define CR_APIENTRY __stdcall
#include <windows.h>
#else
#include <GL/glx.h>
#define CR_APIENTRY
#endif

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef APIENTRY
#define APIENTRY
#endif


/**********************************************************************/
/*****     Define things that might have been missing in gl.h     *****/
/**********************************************************************/

/*
 * Define missing 1.2 tokens:
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
 * Define missing ARB_imaging tokens
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


typedef void (*CR_GLXFuncPtr)();
#ifndef GLX_ARB_get_proc_address
#define GLX_ARB_get_proc_address 1
CR_GLXFuncPtr glXGetProcAddressARB( const GLubyte *name );
#endif /* GLX_ARB_get_proc_address */


#ifdef WINDOWS
/* XXX how about this prototype for wglGetProcAddress()?
PROC WINAPI wglGetProcAddress_prox( LPCSTR name )
*/
#endif



/**********************************************************************/
/*****            Chromium Extensions to OpenGL                   *****/
/**********************************************************************/

#ifndef GL_CR_synchronization
#define GL_CR_synchronization 1

typedef void (APIENTRY *glBarrierCreateCRProc) (GLuint name, GLuint count);
typedef void (APIENTRY *glBarrierDestroyCRProc) (GLuint name);
typedef void (APIENTRY *glBarrierExecCRProc) (GLuint name);
typedef void (APIENTRY *glSemaphoreCreateCRProc) (GLuint name, GLuint count);
typedef void (APIENTRY *glSemaphoreDestroyCRProc) (GLuint name);
typedef void (APIENTRY *glSemaphorePCRProc) (GLuint name);
typedef void (APIENTRY *glSemaphoreVCRProc) (GLuint name);

#endif /* GL_CR_synchronization */


#ifndef GL_CR_bounds_info
#define GL_CR_bounds_info 1
/*
typedef void (APIENTRY *glBoundsInfoCRProc)(const GLrecti *, const GLbyte *, GLint, GLint);
*/
#endif /* GL_CR_bounds_info */


#ifndef GL_CR_state_parameter
#define GL_CR_state_parameter 1

typedef void (APIENTRY *glChromiumParameteriCRProc) (GLenum target, GLint value);
typedef void (APIENTRY *glChromiumParameterfCRProc) (GLenum target, GLfloat value);
typedef void (APIENTRY *glChromiumParametervCRProc) (GLenum target, GLenum type, GLsizei count, const GLvoid *values);
typedef void (APIENTRY *glGetChromiumParametervCRProc) (GLenum target, GLuint index, GLenum type, GLsizei count, GLvoid *values);

#endif /* GL_CR_state_parameter */


#ifndef GL_CR_cursor_position
#define GL_CR_cursor_position 1
/* For virtual cursor feature (show_cursor) */

#define GL_CURSOR_POSITION_CR  0x9900  /* unofficial! */

#endif /* GL_CR_cursor_position */


#ifndef GL_CR_bounding_box
#define GL_CR_bounding_box 1
/* To set bounding box from client app */

#define GL_DEFAULT_BBOX_CR	0x9901 /* unofficial! */
#define GL_SCREEN_BBOX_CR	0x9902 /* unofficial! */
#define GL_OBJECT_BBOX_CR	0x9903 /* unofficial! */

#endif /* GL_CR_bounding_box */


#ifndef GL_CR_print_string
#define GL_CR_print_string 1
/* To print a string to stdout */
#define GL_PRINT_STRING_CR	0x9904 /* unofficial! */

#endif /* GL_CR_print_string */


#ifndef GL_CR_tilesort_info
#define GL_CR_tilesort_info 1
/* To query tilesort information */

#define GL_MURAL_SIZE_CR             0x9905 /* unofficial! */
#define GL_NUM_SERVERS_CR            0x9906 /* unofficial! */
#define GL_NUM_TILES_CR              0x9907 /* unofficial! */
#define GL_TILE_BOUNDS_CR            0x9908 /* unofficial! */
#define GL_VERTEX_COUNTS_CR          0x9909 /* unofficial! */
#define GL_RESET_VERTEX_COUNTERS_CR  0x990A /* unofficial! */
#define GL_SET_MAX_VIEWPORT_CR       0x990B /* unofficial! */

#endif /* GL_CR_tilesort_info */


#ifndef GL_CR_head_spu_name
#define GL_CR_head_spu_name 1
/* To fetch name of first SPU on a node */

#define GL_HEAD_SPU_NAME_CR 0x990C /* unofficial! */

#endif /* GL_CR_head_spu_name */


#ifndef GL_CR_performance_info
#define GL_CR_performance_info 1
/* For gathering performance metrics */

#define GL_PERF_GET_FRAME_DATA_CR       0x990D /* unofficial! */
#define GL_PERF_GET_TIMER_DATA_CR       0x990E /* unofficial! */
#define GL_PERF_DUMP_COUNTERS_CR        0x990F /* unofficial! */
#define GL_PERF_SET_TOKEN_CR            0x9910 /* unofficial! */
#define GL_PERF_SET_DUMP_ON_SWAP_CR     0x9911 /* unofficial! */
#define GL_PERF_SET_DUMP_ON_FINISH_CR   0x9912 /* unofficial! */
#define GL_PERF_SET_DUMP_ON_FLUSH_CR    0x9913 /* unofficial! */
#define GL_PERF_START_TIMER_CR          0x9914 /* unofficial! */
#define GL_PERF_STOP_TIMER_CR           0x9915 /* unofficial! */

#endif /* GL_CR_performance_info */


#ifndef GL_CR_window_size
#define GL_CR_window_size 1
/* To communicate window size changes */

#define GL_WINDOW_SIZE_CR               0x9920 /* unofficial! */

#endif /* GL_CR_window_size */


#ifndef GL_CR_tile_info
#define GL_CR_tile_info 1
/* To send new tile information to a server */

#define GL_TILE_INFO_CR                 0x9921 /* unofficial! */

#endif /* GL_CR_tile_info */


#ifndef GL_CR_gather
#define GL_CR_gather 1
/* For aggregate transfers  */

#define GL_GATHER_DRAWPIXELS_CR         0x9922 /* unofficial! */
#define GL_GATHER_PACK_CR               0x9923 /* unofficial! */
#define GL_GATHER_CONNECT_CR            0x9924 /* unofficial! */
#define GL_GATHER_POST_SWAPBUFFERS_CR   0x9925 /* unofficial! */
#endif /* GL_CR_gather */


#ifndef GL_CR_saveframe
#define GL_CR_saveframe 1

#define GL_SAVEFRAME_ENABLED_CR  0x9926 /* unofficial! */
#define GL_SAVEFRAME_FRAMENUM_CR 0x9927 /* unofficial! */
#define GL_SAVEFRAME_STRIDE_CR   0x9928 /* unofficial! */
#define GL_SAVEFRAME_SINGLE_CR   0x9929 /* unofficial! */
#define GL_SAVEFRAME_FILESPEC_CR 0x992A /* unofficial! */

#endif /* GL_CR_saveframe */


#ifndef GL_CR_readback_barrier_size
#define GL_CR_readback_barrier_size 1

#define GL_READBACK_BARRIER_SIZE_CR 0x992B /* unofficial! */

#endif /* GL_CR_readback_barrier_size */




/**********************************************************************/
/*****                Chromium-specifid API                       *****/
/**********************************************************************/


/*
 * Accepted by crCreateContext() and crCreateWindow() visBits parameter.
 * Used to communicate visual attributes throughout Chromium.
 */
#define CR_RGB_BIT            0x1
#define CR_ALPHA_BIT          0x2
#define CR_DEPTH_BIT          0x4
#define CR_STENCIL_BIT        0x8
#define CR_ACCUM_BIT          0x10
#define CR_DOUBLE_BIT         0x20
#define CR_STEREO_BIT         0x40
#define CR_MULTISAMPLE_BIT    0x80


/* Accepted by crSwapBuffers() flag parameter */
#define CR_SUPPRESS_SWAP_BIT 0x1


typedef GLint (APIENTRY *crCreateContextProc)(const char *dpyName, GLint visBits);
typedef void (APIENTRY *crDestroyContextProc)(GLint context);
typedef void (APIENTRY *crMakeCurrentProc)(GLint window, GLint context);
typedef void (APIENTRY *crSwapBuffersProc)(GLint window, GLint flags);

typedef GLint (APIENTRY *crCreateWindowProc)(const char *dpyName, GLint visBits);
typedef void (APIENTRY *crDestroyWindowProc)(GLint window);
typedef void (APIENTRY *crWindowSizeProc)(GLint window, GLint w, GLint h);
typedef void (APIENTRY *crWindowPositionProc)(GLint window, GLint x, GLint y);

typedef int (CR_APIENTRY *CR_PROC)();
CR_PROC APIENTRY crGetProcAddress( const char *name );



/**********************************************************************/
/*****                 Other useful stuff                         *****/
/**********************************************************************/

#ifdef WINDOWS
#define GET_PROC(NAME) wglGetProcAddress((const GLbyte *) (NAME))
#elif defined(GLX_ARB_get_proc_address)
#define GET_PROC(NAME) glXGetProcAddressARB((const GLubyte *) (NAME))
#else
/* For SGI, etc that don't have glXGetProcAddress(). */
#define GET_PROC(NAME) NULL
#endif


#ifdef __cplusplus
}
#endif

#endif /* __CHROMIUM_H__ */
