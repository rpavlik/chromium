/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*

  exec.cpp

  This is an example of GL_NV_fog_distance, described
  on page 258 of the NVidia OpenGL Extension Specifications.

  Christopher Niederauer, ccn@graphics.stanford.edu, 6/12/2001

*/

/* --- Preprocessor --------------------------------------------------------- */

#include "exec.h"
#include "../common/logo.h"
#include <stdlib.h>

#ifdef SunOS
   #include <iostream.h>
   #include <iomanip.h>
#endif

/* --- Global Variables ----------------------------------------------------- */

GLuint	currentWidth, currentHeight;

static GLuint	texture[1];
static GLfloat	bgColor[4] = { 0.8, 0.8, 1.0, 0.0 };
static GLint	defaultFogDistanceMode;


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
	glutSpecialFunc( Special );
}


void	InitSpecial	( void )
{
#ifdef DISPLAY_LIST
	GLfloat	texDetail = 2.0;
	GLint	size = 30.0;
#endif
	GLint	x, y;

	GLubyte	textureData[ 32*32 ];
	
	// Create a noise texture (Grass).
	srand( 2 );
	for( x=0; x<32; x++ )
	{
		for( y=0; y<32; y++ )
		{
			textureData[ y*32+x ] = 255 - 32 + ( rand() % 32 );
		}
	}
	
	glEnable( GL_FOG );
	glFogfv( GL_FOG_COLOR, bgColor );
	glFogf( GL_FOG_START, 0.0 );
	glFogf( GL_FOG_END, 15.0 );
	glFogi( GL_FOG_MODE, GL_LINEAR );
	glGetIntegerv( (GLenum) GL_FOG_DISTANCE_MODE_NV, &defaultFogDistanceMode );

	cout << "FOG_DISTANCE_MODE_NV = ";
	switch( defaultFogDistanceMode )
	{
		case GL_EYE_RADIAL_NV:
			cout << "EYE_RADIAL_NV";
			break;
		case GL_EYE_PLANE:
			cout << "EYE_PLANE";
			break;
		case GL_EYE_PLANE_ABSOLUTE_NV:
			cout << "EYE_PLANE_ABSOLUTE_NV";
			break;
		default:
			cout << "Uh oh...  (0x" << hex << defaultFogDistanceMode << ")";
			break;
	}
	cout << endl;

	// Create the tile texture.
	glGenTextures( 1, texture );
	// Create Trillenar MipMapped Texture
	glBindTexture( GL_TEXTURE_2D, texture[0] );
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR );
	gluBuild2DMipmaps( GL_TEXTURE_2D, GL_LUMINANCE, 32, 32, GL_LUMINANCE, GL_UNSIGNED_BYTE, textureData );
	
#ifdef DISPLAY_LIST
	// Make the display list for the grass.
	glNewList( 1, GL_COMPILE );
		glEnable( GL_TEXTURE_2D );
		glColor3f( 0.6, 1.0, 0.2 );
		glBindTexture( GL_TEXTURE_2D, texture[0] );
		for( y=-size; y<size; y++ )
		{
			glBegin( GL_QUAD_STRIP );
			for( x=-size; x<=size; x++ )
			{
				glTexCoord2f( (x)*texDetail, (y+1)*texDetail ); glVertex3f( x, 0, y+1 );
				glTexCoord2f( (x)*texDetail, (y+0)*texDetail ); glVertex3f( x, 0, y+0 );
			}
			glEnd();
		}
		glDisable( GL_TEXTURE_2D );
	glEndList();
#endif
}


void	Idle		( void )
{
	glutPostRedisplay();
	return;
}


