/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */
	
#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>

#include "api_templates.h"

void glAccum( GLenum op, GLfloat value )
{
	glim.Accum( op, value );
}

void glActiveTextureARB( GLenum texture )
{
	glim.ActiveTextureARB( texture );
}

void glAlphaFunc( GLenum func, GLclampf ref )
{
	glim.AlphaFunc( func, ref );
}

GLboolean glAreTexturesResident( GLsizei n, const GLuint *textures, GLboolean *residences )
{
	return  glim.AreTexturesResident( n, textures, residences );
}

void glArrayElement( GLint i )
{
	glim.ArrayElement( i );
}

void glBarrierCreate( GLuint name, GLuint count )
{
	glim.BarrierCreate( name, count );
}

void glBarrierDestroy( GLuint name )
{
	glim.BarrierDestroy( name );
}

void glBarrierExec( GLuint name )
{
	glim.BarrierExec( name );
}

void glBegin( GLenum mode )
{
	glim.Begin( mode );
}

void glBindTexture( GLenum target, GLuint texture )
{
	glim.BindTexture( target, texture );
}

void glBitmap( GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap )
{
	glim.Bitmap( width, height, xorig, yorig, xmove, ymove, bitmap );
}

void glBlendColorEXT( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )
{
	glim.BlendColorEXT( red, green, blue, alpha );
}

void glBlendEquationEXT( GLenum mode )
{
	glim.BlendEquationEXT( mode );
}

void glBlendFunc( GLenum sfactor, GLenum dfactor )
{
	glim.BlendFunc( sfactor, dfactor );
}

void glCallList( GLuint list )
{
	glim.CallList( list );
}

void glCallLists( GLsizei n, GLenum type, const GLvoid *lists )
{
	glim.CallLists( n, type, lists );
}

void glClear( GLbitfield mask )
{
	glim.Clear( mask );
}

void glClearAccum( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{
	glim.ClearAccum( red, green, blue, alpha );
}

void glClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )
{
	glim.ClearColor( red, green, blue, alpha );
}

void glClearDepth( GLclampd depth )
{
	glim.ClearDepth( depth );
}

void glClearIndex( GLfloat c )
{
	glim.ClearIndex( c );
}

void glClearStencil( GLint s )
{
	glim.ClearStencil( s );
}

void glClientActiveTextureARB( GLenum texture )
{
	glim.ClientActiveTextureARB( texture );
}

void glClipPlane( GLenum plane, const GLdouble *equation )
{
	glim.ClipPlane( plane, equation );
}

void glColor3b( GLbyte red, GLbyte green, GLbyte blue )
{
	glim.Color3b( red, green, blue );
}

void glColor3bv( const GLbyte *v )
{
	glim.Color3bv( v );
}

void glColor3d( GLdouble red, GLdouble green, GLdouble blue )
{
	glim.Color3d( red, green, blue );
}

void glColor3dv( const GLdouble *v )
{
	glim.Color3dv( v );
}

void glColor3f( GLfloat red, GLfloat green, GLfloat blue )
{
	glim.Color3f( red, green, blue );
}

void glColor3fv( const GLfloat *v )
{
	glim.Color3fv( v );
}

void glColor3i( GLint red, GLint green, GLint blue )
{
	glim.Color3i( red, green, blue );
}

void glColor3iv( const GLint *v )
{
	glim.Color3iv( v );
}

void glColor3s( GLshort red, GLshort green, GLshort blue )
{
	glim.Color3s( red, green, blue );
}

void glColor3sv( const GLshort *v )
{
	glim.Color3sv( v );
}

void glColor3ub( GLubyte red, GLubyte green, GLubyte blue )
{
	glim.Color3ub( red, green, blue );
}

void glColor3ubv( const GLubyte *v )
{
	glim.Color3ubv( v );
}

void glColor3ui( GLuint red, GLuint green, GLuint blue )
{
	glim.Color3ui( red, green, blue );
}

void glColor3uiv( const GLuint *v )
{
	glim.Color3uiv( v );
}

void glColor3us( GLushort red, GLushort green, GLushort blue )
{
	glim.Color3us( red, green, blue );
}

void glColor3usv( const GLushort *v )
{
	glim.Color3usv( v );
}

void glColor4b( GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha )
{
	glim.Color4b( red, green, blue, alpha );
}

void glColor4bv( const GLbyte *v )
{
	glim.Color4bv( v );
}

void glColor4d( GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha )
{
	glim.Color4d( red, green, blue, alpha );
}

void glColor4dv( const GLdouble *v )
{
	glim.Color4dv( v );
}

void glColor4f( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{
	glim.Color4f( red, green, blue, alpha );
}

void glColor4fv( const GLfloat *v )
{
	glim.Color4fv( v );
}

void glColor4i( GLint red, GLint green, GLint blue, GLint alpha )
{
	glim.Color4i( red, green, blue, alpha );
}

void glColor4iv( const GLint *v )
{
	glim.Color4iv( v );
}

void glColor4s( GLshort red, GLshort green, GLshort blue, GLshort alpha )
{
	glim.Color4s( red, green, blue, alpha );
}

void glColor4sv( const GLshort *v )
{
	glim.Color4sv( v );
}

void glColor4ub( GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha )
{
	glim.Color4ub( red, green, blue, alpha );
}

void glColor4ubv( const GLubyte *v )
{
	glim.Color4ubv( v );
}

void glColor4ui( GLuint red, GLuint green, GLuint blue, GLuint alpha )
{
	glim.Color4ui( red, green, blue, alpha );
}

void glColor4uiv( const GLuint *v )
{
	glim.Color4uiv( v );
}

void glColor4us( GLushort red, GLushort green, GLushort blue, GLushort alpha )
{
	glim.Color4us( red, green, blue, alpha );
}

void glColor4usv( const GLushort *v )
{
	glim.Color4usv( v );
}

void glColorMask( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha )
{
	glim.ColorMask( red, green, blue, alpha );
}

void glColorMaterial( GLenum face, GLenum mode )
{
	glim.ColorMaterial( face, mode );
}

void glColorPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	glim.ColorPointer( size, type, stride, pointer );
}

