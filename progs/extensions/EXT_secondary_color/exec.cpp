/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*

  exec.cpp

  This is an example of GL_EXT_secondary_color, described
  on page 133 of the NVidia OpenGL Extension Specifications.

  Christopher Niederauer, ccn@graphics.stanford.edu, 7/31/2001

*/

/* --- Preprocessor --------------------------------------------------------- */

#include "exec.h"
#include "../common/logo.h"


/* --- Global Variables ----------------------------------------------------- */

PFNGLSECONDARYCOLORPOINTEREXTPROC	glSecondaryColorPointerEXT;
PFNGLSECONDARYCOLOR3FEXTPROC		glSecondaryColor3fEXT;

static GLuint	currentWidth, currentHeight;
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
	const int texWidth = 256, texHeight = 256;

#ifdef WIN32
	glSecondaryColorPointerEXT = (PFNGLSECONDARYCOLORPOINTEREXTPROC)wglGetProcAddress( "glSecondaryColorPointerEXT" );
	glSecondaryColor3fEXT = (PFNGLSECONDARYCOLOR3FEXTPROC)wglGetProcAddress( "glSecondaryColor3fEXT" );

	if( !glSecondaryColor3fEXT || !glSecondaryColorPointerEXT )
	{
    	cout << "Error trying to link to extensions!" << endl;
		exit( 0 );
	}
#endif
}


void	Idle		( void )
{
	glutPostRedisplay();
	return;
}


void	Display		( void )
{
	const  float size = 1.3;
	const  int   stride = sizeof(GLfloat)*(2+3+3);
	static float theta;
	static GLfloat vertexArray[] =
	{
		// Vertex2f
		// Color3f
		// SecondaryColor3fEXT

		-size, -size,
		1.0, 1.0, 1.0,
		1.0, 0.0, 1.0,

		-size, size,
		1.0, 1.0, 1.0,
		1.0, 0.0, 0.0,

		size, size,
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0,

		size, -size,
		0.0, 0.0, 0.0,
		0.0, 0.0, 1.0
	};

	glClear( GL_COLOR_BUFFER_BIT );
	
	glLoadIdentity();
	glTranslatef( 0, 0, -2 );
	glRotatef( theta, 0, 0, 1 );

	theta += 0.01;

#ifdef MULTIPLE_VIEWPORTS
	// Left Viewport
	glViewport( 0, 0, currentWidth >> 1, currentHeight );

	glEnable( GL_COLOR_SUM_EXT );
	glBegin( GL_QUADS );
	{
		glColor3f( 1, 1, 1 );
			glSecondaryColor3fEXT( 1, 0, 1 );
			glVertex2f( -size, -size );

			glSecondaryColor3fEXT( 1, 0, 0 );
			glVertex2f( -size,  size );

		glColor3f( 0, 0, 0 );
			glSecondaryColor3fEXT( 0, 1, 0 );
			glVertex2f(  size,  size );

			glSecondaryColor3fEXT( 0, 0, 1 );
			glVertex2f(  size, -size );
	}
	glEnd();
	glDisable( GL_COLOR_SUM_EXT );

	glColor3f( 1, 1, 1 );
	glPushMatrix();glLoadIdentity();
	RenderString( -1.1, 1, "SecondaryColor3fEXT" );
	glPopMatrix();

	// Upper Right Viewport
	glViewport( currentWidth >> 1, 0, currentWidth >> 1, currentHeight );
#endif

	glEnable( GL_COLOR_SUM_EXT );
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_COLOR_ARRAY );
	glEnableClientState( GL_SECONDARY_COLOR_ARRAY_EXT );
	glVertexPointer( 2, GL_FLOAT, stride, vertexArray );
	glColorPointer( 3, GL_FLOAT, stride, vertexArray+2 );
	glSecondaryColorPointerEXT( 3, GL_FLOAT, stride, vertexArray+5 );
	glDrawArrays( GL_QUADS, 0, 4 );
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );
	glDisableClientState( GL_SECONDARY_COLOR_ARRAY_EXT );
	glDisable( GL_COLOR_SUM_EXT );

	glColor3f( 1, 1, 1 );
	glPushMatrix();glLoadIdentity();
	RenderString( -1.1, 1, "SecondaryColorPointerEXT" );
	glPopMatrix();

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
