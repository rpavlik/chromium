/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*
  main.c

  This is an example of GL_EXT_blend_color.

  Christopher Niederauer, ccn@graphics.stanford.edu, 5/30/2001
*/


#include "../common/logo.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glext.h>

#ifndef GL_EXT_blend_color
#error Please update your GL/glext.h header file.
#endif

#ifndef GLX_ARB_get_proc_address
#ifdef __cplusplus
extern "C"
{
#endif
	extern void *glXGetProcAddressARB(const GLubyte * name);
#ifdef __cplusplus
}
#endif
#endif

/* #define CCN_DEBUG */
#define MULTIPLE_VIEWPORTS

#define TEST_EXTENSION_STRING  "GL_EXT_blend_color"

#ifndef APIENTRY
#define APIENTRY
#endif

typedef void (APIENTRY * GLBLENDCOLOREXTPROC) (GLclampf red, GLclampf green,
					       GLclampf blue, GLclampf alpha);


static GLBLENDCOLOREXTPROC glBlendColor_ext;
static GLuint currentWidth, currentHeight;
static GLuint texture[1];
static GLfloat bgColor[4] = { 0.2, 0.3, 0.8, 0.0 };


static void
Idle(void)
{
	glutPostRedisplay();
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

	/* Left Viewport */
	glViewport(0, 0, currentWidth >> 1, currentHeight);
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
	RenderString(-1.1, 1, "Constant Color Blending Off");
	glPopMatrix();

	/* Upper Right Viewport */
	glViewport(currentWidth >> 1, 0, currentWidth >> 1, currentHeight);
#endif /* MULTIPLE_VIEWPORTS */
	glEnable(GL_BLEND);
	glBlendColor_ext(1.0, 1.0, 0.0, 0.0);
	glBlendFunc(GL_CONSTANT_COLOR_EXT, GL_ZERO);
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
	glDisable(GL_BLEND);
	glPushMatrix();
	glLoadIdentity();
	RenderString(-1.1, 1, "Constant Color Blending On");
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
	currentHeight = 240;

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
	int x, y;
	GLubyte textureData[32 * 32];

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
	glGenTextures(1, texture);

	/* Create Trilinear MipMapped Texture */
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_LUMINANCE, 32, 32, GL_LUMINANCE,
			  GL_UNSIGNED_BYTE, textureData);

#ifdef WIN32
	glBlendColor_ext =
		(GLBLENDCOLOREXTPROC) wglGetProcAddress("glBlendColorEXT");
#elif defined(IRIX) || defined (SunOS)
	glBlendColor_ext = glBlendColorEXT;
#else
	glBlendColor_ext =
		(GLBLENDCOLOREXTPROC) glXGetProcAddressARB((const GLubyte *)
							   "glBlendColorEXT");
#endif
	if (glBlendColor_ext == NULL)
	{
		printf("Error linking to extensions!\n");
		exit(0);
	}

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