void glCombinerInputNV( GLenum stage, GLenum portion, GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage )
{
	glim.CombinerInputNV( stage, portion, variable, input, mapping, componentUsage );
}

void glCombinerOutputNV( GLenum stage, GLenum portion, GLenum abOutput, GLenum cdOutput, GLenum sumOutput, GLenum scale, GLenum bias, GLboolean abDotProduct, GLboolean cdDotProduct, GLboolean muxSum )
{
	glim.CombinerOutputNV( stage, portion, abOutput, cdOutput, sumOutput, scale, bias, abDotProduct, cdDotProduct, muxSum );
}

void glCombinerParameterfNV( GLenum pname, GLfloat param )
{
	glim.CombinerParameterfNV( pname, param );
}

void glCombinerParameterfvNV( GLenum pname, const GLfloat *params )
{
	glim.CombinerParameterfvNV( pname, params );
}

void glCombinerParameteriNV( GLenum pname, GLint param )
{
	glim.CombinerParameteriNV( pname, param );
}

void glCombinerParameterivNV( GLenum pname, const GLint *params )
{
	glim.CombinerParameterivNV( pname, params );
}

void glCombinerStageParameterfvNV( GLenum stage, GLenum pname, const GLfloat *params )
{
	glim.CombinerStageParameterfvNV( stage, pname, params );
}

void glCopyPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum type )
{
	glim.CopyPixels( x, y, width, height, type );
}

void glCopyTexImage1D( GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border )
{
	glim.CopyTexImage1D( target, level, internalFormat, x, y, width, border );
}

void glCopyTexImage2D( GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border )
{
	glim.CopyTexImage2D( target, level, internalFormat, x, y, width, height, border );
}

void glCopyTexSubImage1D( GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width )
{
	glim.CopyTexSubImage1D( target, level, xoffset, x, y, width );
}

void glCopyTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height )
{
	glim.CopyTexSubImage2D( target, level, xoffset, yoffset, x, y, width, height );
}

void glCreateContext( void *arg1, void *arg2 )
{
	glim.CreateContext( arg1, arg2 );
}

void glCullFace( GLenum mode )
{
	glim.CullFace( mode );
}

void glDeleteLists( GLuint list, GLsizei range )
{
	glim.DeleteLists( list, range );
}

void glDeleteTextures( GLsizei n, const GLuint *textures )
{
	glim.DeleteTextures( n, textures );
}

void glDepthFunc( GLenum func )
{
	glim.DepthFunc( func );
}

void glDepthMask( GLboolean flag )
{
	glim.DepthMask( flag );
}

void glDepthRange( GLclampd zNear, GLclampd zFar )
{
	glim.DepthRange( zNear, zFar );
}

void glDisable( GLenum cap )
{
	glim.Disable( cap );
}

void glDisableClientState( GLenum array )
{
	glim.DisableClientState( array );
}

void glDrawArrays( GLenum mode, GLint first, GLsizei count )
{
	glim.DrawArrays( mode, first, count );
}

void glDrawBuffer( GLenum mode )
{
	glim.DrawBuffer( mode );
}

void glDrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices )
{
	glim.DrawElements( mode, count, type, indices );
}

void glDrawPixels( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels )
{
	glim.DrawPixels( width, height, format, type, pixels );
}

void glEdgeFlag( GLboolean flag )
{
	glim.EdgeFlag( flag );
}

void glEdgeFlagv( const GLboolean *flag )
{
	glim.EdgeFlagv( flag );
}

void glEnable( GLenum cap )
{
	glim.Enable( cap );
}

void glEnableClientState( GLenum array )
{
	glim.EnableClientState( array );
}

void glEnd( void )
{
	glim.End(  );
}

void glEndList( void )
{
	glim.EndList(  );
}

void glEvalCoord1d( GLdouble u )
{
	glim.EvalCoord1d( u );
}

void glEvalCoord1dv( const GLdouble *u )
{
	glim.EvalCoord1dv( u );
}

void glEvalCoord1f( GLfloat u )
{
	glim.EvalCoord1f( u );
}

void glEvalCoord1fv( const GLfloat *u )
{
	glim.EvalCoord1fv( u );
}

void glEvalCoord2d( GLdouble u, GLdouble v )
{
	glim.EvalCoord2d( u, v );
}

void glEvalCoord2dv( const GLdouble *u )
{
	glim.EvalCoord2dv( u );
}

void glEvalCoord2f( GLfloat u, GLfloat v )
{
	glim.EvalCoord2f( u, v );
}

void glEvalCoord2fv( const GLfloat *u )
{
	glim.EvalCoord2fv( u );
}

void glEvalMesh1( GLenum mode, GLint i1, GLint i2 )
{
	glim.EvalMesh1( mode, i1, i2 );
}

void glEvalMesh2( GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2 )
{
	glim.EvalMesh2( mode, i1, i2, j1, j2 );
}

void glEvalPoint1( GLint i )
{
	glim.EvalPoint1( i );
}

void glEvalPoint2( GLint i, GLint j )
{
	glim.EvalPoint2( i, j );
}

void glFeedbackBuffer( GLsizei size, GLenum type, GLfloat *buffer )
{
	glim.FeedbackBuffer( size, type, buffer );
}

void glFinalCombinerInputNV( GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage )
{
	glim.FinalCombinerInputNV( variable, input, mapping, componentUsage );
}

void glFinish( void )
{
	glim.Finish(  );
}

void glFlush( void )
{
	glim.Flush(  );
}

void glFogf( GLenum pname, GLfloat param )
{
	glim.Fogf( pname, param );
}

void glFogfv( GLenum pname, const GLfloat *params )
{
	glim.Fogfv( pname, params );
}

void glFogi( GLenum pname, GLint param )
{
	glim.Fogi( pname, param );
}

