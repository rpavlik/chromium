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
 * Define missing GLX tokens:
 */

#ifndef GLX_SAMPLE_BUFFERS_SGIS
#define GLX_SAMPLE_BUFFERS_SGIS    0x186a0 /*100000*/
#endif
#ifndef GLX_SAMPLES_SGIS
#define GLX_SAMPLES_SGIS           0x186a1 /*100001*/
#endif
#ifndef GLX_VISUAL_CAVEAT_EXT
#define GLX_VISUAL_CAVEAT_EXT       0x20  /* visual_rating extension type */
#endif

/*
 * Define missing WGL tokens:
 */
#ifndef WGL_COLOR_BITS_EXT
#define WGL_COLOR_BITS_EXT			0x2014
#endif
#ifndef WGL_DRAW_TO_WINDOW_EXT
#define WGL_DRAW_TO_WINDOW_EXT			0x2001
#endif
#ifndef WGL_FULL_ACCELERATION_EXT
#define WGL_FULL_ACCELERATION_EXT		0x2027
#endif
#ifndef WGL_ACCELERATION_EXT
#define WGL_ACCELERATION_EXT			0x2003
#endif
#ifndef WGL_TYPE_RGBA_EXT
#define WGL_TYPE_RGBA_EXT			0x202B
#endif
#ifndef WGL_RED_BITS_EXT
#define WGL_RED_BITS_EXT			0x2015
#endif
#ifndef WGL_GREEN_BITS_EXT
#define WGL_GREEN_BITS_EXT			0x2017
#endif
#ifndef WGL_BLUE_BITS_EXT
#define WGL_BLUE_BITS_EXT			0x2019
#endif
#ifndef WGL_ALPHA_BITS_EXT
#define WGL_ALPHA_BITS_EXT			0x201B
#endif
#ifndef WGL_DOUBLE_BUFFER_EXT
#define WGL_DOUBLE_BUFFER_EXT			0x2011
#endif
#ifndef WGL_STEREO_EXT
#define WGL_STEREO_EXT				0x2012
#endif
#ifndef WGL_ACCUM_RED_BITS_EXT
#define WGL_ACCUM_RED_BITS_EXT			0x201E
#endif
#ifndef WGL_ACCUM_GREEN_BITS_EXT
#define WGL_ACCUM_GREEN_BITS_EXT		0x201F
#endif
#ifndef WGL_ACCUM_BLUE_BITS_EXT
#define WGL_ACCUM_BLUE_BITS_EXT			0x2020
#endif
#ifndef WGL_ACCUM_ALPHA_BITS_EXT
#define WGL_ACCUM_ALPHA_BITS_EXT		0x2021
#endif
#ifndef WGL_DEPTH_BITS_EXT
#define WGL_DEPTH_BITS_EXT			0x2022
#endif
#ifndef WGL_STENCIL_BITS_EXT
#define WGL_STENCIL_BITS_EXT			0x2023
#endif
#ifndef WGL_SAMPLE_BUFFERS_EXT
#define WGL_SAMPLE_BUFFERS_EXT			0x2041
#endif
#ifndef WGL_SAMPLES_EXT
#define WGL_SAMPLES_EXT				0x2042
#endif

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

#ifndef GL_FOG_COORDINATE_ARRAY_POINTER_EXT
#define GL_FOG_COORDINATE_ARRAY_POINTER_EXT     0x8456
#endif

typedef void (*CR_GLXFuncPtr)();
#ifndef GLX_ARB_get_proc_address
#define GLX_ARB_get_proc_address 1
CR_GLXFuncPtr glXGetProcAddressARB( const GLubyte *name );
#endif /* GLX_ARB_get_proc_address */

#ifndef GLX_VERSION_1_4
CR_GLXFuncPtr glXGetProcAddress( const GLubyte *name );
#endif /* GLX_ARB_get_proc_address */

#ifndef GL_RASTER_POSITION_UNCLIPPED_IBM
#define GL_RASTER_POSITION_UNCLIPPED_IBM  0x19262
#endif

