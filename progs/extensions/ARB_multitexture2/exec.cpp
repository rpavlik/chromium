/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*

  exec.cpp

  This is an example of GL_ARB_multitexture, described
  on page 18 of the NVidia OpenGL Extension Specifications.

  Christopher Niederauer, ccn@graphics.stanford.edu, 6/25/2001

*/

/* --- Preprocessor --------------------------------------------------------- */

#include "exec.h"
#include "../common/logo.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>


enum
  {
    eBrickTex = 0,
    eLightTex,
    eNumTextures
  };


/* --- Global Variables ----------------------------------------------------- */

static GLuint	currentWidth, currentHeight;
static GLuint	textureID[eNumTextures];
static GLfloat	bgColor[4] = { 0.1, 0.1, 0.1, 0.0 };

#ifdef WINDOWS
PFNGLMULTITEXCOORD2FARBPROC	glMultiTexCoord2fARB;
PFNGLACTIVETEXTUREARBPROC	glActiveTextureARB;
#endif


/* --- Function Definitions ------------------------------------------------- */

void	InitGL	( void )
{

  currentWidth = 320;
  currentHeight = 240;

#ifdef MULTIPLE_VIEWPORTS
  currentWidth <<= 1;
#endif

  glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE );
  glutInitWindowPosition( 5, 25 );
  glutInitWindowSize( currentWidth, currentHeight );
  glutCreateWindow( TEST_EXTENSION_STRING );
	
  glClearColor( bgColor[0], bgColor[1], bgColor[2], bgColor[3] );
  glClear( GL_COLOR_BUFFER_BIT );
	
  glutIdleFunc( Idle );
  glutDisplayFunc( Display );
  glutReshapeFunc( Reshape );
  glutKeyboardFunc( Keyboard );
  glutMouseFunc( NULL );
  glutMotionFunc( NULL );
  glutSpecialFunc( NULL );
}


void	InitSpecial	( void )
{
  GLint	numTexUnits,
    currentActiveUnit,
    maxTextureSize;

  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glFogfv( GL_FOG_COLOR, bgColor );
  glFogf( GL_FOG_START, 5 );
  glFogf( GL_FOG_END, 30 );
  glFogf( GL_FOG_MODE, GL_LINEAR );

  glGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, &numTexUnits ); // Up to 4
  glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maxTextureSize );
  glGetIntegerv( GL_ACTIVE_TEXTURE_ARB, &currentActiveUnit );
  cout << "MAX_TEXTURE_SIZE = " << maxTextureSize << endl;
  cout << "MAX_TEXTURE_UNITS_ARB = " << numTexUnits << endl;
  cout << "EXTENSIONS: " << (char *) glGetString(GL_EXTENSIONS) << endl;
  if( numTexUnits < 2 )
    {
      cout << "Sorry, this program requires at least two texture units to work properly." << endl;
      exit( 0 );
    }
  cout << "ACTIVE_TEXTURE_ARB = ";
  switch( currentActiveUnit )
    {
    case GL_TEXTURE0_ARB:
      cout << "TEXTURE0_ARB";
      break;
    case GL_TEXTURE1_ARB:
      cout << "TEXTURE1_ARB";
      break;
    case GL_TEXTURE2_ARB:
      cout << "TEXTURE2_ARB";	
      break;
    case GL_TEXTURE3_ARB:
      cout << "TEXTURE3_ARB";
      break;
    default:
      cout << "Oh no! (0x" << hex << currentActiveUnit << ")";
      break;
    }
  cout << endl << endl;
#ifdef WIN32
  glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress( "glMultiTexCoord2fARB" );
  glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress( "glActiveTextureARB" );
  if( !glMultiTexCoord2fARB || !glActiveTextureARB )
    {
      cout << "Error trying to link to extensions!" << endl;
      exit( 0 );
    }