void glFogiv( GLenum pname, const GLint *params )
{
	glim.Fogiv( pname, params );
}

void glFrontFace( GLenum mode )
{
	glim.FrontFace( mode );
}

void glFrustum( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar )
{
	glim.Frustum( left, right, bottom, top, zNear, zFar );
}

GLuint glGenLists( GLsizei range )
{
	return  glim.GenLists( range );
}

void glGenTextures( GLsizei n, GLuint *textures )
{
	glim.GenTextures( n, textures );
}

void glGetBooleanv( GLenum pname, GLboolean *params )
{
	glim.GetBooleanv( pname, params );
}

void glGetClipPlane( GLenum plane, GLdouble *equation )
{
	glim.GetClipPlane( plane, equation );
}

void glGetCombinerStageParameterfvNV( GLenum stage, GLenum pname, GLfloat *params )
{
	glim.GetCombinerStageParameterfvNV( stage, pname, params );
}

void glGetDoublev( GLenum pname, GLdouble *params )
{
	glim.GetDoublev( pname, params );
}

GLenum glGetError( void )
{
	return  glim.GetError(  );
}

void glGetFloatv( GLenum pname, GLfloat *params )
{
	glim.GetFloatv( pname, params );
}

void glGetIntegerv( GLenum pname, GLint *params )
{
	glim.GetIntegerv( pname, params );
}

void glGetLightfv( GLenum light, GLenum pname, GLfloat *params )
{
	glim.GetLightfv( light, pname, params );
}

void glGetLightiv( GLenum light, GLenum pname, GLint *params )
{
	glim.GetLightiv( light, pname, params );
}

void glGetMapdv( GLenum target, GLenum query, GLdouble *v )
{
	glim.GetMapdv( target, query, v );
}

void glGetMapfv( GLenum target, GLenum query, GLfloat *v )
{
	glim.GetMapfv( target, query, v );
}

void glGetMapiv( GLenum target, GLenum query, GLint *v )
{
	glim.GetMapiv( target, query, v );
}

void glGetMaterialfv( GLenum face, GLenum pname, GLfloat *params )
{
	glim.GetMaterialfv( face, pname, params );
}

void glGetMaterialiv( GLenum face, GLenum pname, GLint *params )
{
	glim.GetMaterialiv( face, pname, params );
}

void glGetPixelMapfv( GLenum map, GLfloat *values )
{
	glim.GetPixelMapfv( map, values );
}

void glGetPixelMapuiv( GLenum map, GLuint *values )
{
	glim.GetPixelMapuiv( map, values );
}

void glGetPixelMapusv( GLenum map, GLushort *values )
{
	glim.GetPixelMapusv( map, values );
}

void glGetPointerv( GLenum pname, GLvoid* *params )
{
	glim.GetPointerv( pname, params );
}

void glGetPolygonStipple( GLubyte *mask )
{
	glim.GetPolygonStipple( mask );
}

const GLubyte * glGetString( GLenum name )
{
	return  glim.GetString( name );
}

void glGetTexEnvfv( GLenum target, GLenum pname, GLfloat *params )
{
	glim.GetTexEnvfv( target, pname, params );
}

void glGetTexEnviv( GLenum target, GLenum pname, GLint *params )
{
	glim.GetTexEnviv( target, pname, params );
}

void glGetTexGendv( GLenum coord, GLenum pname, GLdouble *params )
{
	glim.GetTexGendv( coord, pname, params );
}

void glGetTexGenfv( GLenum coord, GLenum pname, GLfloat *params )
{
	glim.GetTexGenfv( coord, pname, params );
}

void glGetTexGeniv( GLenum coord, GLenum pname, GLint *params )
{
	glim.GetTexGeniv( coord, pname, params );
}

void glGetTexImage( GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels )
{
	glim.GetTexImage( target, level, format, type, pixels );
}

void glGetTexLevelParameterfv( GLenum target, GLint level, GLenum pname, GLfloat *params )
{
	glim.GetTexLevelParameterfv( target, level, pname, params );
}

void glGetTexLevelParameteriv( GLenum target, GLint level, GLenum pname, GLint *params )
{
	glim.GetTexLevelParameteriv( target, level, pname, params );
}

void glGetTexParameterfv( GLenum target, GLenum pname, GLfloat *params )
{
	glim.GetTexParameterfv( target, pname, params );
}

void glGetTexParameteriv( GLenum target, GLenum pname, GLint *params )
{
	glim.GetTexParameteriv( target, pname, params );
}

void glHint( GLenum target, GLenum mode )
{
	glim.Hint( target, mode );
}

void glIndexMask( GLuint mask )
{
	glim.IndexMask( mask );
}

void glIndexPointer( GLenum type, GLsizei stride, const GLvoid *pointer )
{
	glim.IndexPointer( type, stride, pointer );
}

void glIndexd( GLdouble c )
{
	glim.Indexd( c );
}

void glIndexdv( const GLdouble *c )
{
	glim.Indexdv( c );
}

void glIndexf( GLfloat c )
{
	glim.Indexf( c );
}

void glIndexfv( const GLfloat *c )
{
	glim.Indexfv( c );
}

void glIndexi( GLint c )
{
	glim.Indexi( c );
}

void glIndexiv( const GLint *c )
{
	glim.Indexiv( c );
}

void glIndexs( GLshort c )
{
	glim.Indexs( c );
}

void glIndexsv( const GLshort *c )
{
	glim.Indexsv( c );
}

void glIndexub( GLubyte c )
{
	glim.Indexub( c );
}

void glIndexubv( const GLubyte *c )
{
	glim.Indexubv( c );
}

void glInitNames( void )
{
	glim.InitNames(  );
}

void glInterleavedArrays( GLenum format, GLsizei stride, const GLvoid *pointer )
{
	glim.InterleavedArrays( format, stride, pointer );
}

GLboolean glIsEnabled( GLenum cap )
{
	return  glim.IsEnabled( cap );
}

