#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>

/* these pointers live in opengl_stub/api_templates.c */
extern void *__glim_Accum;
extern void *__glim_AlphaFunc;
extern void *__glim_AreTexturesResident;
extern void *__glim_ArrayElement;
extern void *__glim_BarrierCreate;
extern void *__glim_BarrierDestroy;
extern void *__glim_BarrierExec;
extern void *__glim_Begin;
extern void *__glim_BindTexture;
extern void *__glim_Bitmap;
extern void *__glim_BlendFunc;
extern void *__glim_CallList;
extern void *__glim_CallLists;
extern void *__glim_Clear;
extern void *__glim_ClearAccum;
extern void *__glim_ClearColor;
extern void *__glim_ClearDepth;
extern void *__glim_ClearIndex;
extern void *__glim_ClearStencil;
extern void *__glim_ClipPlane;
extern void *__glim_Color3b;
extern void *__glim_Color3bv;
extern void *__glim_Color3d;
extern void *__glim_Color3dv;
extern void *__glim_Color3f;
extern void *__glim_Color3fv;
extern void *__glim_Color3i;
extern void *__glim_Color3iv;
extern void *__glim_Color3s;
extern void *__glim_Color3sv;
extern void *__glim_Color3ub;
extern void *__glim_Color3ubv;
extern void *__glim_Color3ui;
extern void *__glim_Color3uiv;
extern void *__glim_Color3us;
extern void *__glim_Color3usv;
extern void *__glim_Color4b;
extern void *__glim_Color4bv;
extern void *__glim_Color4d;
extern void *__glim_Color4dv;
extern void *__glim_Color4f;
extern void *__glim_Color4fv;
extern void *__glim_Color4i;
extern void *__glim_Color4iv;
extern void *__glim_Color4s;
extern void *__glim_Color4sv;
extern void *__glim_Color4ub;
extern void *__glim_Color4ubv;
extern void *__glim_Color4ui;
extern void *__glim_Color4uiv;
extern void *__glim_Color4us;
extern void *__glim_Color4usv;
extern void *__glim_ColorMask;
extern void *__glim_ColorMaterial;
extern void *__glim_ColorPointer;
extern void *__glim_CopyPixels;
extern void *__glim_CopyTexImage1D;
extern void *__glim_CopyTexImage2D;
extern void *__glim_CopyTexSubImage1D;
extern void *__glim_CopyTexSubImage2D;
extern void *__glim_CreateContext;
extern void *__glim_CullFace;
extern void *__glim_DeleteLists;
extern void *__glim_DeleteTextures;
extern void *__glim_DepthFunc;
extern void *__glim_DepthMask;
extern void *__glim_DepthRange;
extern void *__glim_Disable;
extern void *__glim_DisableClientState;
extern void *__glim_DrawArrays;
extern void *__glim_DrawBuffer;
extern void *__glim_DrawElements;
extern void *__glim_DrawPixels;
extern void *__glim_EdgeFlag;
extern void *__glim_EdgeFlagv;
extern void *__glim_Enable;
extern void *__glim_EnableClientState;
extern void *__glim_End;
extern void *__glim_EndList;
extern void *__glim_EvalCoord1d;
extern void *__glim_EvalCoord1dv;
extern void *__glim_EvalCoord1f;
extern void *__glim_EvalCoord1fv;
extern void *__glim_EvalCoord2d;
extern void *__glim_EvalCoord2dv;
extern void *__glim_EvalCoord2f;
extern void *__glim_EvalCoord2fv;
extern void *__glim_EvalMesh1;
extern void *__glim_EvalMesh2;
extern void *__glim_EvalPoint1;
extern void *__glim_EvalPoint2;
extern void *__glim_FeedbackBuffer;
extern void *__glim_Finish;
extern void *__glim_Flush;
extern void *__glim_Fogf;
extern void *__glim_Fogfv;
extern void *__glim_Fogi;
extern void *__glim_Fogiv;
extern void *__glim_FrontFace;
extern void *__glim_Frustum;
extern void *__glim_GenLists;
extern void *__glim_GenTextures;
extern void *__glim_GetBooleanv;
extern void *__glim_GetClipPlane;
extern void *__glim_GetDoublev;
extern void *__glim_GetError;
extern void *__glim_GetFloatv;
extern void *__glim_GetIntegerv;
extern void *__glim_GetLightfv;
extern void *__glim_GetLightiv;
extern void *__glim_GetMapdv;
extern void *__glim_GetMapfv;
extern void *__glim_GetMapiv;
extern void *__glim_GetMaterialfv;
extern void *__glim_GetMaterialiv;
extern void *__glim_GetPixelMapfv;
extern void *__glim_GetPixelMapuiv;
extern void *__glim_GetPixelMapusv;
extern void *__glim_GetPointerv;
extern void *__glim_GetPolygonStipple;
extern void *__glim_GetString;
extern void *__glim_GetTexEnvfv;
extern void *__glim_GetTexEnviv;
extern void *__glim_GetTexGendv;
extern void *__glim_GetTexGenfv;
extern void *__glim_GetTexGeniv;
extern void *__glim_GetTexImage;
extern void *__glim_GetTexLevelParameterfv;
extern void *__glim_GetTexLevelParameteriv;
extern void *__glim_GetTexParameterfv;
extern void *__glim_GetTexParameteriv;
extern void *__glim_Hint;
extern void *__glim_IndexMask;
extern void *__glim_IndexPointer;
extern void *__glim_Indexd;
extern void *__glim_Indexdv;
extern void *__glim_Indexf;
extern void *__glim_Indexfv;
extern void *__glim_Indexi;
extern void *__glim_Indexiv;
extern void *__glim_Indexs;
extern void *__glim_Indexsv;
extern void *__glim_Indexub;
extern void *__glim_Indexubv;
extern void *__glim_InitNames;
extern void *__glim_InterleavedArrays;
extern void *__glim_IsEnabled;
extern void *__glim_IsList;
extern void *__glim_IsTexture;
extern void *__glim_LightModelf;
extern void *__glim_LightModelfv;
extern void *__glim_LightModeli;
extern void *__glim_LightModeliv;
extern void *__glim_Lightf;
extern void *__glim_Lightfv;
extern void *__glim_Lighti;
extern void *__glim_Lightiv;
extern void *__glim_LineStipple;
extern void *__glim_LineWidth;
extern void *__glim_ListBase;
extern void *__glim_LoadIdentity;
extern void *__glim_LoadMatrixd;
extern void *__glim_LoadMatrixf;
extern void *__glim_LoadName;
extern void *__glim_LogicOp;
extern void *__glim_MakeCurrent;
extern void *__glim_Map1d;
extern void *__glim_Map1f;
extern void *__glim_Map2d;
extern void *__glim_Map2f;
extern void *__glim_MapGrid1d;
extern void *__glim_MapGrid1f;
extern void *__glim_MapGrid2d;
extern void *__glim_MapGrid2f;
extern void *__glim_Materialf;
extern void *__glim_Materialfv;
extern void *__glim_Materiali;
extern void *__glim_Materialiv;
extern void *__glim_MatrixMode;
extern void *__glim_MultMatrixd;
extern void *__glim_MultMatrixf;
extern void *__glim_NewList;
extern void *__glim_Normal3b;
extern void *__glim_Normal3bv;
extern void *__glim_Normal3d;
extern void *__glim_Normal3dv;
extern void *__glim_Normal3f;
extern void *__glim_Normal3fv;
extern void *__glim_Normal3i;
extern void *__glim_Normal3iv;
extern void *__glim_Normal3s;
extern void *__glim_Normal3sv;
extern void *__glim_NormalPointer;
extern void *__glim_Ortho;
extern void *__glim_PassThrough;
extern void *__glim_PixelMapfv;
extern void *__glim_PixelMapuiv;
extern void *__glim_PixelMapusv;
extern void *__glim_PixelStoref;
extern void *__glim_PixelStorei;
extern void *__glim_PixelTransferf;
extern void *__glim_PixelTransferi;
extern void *__glim_PixelZoom;
extern void *__glim_PointSize;
extern void *__glim_PolygonMode;
extern void *__glim_PolygonOffset;
extern void *__glim_PolygonStipple;
extern void *__glim_PopAttrib;
extern void *__glim_PopClientAttrib;
extern void *__glim_PopMatrix;
extern void *__glim_PopName;
extern void *__glim_PrioritizeTextures;
extern void *__glim_PushAttrib;
extern void *__glim_PushClientAttrib;
extern void *__glim_PushMatrix;
extern void *__glim_PushName;
extern void *__glim_RasterPos2d;
extern void *__glim_RasterPos2dv;
extern void *__glim_RasterPos2f;
extern void *__glim_RasterPos2fv;
extern void *__glim_RasterPos2i;
extern void *__glim_RasterPos2iv;
extern void *__glim_RasterPos2s;
extern void *__glim_RasterPos2sv;
extern void *__glim_RasterPos3d;
extern void *__glim_RasterPos3dv;
extern void *__glim_RasterPos3f;
extern void *__glim_RasterPos3fv;
extern void *__glim_RasterPos3i;
extern void *__glim_RasterPos3iv;
extern void *__glim_RasterPos3s;
extern void *__glim_RasterPos3sv;
extern void *__glim_RasterPos4d;
extern void *__glim_RasterPos4dv;
extern void *__glim_RasterPos4f;
extern void *__glim_RasterPos4fv;
extern void *__glim_RasterPos4i;
extern void *__glim_RasterPos4iv;
extern void *__glim_RasterPos4s;
extern void *__glim_RasterPos4sv;
extern void *__glim_ReadBuffer;
extern void *__glim_ReadPixels;
extern void *__glim_Rectd;
extern void *__glim_Rectdv;
extern void *__glim_Rectf;
extern void *__glim_Rectfv;
extern void *__glim_Recti;
extern void *__glim_Rectiv;
extern void *__glim_Rects;
extern void *__glim_Rectsv;
extern void *__glim_RenderMode;
extern void *__glim_Rotated;
extern void *__glim_Rotatef;
extern void *__glim_Scaled;
extern void *__glim_Scalef;
extern void *__glim_Scissor;
extern void *__glim_SelectBuffer;
extern void *__glim_SemaphoreCreate;
extern void *__glim_SemaphoreDestroy;
extern void *__glim_SemaphoreP;
extern void *__glim_SemaphoreV;
extern void *__glim_ShadeModel;
extern void *__glim_StencilFunc;
extern void *__glim_StencilMask;
extern void *__glim_StencilOp;
extern void *__glim_SwapBuffers;
extern void *__glim_TexCoord1d;
extern void *__glim_TexCoord1dv;
extern void *__glim_TexCoord1f;
extern void *__glim_TexCoord1fv;
extern void *__glim_TexCoord1i;
extern void *__glim_TexCoord1iv;
extern void *__glim_TexCoord1s;
extern void *__glim_TexCoord1sv;
extern void *__glim_TexCoord2d;
extern void *__glim_TexCoord2dv;
extern void *__glim_TexCoord2f;
extern void *__glim_TexCoord2fv;
extern void *__glim_TexCoord2i;
extern void *__glim_TexCoord2iv;
extern void *__glim_TexCoord2s;
extern void *__glim_TexCoord2sv;
extern void *__glim_TexCoord3d;
extern void *__glim_TexCoord3dv;
extern void *__glim_TexCoord3f;
extern void *__glim_TexCoord3fv;
extern void *__glim_TexCoord3i;
extern void *__glim_TexCoord3iv;
extern void *__glim_TexCoord3s;
extern void *__glim_TexCoord3sv;
extern void *__glim_TexCoord4d;
extern void *__glim_TexCoord4dv;
extern void *__glim_TexCoord4f;
extern void *__glim_TexCoord4fv;
extern void *__glim_TexCoord4i;
extern void *__glim_TexCoord4iv;
extern void *__glim_TexCoord4s;
extern void *__glim_TexCoord4sv;
extern void *__glim_TexCoordPointer;
extern void *__glim_TexEnvf;
extern void *__glim_TexEnvfv;
extern void *__glim_TexEnvi;
extern void *__glim_TexEnviv;
extern void *__glim_TexGend;
extern void *__glim_TexGendv;
extern void *__glim_TexGenf;
extern void *__glim_TexGenfv;
extern void *__glim_TexGeni;
extern void *__glim_TexGeniv;
extern void *__glim_TexImage1D;
extern void *__glim_TexImage2D;
extern void *__glim_TexParameterf;
extern void *__glim_TexParameterfv;
extern void *__glim_TexParameteri;
extern void *__glim_TexParameteriv;
extern void *__glim_TexSubImage1D;
extern void *__glim_TexSubImage2D;
extern void *__glim_Translated;
extern void *__glim_Translatef;
extern void *__glim_Vertex2d;
extern void *__glim_Vertex2dv;
extern void *__glim_Vertex2f;
extern void *__glim_Vertex2fv;
extern void *__glim_Vertex2i;
extern void *__glim_Vertex2iv;
extern void *__glim_Vertex2s;
extern void *__glim_Vertex2sv;
extern void *__glim_Vertex3d;
extern void *__glim_Vertex3dv;
extern void *__glim_Vertex3f;
extern void *__glim_Vertex3fv;
extern void *__glim_Vertex3i;
extern void *__glim_Vertex3iv;
extern void *__glim_Vertex3s;
extern void *__glim_Vertex3sv;
extern void *__glim_Vertex4d;
extern void *__glim_Vertex4dv;
extern void *__glim_Vertex4f;
extern void *__glim_Vertex4fv;
extern void *__glim_Vertex4i;
extern void *__glim_Vertex4iv;
extern void *__glim_Vertex4s;
extern void *__glim_Vertex4sv;
extern void *__glim_VertexPointer;
extern void *__glim_Viewport;
extern void *__glim_Writeback;