#ifdef WINDOWS
/* XXX how about this prototype for wglGetProcAddress()?
PROC WINAPI wglGetProcAddress_prox( LPCSTR name )
*/
#endif



/**********************************************************************/
/*****            Chromium Extensions to OpenGL                   *****/
/*****                                                            *****/
/***** Chromium owns the OpenGL enum range 0x8AF0-0x8B2F          *****/
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
/* Private, internal Chromium function */
/*
typedef void (APIENTRY *glBoundsInfoCRProc)(const CRrecti *, const GLbyte *, GLint, GLint);
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

#define GL_CURSOR_POSITION_CR  0x8AF0

#endif /* GL_CR_cursor_position */


#ifndef GL_CR_bounding_box
#define GL_CR_bounding_box 1
/* To set bounding box from client app */

#define GL_DEFAULT_BBOX_CR	0x8AF1
#define GL_SCREEN_BBOX_CR	0x8AF2
#define GL_OBJECT_BBOX_CR	0x8AF3

#endif /* GL_CR_bounding_box */


#ifndef GL_CR_print_string
#define GL_CR_print_string 1
/* To print a string to stdout */
#define GL_PRINT_STRING_CR	0x8AF4

#endif /* GL_CR_print_string */


#ifndef GL_CR_tilesort_info
#define GL_CR_tilesort_info 1
/* To query tilesort information */

#define GL_MURAL_SIZE_CR             0x8AF5
#define GL_NUM_SERVERS_CR            0x8AF6
#define GL_NUM_TILES_CR              0x8AF7
#define GL_TILE_BOUNDS_CR            0x8AF8
#define GL_VERTEX_COUNTS_CR          0x8AF9
#define GL_RESET_VERTEX_COUNTERS_CR  0x8AFA
#define GL_SET_MAX_VIEWPORT_CR       0x8AFB

#endif /* GL_CR_tilesort_info */


#ifndef GL_CR_head_spu_name
#define GL_CR_head_spu_name 1
/* To fetch name of first SPU on a node */

#define GL_HEAD_SPU_NAME_CR         0x8AFC

#endif /* GL_CR_head_spu_name */


#ifndef GL_CR_performance_info
#define GL_CR_performance_info 1
/* For gathering performance metrics */

#define GL_PERF_GET_FRAME_DATA_CR       0x8AFD
#define GL_PERF_GET_TIMER_DATA_CR       0x8AFE
#define GL_PERF_DUMP_COUNTERS_CR        0x8AFF
#define GL_PERF_SET_TOKEN_CR            0x8B00
#define GL_PERF_SET_DUMP_ON_SWAP_CR     0x8B01
#define GL_PERF_SET_DUMP_ON_FINISH_CR   0x8B02
#define GL_PERF_SET_DUMP_ON_FLUSH_CR    0x8B03
#define GL_PERF_START_TIMER_CR          0x8B04
#define GL_PERF_STOP_TIMER_CR           0x8B05

#endif /* GL_CR_performance_info */


#ifndef GL_CR_window_size
#define GL_CR_window_size 1
/* To communicate window size changes */

#define GL_WINDOW_SIZE_CR               0x8B06

#endif /* GL_CR_window_size */


#ifndef GL_CR_tile_info
#define GL_CR_tile_info 1
/* To send new tile information to a server */

#define GL_TILE_INFO_CR                 0x8B07

#endif /* GL_CR_tile_info */


#ifndef GL_CR_gather
#define GL_CR_gather 1
/* For aggregate transfers  */

#define GL_GATHER_DRAWPIXELS_CR         0x8B08
#define GL_GATHER_PACK_CR               0x8B09
#define GL_GATHER_CONNECT_CR            0x8B0A
#define GL_GATHER_POST_SWAPBUFFERS_CR   0x8B0B

#endif /* GL_CR_gather */


#ifndef GL_CR_saveframe
#define GL_CR_saveframe 1

#define GL_SAVEFRAME_ENABLED_CR         0x8B0C
#define GL_SAVEFRAME_FRAMENUM_CR        0x8B0D
#define GL_SAVEFRAME_STRIDE_CR          0x8B0E
#define GL_SAVEFRAME_SINGLE_CR          0x8B0F
#define GL_SAVEFRAME_FILESPEC_CR        0x8B10