GLboolean glIsList( GLuint list )
{
	return  glim.IsList( list );
}

GLboolean glIsTexture( GLuint texture )
{
	return  glim.IsTexture( texture );
}

void glLightModelf( GLenum pname, GLfloat param )
{
	glim.LightModelf( pname, param );
}

void glLightModelfv( GLenum pname, const GLfloat *params )
{
	glim.LightModelfv( pname, params );
}

void glLightModeli( GLenum pname, GLint param )
{
	glim.LightModeli( pname, param );
}

void glLightModeliv( GLenum pname, const GLint *params )
{
	glim.LightModeliv( pname, params );
}

void glLightf( GLenum light, GLenum pname, GLfloat param )
{
	glim.Lightf( light, pname, param );
}

void glLightfv( GLenum light, GLenum pname, const GLfloat *params )
{
	glim.Lightfv( light, pname, params );
}

void glLighti( GLenum light, GLenum pname, GLint param )
{
	glim.Lighti( light, pname, param );
}

void glLightiv( GLenum light, GLenum pname, const GLint *params )
{
	glim.Lightiv( light, pname, params );
}

void glLineStipple( GLint factor, GLushort pattern )
{
	glim.LineStipple( factor, pattern );
}

void glLineWidth( GLfloat width )
{
	glim.LineWidth( width );
}

void glListBase( GLuint base )
{
	glim.ListBase( base );
}

void glLoadIdentity( void )
{
	glim.LoadIdentity(  );
}

void glLoadMatrixd( const GLdouble *m )
{
	glim.LoadMatrixd( m );
}

void glLoadMatrixf( const GLfloat *m )
{
	glim.LoadMatrixf( m );
}

void glLoadName( GLuint name )
{
	glim.LoadName( name );
}

void glLogicOp( GLenum opcode )
{
	glim.LogicOp( opcode );
}

void glMakeCurrent( void )
{
	glim.MakeCurrent(  );
}

void glMap1d( GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points )
{
	glim.Map1d( target, u1, u2, stride, order, points );
}

void glMap1f( GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points )
{
	glim.Map1f( target, u1, u2, stride, order, points );
}

void glMap2d( GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points )
{
	glim.Map2d( target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points );
}

void glMap2f( GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points )
{
	glim.Map2f( target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points );
}

void glMapGrid1d( GLint un, GLdouble u1, GLdouble u2 )
{
	glim.MapGrid1d( un, u1, u2 );
}

void glMapGrid1f( GLint un, GLfloat u1, GLfloat u2 )
{
	glim.MapGrid1f( un, u1, u2 );
}

void glMapGrid2d( GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2 )
{
	glim.MapGrid2d( un, u1, u2, vn, v1, v2 );
}

void glMapGrid2f( GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2 )
{
	glim.MapGrid2f( un, u1, u2, vn, v1, v2 );
}

void glMaterialf( GLenum face, GLenum pname, GLfloat param )
{
	glim.Materialf( face, pname, param );
}

void glMaterialfv( GLenum face, GLenum pname, const GLfloat *params )
{
	glim.Materialfv( face, pname, params );
}

void glMateriali( GLenum face, GLenum pname, GLint param )
{
	glim.Materiali( face, pname, param );
}

void glMaterialiv( GLenum face, GLenum pname, const GLint *params )
{
	glim.Materialiv( face, pname, params );
}

void glMatrixMode( GLenum mode )
{
	glim.MatrixMode( mode );
}

void glMultMatrixd( const GLdouble *m )
{
	glim.MultMatrixd( m );
}

void glMultMatrixf( const GLfloat *m )
{
	glim.MultMatrixf( m );
}

void glMultiTexCoord1dARB( GLenum texture, GLdouble s )
{
	glim.MultiTexCoord1dARB( texture, s );
}

void glMultiTexCoord1dvARB( GLenum texture, const GLdouble *t )
{
	glim.MultiTexCoord1dvARB( texture, t );
}

void glMultiTexCoord1fARB( GLenum texture, GLfloat s )
{
	glim.MultiTexCoord1fARB( texture, s );
}

void glMultiTexCoord1fvARB( GLenum texture, const GLfloat *t )
{
	glim.MultiTexCoord1fvARB( texture, t );
}

void glMultiTexCoord1iARB( GLenum texture, GLint s )
{
	glim.MultiTexCoord1iARB( texture, s );
}

void glMultiTexCoord1ivARB( GLenum texture, const GLint *t )
{
	glim.MultiTexCoord1ivARB( texture, t );
}

void glMultiTexCoord1sARB( GLenum texture, GLshort s )
{
	glim.MultiTexCoord1sARB( texture, s );
}

void glMultiTexCoord1svARB( GLenum texture, const GLshort *t )
{
	glim.MultiTexCoord1svARB( texture, t );
}

void glMultiTexCoord2dARB( GLenum texture, GLdouble s, GLdouble t )
{
	glim.MultiTexCoord2dARB( texture, s, t );
}

void glMultiTexCoord2dvARB( GLenum texture, const GLdouble *t )
{
	glim.MultiTexCoord2dvARB( texture, t );
}

void glMultiTexCoord2fARB( GLenum texture, GLfloat s, GLfloat t )
{
	glim.MultiTexCoord2fARB( texture, s, t );
}

void glMultiTexCoord2fvARB( GLenum texture, const GLfloat *t )
{
	glim.MultiTexCoord2fvARB( texture, t );
}

void glMultiTexCoord2iARB( GLenum texture, GLint s, GLint t )
{
	glim.MultiTexCoord2iARB( texture, s, t );
}

void glMultiTexCoord2ivARB( GLenum texture, const GLint *t )
{
	glim.MultiTexCoord2ivARB( texture, t );
}

void glMultiTexCoord2sARB( GLenum texture, GLshort s, GLshort t )
{
	glim.MultiTexCoord2sARB( texture, s, t );
}

void glMultiTexCoord2svARB( GLenum texture, const GLshort *t )
{
	glim.MultiTexCoord2svARB( texture, t );
}