#endif
  // Gets of CURRENT_TEXTURE_COORDS return that of the active unit.
  // ActiveTextureARB( enum texture ) changes active unit for:
  // 	- Seperate Texture Matrices
  // 	- TexEnv calls
  //	- Get CURRENT_TEXTURE_COORDS

    // Create the tile texture.
  glGenTextures( eNumTextures, textureID );

  // Load the textures.
  {
    const int	texmapX = 128,
      texmapY = 128,
      texmapSize = texmapX*texmapY*3;
    FILE		*file;
    GLubyte		textureData[ texmapSize ];
    // Load brick texture.
    {
      if(( file = fopen( "brick.raw", "rb" )) == NULL )
	{
	  cout << "Error opening file: brick.raw" << endl;
	  exit( 0 );
	}
      else
	{
	  fread( textureData, texmapSize, 1, file );
	  fclose( file );
				
	  // Create Trilinear MipMapped Noise Texture
	  glBindTexture( GL_TEXTURE_2D, textureID[eBrickTex] );
	  glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
	  glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR );
	  gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGB8, texmapX, texmapY, GL_RGB, GL_UNSIGNED_BYTE, textureData );
	}
    }
    // Load light texture.
    {
      if(( file = fopen( "light.raw", "rb" )) == NULL )
	{
	  cout << "Error opening file: light.raw" << endl;
	  exit( 0 );
	}
      else
	{
	  fread( textureData, texmapSize/3, 1, file );
	  fclose( file );
				
	  // Create Trilinear MipMapped Circle Texture
	  glBindTexture( GL_TEXTURE_2D, textureID[eLightTex] );
	  glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
	  glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR );
	  gluBuild2DMipmaps( GL_TEXTURE_2D, GL_LUMINANCE, texmapX, texmapY, GL_LUMINANCE, GL_UNSIGNED_BYTE, textureData );
	}
    }
  }

  return;
}


void	Idle		( void )
{
  glutPostRedisplay();
  return;
}


