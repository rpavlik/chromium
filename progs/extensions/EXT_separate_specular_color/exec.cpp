/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*

  exec.cpp

  This is an example of GL_EXT_separate_specular_color, described
  on page 141 of the NVidia OpenGL Extension Specifications.

  Christopher Niederauer, ccn@graphics.stanford.edu, 8/9/2001

*/

/* --- Preprocessor --------------------------------------------------------- */

#include <stdlib.h>
#include "exec.h"
#include "../common/logo.h"


/* --- Global Variables ----------------------------------------------------- */

static GLuint	currentWidth, currentHeight;
static GLuint	textureID, sphereList;
static GLfloat	bgColor[4] = { 0.1, 0.2, 0.4, 0.0 };


/* --- Function Definitions ------------------------------------------------- */

void	InitGL	( void )
{
	currentWidth = 240;
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
	const int	texWidth = 16, texHeight = 16;
	GLfloat		position[4] = { 1, 1, 0, 1 },
				specular[4] = { 1, 1, 1, 1 };
	GLubyte		textureData[16*16];
	GLUquadric	*q;

	glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, 40 );
	glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, specular );
	glLightfv( GL_LIGHT0, GL_POSITION, position );
	glColor3f( 1, 1, 1 );
	glEnable( GL_LIGHT0 );

	q = gluNewQuadric();
	gluQuadricDrawStyle( q, (GLenum)GLU_FILL );
	gluQuadricNormals( q, (GLenum)GLU_SMOOTH );
	gluQuadricTexture( q, GL_TRUE );
	glEnable( GL_CULL_FACE );
	glFrontFace( GL_CCW );
	glCullFace( GL_BACK );
	sphereList = glGenLists( 1 );
	glNewList( sphereList, GL_COMPILE );
	gluSphere( q, 2.5, 128, 64 );
	glEndList();

	for( int x=0; x<texWidth; x++ )
	{
		for( int y=0; y<texHeight; y++ )
		{
			if(( x < (texWidth>>1) && y < (texHeight>>1) ) || ( x >= (texWidth>>1) && y >= (texHeight>>1) ))
				textureData[ y*texWidth+x ] = (GLubyte)255;
			else
				textureData[ y*texWidth+x ] = (GLubyte)0;
		}
	}
	glGenTextures( 1, &textureID );
	glBindTexture( GL_TEXTURE_2D, textureID );
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE, 16, 16, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, textureData );

	glMatrixMode( GL_TEXTURE );
	glLoadIdentity();
	glMatrixMode( GL_MODELVIEW );
}


void	Idle		( void )
{
	glutPostRedisplay();
	return;
}


void	Display		( void )
{
	static float theta;

	glClear( GL_COLOR_BUFFER_BIT );
	
	glLoadIdentity();
	glTranslatef( 0, 0, -4 );
	glRotatef( theta, 0, 1, 0 );
	glRotatef( 90, 1, 0, 0 );

	theta += 0.1;
	if( theta > 90 )
		theta -= 90;

	glMatrixMode( GL_TEXTURE );
	glPushMatrix();
	glScalef( 16, 8, 1 );
	glMatrixMode( GL_MODELVIEW );

#ifdef MULTIPLE_VIEWPORTS
	// Left Viewport
	glViewport( 0, 0, currentWidth >> 1, currentHeight );

	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, textureID );
	glEnable( GL_LIGHTING );
	glLightModelf((GLenum) GL_LIGHT_MODEL_COLOR_CONTROL_EXT, GL_SINGLE_COLOR_EXT );
	glCallList( sphereList );
	glDisable( GL_LIGHTING );
	glDisable( GL_TEXTURE_2D );

	glPushMatrix();glLoadIdentity();
	RenderString( -1.1, 1, "SINGLE_COLOR_EXT" );
	glPopMatrix();

	// Upper Right Viewport
	glViewport( currentWidth >> 1, 0, currentWidth >> 1, currentHeight );
#endif

	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, textureID );
	glEnable( GL_LIGHTING );
	glLightModelf((GLenum) GL_LIGHT_MODEL_COLOR_CONTROL_EXT, GL_SEPARATE_SPECULAR_COLOR_EXT );
	glCallList( sphereList );
	glDisable( GL_LIGHTING );
	glDisable( GL_TEXTURE_2D );

	glPushMatrix();glLoadIdentity();
	RenderString( -1.1, 1, "SEPARATE_SPECULAR_COLOR_EXT" );
	glPopMatrix();

	glMatrixMode( GL_TEXTURE );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );

	glViewport( 0, 0, currentWidth, currentHeight );
	crExtensionsDrawLogo( currentWidth, currentHeight );
	glutSwapBuffers();
}

void	Reshape		( int width, int height )
{
	currentWidth = width;
	currentHeight = height;

	glViewport( 0, 0, currentWidth, currentHeight );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glFrustum( -1.0, 1.0, -1.0, 1.0, 1.0, 10.0 );
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