void glMultiTexCoord3dARB( GLenum texture, GLdouble s, GLdouble t, GLdouble r )
{
	glim.MultiTexCoord3dARB( texture, s, t, r );
}

void glMultiTexCoord3dvARB( GLenum texture, const GLdouble *t )
{
	glim.MultiTexCoord3dvARB( texture, t );
}

void glMultiTexCoord3fARB( GLenum texture, GLfloat s, GLfloat t, GLfloat r )
{
	glim.MultiTexCoord3fARB( texture, s, t, r );
}

void glMultiTexCoord3fvARB( GLenum texture, const GLfloat *t )
{
	glim.MultiTexCoord3fvARB( texture, t );
}

void glMultiTexCoord3iARB( GLenum texture, GLint s, GLint t, GLint r )
{
	glim.MultiTexCoord3iARB( texture, s, t, r );
}

void glMultiTexCoord3ivARB( GLenum texture, const GLint *t )
{
	glim.MultiTexCoord3ivARB( texture, t );
}

void glMultiTexCoord3sARB( GLenum texture, GLshort s, GLshort t, GLshort r )
{
	glim.MultiTexCoord3sARB( texture, s, t, r );
}

void glMultiTexCoord3svARB( GLenum texture, const GLshort *t )
{
	glim.MultiTexCoord3svARB( texture, t );
}

void glMultiTexCoord4dARB( GLenum texture, GLdouble s, GLdouble t, GLdouble r, GLdouble q )
{
	glim.MultiTexCoord4dARB( texture, s, t, r, q );
}

void glMultiTexCoord4dvARB( GLenum texture, const GLdouble *t )
{
	glim.MultiTexCoord4dvARB( texture, t );
}

void glMultiTexCoord4fARB( GLenum texture, GLfloat s, GLfloat t, GLfloat r, GLfloat q )
{
	glim.MultiTexCoord4fARB( texture, s, t, r, q );
}

void glMultiTexCoord4fvARB( GLenum texture, const GLfloat *t )
{
	glim.MultiTexCoord4fvARB( texture, t );
}

void glMultiTexCoord4iARB( GLenum texture, GLint s, GLint t, GLint r, GLint q )
{
	glim.MultiTexCoord4iARB( texture, s, t, r, q );
}

void glMultiTexCoord4ivARB( GLenum texture, const GLint *t )
{
	glim.MultiTexCoord4ivARB( texture, t );
}

void glMultiTexCoord4sARB( GLenum texture, GLshort s, GLshort t, GLshort r, GLshort q )
{
	glim.MultiTexCoord4sARB( texture, s, t, r, q );
}

void glMultiTexCoord4svARB( GLenum texture, const GLshort *t )
{
	glim.MultiTexCoord4svARB( texture, t );
}

void glNewList( GLuint list, GLenum mode )
{
	glim.NewList( list, mode );
}

void glNormal3b( GLbyte nx, GLbyte ny, GLbyte nz )
{
	glim.Normal3b( nx, ny, nz );
}

void glNormal3bv( const GLbyte *v )
{
	glim.Normal3bv( v );
}

void glNormal3d( GLdouble nx, GLdouble ny, GLdouble nz )
{
	glim.Normal3d( nx, ny, nz );
}

void glNormal3dv( const GLdouble *v )
{
	glim.Normal3dv( v );
}

void glNormal3f( GLfloat nx, GLfloat ny, GLfloat nz )
{
	glim.Normal3f( nx, ny, nz );
}

void glNormal3fv( const GLfloat *v )
{
	glim.Normal3fv( v );
}

void glNormal3i( GLint nx, GLint ny, GLint nz )
{
	glim.Normal3i( nx, ny, nz );
}

void glNormal3iv( const GLint *v )
{
	glim.Normal3iv( v );
}

void glNormal3s( GLshort nx, GLshort ny, GLshort nz )
{
	glim.Normal3s( nx, ny, nz );
}

void glNormal3sv( const GLshort *v )
{
	glim.Normal3sv( v );
}

void glNormalPointer( GLenum type, GLsizei stride, const GLvoid *pointer )
{
	glim.NormalPointer( type, stride, pointer );
}

void glOrtho( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar )
{
	glim.Ortho( left, right, bottom, top, zNear, zFar );
}

void glPassThrough( GLfloat token )
{
	glim.PassThrough( token );
}

void glPixelMapfv( GLenum map, GLsizei mapsize, const GLfloat *values )
{
	glim.PixelMapfv( map, mapsize, values );
}

void glPixelMapuiv( GLenum map, GLsizei mapsize, const GLuint *values )
{
	glim.PixelMapuiv( map, mapsize, values );
}

void glPixelMapusv( GLenum map, GLsizei mapsize, const GLushort *values )
{
	glim.PixelMapusv( map, mapsize, values );
}

void glPixelStoref( GLenum pname, GLfloat param )
{
	glim.PixelStoref( pname, param );
}

void glPixelStorei( GLenum pname, GLint param )
{
	glim.PixelStorei( pname, param );
}

void glPixelTransferf( GLenum pname, GLfloat param )
{
	glim.PixelTransferf( pname, param );
}

void glPixelTransferi( GLenum pname, GLint param )
{
	glim.PixelTransferi( pname, param );
}

void glPixelZoom( GLfloat xfactor, GLfloat yfactor )
{
	glim.PixelZoom( xfactor, yfactor );
}

void glPointSize( GLfloat size )
{
	glim.PointSize( size );
}

void glPolygonMode( GLenum face, GLenum mode )
{
	glim.PolygonMode( face, mode );
}

void glPolygonOffset( GLfloat factor, GLfloat units )
{
	glim.PolygonOffset( factor, units );
}

void glPolygonStipple( const GLubyte *mask )
{
	glim.PolygonStipple( mask );
}

void glPopAttrib( void )
{
	glim.PopAttrib(  );
}

