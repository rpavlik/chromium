/*

  exec.cpp

  This is an example of GL_EXT_blend_minmax, described
  on page 89 of the NVidia OpenGL Extension Specifications.

  Christopher Niederauer, ccn@graphics.stanford.edu, 6/29/2001

*/

/* --- Preprocessor --------------------------------------------------------- */

#include "exec.h"
#include "../common/logo.h"
#include "cr_glwrapper.h"

PFNGLBLENDEQUATIONEXTPROC glBlendEquationEXT;


/* --- Global Variables ----------------------------------------------------- */

static GLuint	currentWidth, currentHeight;
static GLfloat	bgColor[4] = { 0.2, 0.3, 0.8, 0.0 };


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
	glBlendEquationEXT = (PFNGLBLENDEQUATIONEXTPROC)wglGetProcAddress( "glBlendEquationEXT" );
#else
	glBlendEquationEXT = (PFNGLBLENDEQUATIONEXTPROC)glXGetProcAddressARB( (const GLubyte *) "glBlendEquationEXT" );
#endif
	if ( glBlendEquationEXT == NULL )
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
	
	glBegin( GL_QUADS );
	glColor3f( 0.5, 0.5, 0.5 );
		glVertex3f( -size, 0.0,  size );
		glVertex3f(  size, 0.0,  size );
		glVertex3f(  size, 0.0, -size );
		glVertex3f( -size, 0.0, -size );
	glEnd();
	
	glEnable( GL_POLYGON_OFFSET_FILL );
	glPolygonOffset( 0, -1 );
	glBlendEquationEXT( GL_MIN_EXT );
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
	glBlendEquationEXT( GL_FUNC_ADD_EXT );
	
	glColor3f( 1, 1, 1 );
	glPushMatrix();glLoadIdentity();
	RenderString( -1.1, 1, "MIN_EXT Blending" );
	glPopMatrix();

	// Upper Right Viewport
	glViewport( currentWidth >> 1, 0, currentWidth >> 1, currentHeight );
#endif
	glBegin( GL_QUADS );
	glColor3f( 0.5, 0.5, 0.5 );
		glVertex3f( -size, 0.0,  size );
		glVertex3f(  size, 0.0,  size );
		glVertex3f(  size, 0.0, -size );
		glVertex3f( -size, 0.0, -size );
	glEnd();
	
	glEnable( GL_POLYGON_OFFSET_FILL );
	glPolygonOffset( 0, -1 );
	glBlendEquationEXT( GL_MAX_EXT );
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
	glBlendEquationEXT( GL_FUNC_ADD_EXT );
	
	glColor3f( 1, 1, 1 );
	glPushMatrix();glLoadIdentity();
	RenderString( -1.1, 1, "MAX_EXT Blending" );
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
