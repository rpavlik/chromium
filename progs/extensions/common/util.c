/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <string.h>
#include <GL/glut.h>
#include "logo.h"

#define	SMOOTH_TEXT

void
RenderString(float x, float y, char *string)
{
	GLfloat size = 0.0009;
	int len, i;

	glPushMatrix();
	glLoadIdentity();
	glTranslatef(x, y, -1.2);
	glScalef(size, size, size);
	len = (int) strlen(string);

#ifdef SMOOTH_TEXT
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif

	for (i = 0; i < len; i++)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, string[i]);
	glPopMatrix();

#ifdef SMOOTH_TEXT
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
#endif
}


int
CheckForExtension(const char *extension)
//  Searches through the OpenGL extensions string for extension.
//  Returns true if found, otherwise returns false.
{
	const GLubyte *extensions = NULL, *start;
	GLubyte *where, *terminator;

	//  Extension names should not have spaces.
	where = (GLubyte *) strchr(extension, ' ');
	if (where || *extension == '\0')
		return 0;
	extensions = glGetString(GL_EXTENSIONS);
	//  It takes a bit of care to be fool-proof about parsing the
	//  OpenGL extensions string. Don't be fooled by sub-strings,
	//  etc.
	start = extensions;
	for (;;)
	{
		where = (GLubyte *) strstr((const char *) start, extension);
		if (!where)
			break;
		terminator = where + strlen(extension);
		if (where == start || *(where - 1) == ' ')
			if (*terminator == ' ' || *terminator == '\0')
				return 1;
		start = terminator;
	}
	return 0;
}