#endif /* GL_CR_saveframe */


#ifndef GL_CR_readback_barrier_size
#define GL_CR_readback_barrier_size 1

#define GL_READBACK_BARRIER_SIZE_CR     0x8B11

#endif /* GL_CR_readback_barrier_size */


#ifndef GL_CR_server_id_sharing
#define GL_CR_server_id_sharing 1

#define GL_SHARED_DISPLAY_LISTS_CR      0x8B12
#define GL_SHARED_TEXTURE_OBJECTS_CR    0x8B13
#define GL_SHARED_PROGRAMS_CR           0x8B14

#endif /* GL_CR_server_id_sharing */



/**********************************************************************/
/*****                Chromium-specific API                       *****/
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
#define CR_OVERLAY_BIT        0x100
#define CR_ALL_BITS           0x1ff


/* Accepted by crSwapBuffers() flag parameter */
#define CR_SUPPRESS_SWAP_BIT 0x1


typedef GLint (APIENTRY *crCreateContextProc)(const char *dpyName, GLint visBits);
typedef void (APIENTRY *crDestroyContextProc)(GLint context);
typedef void (APIENTRY *crMakeCurrentProc)(GLint window, GLint context);
typedef GLint (APIENTRY *crGetCurrentContextProc)(void);
typedef GLint (APIENTRY *crGetCurrentWindowProc)(void);
typedef void (APIENTRY *crSwapBuffersProc)(GLint window, GLint flags);

typedef GLint (APIENTRY *crWindowCreateProc)(const char *dpyName, GLint visBits);
typedef void (APIENTRY *crWindowDestroyProc)(GLint window);
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


/**********************************************************************/
/*****                           PICA                             *****/
/**********************************************************************/

#ifndef CR_PICA
#define CR_PICA 1

