/*

  exec.cpp

  This is an example of GL_EXT_texture_filter_anisotropic, described
  on page 185 of the NVidia OpenGL Extension Specifications.

  Christopher Niederauer, ccn@graphics.stanford.edu, 5/30/2001

*/

/* --- Preprocessor --------------------------------------------------------- */

#include "exec.h"
#include "../common/logo.h"

/* --- Global Variables ----------------------------------------------------- */

static GLuint	texture[4];

extern "C"
{
	GLuint	currentWidth, currentHeight;
}



/* --- Function Definitions ------------------------------------------------- */

void	InitGL	( void )
{
	currentWidth = 512;
	currentHeight = 384;
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE );
	glutInitWindowPosition( 5, 25 );
	glutInitWindowSize( currentWidth, currentHeight );
	glutCreateWindow( TEST_EXTENSION_STRING );
	
	glClearColor( 0.2, 0.3, 0.8, 0.0 );
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
	glGenTextures( 4, texture );
	
#ifdef MULTIPLE_VIEWPORTS
	
	// Create Bilinear Filtered Texture
	glBindTexture( GL_TEXTURE_2D, texture[0] );
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE, 32, 32, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, textureData );
	
	// Create Bilinear MipMapped Texture
	glBindTexture( GL_TEXTURE_2D, texture[1] );
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST );
	GLint err = gluBuild2DMipmaps( GL_TEXTURE_2D, GL_LUMINANCE, 32, 32, GL_LUMINANCE, GL_UNSIGNED_BYTE, textureData );
	
	// Create Trilinear MipMapped Texture
	glBindTexture( GL_TEXTURE_2D, texture[2] );
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR );
	err = gluBuild2DMipmaps( GL_TEXTURE_2D, GL_LUMINANCE, 32, 32, GL_LUMINANCE, GL_UNSIGNED_BYTE, textureData );
	
#endif

	float	testAni = 16;

	// Create Anisotropic Texture
	glBindTexture( GL_TEXTURE_2D, texture[3] );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameterfv( GL_TEXTURE_2D, (GLenum) GL_TEXTURE_MAX_ANISOTROPY_EXT, &testAni );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE, 32, 32, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, textureData );

	glGetTexParameterfv( GL_TEXTURE_2D, (GLenum) GL_TEXTURE_MAX_ANISOTROPY_EXT, &testAni );
	cout << "Current Anisotropy: " << testAni << endl;
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

	// Lower Left Viewport
	glViewport( 0, 0, currentWidth >> 1, currentHeight >> 1 );
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
	RenderString( -1.1, 1, "Bilinear Filtering" );
	glPopMatrix();

	// Upper Left Viewport
	glViewport( 0, currentHeight >> 1, currentWidth >> 1, currentHeight >> 1 );
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, texture[1] );
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
	RenderString( -1.08, 1, "Bilinear Mipmapped Filtering" );
	glPopMatrix();

	// Upper Right Viewport
	glViewport( currentWidth >> 1, currentHeight >> 1, currentWidth >> 1, currentHeight >> 1 );
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, texture[2] );
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
	RenderString( -1.1, 1, "Trilinear Mipmapped Filtering" );
	glPopMatrix();

	// Lower Right Viewport
	glViewport( currentWidth >> 1, 0, currentWidth >> 1, currentHeight >> 1 );
#endif
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, texture[3] );
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
	RenderString( -1.13, 1, "Anisotropic Filtering" );
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
	glFrustum( -1.0, 1.0, -1.0, 1.0, 1.0, 30.0 );
	glMatrixMode( GL_MODELVIEW );

	GLfloat	lineWidth;
	if(( lineWidth = currentWidth * currentHeight / (512*768)) < 1.0 )
		lineWidth = 1.0;
	glLineWidth( lineWidth );
}


void	Keyboard	( unsigned char key, int , int  )
{
	switch( key )
	{
		case 'q':
		case 'Q':
			cout << "User has quit. Exiting." << endl;
			exit( 0 );
		default:
			break;
	}
	return;
}
