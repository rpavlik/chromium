/*

  exec.cpp

  This is an example of GL_EXT_secondary_color, described
  on page 170 of the NVidia OpenGL Extension Specifications.

  Christopher Niederauer, ccn@graphics.stanford.edu, 6/29/2001

*/

/* --- Preprocessor --------------------------------------------------------- */

#include "exec.h"
#include "../common/logo.h"

#include <math.h>


/* --- Global Variables ----------------------------------------------------- */

static GLuint	currentWidth, currentHeight;
static GLuint	textureID[2];
static GLfloat	bgColor[4] = { 0.2, 0.3, 0.8, 0 };


/* --- Extension Declarations ---------------------------------------------- */

//PFNGLBLENDCOLOREXTPROC glBlendColorEXT;


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
	GLubyte	textureData[ 32*32 ];

	// Create the tile texture.
	for( int x=0; x<32; x++ )
	{
		for( int y=0; y<32; y++ )
		{
			float result = (255.0/15)*sqrt( (x-16)*(x-16)+(y-16)*(y-16) );
			
			textureData[ y*32+x ] = result > 255 ? 255: (GLubyte) result;
		}
	}

	// Create the texture IDs.
	glGenTextures( 2, textureID );

#ifdef MULTIPLE_VIEWPORTS
	// Create Bilinear Filtered texture with normal clamping.
	glBindTexture( GL_TEXTURE_2D, textureID[0] );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE, 32, 32, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, textureData );
#endif

	// Create Bilinear Filtered texture with normal clamping.
	glBindTexture( GL_TEXTURE_2D, textureID[1] );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE_EXT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE_EXT );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE, 32, 32, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, textureData );
	
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
					texDetail = 1.0,
					texOffset = 0.5;
	static double	theta = 0.0;

	glClear( GL_COLOR_BUFFER_BIT );
	
	theta += 0.05;

	glLoadIdentity();
	glRotated( 90.0, 1.0, 0.0, 0.0 );
	glTranslatef( 0.0, -2.0, .0 );
	glRotated( theta, 0.0, -1.0, 0.0 );

#ifdef MULTIPLE_VIEWPORTS
	// Left Viewport
	glViewport( 0, 0, currentWidth >> 1, currentHeight );
	glColor3f( 1.0, 1.0, 1.0 );
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, textureID[0] );
	glBegin( GL_QUADS );
		glTexCoord2f( -texDetail+texOffset,  texDetail+texOffset );
		glVertex3f( -size, 0.0,  size );

		glTexCoord2f(  texDetail+texOffset,  texDetail+texOffset );
		glVertex3f(  size, 0.0,  size );
		
		glTexCoord2f(  texDetail+texOffset, -texDetail+texOffset );
		glVertex3f(  size, 0.0, -size );
		
		glTexCoord2f( -texDetail+texOffset, -texDetail+texOffset );
		glVertex3f( -size, 0.0, -size );
	glEnd();

	glDisable( GL_TEXTURE_2D );
	glPushMatrix();glLoadIdentity();
	RenderString( -1.1, 1, "GL_CLAMP" );
	glPopMatrix();

	// Upper Right Viewport
	glViewport( currentWidth >> 1, 0, currentWidth >> 1, currentHeight );
#endif
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, textureID[1] );
	glBegin( GL_QUADS );
		glTexCoord2f( -texDetail+texOffset,  texDetail+texOffset );
		glVertex3f( -size, 0.0,  size );

		glTexCoord2f(  texDetail+texOffset,  texDetail+texOffset );
		glVertex3f(  size, 0.0,  size );
		
		glTexCoord2f(  texDetail+texOffset, -texDetail+texOffset );
		glVertex3f(  size, 0.0, -size );
		
		glTexCoord2f( -texDetail+texOffset, -texDetail+texOffset );
		glVertex3f( -size, 0.0, -size );
	glEnd();
	
	glDisable( GL_TEXTURE_2D );
	glPushMatrix();glLoadIdentity();
	RenderString( -1.1, 1, "GL_CLAMP_TO_EDGE_EXT" );
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
