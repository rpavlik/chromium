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

#include "../common/logo.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glext.h>

#define	TEST_EXTENSION_STRING	"GL_EXT_texture_filter_anisotropic"
#ifndef	GL_EXT_texture_filter_anisotropic
#error	Please update your GL/glext.h header file.
#endif

/* #define CCN_DEBUG */
#define MULTIPLE_VIEWPORTS

static GLuint texture[4];
static GLuint currentWidth, currentHeight;


static void
Idle(void)
{
	glutPostRedisplay();
	return;
}


static void
Display(void)
{
	const float size = 50.0;
	static double theta = 0.0;

	glClear(GL_COLOR_BUFFER_BIT);

	theta += 0.005;

	glLoadIdentity();
	glTranslatef(0.0, -2.0, 0.0);
	glRotated(30.0, 1.0, 0.0, 0.0);
	glRotated(theta, 0.0, 1.0, 0.0);
	glColor3f(1.0, 1.0, 1.0);

#ifdef MULTIPLE_VIEWPORTS

	/* Lower Left Viewport */
	glViewport(0, 0, currentWidth >> 1, currentHeight >> 1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glBegin(GL_QUADS);
	glTexCoord2f(-size / 2, size / 2);
	glVertex3f(-size, 0.0, size);
	glTexCoord2f(size / 2, size / 2);
	glVertex3f(size, 0.0, size);
	glTexCoord2f(size / 2, -size / 2);
	glVertex3f(size, 0.0, -size);
	glTexCoord2f(-size / 2, -size / 2);
	glVertex3f(-size, 0.0, -size);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glPushMatrix();
	glLoadIdentity();
	RenderString(-1.1, 1, "Bilinear Filtering");
	glPopMatrix();

	/* Upper Left Viewport */
	glViewport(0, currentHeight >> 1, currentWidth >> 1, currentHeight >> 1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glBegin(GL_QUADS);
	glTexCoord2f(-size / 2, size / 2);
	glVertex3f(-size, 0.0, size);
	glTexCoord2f(size / 2, size / 2);
	glVertex3f(size, 0.0, size);
	glTexCoord2f(size / 2, -size / 2);
	glVertex3f(size, 0.0, -size);
	glTexCoord2f(-size / 2, -size / 2);
	glVertex3f(-size, 0.0, -size);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glPushMatrix();
	glLoadIdentity();
	RenderString(-1.08, 1, "Bilinear Mipmapped Filtering");
	glPopMatrix();

	/* Upper Right Viewport */
	glViewport(currentWidth >> 1, currentHeight >> 1, currentWidth >> 1,
		   currentHeight >> 1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glBegin(GL_QUADS);
	glTexCoord2f(-size / 2, size / 2);
	glVertex3f(-size, 0.0, size);
	glTexCoord2f(size / 2, size / 2);
	glVertex3f(size, 0.0, size);
	glTexCoord2f(size / 2, -size / 2);
	glVertex3f(size, 0.0, -size);
	glTexCoord2f(-size / 2, -size / 2);
	glVertex3f(-size, 0.0, -size);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glPushMatrix();
	glLoadIdentity();
	RenderString(-1.1, 1, "Trilinear Mipmapped Filtering");
	glPopMatrix();

	/* Lower Right Viewport */
	glViewport(currentWidth >> 1, 0, currentWidth >> 1, currentHeight >> 1);
#endif /* MULTIPLE_VIEWPORTS */
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glBegin(GL_QUADS);
	glTexCoord2f(-size / 2, size / 2);
	glVertex3f(-size, 0.0, size);
	glTexCoord2f(size / 2, size / 2);
	glVertex3f(size, 0.0, size);
	glTexCoord2f(size / 2, -size / 2);
	glVertex3f(size, 0.0, -size);
	glTexCoord2f(-size / 2, -size / 2);
	glVertex3f(-size, 0.0, -size);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glPushMatrix();
	glLoadIdentity();
	RenderString(-1.13, 1, "Anisotropic Filtering");
	glPopMatrix();

	glViewport(0, 0, currentWidth, currentHeight);
	crExtensionsDrawLogo(currentWidth, currentHeight);

	glutSwapBuffers();
}


static void
Reshape(int width, int height)
{
	GLfloat lineWidth;

	currentWidth = width;
	currentHeight = height;

	glViewport(0, 0, currentWidth, currentHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 30.0);
	glMatrixMode(GL_MODELVIEW);

	if ((lineWidth = currentWidth * currentHeight / (512 * 768)) < 1.0)
		lineWidth = 1.0;
	glLineWidth(lineWidth);
}


static void
Keyboard(unsigned char key, int x, int y)
{
	(void) x;
	(void) y;
	switch (key)
	{
	case 'q':
	case 'Q':
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
	currentWidth = 512;
	currentHeight = 384;
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowPosition(5, 25);
	glutInitWindowSize(currentWidth, currentHeight);
	glutCreateWindow(TEST_EXTENSION_STRING);

	glClearColor(0.2, 0.3, 0.8, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glutIdleFunc(Idle);
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(NULL);
	glutMotionFunc(NULL);
	glutSpecialFunc(NULL);
}


static void
InitSpecial(void)
{
	GLubyte textureData[32 * 32];
	float testAni = 16;
	int x, y;
	GLint err;

	for (x = 0; x < 32; x++)
	{
		for (y = 0; y < 32; y++)
		{
			if ((x < 16 && y < 16) || (x >= 16 && y >= 16))
				textureData[y * 32 + x] = 0;
			else
				textureData[y * 32 + x] = 255;
		}
	}

	/* Create the tile texture. */
	glGenTextures(4, texture);

#ifdef MULTIPLE_VIEWPORTS

	/* Create Bilinear Filtered Texture */
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 32, 32, 0, GL_LUMINANCE,
		     GL_UNSIGNED_BYTE, textureData);

	/* Create Bilinear MipMapped Texture */
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_NEAREST);
	err =gluBuild2DMipmaps(GL_TEXTURE_2D, GL_LUMINANCE, 32, 32, GL_LUMINANCE,
			       GL_UNSIGNED_BYTE, textureData);

	/* Create Trilinear MipMapped Texture */
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
	err = gluBuild2DMipmaps(GL_TEXTURE_2D, GL_LUMINANCE, 32, 32, GL_LUMINANCE,
				GL_UNSIGNED_BYTE, textureData);

#endif

	/* Create Anisotropic Texture */
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterfv(GL_TEXTURE_2D, (GLenum) GL_TEXTURE_MAX_ANISOTROPY_EXT, &testAni);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 32, 32, 0, GL_LUMINANCE,
		     GL_UNSIGNED_BYTE, textureData);

	glGetTexParameterfv(GL_TEXTURE_2D, (GLenum) GL_TEXTURE_MAX_ANISOTROPY_EXT, &testAni);
	printf("Current Anisotropy: %f", testAni);
}


int
main(int argc, char *argv[])
{
	glutInit(&argc, argv);

	printf("Written by Christopher Niederauer\n");
	printf("ccn@graphics.stanford.edu\n");

	InitGL();

#ifdef CCN_DEBUG
	printf("    Vendor: %s\n", glGetString(GL_VENDOR));
	printf("  Renderer: %s\n", glGetString(GL_RENDERER));
	printf("   Version: %s\n", glGetString(GL_VERSION));
	printf("Extensions: %s\n", glGetString(GL_EXTENSIONS));
#endif

	if (CheckForExtension(TEST_EXTENSION_STRING))
	{
		float maxAni;
		glGetFloatv((GLenum) GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAni);

		printf("Extension %s supported.  Executing...\n", TEST_EXTENSION_STRING);
		printf("GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT = %f\n", maxAni);
	}
	else
	{
		printf("Error: %s not supported.  Exiting.\n", TEST_EXTENSION_STRING);
		return 0;
	}

	InitSpecial();
	glutMainLoop();

	return 0;
}