#define PICA_OPENGL_FRAMELET           1
#define PICA_PREFERRED_FORMAT          2
#define PICA_NUM_PREFERRED_FORMAT      3
#define PICA_LOCAL_MAX_WIDTH           4
#define PICA_LOCAL_MAX_HEIGHT          5
#define PICA_COMPOSITOR_ID             6
#define PICA_RED_SIZE                  7
#define PICA_GREEN_SIZE                8
#define PICA_BLUE_SIZE                 9
#define PICA_ALPHA_SIZE               10
#define PICA_DEPTH_SIZE               11
#define PICA_STENCIL_SIZE             12
#define PICA_ACCUM_SIZE               13
#define PICA_FRAME_MAX_WIDTH          15
#define PICA_FRAME_MAX_HEIGHT         16
#define PICA_CONFIGURE_END            17
#define PICA_FRAME_MAXIMUM_SIZE       18
#define PICA_NUM_FRAG_CHANNELS        21
#define PICA_WINDOWED                 22
#define PICA_SUB_RECT_FRAMELETS       23
#define PICA_MAX_NUM_FRAME_FRAMELETS  24
#define PICA_NONVOLATILE_BUFFER       25
#define PICA_VOLATILE_FRAMELETS       26
#define PICA_READBACK                 27
#define PICA_DYNAMIC_STAGE_PARAMETERS 28
#define PICA_CONTEXT_ISGLOBAL         29
#define PICA_GLOBAL_CONTEXT           30
#define PICA_FRAMELET_CHANNEL         32
#define PICA_STAGE_NUMBER             33
#define PICA_MAX_STAGES               34
#define PICA_MAX_ORDERS_PER_STAGE     35
#define PICA_FRAMELET_ZOOM_X          36
#define PICA_FRAMELET_ZOOM_Y          37
#define PICA_FRAMELET_STEP_X          38
#define PICA_FRAMELET_STEP_Y          39
#define PICA_FRAMELET_STEP            40
#define PICA_FRAMELET_ZOOM            41
#define PICA_XWINDOW                  42
#define PICA_FRAME_WIDTH              43
#define PICA_FRAME_HEIGHT             44 
#define PICA_LOCAL_HEIGHT             45
#define PICA_LOCAL_WIDTH              46
#define PICA_WIN_RECONFIG             47
#define PICA_EXTENSIONS               48
#define PICA_LOCAL_FRAMELET_RESTRICTED_RECTS 49
#define PICA_LOCAL_FRAMELET_LEFT      50
#define PICA_LOCAL_FRAMELET_RIGHT     51
#define PICA_LOCAL_FRAMELET_TOP       52
#define PICA_LOCAL_FRAMELET_BOTTOM    53
#define PICA_LOCAL_ORDER_LIMITS       54
#define PICA_LOCAL_ORDER_MIN          55
#define PICA_LOCAL_ORDER_MAX          56
#define PICA_CAVEAT_VALUE             57
#define PICA_FORMAT_COLOR_R8G8B8A8    58
#define PICA_FORMAT_COLOR_A8B8G8R8    59
#define PICA_FORMAT_COLOR_R8G8B8      60
#define PICA_FORMAT_COLOR_B8G8R8      61
#define PICA_FORMAT_COLOR_R4G4B4A4    62
#define PICA_FORMAT_COLOR_A4B4G4R4    63
#define PICA_FORMAT_COLOR_R5G5B5A1    64
#define PICA_FORMAT_COLOR_A1B5G5R5    65
#define PICA_FORMAT_COLOR_R5G5B5      66
#define PICA_FORMAT_COLOR_B5G5R5      67
#define PICA_FORMAT_COLOR_RGBA_FLOAT  68
#define PICA_FORMAT_COLOR_RGBA_HALF_FLOAT 69 
#define PICA_FORMAT_DEPTH_Z_FLOAT    70
#define PICA_FORMAT_DEPTH_Z_INT16    71
#define PICA_FORMAT_DEPTH_Z_UINT16   72
#define PICA_FORMAT_DEPTH_Z_INT32    73
#define PICA_FORMAT_DEPTH_Z_UINT32   74
#define PICA_ERR_NO_ERROR            75
#define PICA_ERR_BAD_ARGS            76
#define PICA_ERR_WIN_RECONFIG        77
#define PICA_STATUS_COMPLETE         78
#define PICA_STATUS_ERROR            79
#define PICA_STATUS_CANCELLED        80
#define PICA_STATUS_TIMEOUT          81
#define PICA_FRAMELET_READONLY       82
#define PICA_FRAMELET_VOLATILE       83

typedef unsigned char PICAboolean;
typedef int           PICAint;
typedef unsigned int  PICAuint;
typedef unsigned long PICAulong;
typedef float         PICAfloat;
typedef unsigned char PICAchar;
typedef void          PICAvoid;
typedef int           PICAcompID;
typedef unsigned long PICAnodeID;
typedef unsigned long PICAcontextID;
typedef int           PICAerror;
typedef int           PICAstatus;
typedef int           PICAframeID;
typedef int           PICAparam;

typedef struct{
     const PICAchar *name;
     PICAcompID id;
}PICAcompItem;

typedef struct{
     const PICAchar *name;
     PICAnodeID id;
     PICAint bIsRequired;
}PICAnodeItem;

typedef struct{
     PICAint x,y;
}PICApoint;

typedef struct{
     PICAint x1,x2,y1,y2;
}PICArect;

typedef PICAerror (APIENTRY *crPicaListCompositorsProc)(const PICAuint *config, 
							PICAint *numResults, 
							PICAcompItem *results);
typedef PICAerror (APIENTRY *crPicaGetCompositorParamivProc)(PICAcompID compositor,
							     PICAparam pname,
							     PICAint *params);
typedef PICAerror (APIENTRY *crPicaGetCompositorParamfvProc)(PICAcompID compositor,
							     PICAparam pname,
							     PICAfloat *params);
typedef PICAerror (APIENTRY *crPicaGetCompositorParamcvProc)(PICAcompID compositor,
							     PICAparam pname,
							     PICAchar **params);
