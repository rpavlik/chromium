/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*

  main.cpp

  This is an example of GL_EXT_texture_filter_anisotropic, described
  on page 185 of the NVidia OpenGL Extension Specifications.

  Christopher Niederauer, ccn@graphics.stanford.edu, 5/30/2001

*/

/* --- Preprocessor --------------------------------------------------------- */

#include "exec.h"
#include <string.h>

#ifndef bool
#define bool char
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif



/* --- Function Prototypes -------------------------------------------------- */

bool	CheckForExtension	( const char* );
int		main				( int, char*[] );



/* --- Function Definitions ------------------------------------------------- */


void	RenderString	( float x, float y, char *string )
{
	GLfloat	size = 0.0009;
	int		len;
	
	glPushMatrix();
	glLoadIdentity();
	glTranslatef( x, y, -1.2 );
	glScalef( size, size, size );
	len = (int)strlen( string );

#ifdef SMOOTH_TEXT
	glEnable( GL_LINE_SMOOTH );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
#endif

	for( int i=0; i<len; i++ )
		glutStrokeCharacter( GLUT_STROKE_ROMAN, string[i] );
	glPopMatrix();

#ifdef SMOOTH_TEXT
	glDisable( GL_BLEND );
	glDisable( GL_LINE_SMOOTH );
#endif
}


bool	CheckForExtension	( const char *extension )
//	Searches through the OpenGL extensions string for extension.
//	Returns true if found, otherwise returns false.
{
	const GLubyte	*extensions = NULL,
					*start;
	GLubyte			*where,
					*terminator;
	
	//	Extension names should not have spaces.
	where = (GLubyte*)strchr( extension, ' ' );
	if( where || *extension == '\0' )
		return 0;
	extensions = glGetString( GL_EXTENSIONS );
	//	It takes a bit of care to be fool-proof about parsing the
	//	OpenGL extensions string. Don't be fooled by sub-strings,
	//	etc.
	start = extensions;
	for (;;)
	{
		where = (GLubyte*)strstr((const char*)start, extension );
		if( !where )
			break;
		terminator = where + strlen( extension );
		if( where == start || *(where - 1) == ' ' )
			if( *terminator == ' ' || *terminator == '\0' )
				return true;
		start = terminator;
	}
	return false;
}


int		main	( int argc, char *argv[] )
{
	#ifndef macintosh
		glutInit( &argc, argv );
	#endif

	cout << "Written by Christopher Niederauer" << endl;
	cout << "ccn@graphics.stanford.edu" << endl << endl;

	InitGL();
	InitSpecial();

	#ifdef CCN_DEBUG
		cout << "  Vendor: " << glGetString( GL_VENDOR ) << endl;
		cout << "Renderer: " << glGetString( GL_RENDERER ) << endl;
		cout << " Version: " << glGetString( GL_VERSION ) << endl << endl;
		cout << "Extensions: " << glGetString( GL_EXTENSIONS ) << endl << endl;
	#endif

	if( CheckForExtension( TEST_EXTENSION_STRING ))
	{
		cout << "Extension " << TEST_EXTENSION_STRING
			<< " supported.  Executing..." << endl;
	}
	else
	{
		cout << "Error: " << TEST_EXTENSION_STRING
			<< " not supported.  Exiting." << endl;
		return 0;
	}

	glutMainLoop();
	
	return 0;
}