void	Display		( void )
{
	static double	theta = 0.0;
	
	// begin temp
	static GLfloat	texDetail = 2.0;
	static GLint	size = 30;
	static GLint	x, y;
	// end temp

	glClear( GL_COLOR_BUFFER_BIT );
	
	theta += 0.005;

	glLoadIdentity();
	glTranslated( 0.0, -2.0, 0.0 );
	glRotated( 30.0, 1.0, 0.0, 0.0 );
	glRotated( theta, 0.0, 1.0, 0.0 );
	
#ifdef MULTIPLE_VIEWPORTS
	// Left Viewport
	glViewport( 0, 0, currentWidth >> 1, currentHeight );
	glFogi( (GLenum) GL_FOG_DISTANCE_MODE_NV, defaultFogDistanceMode );
#ifdef DISPLAY_LIST
	glCallList( 1 );
#else
	glEnable( GL_TEXTURE_2D );
	glColor3f( 0.6, 1.0, 0.2 );
	glBindTexture( GL_TEXTURE_2D, texture[0] );
	for( y=-size; y<size; y++ )
	{
		glBegin( GL_QUAD_STRIP );
		for( x=-size; x<=size; x++ )
		{
			glTexCoord2f( (x)*texDetail, (y+1)*texDetail ); glVertex3f( x, 0, y+1 );
			glTexCoord2f( (x)*texDetail, (y+0)*texDetail ); glVertex3f( x, 0, y+0 );
		}
		glEnd();
	}
	glDisable( GL_TEXTURE_2D );
#endif
	
	glColor3f( 0, 0, 0 );
	glDisable( GL_FOG );
	RenderString( -1.08, 1, "EYE_PLANE_ABSOLUTE_NV" );
	RenderString( -1.08, .85, "(implementation specific default)" );
	glEnable( GL_FOG );

	// Right Viewport
	glViewport( currentWidth >> 1, 0, currentWidth >> 1, currentHeight );
#endif
	glFogi( (GLenum) GL_FOG_DISTANCE_MODE_NV, GL_EYE_RADIAL_NV );
#ifdef DISPLAY_LIST
	glCallList( 1 );
#else
	glEnable( GL_TEXTURE_2D );
	glColor3f( 0.6, 1.0, 0.2 );
	glBindTexture( GL_TEXTURE_2D, texture[0] );
	for( y=-size; y<size; y++ )
	{
		glBegin( GL_QUAD_STRIP );
		for( x=-size; x<=size; x++ )
		{
			glTexCoord2f( (x)*texDetail, (y+1)*texDetail ); glVertex3f( x, 0, y+1 );
			glTexCoord2f( (x)*texDetail, (y+0)*texDetail ); glVertex3f( x, 0, y+0 );
		}
		glEnd();
	}
	glDisable( GL_TEXTURE_2D );
#endif
	glColor3f( 0, 0, 0 );
	glDisable( GL_FOG );
	RenderString( -1.08, 1, "EYE_RADIAL_NV" );
	glViewport( 0, 0, currentWidth, currentHeight );
	crExtensionsDrawLogo( currentWidth, currentHeight );
	glEnable( GL_FOG );

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


void	Keyboard	( unsigned char key, int , int )
{
	static	GLboolean	wireframe = false;

	switch( key )
	{
		case 'Q':
		case 'q':
			cout << "User has quit. Exiting." << endl;
			exit( 0 );
		case 'W':
		case 'w':
			wireframe = !wireframe;
			if( wireframe )
			{
				cout << "Outputting wireframe mode..." << endl;
				glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
				glClearColor( 0.2, 0.3, 0.8, 0.0 );
			}
			else
			{
				cout << "Outputting solid mode..." << endl;
				glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
				glClearColor( bgColor[0], bgColor[1], bgColor[2], bgColor[3] );
			}
		default:
			break;
	}
	return;
}


void	Special		( int key, int, int )
{
	static GLfloat	fogDistance = 15;

	switch( key )
	{
		case GLUT_KEY_UP:
			fogDistance += 1;
			if( fogDistance > 25 )
				fogDistance = 25;
			else
				cout << "Fog Distance: " << fogDistance << endl;
			glFogf( GL_FOG_END, fogDistance );
			break;
		case GLUT_KEY_DOWN:
			fogDistance -= 1;
			if( fogDistance < 3 )
				fogDistance = 3;
			else
				cout << "Fog Distance: " << fogDistance << endl;
			glFogf( GL_FOG_END, fogDistance );
			break;
	}
	return;
}
