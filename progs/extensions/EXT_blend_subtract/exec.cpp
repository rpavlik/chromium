/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*

  exec.cpp

  This is an example of GL_EXT_blend_subtract, described
  on page 92 of the NVidia OpenGL Extension Specifications.

  Christopher Niederauer, ccn@graphics.stanford.edu, 6/29/2001

*/

/* --- Preprocessor --------------------------------------------------------- */

#include <stdlib.h>
#include "exec.h"
#include "../common/logo.h"

typedef void (APIENTRY * GLBLENDEQUATIONEXTPROC) (GLenum mode);

GLBLENDEQUATIONEXTPROC glBlendEquation_ext;


/* --- Global Variables ----------------------------------------------------- */

static GLuint	currentWidth, currentHeight;
static GLfloat	bgColor[4] = { 0, 0, 1, 0.0 };


/* --- Function Definitions ------------------------------------------------- */

void	InitGL	( void )
{
	currentWidth = 320;
	currentHeight = 320;

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
#ifdef WIN32
	glBlendEquation_ext = (GLBLENDEQUATIONEXTPROC)wglGetProcAddress( "glBlendEquationEXT" );
#else
	glBlendEquation_ext = (GLBLENDEQUATIONEXTPROC)glXGetProcAddressARB( (const GLubyte *) "glBlendEquationEXT" );
#endif
	if ( glBlendEquation_ext == NULL )
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
	const float		size = 1.0;
	static double	theta = 0.0;

	glClear( GL_COLOR_BUFFER_BIT );
	
	theta += 0.05;

	glLoadIdentity();
	glRotated( 90, 1, 0, 0 );
	glTranslatef( 0, -2, 0 );
	glRotated( theta, 0, 1, 0 );
	glColor3f( 1, 1, 1 );

#ifdef MULTIPLE_VIEWPORTS

	// Left Viewport
	glViewport( 0, 0, currentWidth >> 1, currentHeight );
	
	glPushMatrix();
	glLoadIdentity();
	glRotated( 90, 1, 0, 0 );
	glTranslatef( 0, -2.1, 0 );
	glBegin( GL_QUADS );
		glColor3f( 0, 1, 0 );
		glVertex3f( -size, 0.0,  size );
		glVertex3f(  size, 0.0,  size );
		glColor3f( 1, 0, 0 );
		glVertex3f(  size, 0.0, -size );
		glVertex3f( -size, 0.0, -size );
	glEnd();
	glPopMatrix();
	
	glEnable( GL_POLYGON_OFFSET_FILL );
	glPolygonOffset( 0, -1 );
	glBlendEquation_ext( GL_FUNC_SUBTRACT_EXT );
	glBlendFunc( GL_ONE, GL_ONE );
	glEnable( GL_BLEND );
	glBegin( GL_QUADS );
		glColor3f( 0, 0, 0 );
		glVertex3f( -size, 0.0,  size );
		glVertex3f(  size, 0.0,  size );
		
		glColor3f( 1, 1, 1 );
		glVertex3f(  size, 0.0, -size );
		glVertex3f( -size, 0.0, -size );
	glEnd();
	glDisable( GL_BLEND );
	glDisable( GL_POLYGON_OFFSET_FILL );
	glBlendEquation_ext( GL_FUNC_ADD_EXT );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
	glColor3f( 1, 1, 1 );
	glPushMatrix();glLoadIdentity();
	RenderString( -1.1, 1, "FUNC_SUBTRACT_EXT" );
	glPopMatrix();

	// Upper Right Viewport
	glViewport( currentWidth >> 1, 0, currentWidth >> 1, currentHeight );
#endif
	glPushMatrix();
	glLoadIdentity();
	glRotated( 90, 1, 0, 0 );
	glTranslatef( 0, -2.1, 0 );
	glBegin( GL_QUADS );
		glColor3f( 0, 1, 0 );
		glVertex3f( -size, 0.0,  size );
		glVertex3f(  size, 0.0,  size );
		glColor3f( 1, 0, 0 );
		glVertex3f(  size, 0.0, -size );
		glVertex3f( -size, 0.0, -size );
	glEnd();
	glPopMatrix();
	
	glEnable( GL_POLYGON_OFFSET_FILL );
	glPolygonOffset( 0, -1 );
	glBlendEquation_ext( GL_FUNC_REVERSE_SUBTRACT_EXT );
	glBlendFunc( GL_ONE, GL_ONE );
	glEnable( GL_BLEND );
	glBegin( GL_QUADS );
		glColor3f( 0, 0, 0 );
		glVertex3f( -size, 0.0,  size );
		glVertex3f(  size, 0.0,  size );
		
		glColor3f( 1, 1, 1 );
		glVertex3f(  size, 0.0, -size );
		glVertex3f( -size, 0.0, -size );
	glEnd();
	glDisable( GL_BLEND );
	glDisable( GL_POLYGON_OFFSET_FILL );
	glBlendEquation_ext( GL_FUNC_ADD_EXT );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
	glColor3f( 1, 1, 1 );
	glPushMatrix();glLoadIdentity();
	RenderString( -1.1, 1, "FUNC_REVERSE_SUBTRACT_EXT" );
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