void glPopClientAttrib( void )
{
	glim.PopClientAttrib(  );
}

void glPopMatrix( void )
{
	glim.PopMatrix(  );
}

void glPopName( void )
{
	glim.PopName(  );
}

void glPrioritizeTextures( GLsizei n, const GLuint *textures, const GLclampf *priorities )
{
	glim.PrioritizeTextures( n, textures, priorities );
}

void glPushAttrib( GLbitfield mask )
{
	glim.PushAttrib( mask );
}

void glPushClientAttrib( GLbitfield mask )
{
	glim.PushClientAttrib( mask );
}

void glPushMatrix( void )
{
	glim.PushMatrix(  );
}

void glPushName( GLuint name )
{
	glim.PushName( name );
}

void glRasterPos2d( GLdouble x, GLdouble y )
{
	glim.RasterPos2d( x, y );
}

void glRasterPos2dv( const GLdouble *v )
{
	glim.RasterPos2dv( v );
}

void glRasterPos2f( GLfloat x, GLfloat y )
{
	glim.RasterPos2f( x, y );
}

void glRasterPos2fv( const GLfloat *v )
{
	glim.RasterPos2fv( v );
}

void glRasterPos2i( GLint x, GLint y )
{
	glim.RasterPos2i( x, y );
}

void glRasterPos2iv( const GLint *v )
{
	glim.RasterPos2iv( v );
}

void glRasterPos2s( GLshort x, GLshort y )
{
	glim.RasterPos2s( x, y );
}

void glRasterPos2sv( const GLshort *v )
{
	glim.RasterPos2sv( v );
}

void glRasterPos3d( GLdouble x, GLdouble y, GLdouble z )
{
	glim.RasterPos3d( x, y, z );
}

void glRasterPos3dv( const GLdouble *v )
{
	glim.RasterPos3dv( v );
}

void glRasterPos3f( GLfloat x, GLfloat y, GLfloat z )
{
	glim.RasterPos3f( x, y, z );
}

void glRasterPos3fv( const GLfloat *v )
{
	glim.RasterPos3fv( v );
}

void glRasterPos3i( GLint x, GLint y, GLint z )
{
	glim.RasterPos3i( x, y, z );
}

void glRasterPos3iv( const GLint *v )
{
	glim.RasterPos3iv( v );
}

void glRasterPos3s( GLshort x, GLshort y, GLshort z )
{
	glim.RasterPos3s( x, y, z );
}

void glRasterPos3sv( const GLshort *v )
{
	glim.RasterPos3sv( v );
}

void glRasterPos4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w )
{
	glim.RasterPos4d( x, y, z, w );
}

void glRasterPos4dv( const GLdouble *v )
{
	glim.RasterPos4dv( v );
}

void glRasterPos4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
	glim.RasterPos4f( x, y, z, w );
}

void glRasterPos4fv( const GLfloat *v )
{
	glim.RasterPos4fv( v );
}

void glRasterPos4i( GLint x, GLint y, GLint z, GLint w )
{
	glim.RasterPos4i( x, y, z, w );
}

void glRasterPos4iv( const GLint *v )
{
	glim.RasterPos4iv( v );
}

void glRasterPos4s( GLshort x, GLshort y, GLshort z, GLshort w )
{
	glim.RasterPos4s( x, y, z, w );
}

void glRasterPos4sv( const GLshort *v )
{
	glim.RasterPos4sv( v );
}

void glReadBuffer( GLenum mode )
{
	glim.ReadBuffer( mode );
}

void glReadPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels )
{
	glim.ReadPixels( x, y, width, height, format, type, pixels );
}

void glRectd( GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 )
{
	glim.Rectd( x1, y1, x2, y2 );
}

void glRectdv( const GLdouble *v1, const GLdouble *v2 )
{
	glim.Rectdv( v1, v2 );
}

void glRectf( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 )
{
	glim.Rectf( x1, y1, x2, y2 );
}

void glRectfv( const GLfloat *v1, const GLfloat *v2 )
{
	glim.Rectfv( v1, v2 );
}

void glRecti( GLint x1, GLint y1, GLint x2, GLint y2 )
{
	glim.Recti( x1, y1, x2, y2 );
}

void glRectiv( const GLint *v1, const GLint *v2 )
{
	glim.Rectiv( v1, v2 );
}

void glRects( GLshort x1, GLshort y1, GLshort x2, GLshort y2 )
{
	glim.Rects( x1, y1, x2, y2 );
}

void glRectsv( const GLshort *v1, const GLshort *v2 )
{
	glim.Rectsv( v1, v2 );
}

GLint glRenderMode( GLenum mode )
{
	return  glim.RenderMode( mode );
}

void glRotated( GLdouble angle, GLdouble x, GLdouble y, GLdouble z )
{
	glim.Rotated( angle, x, y, z );
}

void glRotatef( GLfloat angle, GLfloat x, GLfloat y, GLfloat z )
{
	glim.Rotatef( angle, x, y, z );
}

void glScaled( GLdouble x, GLdouble y, GLdouble z )
{
	glim.Scaled( x, y, z );
}

void glScalef( GLfloat x, GLfloat y, GLfloat z )
{
	glim.Scalef( x, y, z );
}

void glScissor( GLint x, GLint y, GLsizei width, GLsizei height )
{
	glim.Scissor( x, y, width, height );
}

void glSecondaryColor3bEXT( GLbyte red, GLbyte green, GLbyte blue )
{
	glim.SecondaryColor3bEXT( red, green, blue );
}

void glSecondaryColor3bvEXT( const GLbyte *v )
{
	glim.SecondaryColor3bvEXT( v );
}

void glSecondaryColor3dEXT( GLdouble red, GLdouble green, GLdouble blue )
{
	glim.SecondaryColor3dEXT( red, green, blue );
}

void glSecondaryColor3dvEXT( const GLdouble *v )
{
	glim.SecondaryColor3dvEXT( v );
}

