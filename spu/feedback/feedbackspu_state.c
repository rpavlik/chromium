/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */
	

#include <stdio.h>
#include "cr_server.h"
#include "feedbackspu.h"

void FEEDBACKSPU_APIENTRY feedbackspu_ActiveTextureARB( GLenum texture )
{
	crStateActiveTextureARB( texture );

	feedback_spu.super.ActiveTextureARB( texture );
}
void FEEDBACKSPU_APIENTRY feedbackspu_AlphaFunc( GLenum func, GLclampf ref )
{
	crStateAlphaFunc( func, ref );

	feedback_spu.super.AlphaFunc( func, ref );
}
void FEEDBACKSPU_APIENTRY feedbackspu_BlendColorEXT( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )
{
	crStateBlendColorEXT( red, green, blue, alpha );

	feedback_spu.super.BlendColorEXT( red, green, blue, alpha );
}
void FEEDBACKSPU_APIENTRY feedbackspu_BlendFunc( GLenum sfactor, GLenum dfactor )
{
	crStateBlendFunc( sfactor, dfactor );

	feedback_spu.super.BlendFunc( sfactor, dfactor );
}
void FEEDBACKSPU_APIENTRY feedbackspu_ClearAccum( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{
	crStateClearAccum( red, green, blue, alpha );

	feedback_spu.super.ClearAccum( red, green, blue, alpha );
}
void FEEDBACKSPU_APIENTRY feedbackspu_ClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )
{
	crStateClearColor( red, green, blue, alpha );

	feedback_spu.super.ClearColor( red, green, blue, alpha );
}
void FEEDBACKSPU_APIENTRY feedbackspu_ClearDepth( GLclampd depth )
{
	crStateClearDepth( depth );

	feedback_spu.super.ClearDepth( depth );
}
void FEEDBACKSPU_APIENTRY feedbackspu_ClearIndex( GLfloat c )
{
	crStateClearIndex( c );

	feedback_spu.super.ClearIndex( c );
}
void FEEDBACKSPU_APIENTRY feedbackspu_ClearStencil( GLint s )
{
	crStateClearStencil( s );

	feedback_spu.super.ClearStencil( s );
}
void FEEDBACKSPU_APIENTRY feedbackspu_ClientActiveTextureARB( GLenum texture )
{
	crStateClientActiveTextureARB( texture );

	feedback_spu.super.ClientActiveTextureARB( texture );
}
void FEEDBACKSPU_APIENTRY feedbackspu_ClipPlane( GLenum plane, const GLdouble *equation )
{
	crStateClipPlane( plane, equation );

	feedback_spu.super.ClipPlane( plane, equation );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Color3f( GLfloat red, GLfloat green, GLfloat blue )
{
	crStateColor3f( red, green, blue );

	feedback_spu.super.Color3f( red, green, blue );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Color3fv( const GLfloat *v )
{
	crStateColor3fv( v );

	feedback_spu.super.Color3fv( v );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Color4f( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{
	crStateColor4f( red, green, blue, alpha );

	feedback_spu.super.Color4f( red, green, blue, alpha );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Color4fv( const GLfloat *v )
{
	crStateColor4fv( v );

	feedback_spu.super.Color4fv( v );
}
void FEEDBACKSPU_APIENTRY feedbackspu_ColorMask( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha )
{
	crStateColorMask( red, green, blue, alpha );

	feedback_spu.super.ColorMask( red, green, blue, alpha );
}
void FEEDBACKSPU_APIENTRY feedbackspu_ColorMaterial( GLenum face, GLenum mode )
{
	crStateColorMaterial( face, mode );

	feedback_spu.super.ColorMaterial( face, mode );
}
void FEEDBACKSPU_APIENTRY feedbackspu_ColorPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	crStateColorPointer( size, type, stride, pointer );

	feedback_spu.super.ColorPointer( size, type, stride, pointer );
}
void FEEDBACKSPU_APIENTRY feedbackspu_CombinerInputNV( GLenum stage, GLenum portion, GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage )
{
	crStateCombinerInputNV( stage, portion, variable, input, mapping, componentUsage );

	feedback_spu.super.CombinerInputNV( stage, portion, variable, input, mapping, componentUsage );
}
void FEEDBACKSPU_APIENTRY feedbackspu_CombinerOutputNV( GLenum stage, GLenum portion, GLenum abOutput, GLenum cdOutput, GLenum sumOutput, GLenum scale, GLenum bias, GLboolean abDotProduct, GLboolean cdDotProduct, GLboolean muxSum )
{
	crStateCombinerOutputNV( stage, portion, abOutput, cdOutput, sumOutput, scale, bias, abDotProduct, cdDotProduct, muxSum );

	feedback_spu.super.CombinerOutputNV( stage, portion, abOutput, cdOutput, sumOutput, scale, bias, abDotProduct, cdDotProduct, muxSum );
}
void FEEDBACKSPU_APIENTRY feedbackspu_CombinerParameterfNV( GLenum pname, GLfloat param )
{
	crStateCombinerParameterfNV( pname, param );

	feedback_spu.super.CombinerParameterfNV( pname, param );
}
void FEEDBACKSPU_APIENTRY feedbackspu_CombinerParameterfvNV( GLenum pname, const GLfloat *params )
{
	crStateCombinerParameterfvNV( pname, params );

	feedback_spu.super.CombinerParameterfvNV( pname, params );
}
void FEEDBACKSPU_APIENTRY feedbackspu_CombinerParameteriNV( GLenum pname, GLint param )
{
	crStateCombinerParameteriNV( pname, param );

	feedback_spu.super.CombinerParameteriNV( pname, param );
}
void FEEDBACKSPU_APIENTRY feedbackspu_CombinerParameterivNV( GLenum pname, const GLint *params )
{
	crStateCombinerParameterivNV( pname, params );

	feedback_spu.super.CombinerParameterivNV( pname, params );
}
void FEEDBACKSPU_APIENTRY feedbackspu_CombinerStageParameterfvNV( GLenum stage, GLenum pname, const GLfloat *params )
{
	crStateCombinerStageParameterfvNV( stage, pname, params );

	feedback_spu.super.CombinerStageParameterfvNV( stage, pname, params );
}
void FEEDBACKSPU_APIENTRY feedbackspu_CullFace( GLenum mode )
{
	crStateCullFace( mode );

	feedback_spu.super.CullFace( mode );
}
void FEEDBACKSPU_APIENTRY feedbackspu_DeleteLists( GLuint list, GLsizei range )
{
	crStateDeleteLists( list, range );

	feedback_spu.super.DeleteLists( list, range );
}
void FEEDBACKSPU_APIENTRY feedbackspu_DeleteTextures( GLsizei n, const GLuint *textures )
{
	crStateDeleteTextures( n, textures );

	feedback_spu.super.DeleteTextures( n, textures );
}
void FEEDBACKSPU_APIENTRY feedbackspu_DepthFunc( GLenum func )
{
	crStateDepthFunc( func );

	feedback_spu.super.DepthFunc( func );
}
void FEEDBACKSPU_APIENTRY feedbackspu_DepthMask( GLboolean flag )
{
	crStateDepthMask( flag );

	feedback_spu.super.DepthMask( flag );
}
void FEEDBACKSPU_APIENTRY feedbackspu_DepthRange( GLclampd zNear, GLclampd zFar )
{
	crStateDepthRange( zNear, zFar );

	feedback_spu.super.DepthRange( zNear, zFar );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Disable( GLenum cap )
{
	crStateDisable( cap );

	feedback_spu.super.Disable( cap );
}
void FEEDBACKSPU_APIENTRY feedbackspu_DisableClientState( GLenum array )
{
	crStateDisableClientState( array );

	feedback_spu.super.DisableClientState( array );
}
void FEEDBACKSPU_APIENTRY feedbackspu_DrawBuffer( GLenum mode )
{
	crStateDrawBuffer( mode );

	feedback_spu.super.DrawBuffer( mode );
}
void FEEDBACKSPU_APIENTRY feedbackspu_EdgeFlagPointer( GLsizei stride, const GLvoid *pointer )
{
	crStateEdgeFlagPointer( stride, pointer );

	feedback_spu.super.EdgeFlagPointer( stride, pointer );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Enable( GLenum cap )
{
	crStateEnable( cap );

	feedback_spu.super.Enable( cap );
}
void FEEDBACKSPU_APIENTRY feedbackspu_EnableClientState( GLenum array )
{
	crStateEnableClientState( array );

	feedback_spu.super.EnableClientState( array );
}
void FEEDBACKSPU_APIENTRY feedbackspu_EndList( void )
{
	crStateEndList(  );

	feedback_spu.super.EndList(  );
}
void FEEDBACKSPU_APIENTRY feedbackspu_FinalCombinerInputNV( GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage )
{
	crStateFinalCombinerInputNV( variable, input, mapping, componentUsage );

	feedback_spu.super.FinalCombinerInputNV( variable, input, mapping, componentUsage );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Fogf( GLenum pname, GLfloat param )
{
	crStateFogf( pname, param );

	feedback_spu.super.Fogf( pname, param );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Fogfv( GLenum pname, const GLfloat *params )
{
	crStateFogfv( pname, params );

	feedback_spu.super.Fogfv( pname, params );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Fogi( GLenum pname, GLint param )
{
	crStateFogi( pname, param );

	feedback_spu.super.Fogi( pname, param );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Fogiv( GLenum pname, const GLint *params )
{
	crStateFogiv( pname, params );

	feedback_spu.super.Fogiv( pname, params );
}
void FEEDBACKSPU_APIENTRY feedbackspu_FrontFace( GLenum mode )
{
	crStateFrontFace( mode );

	feedback_spu.super.FrontFace( mode );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Frustum( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar )
{
	crStateFrustum( left, right, bottom, top, zNear, zFar );

	feedback_spu.super.Frustum( left, right, bottom, top, zNear, zFar );
}
void FEEDBACKSPU_APIENTRY feedbackspu_GenTextures( GLsizei n, GLuint *textures )
{
	crStateGenTextures( n, textures );

	feedback_spu.super.GenTextures( n, textures );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Hint( GLenum target, GLenum mode )
{
	crStateHint( target, mode );

	feedback_spu.super.Hint( target, mode );
}
void FEEDBACKSPU_APIENTRY feedbackspu_IndexMask( GLuint mask )
{
	crStateIndexMask( mask );

	feedback_spu.super.IndexMask( mask );
}
void FEEDBACKSPU_APIENTRY feedbackspu_IndexPointer( GLenum type, GLsizei stride, const GLvoid *pointer )
{
	crStateIndexPointer( type, stride, pointer );

	feedback_spu.super.IndexPointer( type, stride, pointer );
}
void FEEDBACKSPU_APIENTRY feedbackspu_InitNames( void )
{
	crStateInitNames(  );

	feedback_spu.super.InitNames(  );
}
void FEEDBACKSPU_APIENTRY feedbackspu_InterleavedArrays( GLenum format, GLsizei stride, const GLvoid *pointer )
{
	crStateInterleavedArrays( format, stride, pointer );

	feedback_spu.super.InterleavedArrays( format, stride, pointer );
}
void FEEDBACKSPU_APIENTRY feedbackspu_LightModelf( GLenum pname, GLfloat param )
{
	crStateLightModelf( pname, param );

	feedback_spu.super.LightModelf( pname, param );
}
void FEEDBACKSPU_APIENTRY feedbackspu_LightModelfv( GLenum pname, const GLfloat *params )
{
	crStateLightModelfv( pname, params );

	feedback_spu.super.LightModelfv( pname, params );
}
void FEEDBACKSPU_APIENTRY feedbackspu_LightModeli( GLenum pname, GLint param )
{
	crStateLightModeli( pname, param );

	feedback_spu.super.LightModeli( pname, param );
}
void FEEDBACKSPU_APIENTRY feedbackspu_LightModeliv( GLenum pname, const GLint *params )
{
	crStateLightModeliv( pname, params );

	feedback_spu.super.LightModeliv( pname, params );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Lightf( GLenum light, GLenum pname, GLfloat param )
{
	crStateLightf( light, pname, param );

	feedback_spu.super.Lightf( light, pname, param );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Lightfv( GLenum light, GLenum pname, const GLfloat *params )
{
	crStateLightfv( light, pname, params );

	feedback_spu.super.Lightfv( light, pname, params );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Lighti( GLenum light, GLenum pname, GLint param )
{
	crStateLighti( light, pname, param );

	feedback_spu.super.Lighti( light, pname, param );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Lightiv( GLenum light, GLenum pname, const GLint *params )
{
	crStateLightiv( light, pname, params );

	feedback_spu.super.Lightiv( light, pname, params );
}
void FEEDBACKSPU_APIENTRY feedbackspu_LineStipple( GLint factor, GLushort pattern )
{
	crStateLineStipple( factor, pattern );

	feedback_spu.super.LineStipple( factor, pattern );
}
void FEEDBACKSPU_APIENTRY feedbackspu_LineWidth( GLfloat width )
{
	crStateLineWidth( width );

	feedback_spu.super.LineWidth( width );
}
void FEEDBACKSPU_APIENTRY feedbackspu_ListBase( GLuint base )
{
	crStateListBase( base );

	feedback_spu.super.ListBase( base );
}
void FEEDBACKSPU_APIENTRY feedbackspu_LoadIdentity( void )
{
	crStateLoadIdentity(  );

	feedback_spu.super.LoadIdentity(  );
}
void FEEDBACKSPU_APIENTRY feedbackspu_LoadMatrixd( const GLdouble *m )
{
	crStateLoadMatrixd( m );

	feedback_spu.super.LoadMatrixd( m );
}
void FEEDBACKSPU_APIENTRY feedbackspu_LoadMatrixf( const GLfloat *m )
{
	crStateLoadMatrixf( m );

	feedback_spu.super.LoadMatrixf( m );
}
void FEEDBACKSPU_APIENTRY feedbackspu_LoadName( GLuint name )
{
	crStateLoadName( name );

	feedback_spu.super.LoadName( name );
}
void FEEDBACKSPU_APIENTRY feedbackspu_LogicOp( GLenum opcode )
{
	crStateLogicOp( opcode );

	feedback_spu.super.LogicOp( opcode );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Map1d( GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points )
{
	crStateMap1d( target, u1, u2, stride, order, points );

	feedback_spu.super.Map1d( target, u1, u2, stride, order, points );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Map1f( GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points )
{
	crStateMap1f( target, u1, u2, stride, order, points );

	feedback_spu.super.Map1f( target, u1, u2, stride, order, points );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Map2d( GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points )
{
	crStateMap2d( target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points );

	feedback_spu.super.Map2d( target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Map2f( GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points )
{
	crStateMap2f( target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points );

	feedback_spu.super.Map2f( target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points );
}
void FEEDBACKSPU_APIENTRY feedbackspu_MapGrid1d( GLint un, GLdouble u1, GLdouble u2 )
{
	crStateMapGrid1d( un, u1, u2 );

	feedback_spu.super.MapGrid1d( un, u1, u2 );
}
void FEEDBACKSPU_APIENTRY feedbackspu_MapGrid1f( GLint un, GLfloat u1, GLfloat u2 )
{
	crStateMapGrid1f( un, u1, u2 );

	feedback_spu.super.MapGrid1f( un, u1, u2 );
}
void FEEDBACKSPU_APIENTRY feedbackspu_MapGrid2d( GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2 )
{
	crStateMapGrid2d( un, u1, u2, vn, v1, v2 );

	feedback_spu.super.MapGrid2d( un, u1, u2, vn, v1, v2 );
}
void FEEDBACKSPU_APIENTRY feedbackspu_MapGrid2f( GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2 )
{
	crStateMapGrid2f( un, u1, u2, vn, v1, v2 );

	feedback_spu.super.MapGrid2f( un, u1, u2, vn, v1, v2 );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Materialf( GLenum face, GLenum pname, GLfloat param )
{
	crStateMaterialf( face, pname, param );

	feedback_spu.super.Materialf( face, pname, param );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Materialfv( GLenum face, GLenum pname, const GLfloat *params )
{
	crStateMaterialfv( face, pname, params );

	feedback_spu.super.Materialfv( face, pname, params );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Materiali( GLenum face, GLenum pname, GLint param )
{
	crStateMateriali( face, pname, param );

	feedback_spu.super.Materiali( face, pname, param );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Materialiv( GLenum face, GLenum pname, const GLint *params )
{
	crStateMaterialiv( face, pname, params );

	feedback_spu.super.Materialiv( face, pname, params );
}
void FEEDBACKSPU_APIENTRY feedbackspu_MatrixMode( GLenum mode )
{
	crStateMatrixMode( mode );

	feedback_spu.super.MatrixMode( mode );
}
void FEEDBACKSPU_APIENTRY feedbackspu_MultMatrixd( const GLdouble *m )
{
	crStateMultMatrixd( m );

	feedback_spu.super.MultMatrixd( m );
}
void FEEDBACKSPU_APIENTRY feedbackspu_MultMatrixf( const GLfloat *m )
{
	crStateMultMatrixf( m );

	feedback_spu.super.MultMatrixf( m );
}
void FEEDBACKSPU_APIENTRY feedbackspu_NewList( GLuint list, GLenum mode )
{
	crStateNewList( list, mode );

	feedback_spu.super.NewList( list, mode );
}
void FEEDBACKSPU_APIENTRY feedbackspu_NormalPointer( GLenum type, GLsizei stride, const GLvoid *pointer )
{
	crStateNormalPointer( type, stride, pointer );

	feedback_spu.super.NormalPointer( type, stride, pointer );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Ortho( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar )
{
	crStateOrtho( left, right, bottom, top, zNear, zFar );

	feedback_spu.super.Ortho( left, right, bottom, top, zNear, zFar );
}
void FEEDBACKSPU_APIENTRY feedbackspu_PassThrough( GLfloat token )
{
	crStatePassThrough( token );

	feedback_spu.super.PassThrough( token );
}
void FEEDBACKSPU_APIENTRY feedbackspu_PixelMapfv( GLenum map, GLsizei mapsize, const GLfloat *values )
{
	crStatePixelMapfv( map, mapsize, values );

	feedback_spu.super.PixelMapfv( map, mapsize, values );
}
void FEEDBACKSPU_APIENTRY feedbackspu_PixelMapuiv( GLenum map, GLsizei mapsize, const GLuint *values )
{
	crStatePixelMapuiv( map, mapsize, values );

	feedback_spu.super.PixelMapuiv( map, mapsize, values );
}
void FEEDBACKSPU_APIENTRY feedbackspu_PixelMapusv( GLenum map, GLsizei mapsize, const GLushort *values )
{
	crStatePixelMapusv( map, mapsize, values );

	feedback_spu.super.PixelMapusv( map, mapsize, values );
}
void FEEDBACKSPU_APIENTRY feedbackspu_PixelStoref( GLenum pname, GLfloat param )
{
	crStatePixelStoref( pname, param );

	feedback_spu.super.PixelStoref( pname, param );
}
void FEEDBACKSPU_APIENTRY feedbackspu_PixelStorei( GLenum pname, GLint param )
{
	crStatePixelStorei( pname, param );

	feedback_spu.super.PixelStorei( pname, param );
}
void FEEDBACKSPU_APIENTRY feedbackspu_PixelTransferf( GLenum pname, GLfloat param )
{
	crStatePixelTransferf( pname, param );

	feedback_spu.super.PixelTransferf( pname, param );
}
void FEEDBACKSPU_APIENTRY feedbackspu_PixelTransferi( GLenum pname, GLint param )
{
	crStatePixelTransferi( pname, param );

	feedback_spu.super.PixelTransferi( pname, param );
}
void FEEDBACKSPU_APIENTRY feedbackspu_PixelZoom( GLfloat xfactor, GLfloat yfactor )
{
	crStatePixelZoom( xfactor, yfactor );

	feedback_spu.super.PixelZoom( xfactor, yfactor );
}
void FEEDBACKSPU_APIENTRY feedbackspu_PointSize( GLfloat size )
{
	crStatePointSize( size );

	feedback_spu.super.PointSize( size );
}
void FEEDBACKSPU_APIENTRY feedbackspu_PolygonMode( GLenum face, GLenum mode )
{
	crStatePolygonMode( face, mode );

	feedback_spu.super.PolygonMode( face, mode );
}
void FEEDBACKSPU_APIENTRY feedbackspu_PolygonOffset( GLfloat factor, GLfloat units )
{
	crStatePolygonOffset( factor, units );

	feedback_spu.super.PolygonOffset( factor, units );
}
void FEEDBACKSPU_APIENTRY feedbackspu_PolygonStipple( const GLubyte *mask )
{
	crStatePolygonStipple( mask );

	feedback_spu.super.PolygonStipple( mask );
}
void FEEDBACKSPU_APIENTRY feedbackspu_PopAttrib( void )
{
	crStatePopAttrib(  );

	feedback_spu.super.PopAttrib(  );
}
void FEEDBACKSPU_APIENTRY feedbackspu_PopMatrix( void )
{
	crStatePopMatrix(  );

	feedback_spu.super.PopMatrix(  );
}
void FEEDBACKSPU_APIENTRY feedbackspu_PopName( void )
{
	crStatePopName(  );

	feedback_spu.super.PopName(  );
}
void FEEDBACKSPU_APIENTRY feedbackspu_PushAttrib( GLbitfield mask )
{
	crStatePushAttrib( mask );

	feedback_spu.super.PushAttrib( mask );
}
void FEEDBACKSPU_APIENTRY feedbackspu_PushMatrix( void )
{
	crStatePushMatrix(  );

	feedback_spu.super.PushMatrix(  );
}
void FEEDBACKSPU_APIENTRY feedbackspu_PushName( GLuint name )
{
	crStatePushName( name );

	feedback_spu.super.PushName( name );
}
void FEEDBACKSPU_APIENTRY feedbackspu_ReadBuffer( GLenum mode )
{
	crStateReadBuffer( mode );

	feedback_spu.super.ReadBuffer( mode );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Rotated( GLdouble angle, GLdouble x, GLdouble y, GLdouble z )
{
	crStateRotated( angle, x, y, z );

	feedback_spu.super.Rotated( angle, x, y, z );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Rotatef( GLfloat angle, GLfloat x, GLfloat y, GLfloat z )
{
	crStateRotatef( angle, x, y, z );

	feedback_spu.super.Rotatef( angle, x, y, z );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Scaled( GLdouble x, GLdouble y, GLdouble z )
{
	crStateScaled( x, y, z );

	feedback_spu.super.Scaled( x, y, z );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Scalef( GLfloat x, GLfloat y, GLfloat z )
{
	crStateScalef( x, y, z );

	feedback_spu.super.Scalef( x, y, z );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Scissor( GLint x, GLint y, GLsizei width, GLsizei height )
{
	crStateScissor( x, y, width, height );

	feedback_spu.super.Scissor( x, y, width, height );
}
void FEEDBACKSPU_APIENTRY feedbackspu_SecondaryColorPointerEXT( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	crStateSecondaryColorPointerEXT( size, type, stride, pointer );

	feedback_spu.super.SecondaryColorPointerEXT( size, type, stride, pointer );
}
void FEEDBACKSPU_APIENTRY feedbackspu_ShadeModel( GLenum mode )
{
	crStateShadeModel( mode );

	feedback_spu.super.ShadeModel( mode );
}
void FEEDBACKSPU_APIENTRY feedbackspu_StencilFunc( GLenum func, GLint ref, GLuint mask )
{
	crStateStencilFunc( func, ref, mask );

	feedback_spu.super.StencilFunc( func, ref, mask );
}
void FEEDBACKSPU_APIENTRY feedbackspu_StencilMask( GLuint mask )
{
	crStateStencilMask( mask );

	feedback_spu.super.StencilMask( mask );
}
void FEEDBACKSPU_APIENTRY feedbackspu_StencilOp( GLenum fail, GLenum zfail, GLenum zpass )
{
	crStateStencilOp( fail, zfail, zpass );

	feedback_spu.super.StencilOp( fail, zfail, zpass );
}
void FEEDBACKSPU_APIENTRY feedbackspu_TexCoordPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	crStateTexCoordPointer( size, type, stride, pointer );

	feedback_spu.super.TexCoordPointer( size, type, stride, pointer );
}
void FEEDBACKSPU_APIENTRY feedbackspu_TexEnvf( GLenum target, GLenum pname, GLfloat param )
{
	crStateTexEnvf( target, pname, param );

	feedback_spu.super.TexEnvf( target, pname, param );
}
void FEEDBACKSPU_APIENTRY feedbackspu_TexEnvfv( GLenum target, GLenum pname, const GLfloat *params )
{
	crStateTexEnvfv( target, pname, params );

	feedback_spu.super.TexEnvfv( target, pname, params );
}
void FEEDBACKSPU_APIENTRY feedbackspu_TexEnvi( GLenum target, GLenum pname, GLint param )
{
	crStateTexEnvi( target, pname, param );

	feedback_spu.super.TexEnvi( target, pname, param );
}
void FEEDBACKSPU_APIENTRY feedbackspu_TexEnviv( GLenum target, GLenum pname, const GLint *params )
{
	crStateTexEnviv( target, pname, params );

	feedback_spu.super.TexEnviv( target, pname, params );
}
void FEEDBACKSPU_APIENTRY feedbackspu_TexGend( GLenum coord, GLenum pname, GLdouble param )
{
	crStateTexGend( coord, pname, param );

	feedback_spu.super.TexGend( coord, pname, param );
}
void FEEDBACKSPU_APIENTRY feedbackspu_TexGendv( GLenum coord, GLenum pname, const GLdouble *params )
{
	crStateTexGendv( coord, pname, params );

	feedback_spu.super.TexGendv( coord, pname, params );
}
void FEEDBACKSPU_APIENTRY feedbackspu_TexGenf( GLenum coord, GLenum pname, GLfloat param )
{
	crStateTexGenf( coord, pname, param );

	feedback_spu.super.TexGenf( coord, pname, param );
}
void FEEDBACKSPU_APIENTRY feedbackspu_TexGenfv( GLenum coord, GLenum pname, const GLfloat *params )
{
	crStateTexGenfv( coord, pname, params );

	feedback_spu.super.TexGenfv( coord, pname, params );
}
void FEEDBACKSPU_APIENTRY feedbackspu_TexGeni( GLenum coord, GLenum pname, GLint param )
{
	crStateTexGeni( coord, pname, param );

	feedback_spu.super.TexGeni( coord, pname, param );
}
void FEEDBACKSPU_APIENTRY feedbackspu_TexGeniv( GLenum coord, GLenum pname, const GLint *params )
{
	crStateTexGeniv( coord, pname, params );

	feedback_spu.super.TexGeniv( coord, pname, params );
}
void FEEDBACKSPU_APIENTRY feedbackspu_TexParameterf( GLenum target, GLenum pname, GLfloat param )
{
	crStateTexParameterf( target, pname, param );

	feedback_spu.super.TexParameterf( target, pname, param );
}
void FEEDBACKSPU_APIENTRY feedbackspu_TexParameterfv( GLenum target, GLenum pname, const GLfloat *params )
{
	crStateTexParameterfv( target, pname, params );

	feedback_spu.super.TexParameterfv( target, pname, params );
}
void FEEDBACKSPU_APIENTRY feedbackspu_TexParameteri( GLenum target, GLenum pname, GLint param )
{
	crStateTexParameteri( target, pname, param );

	feedback_spu.super.TexParameteri( target, pname, param );
}
void FEEDBACKSPU_APIENTRY feedbackspu_TexParameteriv( GLenum target, GLenum pname, const GLint *params )
{
	crStateTexParameteriv( target, pname, params );

	feedback_spu.super.TexParameteriv( target, pname, params );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Translated( GLdouble x, GLdouble y, GLdouble z )
{
	crStateTranslated( x, y, z );

	feedback_spu.super.Translated( x, y, z );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Translatef( GLfloat x, GLfloat y, GLfloat z )
{
	crStateTranslatef( x, y, z );

	feedback_spu.super.Translatef( x, y, z );
}
void FEEDBACKSPU_APIENTRY feedbackspu_VertexPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	crStateVertexPointer( size, type, stride, pointer );

	feedback_spu.super.VertexPointer( size, type, stride, pointer );
}
void FEEDBACKSPU_APIENTRY feedbackspu_Viewport( GLint x, GLint y, GLsizei width, GLsizei height )
{
	crStateViewport( x, y, width, height );

	feedback_spu.super.Viewport( x, y, width, height );
}