typedef PICAerror (APIENTRY *crPicaListNodesProc)(PICAcompID compositor, 
						  PICAint *num,
						  PICAnodeItem *results);

typedef PICAcontextID (APIENTRY *crPicaCreateContextProc)(const PICAuint *config, 
							 const PICAnodeID *nodes, 
							 PICAuint numNodes);
typedef PICAerror (APIENTRY *crPicaDestroyContextProc)(PICAcontextID ctx);

typedef PICAerror (APIENTRY *crPicaSetContextParamiProc)(PICAcontextID ctx,
							 PICAparam pname,
							 PICAint param);
typedef PICAerror (APIENTRY *crPicaSetContextParamivProc)(PICAcontextID ctx,
							  PICAparam pname,
							  const PICAint *param);
typedef PICAerror (APIENTRY *crPicaSetContextParamfProc)(PICAcontextID ctx,
							 PICAparam pname,
							 PICAfloat param);
typedef PICAerror (APIENTRY *crPicaSetContextParamfvProc)(PICAcontextID ctx,
							  PICAparam pname,
							  const PICAfloat *param);
typedef PICAerror (APIENTRY *crPicaSetContextParamvProc)(PICAcontextID ctx,
							 PICAparam pname,
							 const PICAvoid *param);

typedef PICAerror (APIENTRY *crPicaGetContextParamivProc)(PICAcontextID ctx,
							  PICAparam pname,
							  PICAint *param);  
typedef PICAerror (APIENTRY *crPicaGetContextParamfvProc)(PICAcontextID ctx,
							  PICAparam pname,
							  PICAfloat *param);  
typedef PICAerror (APIENTRY *crPicaGetContextParamcvProc)(PICAcontextID ctx,
							  PICAparam pname,
							  PICAchar **param);  
typedef PICAerror (APIENTRY *crPicaGetContextParamvProc)(PICAcontextID ctx,
							 PICAparam pname,
							 PICAvoid *param);  

typedef PICAcontextID (APIENTRY *crPicaBindLocalContextProc)(PICAcontextID globalCtx, 
							     PICAnodeID node);
typedef PICAerror (APIENTRY *crPicaDestroyLocalContextProc)(PICAcontextID lctx);

typedef PICAerror (APIENTRY *crPicaStartFrameProc)(PICAcontextID lctx,
						   const PICAframeID *frameID,
						   PICAuint numLocalFramlets,
						   PICAuint numOrders,
						   const PICAuint *orderList,
						   const PICArect *srcRectList,
						   const PICApoint *dstList);
typedef PICAerror (APIENTRY *crPicaEndFrameProc)(PICAcontextID lctx);
typedef PICAerror (APIENTRY *crPicaCancelFrameProc)(PICAcontextID ctx, 
						    PICAframeID frameID);
typedef PICAstatus (APIENTRY *crPicaQueryFrameProc)(PICAcontextID ctx,
						    PICAframeID frameID,
						    PICAnodeID nodeID,
						    PICAfloat timeout);
typedef PICAerror (APIENTRY *crPicaAddGfxFrameletProc)(PICAcontextID lctx,
						       const PICArect *srcRect,
						       const PICApoint *dstpos,
						       PICAuint order,
						       PICAint iVolatile);
typedef PICAerror (APIENTRY *crPicaAddMemFrameletProc)(PICAcontextID lctx,
						       const PICAvoid *colorBuffer,
						       const PICAvoid *depthBuffer,
						       PICAuint span_x,
						       const PICArect *srcRect,
						       const PICApoint *dstpos,
						       PICAuint order,
						       PICAint iVolatile);
typedef PICAerror (APIENTRY *crPicaReadFrameProc)(PICAcontextID lctx,
						  PICAframeID frameID,
						  PICAuint format,
						  PICAvoid *colorbuffer,
						  PICAvoid *depthbuffer,
						  const PICArect *rect);
typedef PICAerror (APIENTRY *crPicaBarProc)(char* phrase);
#endif /* CR_PICA */

#ifdef __cplusplus
}
#endif

#endif /* __CHROMIUM_H__ */