void glSecondaryColor3fEXT( GLfloat red, GLfloat green, GLfloat blue )
{
	glim.SecondaryColor3fEXT( red, green, blue );
}

void glSecondaryColor3fvEXT( const GLfloat *v )
{
	glim.SecondaryColor3fvEXT( v );
}

void glSecondaryColor3iEXT( GLint red, GLint green, GLint blue )
{
	glim.SecondaryColor3iEXT( red, green, blue );
}

void glSecondaryColor3ivEXT( const GLint *v )
{
	glim.SecondaryColor3ivEXT( v );
}

void glSecondaryColor3sEXT( GLshort red, GLshort green, GLshort blue )
{
	glim.SecondaryColor3sEXT( red, green, blue );
}

void glSecondaryColor3svEXT( const GLshort *v )
{
	glim.SecondaryColor3svEXT( v );
}

void glSecondaryColor3ubEXT( GLubyte red, GLubyte green, GLubyte blue )
{
	glim.SecondaryColor3ubEXT( red, green, blue );
}

void glSecondaryColor3ubvEXT( const GLubyte *v )
{
	glim.SecondaryColor3ubvEXT( v );
}

void glSecondaryColor3uiEXT( GLuint red, GLuint green, GLuint blue )
{
	glim.SecondaryColor3uiEXT( red, green, blue );
}

void glSecondaryColor3uivEXT( const GLuint *v )
{
	glim.SecondaryColor3uivEXT( v );
}

void glSecondaryColor3usEXT( GLushort red, GLushort green, GLushort blue )
{
	glim.SecondaryColor3usEXT( red, green, blue );
}

void glSecondaryColor3usvEXT( const GLushort *v )
{
	glim.SecondaryColor3usvEXT( v );
}

void glSecondaryColorPointerEXT( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	glim.SecondaryColorPointerEXT( size, type, stride, pointer );
}

void glSelectBuffer( GLsizei size, GLuint *buffer )
{
	glim.SelectBuffer( size, buffer );
}

void glSemaphoreCreate( GLuint name, GLuint count )
{
	glim.SemaphoreCreate( name, count );
}

void glSemaphoreDestroy( GLuint name )
{
	glim.SemaphoreDestroy( name );
}

void glSemaphoreP( GLuint name )
{
	glim.SemaphoreP( name );
}

void glSemaphoreV( GLuint name )
{
	glim.SemaphoreV( name );
}

void glShadeModel( GLenum mode )
{
	glim.ShadeModel( mode );
}

void glStencilFunc( GLenum func, GLint ref, GLuint mask )
{
	glim.StencilFunc( func, ref, mask );
}

void glStencilMask( GLuint mask )
{
	glim.StencilMask( mask );
}

void glStencilOp( GLenum fail, GLenum zfail, GLenum zpass )
{
	glim.StencilOp( fail, zfail, zpass );
}

void glSwapBuffers( void )
{
	glim.SwapBuffers(  );
}

void glTexCoord1d( GLdouble s )
{
	glim.TexCoord1d( s );
}

void glTexCoord1dv( const GLdouble *v )
{
	glim.TexCoord1dv( v );
}

void glTexCoord1f( GLfloat s )
{
	glim.TexCoord1f( s );
}

void glTexCoord1fv( const GLfloat *v )
{
	glim.TexCoord1fv( v );
}

void glTexCoord1i( GLint s )
{
	glim.TexCoord1i( s );
}

void glTexCoord1iv( const GLint *v )
{
	glim.TexCoord1iv( v );
}

void glTexCoord1s( GLshort s )
{
	glim.TexCoord1s( s );
}

void glTexCoord1sv( const GLshort *v )
{
	glim.TexCoord1sv( v );
}

void glTexCoord2d( GLdouble s, GLdouble t )
{
	glim.TexCoord2d( s, t );
}

void glTexCoord2dv( const GLdouble *v )
{
	glim.TexCoord2dv( v );
}

void glTexCoord2f( GLfloat s, GLfloat t )
{
	glim.TexCoord2f( s, t );
}

void glTexCoord2fv( const GLfloat *v )
{
	glim.TexCoord2fv( v );
}

void glTexCoord2i( GLint s, GLint t )
{
	glim.TexCoord2i( s, t );
}

void glTexCoord2iv( const GLint *v )
{
	glim.TexCoord2iv( v );
}

void glTexCoord2s( GLshort s, GLshort t )
{
	glim.TexCoord2s( s, t );
}

void glTexCoord2sv( const GLshort *v )
{
	glim.TexCoord2sv( v );
}

void glTexCoord3d( GLdouble s, GLdouble t, GLdouble r )
{
	glim.TexCoord3d( s, t, r );
}

void glTexCoord3dv( const GLdouble *v )
{
	glim.TexCoord3dv( v );
}

void glTexCoord3f( GLfloat s, GLfloat t, GLfloat r )
{
	glim.TexCoord3f( s, t, r );
}

void glTexCoord3fv( const GLfloat *v )
{
	glim.TexCoord3fv( v );
}

void glTexCoord3i( GLint s, GLint t, GLint r )
{
	glim.TexCoord3i( s, t, r );
}

void glTexCoord3iv( const GLint *v )
{
	glim.TexCoord3iv( v );
}

void glTexCoord3s( GLshort s, GLshort t, GLshort r )
{
	glim.TexCoord3s( s, t, r );
}

void glTexCoord3sv( const GLshort *v )
{
	glim.TexCoord3sv( v );
}

void glTexCoord4d( GLdouble s, GLdouble t, GLdouble r, GLdouble q )
{
	glim.TexCoord4d( s, t, r, q );
}

void glTexCoord4dv( const GLdouble *v )
{
	glim.TexCoord4dv( v );
}

void glTexCoord4f( GLfloat s, GLfloat t, GLfloat r, GLfloat q )
{
	glim.TexCoord4f( s, t, r, q );
}

void glTexCoord4fv( const GLfloat *v )
{
	glim.TexCoord4fv( v );
}

