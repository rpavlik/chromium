/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*

  exec.cpp

  This is an example of GL_EXT_blend_color, described
  on page 86 of the NVidia OpenGL Extension Specifications.

  Christopher Niederauer, ccn@graphics.stanford.edu, 6/27/2001

*/

/* --- Preprocessor --------------------------------------------------------- */

#include <stdlib.h>
#include "exec.h"
#include "../common/logo.h"

#ifndef APIENTRY
#define APIENTRY
#endif

typedef void (APIENTRY * GLBLENDCOLOREXTPROC) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);

GLBLENDCOLOREXTPROC glBlendColor_ext;


/* --- Global Variables ----------------------------------------------------- */

GLuint	currentWidth, currentHeight;

static GLuint	texture[1];
static GLfloat	bgColor[4] = { 0.2, 0.3, 0.8, 0.0 };


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
	GLubyte	textureData[ 32*32 ];

	for( int x=0; x<32; x++ )
	{
		for( int y=0; y<32; y++ )
		{
			if(( x < 16 && y < 16 ) || ( x >= 16 && y >= 16 ))
				textureData[ y*32+x ] = 0;
			else
				textureData[ y*32+x ] = 255;
		}
	}
	
	// Create the tile texture.
	glGenTextures( 1, texture );
	
	// Create Trilinear MipMapped Texture
	glBindTexture( GL_TEXTURE_2D, texture[0] );
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR );
	gluBuild2DMipmaps( GL_TEXTURE_2D, GL_LUMINANCE, 32, 32, GL_LUMINANCE, GL_UNSIGNED_BYTE, textureData );
	
#ifdef WIN32
	glBlendColor_ext = (GLBLENDCOLOREXTPROC)wglGetProcAddress( "glBlendColorEXT" );
#elif defined(IRIX) || defined (SunOS)
	glBlendColor_ext = glBlendColorEXT;
#else
	glBlendColor_ext = (GLBLENDCOLOREXTPROC)glXGetProcAddressARB( (const GLubyte *) "glBlendColorEXT" );
#endif
	if ( glBlendColor_ext == NULL )
	{
		cout << "Error linking to extensions!" << endl;
		exit( 0 );
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
	const float		size = 50.0;
	static double	theta = 0.0;

	glClear( GL_COLOR_BUFFER_BIT );
	
	theta += 0.005;

	glLoadIdentity();
	glTranslatef( 0.0, -2.0, 0.0 );
	glRotated( 30.0, 1.0, 0.0, 0.0 );
	glRotated( theta, 0.0, 1.0, 0.0 );
	glColor3f( 1.0, 1.0, 1.0 );

#ifdef MULTIPLE_VIEWPORTS

	// Left Viewport
	glViewport( 0, 0, currentWidth >> 1, currentHeight );
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, texture[0] );
	glBegin( GL_QUADS );
		glTexCoord2f( -size/2, size/2 );
		glVertex3f( -size, 0.0,  size );
		glTexCoord2f( size/2, size/2 );
		glVertex3f(  size, 0.0,  size );
		glTexCoord2f( size/2, -size/2 );
		glVertex3f(  size, 0.0, -size );
		glTexCoord2f( -size/2, -size/2 );
		glVertex3f( -size, 0.0, -size );
	glEnd();
	glDisable( GL_TEXTURE_2D );
	glPushMatrix();glLoadIdentity();
	RenderString( -1.1, 1, "Constant Color Blending Off" );
	glPopMatrix();

	// Upper Right Viewport
	glViewport( currentWidth >> 1, 0, currentWidth >> 1, currentHeight );
#endif
	glEnable( GL_BLEND );
	glBlendColor_ext( 1.0, 1.0, 0.0, 0.0 );
	glBlendFunc( GL_CONSTANT_COLOR_EXT, GL_ZERO );
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, texture[0] );
	glBegin( GL_QUADS );
		glTexCoord2f( -size/2, size/2 );
		glVertex3f( -size, 0.0,  size );
		glTexCoord2f( size/2, size/2 );
		glVertex3f(  size, 0.0,  size );
		glTexCoord2f( size/2, -size/2 );
		glVertex3f(  size, 0.0, -size );
		glTexCoord2f( -size/2, -size/2 );
		glVertex3f( -size, 0.0, -size );
	glEnd();
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );
	glPushMatrix();glLoadIdentity();
	RenderString( -1.1, 1, "Constant Color Blending On" );
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


void	Keyboard	( unsigned char key, int x, int y )
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


void	Mouse		( int button, int state, int x, int y )
{
	return;
}


void	Motion		( int x, int y )
{
	return;
}


void	Special		( int key, int x, int y )
{
	return;
}