typedef void (*glAccum_ptr) ( GLenum op, GLfloat value ) ;
typedef void (*glAlphaFunc_ptr) ( GLenum func, GLclampf ref ) ;
typedef GLboolean (*glAreTexturesResident_ptr) ( GLsizei n, const GLuint *textures, GLboolean *residences ) ;
typedef void (*glArrayElement_ptr) ( GLint i ) ;
typedef void (*glBarrierCreate_ptr) ( GLuint name, GLuint count ) ;
typedef void (*glBarrierDestroy_ptr) ( GLuint name ) ;
typedef void (*glBarrierExec_ptr) ( GLuint name ) ;
typedef void (*glBegin_ptr) ( GLenum mode ) ;
typedef void (*glBindTexture_ptr) ( GLenum target, GLuint texture ) ;
typedef void (*glBitmap_ptr) ( GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap ) ;
typedef void (*glBlendFunc_ptr) ( GLenum sfactor, GLenum dfactor ) ;
typedef void (*glCallList_ptr) ( GLuint list ) ;
typedef void (*glCallLists_ptr) ( GLsizei n, GLenum type, const GLvoid *lists ) ;
typedef void (*glClear_ptr) ( GLbitfield mask ) ;
typedef void (*glClearAccum_ptr) ( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ) ;
typedef void (*glClearColor_ptr) ( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha ) ;
typedef void (*glClearDepth_ptr) ( GLclampd depth ) ;
typedef void (*glClearIndex_ptr) ( GLfloat c ) ;
typedef void (*glClearStencil_ptr) ( GLint s ) ;
typedef void (*glClipPlane_ptr) ( GLenum plane, const GLdouble *equation ) ;
typedef void (*glColor3b_ptr) ( GLbyte red, GLbyte green, GLbyte blue ) ;
typedef void (*glColor3bv_ptr) ( const GLbyte *v ) ;
typedef void (*glColor3d_ptr) ( GLdouble red, GLdouble green, GLdouble blue ) ;
typedef void (*glColor3dv_ptr) ( const GLdouble *v ) ;
typedef void (*glColor3f_ptr) ( GLfloat red, GLfloat green, GLfloat blue ) ;
typedef void (*glColor3fv_ptr) ( const GLfloat *v ) ;
typedef void (*glColor3i_ptr) ( GLint red, GLint green, GLint blue ) ;
typedef void (*glColor3iv_ptr) ( const GLint *v ) ;
typedef void (*glColor3s_ptr) ( GLshort red, GLshort green, GLshort blue ) ;
typedef void (*glColor3sv_ptr) ( const GLshort *v ) ;
typedef void (*glColor3ub_ptr) ( GLubyte red, GLubyte green, GLubyte blue ) ;
typedef void (*glColor3ubv_ptr) ( const GLubyte *v ) ;
typedef void (*glColor3ui_ptr) ( GLuint red, GLuint green, GLuint blue ) ;
typedef void (*glColor3uiv_ptr) ( const GLuint *v ) ;
typedef void (*glColor3us_ptr) ( GLushort red, GLushort green, GLushort blue ) ;
typedef void (*glColor3usv_ptr) ( const GLushort *v ) ;
typedef void (*glColor4b_ptr) ( GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha ) ;
typedef void (*glColor4bv_ptr) ( const GLbyte *v ) ;
typedef void (*glColor4d_ptr) ( GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha ) ;
typedef void (*glColor4dv_ptr) ( const GLdouble *v ) ;
typedef void (*glColor4f_ptr) ( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ) ;
typedef void (*glColor4fv_ptr) ( const GLfloat *v ) ;
typedef void (*glColor4i_ptr) ( GLint red, GLint green, GLint blue, GLint alpha ) ;
typedef void (*glColor4iv_ptr) ( const GLint *v ) ;
typedef void (*glColor4s_ptr) ( GLshort red, GLshort green, GLshort blue, GLshort alpha ) ;
typedef void (*glColor4sv_ptr) ( const GLshort *v ) ;
typedef void (*glColor4ub_ptr) ( GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha ) ;
typedef void (*glColor4ubv_ptr) ( const GLubyte *v ) ;
typedef void (*glColor4ui_ptr) ( GLuint red, GLuint green, GLuint blue, GLuint alpha ) ;
typedef void (*glColor4uiv_ptr) ( const GLuint *v ) ;
typedef void (*glColor4us_ptr) ( GLushort red, GLushort green, GLushort blue, GLushort alpha ) ;
typedef void (*glColor4usv_ptr) ( const GLushort *v ) ;
typedef void (*glColorMask_ptr) ( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha ) ;
typedef void (*glColorMaterial_ptr) ( GLenum face, GLenum mode ) ;
typedef void (*glColorPointer_ptr) ( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer ) ;
typedef void (*glCopyPixels_ptr) ( GLint x, GLint y, GLsizei width, GLsizei height, GLenum type ) ;
typedef void (*glCopyTexImage1D_ptr) ( GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border ) ;
typedef void (*glCopyTexImage2D_ptr) ( GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border ) ;
typedef void (*glCopyTexSubImage1D_ptr) ( GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width ) ;
typedef void (*glCopyTexSubImage2D_ptr) ( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height ) ;
typedef void (*glCreateContext_ptr) ( void *arg1, void *arg2 ) ;
typedef void (*glCullFace_ptr) ( GLenum mode ) ;
typedef void (*glDeleteLists_ptr) ( GLuint list, GLsizei range ) ;
typedef void (*glDeleteTextures_ptr) ( GLsizei n, const GLuint *textures ) ;
typedef void (*glDepthFunc_ptr) ( GLenum func ) ;
typedef void (*glDepthMask_ptr) ( GLboolean flag ) ;
typedef void (*glDepthRange_ptr) ( GLclampd zNear, GLclampd zFar ) ;
typedef void (*glDisable_ptr) ( GLenum cap ) ;
typedef void (*glDisableClientState_ptr) ( GLenum array ) ;
typedef void (*glDrawArrays_ptr) ( GLenum mode, GLint first, GLsizei count ) ;
typedef void (*glDrawBuffer_ptr) ( GLenum mode ) ;
typedef void (*glDrawElements_ptr) ( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices ) ;
typedef void (*glDrawPixels_ptr) ( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels ) ;
typedef void (*glEdgeFlag_ptr) ( GLboolean flag ) ;
typedef void (*glEdgeFlagv_ptr) ( const GLboolean *flag ) ;
typedef void (*glEnable_ptr) ( GLenum cap ) ;
typedef void (*glEnableClientState_ptr) ( GLenum array ) ;
typedef void (*glEnd_ptr) ( void ) ;
typedef void (*glEndList_ptr) ( void ) ;
typedef void (*glEvalCoord1d_ptr) ( GLdouble u ) ;
typedef void (*glEvalCoord1dv_ptr) ( const GLdouble *u ) ;
typedef void (*glEvalCoord1f_ptr) ( GLfloat u ) ;
typedef void (*glEvalCoord1fv_ptr) ( const GLfloat *u ) ;
typedef void (*glEvalCoord2d_ptr) ( GLdouble u, GLdouble v ) ;
typedef void (*glEvalCoord2dv_ptr) ( const GLdouble *u ) ;
typedef void (*glEvalCoord2f_ptr) ( GLfloat u, GLfloat v ) ;
typedef void (*glEvalCoord2fv_ptr) ( const GLfloat *u ) ;
typedef void (*glEvalMesh1_ptr) ( GLenum mode, GLint i1, GLint i2 ) ;
typedef void (*glEvalMesh2_ptr) ( GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2 ) ;
typedef void (*glEvalPoint1_ptr) ( GLint i ) ;
typedef void (*glEvalPoint2_ptr) ( GLint i, GLint j ) ;
typedef void (*glFeedbackBuffer_ptr) ( GLsizei size, GLenum type, GLfloat *buffer ) ;
typedef void (*glFinish_ptr) ( void ) ;
typedef void (*glFlush_ptr) ( void ) ;
typedef void (*glFogf_ptr) ( GLenum pname, GLfloat param ) ;
typedef void (*glFogfv_ptr) ( GLenum pname, const GLfloat *params ) ;
typedef void (*glFogi_ptr) ( GLenum pname, GLint param ) ;
typedef void (*glFogiv_ptr) ( GLenum pname, const GLint *params ) ;
typedef void (*glFrontFace_ptr) ( GLenum mode ) ;
typedef void (*glFrustum_ptr) ( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar ) ;
typedef GLuint (*glGenLists_ptr) ( GLsizei range ) ;
typedef void (*glGenTextures_ptr) ( GLsizei n, GLuint *textures ) ;
typedef void (*glGetBooleanv_ptr) ( GLenum pname, GLboolean *params ) ;
typedef void (*glGetClipPlane_ptr) ( GLenum plane, GLdouble *equation ) ;
typedef void (*glGetDoublev_ptr) ( GLenum pname, GLdouble *params ) ;
typedef GLenum (*glGetError_ptr) ( void ) ;
typedef void (*glGetFloatv_ptr) ( GLenum pname, GLfloat *params ) ;
typedef void (*glGetIntegerv_ptr) ( GLenum pname, GLint *params ) ;
typedef void (*glGetLightfv_ptr) ( GLenum light, GLenum pname, GLfloat *params ) ;
typedef void (*glGetLightiv_ptr) ( GLenum light, GLenum pname, GLint *params ) ;
typedef void (*glGetMapdv_ptr) ( GLenum target, GLenum query, GLdouble *v ) ;
typedef void (*glGetMapfv_ptr) ( GLenum target, GLenum query, GLfloat *v ) ;
typedef void (*glGetMapiv_ptr) ( GLenum target, GLenum query, GLint *v ) ;
typedef void (*glGetMaterialfv_ptr) ( GLenum face, GLenum pname, GLfloat *params ) ;
typedef void (*glGetMaterialiv_ptr) ( GLenum face, GLenum pname, GLint *params ) ;
typedef void (*glGetPixelMapfv_ptr) ( GLenum map, GLfloat *values ) ;
typedef void (*glGetPixelMapuiv_ptr) ( GLenum map, GLuint *values ) ;
typedef void (*glGetPixelMapusv_ptr) ( GLenum map, GLushort *values ) ;
typedef void (*glGetPointerv_ptr) ( GLenum pname, GLvoid* *params ) ;
typedef void (*glGetPolygonStipple_ptr) ( GLubyte *mask ) ;
typedef const GLubyte * (*glGetString_ptr) ( GLenum name ) ;
typedef void (*glGetTexEnvfv_ptr) ( GLenum target, GLenum pname, GLfloat *params ) ;
typedef void (*glGetTexEnviv_ptr) ( GLenum target, GLenum pname, GLint *params ) ;
typedef void (*glGetTexGendv_ptr) ( GLenum coord, GLenum pname, GLdouble *params ) ;
typedef void (*glGetTexGenfv_ptr) ( GLenum coord, GLenum pname, GLfloat *params ) ;
typedef void (*glGetTexGeniv_ptr) ( GLenum coord, GLenum pname, GLint *params ) ;
typedef void (*glGetTexImage_ptr) ( GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels ) ;
typedef void (*glGetTexLevelParameterfv_ptr) ( GLenum target, GLint level, GLenum pname, GLfloat *params ) ;
typedef void (*glGetTexLevelParameteriv_ptr) ( GLenum target, GLint level, GLenum pname, GLint *params ) ;
typedef void (*glGetTexParameterfv_ptr) ( GLenum target, GLenum pname, GLfloat *params ) ;
typedef void (*glGetTexParameteriv_ptr) ( GLenum target, GLenum pname, GLint *params ) ;
typedef void (*glHint_ptr) ( GLenum target, GLenum mode ) ;
typedef void (*glIndexMask_ptr) ( GLuint mask ) ;
typedef void (*glIndexPointer_ptr) ( GLenum type, GLsizei stride, const GLvoid *pointer ) ;
typedef void (*glIndexd_ptr) ( GLdouble c ) ;
typedef void (*glIndexdv_ptr) ( const GLdouble *c ) ;
typedef void (*glIndexf_ptr) ( GLfloat c ) ;
typedef void (*glIndexfv_ptr) ( const GLfloat *c ) ;
typedef void (*glIndexi_ptr) ( GLint c ) ;
typedef void (*glIndexiv_ptr) ( const GLint *c ) ;
typedef void (*glIndexs_ptr) ( GLshort c ) ;
typedef void (*glIndexsv_ptr) ( const GLshort *c ) ;
typedef void (*glIndexub_ptr) ( GLubyte c ) ;
typedef void (*glIndexubv_ptr) ( const GLubyte *c ) ;
typedef void (*glInitNames_ptr) ( void ) ;
typedef void (*glInterleavedArrays_ptr) ( GLenum format, GLsizei stride, const GLvoid *pointer ) ;
typedef GLboolean (*glIsEnabled_ptr) ( GLenum cap ) ;
typedef GLboolean (*glIsList_ptr) ( GLuint list ) ;
typedef GLboolean (*glIsTexture_ptr) ( GLuint texture ) ;
typedef void (*glLightModelf_ptr) ( GLenum pname, GLfloat param ) ;
typedef void (*glLightModelfv_ptr) ( GLenum pname, const GLfloat *params ) ;
typedef void (*glLightModeli_ptr) ( GLenum pname, GLint param ) ;
typedef void (*glLightModeliv_ptr) ( GLenum pname, const GLint *params ) ;
typedef void (*glLightf_ptr) ( GLenum light, GLenum pname, GLfloat param ) ;
typedef void (*glLightfv_ptr) ( GLenum light, GLenum pname, const GLfloat *params ) ;
typedef void (*glLighti_ptr) ( GLenum light, GLenum pname, GLint param ) ;
typedef void (*glLightiv_ptr) ( GLenum light, GLenum pname, const GLint *params ) ;
typedef void (*glLineStipple_ptr) ( GLint factor, GLushort pattern ) ;
typedef void (*glLineWidth_ptr) ( GLfloat width ) ;
typedef void (*glListBase_ptr) ( GLuint base ) ;
typedef void (*glLoadIdentity_ptr) ( void ) ;
typedef void (*glLoadMatrixd_ptr) ( const GLdouble *m ) ;
typedef void (*glLoadMatrixf_ptr) ( const GLfloat *m ) ;
typedef void (*glLoadName_ptr) ( GLuint name ) ;
typedef void (*glLogicOp_ptr) ( GLenum opcode ) ;
typedef void (*glMakeCurrent_ptr) ( void ) ;
typedef void (*glMap1d_ptr) ( GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points ) ;
typedef void (*glMap1f_ptr) ( GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points ) ;
typedef void (*glMap2d_ptr) ( GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points ) ;
typedef void (*glMap2f_ptr) ( GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points ) ;
typedef void (*glMapGrid1d_ptr) ( GLint un, GLdouble u1, GLdouble u2 ) ;
typedef void (*glMapGrid1f_ptr) ( GLint un, GLfloat u1, GLfloat u2 ) ;
typedef void (*glMapGrid2d_ptr) ( GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2 ) ;
typedef void (*glMapGrid2f_ptr) ( GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2 ) ;
typedef void (*glMaterialf_ptr) ( GLenum face, GLenum pname, GLfloat param ) ;
typedef void (*glMaterialfv_ptr) ( GLenum face, GLenum pname, const GLfloat *params ) ;
typedef void (*glMateriali_ptr) ( GLenum face, GLenum pname, GLint param ) ;
typedef void (*glMaterialiv_ptr) ( GLenum face, GLenum pname, const GLint *params ) ;
typedef void (*glMatrixMode_ptr) ( GLenum mode ) ;
typedef void (*glMultMatrixd_ptr) ( const GLdouble *m ) ;
typedef void (*glMultMatrixf_ptr) ( const GLfloat *m ) ;
typedef void (*glNewList_ptr) ( GLuint list, GLenum mode ) ;
typedef void (*glNormal3b_ptr) ( GLbyte nx, GLbyte ny, GLbyte nz ) ;
typedef void (*glNormal3bv_ptr) ( const GLbyte *v ) ;
typedef void (*glNormal3d_ptr) ( GLdouble nx, GLdouble ny, GLdouble nz ) ;
typedef void (*glNormal3dv_ptr) ( const GLdouble *v ) ;
typedef void (*glNormal3f_ptr) ( GLfloat nx, GLfloat ny, GLfloat nz ) ;
typedef void (*glNormal3fv_ptr) ( const GLfloat *v ) ;
typedef void (*glNormal3i_ptr) ( GLint nx, GLint ny, GLint nz ) ;
typedef void (*glNormal3iv_ptr) ( const GLint *v ) ;
typedef void (*glNormal3s_ptr) ( GLshort nx, GLshort ny, GLshort nz ) ;
typedef void (*glNormal3sv_ptr) ( const GLshort *v ) ;
typedef void (*glNormalPointer_ptr) ( GLenum type, GLsizei stride, const GLvoid *pointer ) ;
typedef void (*glOrtho_ptr) ( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar ) ;
typedef void (*glPassThrough_ptr) ( GLfloat token ) ;
typedef void (*glPixelMapfv_ptr) ( GLenum map, GLsizei mapsize, const GLfloat *values ) ;
typedef void (*glPixelMapuiv_ptr) ( GLenum map, GLsizei mapsize, const GLuint *values ) ;
typedef void (*glPixelMapusv_ptr) ( GLenum map, GLsizei mapsize, const GLushort *values ) ;
typedef void (*glPixelStoref_ptr) ( GLenum pname, GLfloat param ) ;
typedef void (*glPixelStorei_ptr) ( GLenum pname, GLint param ) ;
typedef void (*glPixelTransferf_ptr) ( GLenum pname, GLfloat param ) ;
typedef void (*glPixelTransferi_ptr) ( GLenum pname, GLint param ) ;
typedef void (*glPixelZoom_ptr) ( GLfloat xfactor, GLfloat yfactor ) ;
typedef void (*glPointSize_ptr) ( GLfloat size ) ;
typedef void (*glPolygonMode_ptr) ( GLenum face, GLenum mode ) ;
typedef void (*glPolygonOffset_ptr) ( GLfloat factor, GLfloat units ) ;
typedef void (*glPolygonStipple_ptr) ( const GLubyte *mask ) ;
typedef void (*glPopAttrib_ptr) ( void ) ;
typedef void (*glPopClientAttrib_ptr) ( void ) ;
typedef void (*glPopMatrix_ptr) ( void ) ;
typedef void (*glPopName_ptr) ( void ) ;
typedef void (*glPrioritizeTextures_ptr) ( GLsizei n, const GLuint *textures, const GLclampf *priorities ) ;
typedef void (*glPushAttrib_ptr) ( GLbitfield mask ) ;
typedef void (*glPushClientAttrib_ptr) ( GLbitfield mask ) ;
typedef void (*glPushMatrix_ptr) ( void ) ;
typedef void (*glPushName_ptr) ( GLuint name ) ;
typedef void (*glRasterPos2d_ptr) ( GLdouble x, GLdouble y ) ;
typedef void (*glRasterPos2dv_ptr) ( const GLdouble *v ) ;
typedef void (*glRasterPos2f_ptr) ( GLfloat x, GLfloat y ) ;
typedef void (*glRasterPos2fv_ptr) ( const GLfloat *v ) ;
typedef void (*glRasterPos2i_ptr) ( GLint x, GLint y ) ;
typedef void (*glRasterPos2iv_ptr) ( const GLint *v ) ;
typedef void (*glRasterPos2s_ptr) ( GLshort x, GLshort y ) ;
typedef void (*glRasterPos2sv_ptr) ( const GLshort *v ) ;
typedef void (*glRasterPos3d_ptr) ( GLdouble x, GLdouble y, GLdouble z ) ;
typedef void (*glRasterPos3dv_ptr) ( const GLdouble *v ) ;
typedef void (*glRasterPos3f_ptr) ( GLfloat x, GLfloat y, GLfloat z ) ;
typedef void (*glRasterPos3fv_ptr) ( const GLfloat *v ) ;
typedef void (*glRasterPos3i_ptr) ( GLint x, GLint y, GLint z ) ;
typedef void (*glRasterPos3iv_ptr) ( const GLint *v ) ;
typedef void (*glRasterPos3s_ptr) ( GLshort x, GLshort y, GLshort z ) ;
typedef void (*glRasterPos3sv_ptr) ( const GLshort *v ) ;
typedef void (*glRasterPos4d_ptr) ( GLdouble x, GLdouble y, GLdouble z, GLdouble w ) ;
typedef void (*glRasterPos4dv_ptr) ( const GLdouble *v ) ;
typedef void (*glRasterPos4f_ptr) ( GLfloat x, GLfloat y, GLfloat z, GLfloat w ) ;
typedef void (*glRasterPos4fv_ptr) ( const GLfloat *v ) ;
typedef void (*glRasterPos4i_ptr) ( GLint x, GLint y, GLint z, GLint w ) ;
typedef void (*glRasterPos4iv_ptr) ( const GLint *v ) ;
typedef void (*glRasterPos4s_ptr) ( GLshort x, GLshort y, GLshort z, GLshort w ) ;
typedef void (*glRasterPos4sv_ptr) ( const GLshort *v ) ;
typedef void (*glReadBuffer_ptr) ( GLenum mode ) ;
typedef void (*glReadPixels_ptr) ( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels ) ;
typedef void (*glRectd_ptr) ( GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 ) ;
typedef void (*glRectdv_ptr) ( const GLdouble *v1, const GLdouble *v2 ) ;
typedef void (*glRectf_ptr) ( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 ) ;
typedef void (*glRectfv_ptr) ( const GLfloat *v1, const GLfloat *v2 ) ;
typedef void (*glRecti_ptr) ( GLint x1, GLint y1, GLint x2, GLint y2 ) ;
typedef void (*glRectiv_ptr) ( const GLint *v1, const GLint *v2 ) ;
typedef void (*glRects_ptr) ( GLshort x1, GLshort y1, GLshort x2, GLshort y2 ) ;
typedef void (*glRectsv_ptr) ( const GLshort *v1, const GLshort *v2 ) ;
typedef GLint (*glRenderMode_ptr) ( GLenum mode ) ;
typedef void (*glRotated_ptr) ( GLdouble angle, GLdouble x, GLdouble y, GLdouble z ) ;
typedef void (*glRotatef_ptr) ( GLfloat angle, GLfloat x, GLfloat y, GLfloat z ) ;
typedef void (*glScaled_ptr) ( GLdouble x, GLdouble y, GLdouble z ) ;
typedef void (*glScalef_ptr) ( GLfloat x, GLfloat y, GLfloat z ) ;
typedef void (*glScissor_ptr) ( GLint x, GLint y, GLsizei width, GLsizei height ) ;
typedef void (*glSelectBuffer_ptr) ( GLsizei size, GLuint *buffer ) ;
typedef void (*glSemaphoreCreate_ptr) ( GLuint name, GLuint count ) ;
typedef void (*glSemaphoreDestroy_ptr) ( GLuint name ) ;
typedef void (*glSemaphoreP_ptr) ( GLuint name ) ;
typedef void (*glSemaphoreV_ptr) ( GLuint name ) ;
typedef void (*glShadeModel_ptr) ( GLenum mode ) ;
typedef void (*glStencilFunc_ptr) ( GLenum func, GLint ref, GLuint mask ) ;
typedef void (*glStencilMask_ptr) ( GLuint mask ) ;
typedef void (*glStencilOp_ptr) ( GLenum fail, GLenum zfail, GLenum zpass ) ;
typedef void (*glSwapBuffers_ptr) ( void ) ;
typedef void (*glTexCoord1d_ptr) ( GLdouble s ) ;
typedef void (*glTexCoord1dv_ptr) ( const GLdouble *v ) ;
typedef void (*glTexCoord1f_ptr) ( GLfloat s ) ;
typedef void (*glTexCoord1fv_ptr) ( const GLfloat *v ) ;
typedef void (*glTexCoord1i_ptr) ( GLint s ) ;
typedef void (*glTexCoord1iv_ptr) ( const GLint *v ) ;
typedef void (*glTexCoord1s_ptr) ( GLshort s ) ;
typedef void (*glTexCoord1sv_ptr) ( const GLshort *v ) ;
typedef void (*glTexCoord2d_ptr) ( GLdouble s, GLdouble t ) ;
typedef void (*glTexCoord2dv_ptr) ( const GLdouble *v ) ;
typedef void (*glTexCoord2f_ptr) ( GLfloat s, GLfloat t ) ;
typedef void (*glTexCoord2fv_ptr) ( const GLfloat *v ) ;
typedef void (*glTexCoord2i_ptr) ( GLint s, GLint t ) ;
typedef void (*glTexCoord2iv_ptr) ( const GLint *v ) ;
typedef void (*glTexCoord2s_ptr) ( GLshort s, GLshort t ) ;
typedef void (*glTexCoord2sv_ptr) ( const GLshort *v ) ;
typedef void (*glTexCoord3d_ptr) ( GLdouble s, GLdouble t, GLdouble r ) ;
typedef void (*glTexCoord3dv_ptr) ( const GLdouble *v ) ;
typedef void (*glTexCoord3f_ptr) ( GLfloat s, GLfloat t, GLfloat r ) ;
typedef void (*glTexCoord3fv_ptr) ( const GLfloat *v ) ;
typedef void (*glTexCoord3i_ptr) ( GLint s, GLint t, GLint r ) ;
typedef void (*glTexCoord3iv_ptr) ( const GLint *v ) ;
typedef void (*glTexCoord3s_ptr) ( GLshort s, GLshort t, GLshort r ) ;
typedef void (*glTexCoord3sv_ptr) ( const GLshort *v ) ;
typedef void (*glTexCoord4d_ptr) ( GLdouble s, GLdouble t, GLdouble r, GLdouble q ) ;
typedef void (*glTexCoord4dv_ptr) ( const GLdouble *v ) ;
typedef void (*glTexCoord4f_ptr) ( GLfloat s, GLfloat t, GLfloat r, GLfloat q ) ;
typedef void (*glTexCoord4fv_ptr) ( const GLfloat *v ) ;
typedef void (*glTexCoord4i_ptr) ( GLint s, GLint t, GLint r, GLint q ) ;
typedef void (*glTexCoord4iv_ptr) ( const GLint *v ) ;
typedef void (*glTexCoord4s_ptr) ( GLshort s, GLshort t, GLshort r, GLshort q ) ;
typedef void (*glTexCoord4sv_ptr) ( const GLshort *v ) ;
typedef void (*glTexCoordPointer_ptr) ( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer ) ;
typedef void (*glTexEnvf_ptr) ( GLenum target, GLenum pname, GLfloat param ) ;
typedef void (*glTexEnvfv_ptr) ( GLenum target, GLenum pname, const GLfloat *params ) ;
typedef void (*glTexEnvi_ptr) ( GLenum target, GLenum pname, GLint param ) ;
typedef void (*glTexEnviv_ptr) ( GLenum target, GLenum pname, const GLint *params ) ;
typedef void (*glTexGend_ptr) ( GLenum coord, GLenum pname, GLdouble param ) ;
typedef void (*glTexGendv_ptr) ( GLenum coord, GLenum pname, const GLdouble *params ) ;
typedef void (*glTexGenf_ptr) ( GLenum coord, GLenum pname, GLfloat param ) ;
typedef void (*glTexGenfv_ptr) ( GLenum coord, GLenum pname, const GLfloat *params ) ;
typedef void (*glTexGeni_ptr) ( GLenum coord, GLenum pname, GLint param ) ;
typedef void (*glTexGeniv_ptr) ( GLenum coord, GLenum pname, const GLint *params ) ;
typedef void (*glTexImage1D_ptr) ( GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels ) ;
typedef void (*glTexImage2D_ptr) ( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels ) ;
typedef void (*glTexParameterf_ptr) ( GLenum target, GLenum pname, GLfloat param ) ;
typedef void (*glTexParameterfv_ptr) ( GLenum target, GLenum pname, const GLfloat *params ) ;
typedef void (*glTexParameteri_ptr) ( GLenum target, GLenum pname, GLint param ) ;
typedef void (*glTexParameteriv_ptr) ( GLenum target, GLenum pname, const GLint *params ) ;
typedef void (*glTexSubImage1D_ptr) ( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels ) ;
typedef void (*glTexSubImage2D_ptr) ( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels ) ;
typedef void (*glTranslated_ptr) ( GLdouble x, GLdouble y, GLdouble z ) ;
typedef void (*glTranslatef_ptr) ( GLfloat x, GLfloat y, GLfloat z ) ;
typedef void (*glVertex2d_ptr) ( GLdouble x, GLdouble y ) ;
typedef void (*glVertex2dv_ptr) ( const GLdouble *v ) ;
typedef void (*glVertex2f_ptr) ( GLfloat x, GLfloat y ) ;
typedef void (*glVertex2fv_ptr) ( const GLfloat *v ) ;
typedef void (*glVertex2i_ptr) ( GLint x, GLint y ) ;
typedef void (*glVertex2iv_ptr) ( const GLint *v ) ;
typedef void (*glVertex2s_ptr) ( GLshort x, GLshort y ) ;
typedef void (*glVertex2sv_ptr) ( const GLshort *v ) ;
typedef void (*glVertex3d_ptr) ( GLdouble x, GLdouble y, GLdouble z ) ;
typedef void (*glVertex3dv_ptr) ( const GLdouble *v ) ;
typedef void (*glVertex3f_ptr) ( GLfloat x, GLfloat y, GLfloat z ) ;
typedef void (*glVertex3fv_ptr) ( const GLfloat *v ) ;
typedef void (*glVertex3i_ptr) ( GLint x, GLint y, GLint z ) ;
typedef void (*glVertex3iv_ptr) ( const GLint *v ) ;
typedef void (*glVertex3s_ptr) ( GLshort x, GLshort y, GLshort z ) ;
typedef void (*glVertex3sv_ptr) ( const GLshort *v ) ;
typedef void (*glVertex4d_ptr) ( GLdouble x, GLdouble y, GLdouble z, GLdouble w ) ;
typedef void (*glVertex4dv_ptr) ( const GLdouble *v ) ;
typedef void (*glVertex4f_ptr) ( GLfloat x, GLfloat y, GLfloat z, GLfloat w ) ;
typedef void (*glVertex4fv_ptr) ( const GLfloat *v ) ;
typedef void (*glVertex4i_ptr) ( GLint x, GLint y, GLint z, GLint w ) ;
typedef void (*glVertex4iv_ptr) ( const GLint *v ) ;
typedef void (*glVertex4s_ptr) ( GLshort x, GLshort y, GLshort z, GLshort w ) ;
typedef void (*glVertex4sv_ptr) ( const GLshort *v ) ;
typedef void (*glVertexPointer_ptr) ( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer ) ;
typedef void (*glViewport_ptr) ( GLint x, GLint y, GLsizei width, GLsizei height ) ;
typedef void (*glWriteback_ptr) ( GLint *writeback ) ;