void	Display		( void )
{
const float		size = 1.0,
			texDetail = 0.5,
			texOffset = 0.5;
	static double	theta = 0.0;

	glClear( GL_COLOR_BUFFER_BIT );
	
	//theta += 0.05;
        theta = glutGet(GLUT_ELAPSED_TIME) / 100.0;

	glLoadIdentity();
	glRotated( 90.0, 1.0, 0.0, 0.0 );
	glTranslatef( 0.0, -2.0, .0 );
	glRotated( theta, 0.0, -1.0, 0.0 );

#ifdef MULTIPLE_VIEWPORTS
	// Left Viewport
	glViewport( 0, 0, currentWidth >> 1, currentHeight );
	
	glEnable( GL_TEXTURE_2D );
	glActiveTextureARB( GL_TEXTURE0_ARB );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	glBindTexture( GL_TEXTURE_2D, textureID[eBrickTex] );
	glActiveTextureARB( GL_TEXTURE1_ARB );
	glEnable( GL_TEXTURE_2D );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glBindTexture( GL_TEXTURE_2D, textureID[eLightTex] );

	glBegin( GL_QUADS );
		glMultiTexCoord2fARB( GL_TEXTURE0_ARB, -texDetail+texOffset,  texDetail+texOffset );
		glMultiTexCoord2fARB( GL_TEXTURE1_ARB, -texDetail+texOffset,  texDetail+texOffset );
		glVertex3f( -size, 0.0,  size );

		glMultiTexCoord2fARB( GL_TEXTURE0_ARB, texDetail+texOffset,  texDetail+texOffset );
		glMultiTexCoord2fARB( GL_TEXTURE1_ARB, texDetail+texOffset,  texDetail+texOffset );
		glVertex3f(  size, 0.0,  size );

		glMultiTexCoord2fARB( GL_TEXTURE0_ARB, texDetail+texOffset, -texDetail+texOffset );
		glMultiTexCoord2fARB( GL_TEXTURE1_ARB, texDetail+texOffset, -texDetail+texOffset );
		glVertex3f(  size, 0.0, -size );

		glMultiTexCoord2fARB( GL_TEXTURE0_ARB, -texDetail+texOffset, -texDetail+texOffset );
		glMultiTexCoord2fARB( GL_TEXTURE1_ARB, -texDetail+texOffset, -texDetail+texOffset );
		glVertex3f( -size, 0.0, -size );
	glEnd();

	glDisable( GL_TEXTURE_2D );
	glActiveTextureARB( GL_TEXTURE0_ARB );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	glDisable( GL_TEXTURE_2D );
	
	glColor3f( 1, 1, 1 );
	glPushMatrix();glLoadIdentity();
	RenderString( -1.1, 1, "GL_MODULATE" );
	glPopMatrix();

	// Upper Right Viewport
	glViewport( currentWidth >> 1, 0, currentWidth >> 1, currentHeight );
#endif
	glEnable( GL_TEXTURE_2D );
	glActiveTextureARB( GL_TEXTURE0_ARB );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	glBindTexture( GL_TEXTURE_2D, textureID[eBrickTex] );
	glActiveTextureARB( GL_TEXTURE1_ARB );
	glEnable( GL_TEXTURE_2D );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD );
	glBindTexture( GL_TEXTURE_2D, textureID[eLightTex] );
	
	glBegin( GL_QUADS );
		glMultiTexCoord2fARB( GL_TEXTURE0_ARB, -texDetail+texOffset,  texDetail+texOffset );
		glMultiTexCoord2fARB( GL_TEXTURE1_ARB, -texDetail+texOffset,  texDetail+texOffset );
		glVertex3f( -size, 0.0,  size );

		glMultiTexCoord2fARB( GL_TEXTURE0_ARB, texDetail+texOffset,  texDetail+texOffset );
		glMultiTexCoord2fARB( GL_TEXTURE1_ARB, texDetail+texOffset,  texDetail+texOffset );
		glVertex3f(  size, 0.0,  size );

		glMultiTexCoord2fARB( GL_TEXTURE0_ARB, texDetail+texOffset, -texDetail+texOffset );
		glMultiTexCoord2fARB( GL_TEXTURE1_ARB, texDetail+texOffset, -texDetail+texOffset );
		glVertex3f(  size, 0.0, -size );

		glMultiTexCoord2fARB( GL_TEXTURE0_ARB, -texDetail+texOffset, -texDetail+texOffset );
		glMultiTexCoord2fARB( GL_TEXTURE1_ARB, -texDetail+texOffset, -texDetail+texOffset );
		glVertex3f( -size, 0.0, -size );
	glEnd();

	glDisable( GL_TEXTURE_2D );
	glActiveTextureARB( GL_TEXTURE0_ARB );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glDisable( GL_TEXTURE_2D );

	glColor3f( 1, 1, 1 );
	glPushMatrix();glLoadIdentity();
	RenderString( -1.1, 1, "GL_ADD" );
	glViewport( 0, 0, currentWidth, currentHeight );
	crExtensionsDrawLogo( currentWidth, currentHeight );
	glPopMatrix();

	glutSwapBuffers();
}


void	Reshape		( int width, int height )
{
	currentWidth = width;
	currentHeight = height;

	glViewport( 0, 0, currentWidth, currentHeight );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glFrustum( -1.0, 1.0, -1.0, 1.0, 1.0, 30.0 );
	glMatrixMode( GL_MODELVIEW );
}


void	Keyboard	( unsigned char key, int, int )
{
	switch( key )
	{
		case 'Q':
		case 'q':
			cout << "User has quit. Exiting." << endl;
			exit( 0 );
		default:
			break;
	}
	return;
}


void	Mouse		( int, int, int, int )
{
	return;
}


void	Motion		( int, int )
{
	return;
}


void	Special		( int, int, int )
{
	return;
}
