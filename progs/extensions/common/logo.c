/*

	 logo.c

	 This file is for drawing the Chromium logo (raw file format).

	 Christopher Niederauer, ccn@graphics.stanford.edu, 6/20/2001

 */

#include <stdio.h>
#include "cr_logo.h"
#include "cr_glwrapper.h"
#include "cr_error.h"

void crExtensionsDrawLogo( int currentWidth, int currentHeight )
{
	static GLboolean inited = 0;
	static GLuint	   textureID;
	GLint actual_dims[4];

	if( !inited )
	{
		// Create the tile texture.
		glGenTextures( 1, &textureID );

		// Create Billinear Filtered Texture
		glBindTexture( GL_TEXTURE_2D, textureID );
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, CR_LOGO_H_WIDTH, CR_LOGO_H_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, raw_bytes );

		inited = 1;
	}

	glViewport( 0, 0, currentWidth, currentHeight );
	glGetIntegerv( GL_VIEWPORT, actual_dims );
	currentWidth = actual_dims[2];
	currentHeight = actual_dims[3];

	glPushMatrix();
	glLoadIdentity();
	glTranslatef( -5, -5, 0 );
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glOrtho( 0, actual_dims[2], actual_dims[3], 0, -1, 1 );

	glColor3f( 1, 1, 1 );
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glBindTexture( GL_TEXTURE_2D, textureID );
	glBegin( GL_QUAD_STRIP );
	glTexCoord2f( 0, 1 ); glVertex2i( currentWidth-CR_LOGO_H_WIDTH, currentHeight );
	glTexCoord2f( 1, 1 ); glVertex2i( currentWidth,   currentHeight );
	glTexCoord2f( 0, 0 ); glVertex2i( currentWidth-CR_LOGO_H_WIDTH, currentHeight-CR_LOGO_H_HEIGHT );
	glTexCoord2f( 1, 0 ); glVertex2i( currentWidth,   currentHeight-CR_LOGO_H_HEIGHT );
	glEnd();

	glDisable( GL_BLEND );
	glDisable( GL_TEXTURE_2D );

	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
}