void glAccum ( GLenum op, GLfloat value )
{
	((glAccum_ptr) __glim_Accum) ( op , value );
}

void glAlphaFunc ( GLenum func, GLclampf ref )
{
	((glAlphaFunc_ptr) __glim_AlphaFunc) ( func , ref );
}

GLboolean glAreTexturesResident ( GLsizei n, const GLuint *textures, GLboolean *residences )
{
	return  ((glAreTexturesResident_ptr) __glim_AreTexturesResident) ( n , textures , residences );
}

void glArrayElement ( GLint i )
{
	((glArrayElement_ptr) __glim_ArrayElement) ( i );
}

void glBarrierCreate ( GLuint name, GLuint count )
{
	((glBarrierCreate_ptr) __glim_BarrierCreate) ( name , count );
}

void glBarrierDestroy ( GLuint name )
{
	((glBarrierDestroy_ptr) __glim_BarrierDestroy) ( name );
}

void glBarrierExec ( GLuint name )
{
	((glBarrierExec_ptr) __glim_BarrierExec) ( name );
}

void glBegin ( GLenum mode )
{
	((glBegin_ptr) __glim_Begin) ( mode );
}

void glBindTexture ( GLenum target, GLuint texture )
{
	((glBindTexture_ptr) __glim_BindTexture) ( target , texture );
}