void glTexCoord4i( GLint s, GLint t, GLint r, GLint q )
{
	glim.TexCoord4i( s, t, r, q );
}

void glTexCoord4iv( const GLint *v )
{
	glim.TexCoord4iv( v );
}

void glTexCoord4s( GLshort s, GLshort t, GLshort r, GLshort q )
{
	glim.TexCoord4s( s, t, r, q );
}

void glTexCoord4sv( const GLshort *v )
{
	glim.TexCoord4sv( v );
}

void glTexCoordPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	glim.TexCoordPointer( size, type, stride, pointer );
}

void glTexEnvf( GLenum target, GLenum pname, GLfloat param )
{
	glim.TexEnvf( target, pname, param );
}

void glTexEnvfv( GLenum target, GLenum pname, const GLfloat *params )
{
	glim.TexEnvfv( target, pname, params );
}

void glTexEnvi( GLenum target, GLenum pname, GLint param )
{
	glim.TexEnvi( target, pname, param );
}

void glTexEnviv( GLenum target, GLenum pname, const GLint *params )
{
	glim.TexEnviv( target, pname, params );
}

void glTexGend( GLenum coord, GLenum pname, GLdouble param )
{
	glim.TexGend( coord, pname, param );
}

void glTexGendv( GLenum coord, GLenum pname, const GLdouble *params )
{
	glim.TexGendv( coord, pname, params );
}

void glTexGenf( GLenum coord, GLenum pname, GLfloat param )
{
	glim.TexGenf( coord, pname, param );
}

void glTexGenfv( GLenum coord, GLenum pname, const GLfloat *params )
{
	glim.TexGenfv( coord, pname, params );
}

void glTexGeni( GLenum coord, GLenum pname, GLint param )
{
	glim.TexGeni( coord, pname, param );
}

void glTexGeniv( GLenum coord, GLenum pname, const GLint *params )
{
	glim.TexGeniv( coord, pname, params );
}

void glTexImage1D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels )
{
	glim.TexImage1D( target, level, internalformat, width, border, format, type, pixels );
}

void glTexImage2D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels )
{
	glim.TexImage2D( target, level, internalformat, width, height, border, format, type, pixels );
}

void glTexParameterf( GLenum target, GLenum pname, GLfloat param )
{
	glim.TexParameterf( target, pname, param );
}

void glTexParameterfv( GLenum target, GLenum pname, const GLfloat *params )
{
	glim.TexParameterfv( target, pname, params );
}

void glTexParameteri( GLenum target, GLenum pname, GLint param )
{
	glim.TexParameteri( target, pname, param );
}

void glTexParameteriv( GLenum target, GLenum pname, const GLint *params )
{
	glim.TexParameteriv( target, pname, params );
}

void glTexSubImage1D( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels )
{
	glim.TexSubImage1D( target, level, xoffset, width, format, type, pixels );
}

void glTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels )
{
	glim.TexSubImage2D( target, level, xoffset, yoffset, width, height, format, type, pixels );
}

void glTranslated( GLdouble x, GLdouble y, GLdouble z )
{
	glim.Translated( x, y, z );
}

void glTranslatef( GLfloat x, GLfloat y, GLfloat z )
{
	glim.Translatef( x, y, z );
}

void glVertex2d( GLdouble x, GLdouble y )
{
	glim.Vertex2d( x, y );
}

void glVertex2dv( const GLdouble *v )
{
	glim.Vertex2dv( v );
}

void glVertex2f( GLfloat x, GLfloat y )
{
	glim.Vertex2f( x, y );
}

void glVertex2fv( const GLfloat *v )
{
	glim.Vertex2fv( v );
}

void glVertex2i( GLint x, GLint y )
{
	glim.Vertex2i( x, y );
}

void glVertex2iv( const GLint *v )
{
	glim.Vertex2iv( v );
}

void glVertex2s( GLshort x, GLshort y )
{
	glim.Vertex2s( x, y );
}

void glVertex2sv( const GLshort *v )
{
	glim.Vertex2sv( v );
}

void glVertex3d( GLdouble x, GLdouble y, GLdouble z )
{
	glim.Vertex3d( x, y, z );
}

void glVertex3dv( const GLdouble *v )
{
	glim.Vertex3dv( v );
}

void glVertex3f( GLfloat x, GLfloat y, GLfloat z )
{
	glim.Vertex3f( x, y, z );
}

void glVertex3fv( const GLfloat *v )
{
	glim.Vertex3fv( v );
}

void glVertex3i( GLint x, GLint y, GLint z )
{
	glim.Vertex3i( x, y, z );
}

void glVertex3iv( const GLint *v )
{
	glim.Vertex3iv( v );
}

void glVertex3s( GLshort x, GLshort y, GLshort z )
{
	glim.Vertex3s( x, y, z );
}

void glVertex3sv( const GLshort *v )
{
	glim.Vertex3sv( v );
}

void glVertex4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w )
{
	glim.Vertex4d( x, y, z, w );
}

void glVertex4dv( const GLdouble *v )
{
	glim.Vertex4dv( v );
}

void glVertex4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
	glim.Vertex4f( x, y, z, w );
}

void glVertex4fv( const GLfloat *v )
{
	glim.Vertex4fv( v );
}

void glVertex4i( GLint x, GLint y, GLint z, GLint w )
{
	glim.Vertex4i( x, y, z, w );
}

void glVertex4iv( const GLint *v )
{
	glim.Vertex4iv( v );
}

void glVertex4s( GLshort x, GLshort y, GLshort z, GLshort w )
{
	glim.Vertex4s( x, y, z, w );
}

void glVertex4sv( const GLshort *v )
{
	glim.Vertex4sv( v );
}

void glVertexPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *pointer )
{
	glim.VertexPointer( size, type, stride, pointer );
}

void glViewport( GLint x, GLint y, GLsizei width, GLsizei height )
{
	glim.Viewport( x, y, width, height );
}

void glWriteback( GLint *writeback )
{
	glim.Writeback( writeback );
}

