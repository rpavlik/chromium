/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*

  main.cpp

  This is an example of GL_EXT_texture_edge_clamp.

  Christopher Niederauer, ccn@graphics.stanford.edu, 6/25/2001

*/


#include "../common/logo.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glext.h>

#define TEST_EXTENSION_STRING  "GL_EXT_texture_edge_clamp"
#if !defined(GL_EXT_texture_edge_clamp) && !defined(GL_SGIS_texture_edge_clamp)
#error Please update your GL/glext.h header file.
#endif

#ifdef GL_CLAMP_TO_EDGE
/* OpenGL 1.3 - do nothing */
#elif defined(GL_CLAMP_TO_EDGE_SGIS)
#define GL_CLAMP_TO_EDGE GL_CLAMP_TO_EDGE_SGIS
#elif defined(GL_CLAMP_TO_EDGE_EXT)
#define GL_CLAMP_TO_EDGE GL_CLAMP_TO_EDGE_EXT
#endif

/* #define CCN_DEBUG  */
/* #define DISPLAY_LISTS */
#define MULTIPLE_VIEWPORTS

static GLuint currentWidth, currentHeight;
static GLuint textureID[2];
static GLfloat bgColor[4] = { 0.2, 0.3, 0.8, 0 };


static void
Idle(void)
{
	glutPostRedisplay();
	return;
}


static void
Display(void)
{
	const float size = 1.0, texDetail = 1.0, texOffset = 0.5;
	static double theta = 0.0;

	glClear(GL_COLOR_BUFFER_BIT);

	theta += 0.05;

	glLoadIdentity();
	glRotated(90.0, 1.0, 0.0, 0.0);
	glTranslatef(0.0, -2.0, .0);
	glRotated(theta, 0.0, -1.0, 0.0);

#ifdef MULTIPLE_VIEWPORTS
	/* Left Viewport */
	glViewport(0, 0, currentWidth >> 1, currentHeight);
	glColor3f(1.0, 1.0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID[0]);
	glBegin(GL_QUADS);
	glTexCoord2f(-texDetail + texOffset, texDetail + texOffset);
	glVertex3f(-size, 0.0, size);

	glTexCoord2f(texDetail + texOffset, texDetail + texOffset);
	glVertex3f(size, 0.0, size);

	glTexCoord2f(texDetail + texOffset, -texDetail + texOffset);
	glVertex3f(size, 0.0, -size);

	glTexCoord2f(-texDetail + texOffset, -texDetail + texOffset);
	glVertex3f(-size, 0.0, -size);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glPushMatrix();
	glLoadIdentity();
	RenderString(-1.1, 1, "GL_CLAMP");
	glPopMatrix();

	/* Upper Right Viewport */
	glViewport(currentWidth >> 1, 0, currentWidth >> 1, currentHeight);
#endif /* MULTIPLE_VIEWPORTS */
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID[1]);
	glBegin(GL_QUADS);
	glTexCoord2f(-texDetail + texOffset, texDetail + texOffset);
	glVertex3f(-size, 0.0, size);

	glTexCoord2f(texDetail + texOffset, texDetail + texOffset);
	glVertex3f(size, 0.0, size);

	glTexCoord2f(texDetail + texOffset, -texDetail + texOffset);
	glVertex3f(size, 0.0, -size);

	glTexCoord2f(-texDetail + texOffset, -texDetail + texOffset);
	glVertex3f(-size, 0.0, -size);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glPushMatrix();
	glLoadIdentity();
	RenderString(-1.1, 1, "GL_CLAMP_TO_EDGE_EXT");
	glViewport(0, 0, currentWidth, currentHeight);
	crExtensionsDrawLogo(currentWidth, currentHeight);
	glPopMatrix();

	glutSwapBuffers();
}


static void
Reshape(int width, int height)
{
	currentWidth = width;
	currentHeight = height;

	glViewport(0, 0, currentWidth, currentHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 30.0);
	glMatrixMode(GL_MODELVIEW);
}


static void
Keyboard(unsigned char key, int x, int y)
{
	(void) x;
	(void) y;
	switch (key)
	{
	case 'Q':
	case 'q':
		printf("User has quit. Exiting.\n");
		exit(0);
	default:
		break;
	}
	return;
}



static void
InitGL(void)
{
	currentWidth = 320;
	currentHeight = 320;

#ifdef MULTIPLE_VIEWPORTS
	currentWidth <<= 1;
#endif

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowPosition(5, 25);
	glutInitWindowSize(currentWidth, currentHeight);
	glutCreateWindow(TEST_EXTENSION_STRING);

	glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	glutIdleFunc(Idle);
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);

}


static void
InitSpecial(void)
{
	GLubyte textureData[32 * 32];
	int x, y;

	/* Create the tile texture. */
	for (x = 0; x < 32; x++)
	{
		for (y = 0; y < 32; y++)
		{
			float result =
				(255.0 / 15) * sqrt((x - 16) * (x - 16) + (y - 16) * (y - 16));

			textureData[y * 32 + x] = result > 255 ? 255 : (GLubyte) result;
		}
	}

	/* Create the texture IDs. */
	glGenTextures(2, textureID);

#ifdef MULTIPLE_VIEWPORTS
	/* Create Bilinear Filtered texture with normal clamping. */
	glBindTexture(GL_TEXTURE_2D, textureID[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 32, 32, 0, GL_LUMINANCE,
		     GL_UNSIGNED_BYTE, textureData);
#endif

	/* Create Bilinear Filtered texture with normal clamping. */
	glBindTexture(GL_TEXTURE_2D, textureID[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 32, 32, 0, GL_LUMINANCE,
		     GL_UNSIGNED_BYTE, textureData);

	return;
}


int
main(int argc, char *argv[])
{
#ifndef macintosh
	glutInit(&argc, argv);
#endif

	printf("Written by Christopher Niederauer\n");
	printf("ccn@graphics.stanford.edu\n");

	InitGL();
	InitSpecial();

#ifdef CCN_DEBUG
	printf("    Vendor: %s\n", glGetString(GL_VENDOR));
	printf("  Renderer: %s\n", glGetString(GL_RENDERER));
	printf("   Version: %s\n", glGetString(GL_VERSION));
	printf("Extensions: %s\n", glGetString(GL_EXTENSIONS));
#endif

	if (CheckForExtension(TEST_EXTENSION_STRING))
	{
		printf("Extension %s supported.  Executing...\n", TEST_EXTENSION_STRING);
	}
	else
	{
		printf("Error: %s not supported.  Exiting.\n", TEST_EXTENSION_STRING);
		return 0;
	}

	glutMainLoop();

	return 0;
}