void glBitmap ( GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap )
{
	((glBitmap_ptr) __glim_Bitmap) ( width , height , xorig , yorig , xmove , ymove , bitmap );
}

void glBlendFunc ( GLenum sfactor, GLenum dfactor )
{
	((glBlendFunc_ptr) __glim_BlendFunc) ( sfactor , dfactor );
}

void glCallList ( GLuint list )
{
	((glCallList_ptr) __glim_CallList) ( list );
}

void glCallLists ( GLsizei n, GLenum type, const GLvoid *lists )
{
	((glCallLists_ptr) __glim_CallLists) ( n , type , lists );
}

void glClear ( GLbitfield mask )
{
	((glClear_ptr) __glim_Clear) ( mask );
}

void glClearAccum ( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{
	((glClearAccum_ptr) __glim_ClearAccum) ( red , green , blue , alpha );
}

void glClearColor ( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )
{
	((glClearColor_ptr) __glim_ClearColor) ( red , green , blue , alpha );
}

void glClearDepth ( GLclampd depth )
{
	((glClearDepth_ptr) __glim_ClearDepth) ( depth );
}

void glClearIndex ( GLfloat c )
{
	((glClearIndex_ptr) __glim_ClearIndex) ( c );
}

void glClearStencil ( GLint s )
{
	((glClearStencil_ptr) __glim_ClearStencil) ( s );
}

void glClipPlane ( GLenum plane, const GLdouble *equation )
{
	((glClipPlane_ptr) __glim_ClipPlane) ( plane , equation );
}

void glColor3b ( GLbyte red, GLbyte green, GLbyte blue )
{
	((glColor3b_ptr) __glim_Color3b) ( red , green , blue );
}

void glColor3bv ( const GLbyte *v )
{
	((glColor3bv_ptr) __glim_Color3bv) ( v );
}

void glColor3d ( GLdouble red, GLdouble green, GLdouble blue )
{
	((glColor3d_ptr) __glim_Color3d) ( red , green , blue );
}

void glColor3dv ( const GLdouble *v )
{
	((glColor3dv_ptr) __glim_Color3dv) ( v );
}

void glColor3f ( GLfloat red, GLfloat green, GLfloat blue )
{
	((glColor3f_ptr) __glim_Color3f) ( red , green , blue );
}

void glColor3fv ( const GLfloat *v )
{
	((glColor3fv_ptr) __glim_Color3fv) ( v );
}

void glColor3i ( GLint red, GLint green, GLint blue )
{
	((glColor3i_ptr) __glim_Color3i) ( red , green , blue );
}

void glColor3iv ( const GLint *v )
{
	((glColor3iv_ptr) __glim_Color3iv) ( v );
}

void glColor3s ( GLshort red, GLshort green, GLshort blue )
{
	((glColor3s_ptr) __glim_Color3s) ( red , green , blue );
}

void glColor3sv ( const GLshort *v )
{
	((glColor3sv_ptr) __glim_Color3sv) ( v );
}

void glColor3ub ( GLubyte red, GLubyte green, GLubyte blue )
{
	((glColor3ub_ptr) __glim_Color3ub) ( red , green , blue );
}

void glColor3ubv ( const GLubyte *v )
{
	((glColor3ubv_ptr) __glim_Color3ubv) ( v );
}

void glColor3ui ( GLuint red, GLuint green, GLuint blue )
{
	((glColor3ui_ptr) __glim_Color3ui) ( red , green , blue );
}

void glColor3uiv ( const GLuint *v )
{
	((glColor3uiv_ptr) __glim_Color3uiv) ( v );
}

void glColor3us ( GLushort red, GLushort green, GLushort blue )
{
	((glColor3us_ptr) __glim_Color3us) ( red , green , blue );
}

void glColor3usv ( const GLushort *v )
{
	((glColor3usv_ptr) __glim_Color3usv) ( v );
}

void glColor4b ( GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha )
{
	((glColor4b_ptr) __glim_Color4b) ( red , green , blue , alpha );
}

void glColor4bv ( const GLbyte *v )
{
	((glColor4bv_ptr) __glim_Color4bv) ( v );
}

void glColor4d ( GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha )
{
	((glColor4d_ptr) __glim_Color4d) ( red , green , blue , alpha );
}

void glColor4dv ( const GLdouble *v )
{
	((glColor4dv_ptr) __glim_Color4dv) ( v );
}

void glColor4f ( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{
	((glColor4f_ptr) __glim_Color4f) ( red , green , blue , alpha );
}

void glColor4fv ( const GLfloat *v )
{
	((glColor4fv_ptr) __glim_Color4fv) ( v );
}

void glColor4i ( GLint red, GLint green, GLint blue, GLint alpha )
{
	((glColor4i_ptr) __glim_Color4i) ( red , green , blue , alpha );
}

void glColor4iv ( const GLint *v )
{
	((glColor4iv_ptr) __glim_Color4iv) ( v );
}

void glColor4s ( GLshort red, GLshort green, GLshort blue, GLshort alpha )
{
	((glColor4s_ptr) __glim_Color4s) ( red , green , blue , alpha );
}

void glColor4sv ( const GLshort *v )
{
	((glColor4sv_ptr) __glim_Color4sv) ( v );
}

void glColor4ub ( GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha )
{
	((glColor4ub_ptr) __glim_Color4ub) ( red , green , blue , alpha );
}

void glColor4ubv ( const GLubyte *v )
{
	((glColor4ubv_ptr) __glim_Color4ubv) ( v );
}

void glColor4ui ( GLuint red, GLuint green, GLuint blue, GLuint alpha )
{
	((glColor4ui_ptr) __glim_Color4ui) ( red , green , blue , alpha );
}

void glColor4uiv ( const GLuint *v )
{
	((glColor4uiv_ptr) __glim_Color4uiv) ( v );
}

void glColor4us ( GLushort red, GLushort green, GLushort blue, GLushort alpha )
{
	((glColor4us_ptr) __glim_Color4us) ( red , green , blue , alpha );
}

void glColor4usv ( const GLushort *v )
{
	((glColor4usv_ptr) __glim_Color4usv) ( v );
}

void glColorMask ( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha )
{
	((glColorMask_ptr) __glim_ColorMask) ( red , green , blue , alpha );
}

void glColorMaterial ( GLenum face, GLenum mode )
{
	((glColorMaterial_ptr) __glim_ColorMaterial) ( face , mode );
}

void glColorPointer ( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	((glColorPointer_ptr) __glim_ColorPointer) ( size , type , stride , pointer );
}

void glCopyPixels ( GLint x, GLint y, GLsizei width, GLsizei height, GLenum type )
{
	((glCopyPixels_ptr) __glim_CopyPixels) ( x , y , width , height , type );
}

void glCopyTexImage1D ( GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border )
{
	((glCopyTexImage1D_ptr) __glim_CopyTexImage1D) ( target , level , internalFormat , x , y , width , border );
}

void glCopyTexImage2D ( GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border )
{
	((glCopyTexImage2D_ptr) __glim_CopyTexImage2D) ( target , level , internalFormat , x , y , width , height , border );
}

void glCopyTexSubImage1D ( GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width )
{
	((glCopyTexSubImage1D_ptr) __glim_CopyTexSubImage1D) ( target , level , xoffset , x , y , width );
}

void glCopyTexSubImage2D ( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height )
{
	((glCopyTexSubImage2D_ptr) __glim_CopyTexSubImage2D) ( target , level , xoffset , yoffset , x , y , width , height );
}

void glCreateContext ( void *arg1, void *arg2 )
{
	((glCreateContext_ptr) __glim_CreateContext) ( arg1 , arg2 );
}

void glCullFace ( GLenum mode )
{
	((glCullFace_ptr) __glim_CullFace) ( mode );
}

void glDeleteLists ( GLuint list, GLsizei range )
{
	((glDeleteLists_ptr) __glim_DeleteLists) ( list , range );
}

void glDeleteTextures ( GLsizei n, const GLuint *textures )
{
	((glDeleteTextures_ptr) __glim_DeleteTextures) ( n , textures );
}

void glDepthFunc ( GLenum func )
{
	((glDepthFunc_ptr) __glim_DepthFunc) ( func );
}

void glDepthMask ( GLboolean flag )
{
	((glDepthMask_ptr) __glim_DepthMask) ( flag );
}

void glDepthRange ( GLclampd zNear, GLclampd zFar )
{
	((glDepthRange_ptr) __glim_DepthRange) ( zNear , zFar );
}

void glDisable ( GLenum cap )
{
	((glDisable_ptr) __glim_Disable) ( cap );
}

void glDisableClientState ( GLenum array )
{
	((glDisableClientState_ptr) __glim_DisableClientState) ( array );
}

void glDrawArrays ( GLenum mode, GLint first, GLsizei count )
{
	((glDrawArrays_ptr) __glim_DrawArrays) ( mode , first , count );
}

void glDrawBuffer ( GLenum mode )
{
	((glDrawBuffer_ptr) __glim_DrawBuffer) ( mode );
}

void glDrawElements ( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices )
{
	((glDrawElements_ptr) __glim_DrawElements) ( mode , count , type , indices );
}

void glDrawPixels ( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels )
{
	((glDrawPixels_ptr) __glim_DrawPixels) ( width , height , format , type , pixels );
}

void glEdgeFlag ( GLboolean flag )
{
	((glEdgeFlag_ptr) __glim_EdgeFlag) ( flag );
}

void glEdgeFlagv ( const GLboolean *flag )
{
	((glEdgeFlagv_ptr) __glim_EdgeFlagv) ( flag );
}

void glEnable ( GLenum cap )
{
	((glEnable_ptr) __glim_Enable) ( cap );
}

void glEnableClientState ( GLenum array )
{
	((glEnableClientState_ptr) __glim_EnableClientState) ( array );
}

void glEnd ( void )
{
	((glEnd_ptr) __glim_End) ( );
}

void glEndList ( void )
{
	((glEndList_ptr) __glim_EndList) ( );
}

void glEvalCoord1d ( GLdouble u )
{
	((glEvalCoord1d_ptr) __glim_EvalCoord1d) ( u );
}

void glEvalCoord1dv ( const GLdouble *u )
{
	((glEvalCoord1dv_ptr) __glim_EvalCoord1dv) ( u );
}

void glEvalCoord1f ( GLfloat u )
{
	((glEvalCoord1f_ptr) __glim_EvalCoord1f) ( u );
}

void glEvalCoord1fv ( const GLfloat *u )
{
	((glEvalCoord1fv_ptr) __glim_EvalCoord1fv) ( u );
}

void glEvalCoord2d ( GLdouble u, GLdouble v )
{
	((glEvalCoord2d_ptr) __glim_EvalCoord2d) ( u , v );
}

void glEvalCoord2dv ( const GLdouble *u )
{
	((glEvalCoord2dv_ptr) __glim_EvalCoord2dv) ( u );
}

void glEvalCoord2f ( GLfloat u, GLfloat v )
{
	((glEvalCoord2f_ptr) __glim_EvalCoord2f) ( u , v );
}

void glEvalCoord2fv ( const GLfloat *u )
{
	((glEvalCoord2fv_ptr) __glim_EvalCoord2fv) ( u );
}

void glEvalMesh1 ( GLenum mode, GLint i1, GLint i2 )
{
	((glEvalMesh1_ptr) __glim_EvalMesh1) ( mode , i1 , i2 );
}

void glEvalMesh2 ( GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2 )
{
	((glEvalMesh2_ptr) __glim_EvalMesh2) ( mode , i1 , i2 , j1 , j2 );
}

void glEvalPoint1 ( GLint i )
{
	((glEvalPoint1_ptr) __glim_EvalPoint1) ( i );
}

void glEvalPoint2 ( GLint i, GLint j )
{
	((glEvalPoint2_ptr) __glim_EvalPoint2) ( i , j );
}

void glFeedbackBuffer ( GLsizei size, GLenum type, GLfloat *buffer )
{
	((glFeedbackBuffer_ptr) __glim_FeedbackBuffer) ( size , type , buffer );
}

void glFinish ( void )
{
	((glFinish_ptr) __glim_Finish) ( );
}

void glFlush ( void )
{
	((glFlush_ptr) __glim_Flush) ( );
}

void glFogf ( GLenum pname, GLfloat param )
{
	((glFogf_ptr) __glim_Fogf) ( pname , param );
}

void glFogfv ( GLenum pname, const GLfloat *params )
{
	((glFogfv_ptr) __glim_Fogfv) ( pname , params );
}

void glFogi ( GLenum pname, GLint param )
{
	((glFogi_ptr) __glim_Fogi) ( pname , param );
}

void glFogiv ( GLenum pname, const GLint *params )
{
	((glFogiv_ptr) __glim_Fogiv) ( pname , params );
}

void glFrontFace ( GLenum mode )
{
	((glFrontFace_ptr) __glim_FrontFace) ( mode );
}

void glFrustum ( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar )
{
	((glFrustum_ptr) __glim_Frustum) ( left , right , bottom , top , zNear , zFar );
}

GLuint glGenLists ( GLsizei range )
{
	return  ((glGenLists_ptr) __glim_GenLists) ( range );
}

void glGenTextures ( GLsizei n, GLuint *textures )
{
	((glGenTextures_ptr) __glim_GenTextures) ( n , textures );
}

void glGetBooleanv ( GLenum pname, GLboolean *params )
{
	((glGetBooleanv_ptr) __glim_GetBooleanv) ( pname , params );
}

void glGetClipPlane ( GLenum plane, GLdouble *equation )
{
	((glGetClipPlane_ptr) __glim_GetClipPlane) ( plane , equation );
}

void glGetDoublev ( GLenum pname, GLdouble *params )
{
	((glGetDoublev_ptr) __glim_GetDoublev) ( pname , params );
}

GLenum glGetError ( void )
{
	return  ((glGetError_ptr) __glim_GetError) ( );
}

void glGetFloatv ( GLenum pname, GLfloat *params )
{
	((glGetFloatv_ptr) __glim_GetFloatv) ( pname , params );
}

void glGetIntegerv ( GLenum pname, GLint *params )
{
	((glGetIntegerv_ptr) __glim_GetIntegerv) ( pname , params );
}

void glGetLightfv ( GLenum light, GLenum pname, GLfloat *params )
{
	((glGetLightfv_ptr) __glim_GetLightfv) ( light , pname , params );
}

void glGetLightiv ( GLenum light, GLenum pname, GLint *params )
{
	((glGetLightiv_ptr) __glim_GetLightiv) ( light , pname , params );
}

void glGetMapdv ( GLenum target, GLenum query, GLdouble *v )
{
	((glGetMapdv_ptr) __glim_GetMapdv) ( target , query , v );
}

void glGetMapfv ( GLenum target, GLenum query, GLfloat *v )
{
	((glGetMapfv_ptr) __glim_GetMapfv) ( target , query , v );
}

void glGetMapiv ( GLenum target, GLenum query, GLint *v )
{
	((glGetMapiv_ptr) __glim_GetMapiv) ( target , query , v );
}

void glGetMaterialfv ( GLenum face, GLenum pname, GLfloat *params )
{
	((glGetMaterialfv_ptr) __glim_GetMaterialfv) ( face , pname , params );
}

void glGetMaterialiv ( GLenum face, GLenum pname, GLint *params )
{
	((glGetMaterialiv_ptr) __glim_GetMaterialiv) ( face , pname , params );
}

void glGetPixelMapfv ( GLenum map, GLfloat *values )
{
	((glGetPixelMapfv_ptr) __glim_GetPixelMapfv) ( map , values );
}

void glGetPixelMapuiv ( GLenum map, GLuint *values )
{
	((glGetPixelMapuiv_ptr) __glim_GetPixelMapuiv) ( map , values );
}

void glGetPixelMapusv ( GLenum map, GLushort *values )
{
	((glGetPixelMapusv_ptr) __glim_GetPixelMapusv) ( map , values );
}

void glGetPointerv ( GLenum pname, GLvoid* *params )
{
	((glGetPointerv_ptr) __glim_GetPointerv) ( pname , params );
}

void glGetPolygonStipple ( GLubyte *mask )
{
	((glGetPolygonStipple_ptr) __glim_GetPolygonStipple) ( mask );
}

const GLubyte * glGetString ( GLenum name )
{
	return  ((glGetString_ptr) __glim_GetString) ( name );
}

void glGetTexEnvfv ( GLenum target, GLenum pname, GLfloat *params )
{
	((glGetTexEnvfv_ptr) __glim_GetTexEnvfv) ( target , pname , params );
}

void glGetTexEnviv ( GLenum target, GLenum pname, GLint *params )
{
	((glGetTexEnviv_ptr) __glim_GetTexEnviv) ( target , pname , params );
}

void glGetTexGendv ( GLenum coord, GLenum pname, GLdouble *params )
{
	((glGetTexGendv_ptr) __glim_GetTexGendv) ( coord , pname , params );
}

void glGetTexGenfv ( GLenum coord, GLenum pname, GLfloat *params )
{
	((glGetTexGenfv_ptr) __glim_GetTexGenfv) ( coord , pname , params );
}

void glGetTexGeniv ( GLenum coord, GLenum pname, GLint *params )
{
	((glGetTexGeniv_ptr) __glim_GetTexGeniv) ( coord , pname , params );
}

void glGetTexImage ( GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels )
{
	((glGetTexImage_ptr) __glim_GetTexImage) ( target , level , format , type , pixels );
}

void glGetTexLevelParameterfv ( GLenum target, GLint level, GLenum pname, GLfloat *params )
{
	((glGetTexLevelParameterfv_ptr) __glim_GetTexLevelParameterfv) ( target , level , pname , params );
}

void glGetTexLevelParameteriv ( GLenum target, GLint level, GLenum pname, GLint *params )
{
	((glGetTexLevelParameteriv_ptr) __glim_GetTexLevelParameteriv) ( target , level , pname , params );
}

void glGetTexParameterfv ( GLenum target, GLenum pname, GLfloat *params )
{
	((glGetTexParameterfv_ptr) __glim_GetTexParameterfv) ( target , pname , params );
}

void glGetTexParameteriv ( GLenum target, GLenum pname, GLint *params )
{
	((glGetTexParameteriv_ptr) __glim_GetTexParameteriv) ( target , pname , params );
}

void glHint ( GLenum target, GLenum mode )
{
	((glHint_ptr) __glim_Hint) ( target , mode );
}

void glIndexMask ( GLuint mask )
{
	((glIndexMask_ptr) __glim_IndexMask) ( mask );
}

void glIndexPointer ( GLenum type, GLsizei stride, const GLvoid *pointer )
{
	((glIndexPointer_ptr) __glim_IndexPointer) ( type , stride , pointer );
}

void glIndexd ( GLdouble c )
{
	((glIndexd_ptr) __glim_Indexd) ( c );
}

void glIndexdv ( const GLdouble *c )
{
	((glIndexdv_ptr) __glim_Indexdv) ( c );
}

void glIndexf ( GLfloat c )
{
	((glIndexf_ptr) __glim_Indexf) ( c );
}

void glIndexfv ( const GLfloat *c )
{
	((glIndexfv_ptr) __glim_Indexfv) ( c );
}

void glIndexi ( GLint c )
{
	((glIndexi_ptr) __glim_Indexi) ( c );
}

void glIndexiv ( const GLint *c )
{
	((glIndexiv_ptr) __glim_Indexiv) ( c );
}

void glIndexs ( GLshort c )
{
	((glIndexs_ptr) __glim_Indexs) ( c );
}

void glIndexsv ( const GLshort *c )
{
	((glIndexsv_ptr) __glim_Indexsv) ( c );
}

void glIndexub ( GLubyte c )
{
	((glIndexub_ptr) __glim_Indexub) ( c );
}

void glIndexubv ( const GLubyte *c )
{
	((glIndexubv_ptr) __glim_Indexubv) ( c );
}

void glInitNames ( void )
{
	((glInitNames_ptr) __glim_InitNames) ( );
}

void glInterleavedArrays ( GLenum format, GLsizei stride, const GLvoid *pointer )
{
	((glInterleavedArrays_ptr) __glim_InterleavedArrays) ( format , stride , pointer );
}

GLboolean glIsEnabled ( GLenum cap )
{
	return  ((glIsEnabled_ptr) __glim_IsEnabled) ( cap );
}

GLboolean glIsList ( GLuint list )
{
	return  ((glIsList_ptr) __glim_IsList) ( list );
}

GLboolean glIsTexture ( GLuint texture )
{
	return  ((glIsTexture_ptr) __glim_IsTexture) ( texture );
}

void glLightModelf ( GLenum pname, GLfloat param )
{
	((glLightModelf_ptr) __glim_LightModelf) ( pname , param );
}

void glLightModelfv ( GLenum pname, const GLfloat *params )
{
	((glLightModelfv_ptr) __glim_LightModelfv) ( pname , params );
}

void glLightModeli ( GLenum pname, GLint param )
{
	((glLightModeli_ptr) __glim_LightModeli) ( pname , param );
}

void glLightModeliv ( GLenum pname, const GLint *params )
{
	((glLightModeliv_ptr) __glim_LightModeliv) ( pname , params );
}

void glLightf ( GLenum light, GLenum pname, GLfloat param )
{
	((glLightf_ptr) __glim_Lightf) ( light , pname , param );
}

void glLightfv ( GLenum light, GLenum pname, const GLfloat *params )
{
	((glLightfv_ptr) __glim_Lightfv) ( light , pname , params );
}

void glLighti ( GLenum light, GLenum pname, GLint param )
{
	((glLighti_ptr) __glim_Lighti) ( light , pname , param );
}

void glLightiv ( GLenum light, GLenum pname, const GLint *params )
{
	((glLightiv_ptr) __glim_Lightiv) ( light , pname , params );
}

void glLineStipple ( GLint factor, GLushort pattern )
{
	((glLineStipple_ptr) __glim_LineStipple) ( factor , pattern );
}

void glLineWidth ( GLfloat width )
{
	((glLineWidth_ptr) __glim_LineWidth) ( width );
}

void glListBase ( GLuint base )
{
	((glListBase_ptr) __glim_ListBase) ( base );
}

void glLoadIdentity ( void )
{
	((glLoadIdentity_ptr) __glim_LoadIdentity) ( );
}

void glLoadMatrixd ( const GLdouble *m )
{
	((glLoadMatrixd_ptr) __glim_LoadMatrixd) ( m );
}

void glLoadMatrixf ( const GLfloat *m )
{
	((glLoadMatrixf_ptr) __glim_LoadMatrixf) ( m );
}

void glLoadName ( GLuint name )
{
	((glLoadName_ptr) __glim_LoadName) ( name );
}

void glLogicOp ( GLenum opcode )
{
	((glLogicOp_ptr) __glim_LogicOp) ( opcode );
}

void glMakeCurrent ( void )
{
	((glMakeCurrent_ptr) __glim_MakeCurrent) ( );
}

void glMap1d ( GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points )
{
	((glMap1d_ptr) __glim_Map1d) ( target , u1 , u2 , stride , order , points );
}

void glMap1f ( GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points )
{
	((glMap1f_ptr) __glim_Map1f) ( target , u1 , u2 , stride , order , points );
}

void glMap2d ( GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points )
{
	((glMap2d_ptr) __glim_Map2d) ( target , u1 , u2 , ustride , uorder , v1 , v2 , vstride , vorder , points );
}

void glMap2f ( GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points )
{
	((glMap2f_ptr) __glim_Map2f) ( target , u1 , u2 , ustride , uorder , v1 , v2 , vstride , vorder , points );
}

void glMapGrid1d ( GLint un, GLdouble u1, GLdouble u2 )
{
	((glMapGrid1d_ptr) __glim_MapGrid1d) ( un , u1 , u2 );
}

void glMapGrid1f ( GLint un, GLfloat u1, GLfloat u2 )
{
	((glMapGrid1f_ptr) __glim_MapGrid1f) ( un , u1 , u2 );
}

void glMapGrid2d ( GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2 )
{
	((glMapGrid2d_ptr) __glim_MapGrid2d) ( un , u1 , u2 , vn , v1 , v2 );
}

void glMapGrid2f ( GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2 )
{
	((glMapGrid2f_ptr) __glim_MapGrid2f) ( un , u1 , u2 , vn , v1 , v2 );
}

void glMaterialf ( GLenum face, GLenum pname, GLfloat param )
{
	((glMaterialf_ptr) __glim_Materialf) ( face , pname , param );
}

void glMaterialfv ( GLenum face, GLenum pname, const GLfloat *params )
{
	((glMaterialfv_ptr) __glim_Materialfv) ( face , pname , params );
}

void glMateriali ( GLenum face, GLenum pname, GLint param )
{
	((glMateriali_ptr) __glim_Materiali) ( face , pname , param );
}

void glMaterialiv ( GLenum face, GLenum pname, const GLint *params )
{
	((glMaterialiv_ptr) __glim_Materialiv) ( face , pname , params );
}

void glMatrixMode ( GLenum mode )
{
	((glMatrixMode_ptr) __glim_MatrixMode) ( mode );
}

void glMultMatrixd ( const GLdouble *m )
{
	((glMultMatrixd_ptr) __glim_MultMatrixd) ( m );
}

void glMultMatrixf ( const GLfloat *m )
{
	((glMultMatrixf_ptr) __glim_MultMatrixf) ( m );
}

void glNewList ( GLuint list, GLenum mode )
{
	((glNewList_ptr) __glim_NewList) ( list , mode );
}

void glNormal3b ( GLbyte nx, GLbyte ny, GLbyte nz )
{
	((glNormal3b_ptr) __glim_Normal3b) ( nx , ny , nz );
}

void glNormal3bv ( const GLbyte *v )
{
	((glNormal3bv_ptr) __glim_Normal3bv) ( v );
}

void glNormal3d ( GLdouble nx, GLdouble ny, GLdouble nz )
{
	((glNormal3d_ptr) __glim_Normal3d) ( nx , ny , nz );
}

void glNormal3dv ( const GLdouble *v )
{
	((glNormal3dv_ptr) __glim_Normal3dv) ( v );
}

void glNormal3f ( GLfloat nx, GLfloat ny, GLfloat nz )
{
	((glNormal3f_ptr) __glim_Normal3f) ( nx , ny , nz );
}

void glNormal3fv ( const GLfloat *v )
{
	((glNormal3fv_ptr) __glim_Normal3fv) ( v );
}

void glNormal3i ( GLint nx, GLint ny, GLint nz )
{
	((glNormal3i_ptr) __glim_Normal3i) ( nx , ny , nz );
}

void glNormal3iv ( const GLint *v )
{
	((glNormal3iv_ptr) __glim_Normal3iv) ( v );
}

void glNormal3s ( GLshort nx, GLshort ny, GLshort nz )
{
	((glNormal3s_ptr) __glim_Normal3s) ( nx , ny , nz );
}

void glNormal3sv ( const GLshort *v )
{
	((glNormal3sv_ptr) __glim_Normal3sv) ( v );
}

void glNormalPointer ( GLenum type, GLsizei stride, const GLvoid *pointer )
{
	((glNormalPointer_ptr) __glim_NormalPointer) ( type , stride , pointer );
}

void glOrtho ( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar )
{
	((glOrtho_ptr) __glim_Ortho) ( left , right , bottom , top , zNear , zFar );
}

void glPassThrough ( GLfloat token )
{
	((glPassThrough_ptr) __glim_PassThrough) ( token );
}

void glPixelMapfv ( GLenum map, GLsizei mapsize, const GLfloat *values )
{
	((glPixelMapfv_ptr) __glim_PixelMapfv) ( map , mapsize , values );
}

void glPixelMapuiv ( GLenum map, GLsizei mapsize, const GLuint *values )
{
	((glPixelMapuiv_ptr) __glim_PixelMapuiv) ( map , mapsize , values );
}

void glPixelMapusv ( GLenum map, GLsizei mapsize, const GLushort *values )
{
	((glPixelMapusv_ptr) __glim_PixelMapusv) ( map , mapsize , values );
}

void glPixelStoref ( GLenum pname, GLfloat param )
{
	((glPixelStoref_ptr) __glim_PixelStoref) ( pname , param );
}

void glPixelStorei ( GLenum pname, GLint param )
{
	((glPixelStorei_ptr) __glim_PixelStorei) ( pname , param );
}

void glPixelTransferf ( GLenum pname, GLfloat param )
{
	((glPixelTransferf_ptr) __glim_PixelTransferf) ( pname , param );
}

void glPixelTransferi ( GLenum pname, GLint param )
{
	((glPixelTransferi_ptr) __glim_PixelTransferi) ( pname , param );
}

void glPixelZoom ( GLfloat xfactor, GLfloat yfactor )
{
	((glPixelZoom_ptr) __glim_PixelZoom) ( xfactor , yfactor );
}

void glPointSize ( GLfloat size )
{
	((glPointSize_ptr) __glim_PointSize) ( size );
}

void glPolygonMode ( GLenum face, GLenum mode )
{
	((glPolygonMode_ptr) __glim_PolygonMode) ( face , mode );
}

void glPolygonOffset ( GLfloat factor, GLfloat units )
{
	((glPolygonOffset_ptr) __glim_PolygonOffset) ( factor , units );
}

void glPolygonStipple ( const GLubyte *mask )
{
	((glPolygonStipple_ptr) __glim_PolygonStipple) ( mask );
}

void glPopAttrib ( void )
{
	((glPopAttrib_ptr) __glim_PopAttrib) ( );
}

void glPopClientAttrib ( void )
{
	((glPopClientAttrib_ptr) __glim_PopClientAttrib) ( );
}

void glPopMatrix ( void )
{
	((glPopMatrix_ptr) __glim_PopMatrix) ( );
}

void glPopName ( void )
{
	((glPopName_ptr) __glim_PopName) ( );
}

void glPrioritizeTextures ( GLsizei n, const GLuint *textures, const GLclampf *priorities )
{
	((glPrioritizeTextures_ptr) __glim_PrioritizeTextures) ( n , textures , priorities );
}

void glPushAttrib ( GLbitfield mask )
{
	((glPushAttrib_ptr) __glim_PushAttrib) ( mask );
}

void glPushClientAttrib ( GLbitfield mask )
{
	((glPushClientAttrib_ptr) __glim_PushClientAttrib) ( mask );
}

void glPushMatrix ( void )
{
	((glPushMatrix_ptr) __glim_PushMatrix) ( );
}

void glPushName ( GLuint name )
{
	((glPushName_ptr) __glim_PushName) ( name );
}

void glRasterPos2d ( GLdouble x, GLdouble y )
{
	((glRasterPos2d_ptr) __glim_RasterPos2d) ( x , y );
}

void glRasterPos2dv ( const GLdouble *v )
{
	((glRasterPos2dv_ptr) __glim_RasterPos2dv) ( v );
}

void glRasterPos2f ( GLfloat x, GLfloat y )
{
	((glRasterPos2f_ptr) __glim_RasterPos2f) ( x , y );
}

void glRasterPos2fv ( const GLfloat *v )
{
	((glRasterPos2fv_ptr) __glim_RasterPos2fv) ( v );
}

void glRasterPos2i ( GLint x, GLint y )
{
	((glRasterPos2i_ptr) __glim_RasterPos2i) ( x , y );
}

void glRasterPos2iv ( const GLint *v )
{
	((glRasterPos2iv_ptr) __glim_RasterPos2iv) ( v );
}

void glRasterPos2s ( GLshort x, GLshort y )
{
	((glRasterPos2s_ptr) __glim_RasterPos2s) ( x , y );
}

void glRasterPos2sv ( const GLshort *v )
{
	((glRasterPos2sv_ptr) __glim_RasterPos2sv) ( v );
}

void glRasterPos3d ( GLdouble x, GLdouble y, GLdouble z )
{
	((glRasterPos3d_ptr) __glim_RasterPos3d) ( x , y , z );
}

void glRasterPos3dv ( const GLdouble *v )
{
	((glRasterPos3dv_ptr) __glim_RasterPos3dv) ( v );
}

void glRasterPos3f ( GLfloat x, GLfloat y, GLfloat z )
{
	((glRasterPos3f_ptr) __glim_RasterPos3f) ( x , y , z );
}

void glRasterPos3fv ( const GLfloat *v )
{
	((glRasterPos3fv_ptr) __glim_RasterPos3fv) ( v );
}

void glRasterPos3i ( GLint x, GLint y, GLint z )
{
	((glRasterPos3i_ptr) __glim_RasterPos3i) ( x , y , z );
}

void glRasterPos3iv ( const GLint *v )
{
	((glRasterPos3iv_ptr) __glim_RasterPos3iv) ( v );
}

void glRasterPos3s ( GLshort x, GLshort y, GLshort z )
{
	((glRasterPos3s_ptr) __glim_RasterPos3s) ( x , y , z );
}

void glRasterPos3sv ( const GLshort *v )
{
	((glRasterPos3sv_ptr) __glim_RasterPos3sv) ( v );
}

void glRasterPos4d ( GLdouble x, GLdouble y, GLdouble z, GLdouble w )
{
	((glRasterPos4d_ptr) __glim_RasterPos4d) ( x , y , z , w );
}

void glRasterPos4dv ( const GLdouble *v )
{
	((glRasterPos4dv_ptr) __glim_RasterPos4dv) ( v );
}

void glRasterPos4f ( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
	((glRasterPos4f_ptr) __glim_RasterPos4f) ( x , y , z , w );
}

void glRasterPos4fv ( const GLfloat *v )
{
	((glRasterPos4fv_ptr) __glim_RasterPos4fv) ( v );
}

void glRasterPos4i ( GLint x, GLint y, GLint z, GLint w )
{
	((glRasterPos4i_ptr) __glim_RasterPos4i) ( x , y , z , w );
}

void glRasterPos4iv ( const GLint *v )
{
	((glRasterPos4iv_ptr) __glim_RasterPos4iv) ( v );
}

void glRasterPos4s ( GLshort x, GLshort y, GLshort z, GLshort w )
{
	((glRasterPos4s_ptr) __glim_RasterPos4s) ( x , y , z , w );
}

void glRasterPos4sv ( const GLshort *v )
{
	((glRasterPos4sv_ptr) __glim_RasterPos4sv) ( v );
}

void glReadBuffer ( GLenum mode )
{
	((glReadBuffer_ptr) __glim_ReadBuffer) ( mode );
}

void glReadPixels ( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels )
{
	((glReadPixels_ptr) __glim_ReadPixels) ( x , y , width , height , format , type , pixels );
}

void glRectd ( GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 )
{
	((glRectd_ptr) __glim_Rectd) ( x1 , y1 , x2 , y2 );
}

void glRectdv ( const GLdouble *v1, const GLdouble *v2 )
{
	((glRectdv_ptr) __glim_Rectdv) ( v1 , v2 );
}

void glRectf ( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 )
{
	((glRectf_ptr) __glim_Rectf) ( x1 , y1 , x2 , y2 );
}

void glRectfv ( const GLfloat *v1, const GLfloat *v2 )
{
	((glRectfv_ptr) __glim_Rectfv) ( v1 , v2 );
}

void glRecti ( GLint x1, GLint y1, GLint x2, GLint y2 )
{
	((glRecti_ptr) __glim_Recti) ( x1 , y1 , x2 , y2 );
}

void glRectiv ( const GLint *v1, const GLint *v2 )
{
	((glRectiv_ptr) __glim_Rectiv) ( v1 , v2 );
}

void glRects ( GLshort x1, GLshort y1, GLshort x2, GLshort y2 )
{
	((glRects_ptr) __glim_Rects) ( x1 , y1 , x2 , y2 );
}

void glRectsv ( const GLshort *v1, const GLshort *v2 )
{
	((glRectsv_ptr) __glim_Rectsv) ( v1 , v2 );
}

GLint glRenderMode ( GLenum mode )
{
	return  ((glRenderMode_ptr) __glim_RenderMode) ( mode );
}

void glRotated ( GLdouble angle, GLdouble x, GLdouble y, GLdouble z )
{
	((glRotated_ptr) __glim_Rotated) ( angle , x , y , z );
}

void glRotatef ( GLfloat angle, GLfloat x, GLfloat y, GLfloat z )
{
	((glRotatef_ptr) __glim_Rotatef) ( angle , x , y , z );
}

void glScaled ( GLdouble x, GLdouble y, GLdouble z )
{
	((glScaled_ptr) __glim_Scaled) ( x , y , z );
}

void glScalef ( GLfloat x, GLfloat y, GLfloat z )
{
	((glScalef_ptr) __glim_Scalef) ( x , y , z );
}

void glScissor ( GLint x, GLint y, GLsizei width, GLsizei height )
{
	((glScissor_ptr) __glim_Scissor) ( x , y , width , height );
}

void glSelectBuffer ( GLsizei size, GLuint *buffer )
{
	((glSelectBuffer_ptr) __glim_SelectBuffer) ( size , buffer );
}

void glSemaphoreCreate ( GLuint name, GLuint count )
{
	((glSemaphoreCreate_ptr) __glim_SemaphoreCreate) ( name , count );
}

void glSemaphoreDestroy ( GLuint name )
{
	((glSemaphoreDestroy_ptr) __glim_SemaphoreDestroy) ( name );
}

void glSemaphoreP ( GLuint name )
{
	((glSemaphoreP_ptr) __glim_SemaphoreP) ( name );
}

void glSemaphoreV ( GLuint name )
{
	((glSemaphoreV_ptr) __glim_SemaphoreV) ( name );
}

void glShadeModel ( GLenum mode )
{
	((glShadeModel_ptr) __glim_ShadeModel) ( mode );
}

void glStencilFunc ( GLenum func, GLint ref, GLuint mask )
{
	((glStencilFunc_ptr) __glim_StencilFunc) ( func , ref , mask );
}

void glStencilMask ( GLuint mask )
{
	((glStencilMask_ptr) __glim_StencilMask) ( mask );
}

void glStencilOp ( GLenum fail, GLenum zfail, GLenum zpass )
{
	((glStencilOp_ptr) __glim_StencilOp) ( fail , zfail , zpass );
}

void glSwapBuffers ( void )
{
	((glSwapBuffers_ptr) __glim_SwapBuffers) ( );
}

void glTexCoord1d ( GLdouble s )
{
	((glTexCoord1d_ptr) __glim_TexCoord1d) ( s );
}

void glTexCoord1dv ( const GLdouble *v )
{
	((glTexCoord1dv_ptr) __glim_TexCoord1dv) ( v );
}

void glTexCoord1f ( GLfloat s )
{
	((glTexCoord1f_ptr) __glim_TexCoord1f) ( s );
}

void glTexCoord1fv ( const GLfloat *v )
{
	((glTexCoord1fv_ptr) __glim_TexCoord1fv) ( v );
}

void glTexCoord1i ( GLint s )
{
	((glTexCoord1i_ptr) __glim_TexCoord1i) ( s );
}

void glTexCoord1iv ( const GLint *v )
{
	((glTexCoord1iv_ptr) __glim_TexCoord1iv) ( v );
}

void glTexCoord1s ( GLshort s )
{
	((glTexCoord1s_ptr) __glim_TexCoord1s) ( s );
}

void glTexCoord1sv ( const GLshort *v )
{
	((glTexCoord1sv_ptr) __glim_TexCoord1sv) ( v );
}

void glTexCoord2d ( GLdouble s, GLdouble t )
{
	((glTexCoord2d_ptr) __glim_TexCoord2d) ( s , t );
}

void glTexCoord2dv ( const GLdouble *v )
{
	((glTexCoord2dv_ptr) __glim_TexCoord2dv) ( v );
}

void glTexCoord2f ( GLfloat s, GLfloat t )
{
	((glTexCoord2f_ptr) __glim_TexCoord2f) ( s , t );
}

void glTexCoord2fv ( const GLfloat *v )
{
	((glTexCoord2fv_ptr) __glim_TexCoord2fv) ( v );
}

void glTexCoord2i ( GLint s, GLint t )
{
	((glTexCoord2i_ptr) __glim_TexCoord2i) ( s , t );
}

void glTexCoord2iv ( const GLint *v )
{
	((glTexCoord2iv_ptr) __glim_TexCoord2iv) ( v );
}

void glTexCoord2s ( GLshort s, GLshort t )
{
	((glTexCoord2s_ptr) __glim_TexCoord2s) ( s , t );
}

void glTexCoord2sv ( const GLshort *v )
{
	((glTexCoord2sv_ptr) __glim_TexCoord2sv) ( v );
}

void glTexCoord3d ( GLdouble s, GLdouble t, GLdouble r )
{
	((glTexCoord3d_ptr) __glim_TexCoord3d) ( s , t , r );
}

void glTexCoord3dv ( const GLdouble *v )
{
	((glTexCoord3dv_ptr) __glim_TexCoord3dv) ( v );
}

void glTexCoord3f ( GLfloat s, GLfloat t, GLfloat r )
{
	((glTexCoord3f_ptr) __glim_TexCoord3f) ( s , t , r );
}

void glTexCoord3fv ( const GLfloat *v )
{
	((glTexCoord3fv_ptr) __glim_TexCoord3fv) ( v );
}

void glTexCoord3i ( GLint s, GLint t, GLint r )
{
	((glTexCoord3i_ptr) __glim_TexCoord3i) ( s , t , r );
}

void glTexCoord3iv ( const GLint *v )
{
	((glTexCoord3iv_ptr) __glim_TexCoord3iv) ( v );
}

void glTexCoord3s ( GLshort s, GLshort t, GLshort r )
{
	((glTexCoord3s_ptr) __glim_TexCoord3s) ( s , t , r );
}

void glTexCoord3sv ( const GLshort *v )
{
	((glTexCoord3sv_ptr) __glim_TexCoord3sv) ( v );
}

void glTexCoord4d ( GLdouble s, GLdouble t, GLdouble r, GLdouble q )
{
	((glTexCoord4d_ptr) __glim_TexCoord4d) ( s , t , r , q );
}

void glTexCoord4dv ( const GLdouble *v )
{
	((glTexCoord4dv_ptr) __glim_TexCoord4dv) ( v );
}

void glTexCoord4f ( GLfloat s, GLfloat t, GLfloat r, GLfloat q )
{
	((glTexCoord4f_ptr) __glim_TexCoord4f) ( s , t , r , q );
}

void glTexCoord4fv ( const GLfloat *v )
{
	((glTexCoord4fv_ptr) __glim_TexCoord4fv) ( v );
}

void glTexCoord4i ( GLint s, GLint t, GLint r, GLint q )
{
	((glTexCoord4i_ptr) __glim_TexCoord4i) ( s , t , r , q );
}

void glTexCoord4iv ( const GLint *v )
{
	((glTexCoord4iv_ptr) __glim_TexCoord4iv) ( v );
}

void glTexCoord4s ( GLshort s, GLshort t, GLshort r, GLshort q )
{
	((glTexCoord4s_ptr) __glim_TexCoord4s) ( s , t , r , q );
}

void glTexCoord4sv ( const GLshort *v )
{
	((glTexCoord4sv_ptr) __glim_TexCoord4sv) ( v );
}

void glTexCoordPointer ( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	((glTexCoordPointer_ptr) __glim_TexCoordPointer) ( size , type , stride , pointer );
}

void glTexEnvf ( GLenum target, GLenum pname, GLfloat param )
{
	((glTexEnvf_ptr) __glim_TexEnvf) ( target , pname , param );
}

void glTexEnvfv ( GLenum target, GLenum pname, const GLfloat *params )
{
	((glTexEnvfv_ptr) __glim_TexEnvfv) ( target , pname , params );
}

void glTexEnvi ( GLenum target, GLenum pname, GLint param )
{
	((glTexEnvi_ptr) __glim_TexEnvi) ( target , pname , param );
}

void glTexEnviv ( GLenum target, GLenum pname, const GLint *params )
{
	((glTexEnviv_ptr) __glim_TexEnviv) ( target , pname , params );
}

void glTexGend ( GLenum coord, GLenum pname, GLdouble param )
{
	((glTexGend_ptr) __glim_TexGend) ( coord , pname , param );
}

void glTexGendv ( GLenum coord, GLenum pname, const GLdouble *params )
{
	((glTexGendv_ptr) __glim_TexGendv) ( coord , pname , params );
}

void glTexGenf ( GLenum coord, GLenum pname, GLfloat param )
{
	((glTexGenf_ptr) __glim_TexGenf) ( coord , pname , param );
}

void glTexGenfv ( GLenum coord, GLenum pname, const GLfloat *params )
{
	((glTexGenfv_ptr) __glim_TexGenfv) ( coord , pname , params );
}

void glTexGeni ( GLenum coord, GLenum pname, GLint param )
{
	((glTexGeni_ptr) __glim_TexGeni) ( coord , pname , param );
}

void glTexGeniv ( GLenum coord, GLenum pname, const GLint *params )
{
	((glTexGeniv_ptr) __glim_TexGeniv) ( coord , pname , params );
}

void glTexImage1D ( GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels )
{
	((glTexImage1D_ptr) __glim_TexImage1D) ( target , level , internalformat , width , border , format , type , pixels );
}

void glTexImage2D ( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels )
{
	((glTexImage2D_ptr) __glim_TexImage2D) ( target , level , internalformat , width , height , border , format , type , pixels );
}

void glTexParameterf ( GLenum target, GLenum pname, GLfloat param )
{
	((glTexParameterf_ptr) __glim_TexParameterf) ( target , pname , param );
}

void glTexParameterfv ( GLenum target, GLenum pname, const GLfloat *params )
{
	((glTexParameterfv_ptr) __glim_TexParameterfv) ( target , pname , params );
}

void glTexParameteri ( GLenum target, GLenum pname, GLint param )
{
	((glTexParameteri_ptr) __glim_TexParameteri) ( target , pname , param );
}

void glTexParameteriv ( GLenum target, GLenum pname, const GLint *params )
{
	((glTexParameteriv_ptr) __glim_TexParameteriv) ( target , pname , params );
}

void glTexSubImage1D ( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels )
{
	((glTexSubImage1D_ptr) __glim_TexSubImage1D) ( target , level , xoffset , width , format , type , pixels );
}

void glTexSubImage2D ( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels )
{
	((glTexSubImage2D_ptr) __glim_TexSubImage2D) ( target , level , xoffset , yoffset , width , height , format , type , pixels );
}

void glTranslated ( GLdouble x, GLdouble y, GLdouble z )
{
	((glTranslated_ptr) __glim_Translated) ( x , y , z );
}

void glTranslatef ( GLfloat x, GLfloat y, GLfloat z )
{
	((glTranslatef_ptr) __glim_Translatef) ( x , y , z );
}

void glVertex2d ( GLdouble x, GLdouble y )
{
	((glVertex2d_ptr) __glim_Vertex2d) ( x , y );
}

void glVertex2dv ( const GLdouble *v )
{
	((glVertex2dv_ptr) __glim_Vertex2dv) ( v );
}

void glVertex2f ( GLfloat x, GLfloat y )
{
	((glVertex2f_ptr) __glim_Vertex2f) ( x , y );
}

void glVertex2fv ( const GLfloat *v )
{
	((glVertex2fv_ptr) __glim_Vertex2fv) ( v );
}

void glVertex2i ( GLint x, GLint y )
{
	((glVertex2i_ptr) __glim_Vertex2i) ( x , y );
}

void glVertex2iv ( const GLint *v )
{
	((glVertex2iv_ptr) __glim_Vertex2iv) ( v );
}

void glVertex2s ( GLshort x, GLshort y )
{
	((glVertex2s_ptr) __glim_Vertex2s) ( x , y );
}

void glVertex2sv ( const GLshort *v )
{
	((glVertex2sv_ptr) __glim_Vertex2sv) ( v );
}

void glVertex3d ( GLdouble x, GLdouble y, GLdouble z )
{
	((glVertex3d_ptr) __glim_Vertex3d) ( x , y , z );
}

void glVertex3dv ( const GLdouble *v )
{
	((glVertex3dv_ptr) __glim_Vertex3dv) ( v );
}

void glVertex3f ( GLfloat x, GLfloat y, GLfloat z )
{
	((glVertex3f_ptr) __glim_Vertex3f) ( x , y , z );
}

void glVertex3fv ( const GLfloat *v )
{
	((glVertex3fv_ptr) __glim_Vertex3fv) ( v );
}

void glVertex3i ( GLint x, GLint y, GLint z )
{
	((glVertex3i_ptr) __glim_Vertex3i) ( x , y , z );
}

void glVertex3iv ( const GLint *v )
{
	((glVertex3iv_ptr) __glim_Vertex3iv) ( v );
}

void glVertex3s ( GLshort x, GLshort y, GLshort z )
{
	((glVertex3s_ptr) __glim_Vertex3s) ( x , y , z );
}

void glVertex3sv ( const GLshort *v )
{
	((glVertex3sv_ptr) __glim_Vertex3sv) ( v );
}

void glVertex4d ( GLdouble x, GLdouble y, GLdouble z, GLdouble w )
{
	((glVertex4d_ptr) __glim_Vertex4d) ( x , y , z , w );
}

void glVertex4dv ( const GLdouble *v )
{
	((glVertex4dv_ptr) __glim_Vertex4dv) ( v );
}

void glVertex4f ( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
	((glVertex4f_ptr) __glim_Vertex4f) ( x , y , z , w );
}

void glVertex4fv ( const GLfloat *v )
{
	((glVertex4fv_ptr) __glim_Vertex4fv) ( v );
}

void glVertex4i ( GLint x, GLint y, GLint z, GLint w )
{
	((glVertex4i_ptr) __glim_Vertex4i) ( x , y , z , w );
}

void glVertex4iv ( const GLint *v )
{
	((glVertex4iv_ptr) __glim_Vertex4iv) ( v );
}

void glVertex4s ( GLshort x, GLshort y, GLshort z, GLshort w )
{
	((glVertex4s_ptr) __glim_Vertex4s) ( x , y , z , w );
}

void glVertex4sv ( const GLshort *v )
{
	((glVertex4sv_ptr) __glim_Vertex4sv) ( v );
}

void glVertexPointer ( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	((glVertexPointer_ptr) __glim_VertexPointer) ( size , type , stride , pointer );
}

void glViewport ( GLint x, GLint y, GLsizei width, GLsizei height )
{
	((glViewport_ptr) __glim_Viewport) ( x , y , width , height );
}

void glWriteback ( GLint *writeback )
{
	((glWriteback_ptr) __glim_Writeback) ( writeback );
}

