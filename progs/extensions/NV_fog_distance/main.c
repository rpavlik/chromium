/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*

  main.cpp

  This is an example of GL_NV_fog_distance.

  Christopher Niederauer, ccn@graphics.stanford.edu, 5/30/2001

*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glext.h>
#include "../common/logo.h"

#define	TEST_EXTENSION_STRING	"GL_NV_fog_distance"
#ifndef	GL_NV_fog_distance
#error	Please update your GL/glext.h header file.
#endif

/*#define	CCN_DEBUG */
#define	DISPLAY_LISTS
#define	MULTIPLE_VIEWPORTS

static GLuint currentWidth, currentHeight;
static GLuint texture[1];
static GLfloat bgColor[4] = { 0.8, 0.8, 1.0, 0.0 };
static GLint defaultFogDistanceMode;


static void
Idle(void)
{
	glutPostRedisplay();
	return;
}


static void
Display(void)
{
	static double theta = 0.0;

	// begin temp
	static GLfloat texDetail = 2.0;
	static GLint size = 30;
	static GLint x, y;
	// end temp

	glClear(GL_COLOR_BUFFER_BIT);

	theta += 0.005;

	glLoadIdentity();
	glTranslated(0.0, -2.0, 0.0);
	glRotated(30.0, 1.0, 0.0, 0.0);
	glRotated(theta, 0.0, 1.0, 0.0);

#ifdef MULTIPLE_VIEWPORTS
	// Left Viewport
	glViewport(0, 0, currentWidth >> 1, currentHeight);
	glFogi((GLenum) GL_FOG_DISTANCE_MODE_NV, defaultFogDistanceMode);
#ifdef DISPLAY_LIST
	glCallList(1);
#else
	glEnable(GL_TEXTURE_2D);
	glColor3f(0.6, 1.0, 0.2);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	for (y = -size; y < size; y++)
	{
		glBegin(GL_QUAD_STRIP);
		for (x = -size; x <= size; x++)
		{
			glTexCoord2f((x) * texDetail, (y + 1) * texDetail);
			glVertex3f(x, 0, y + 1);
			glTexCoord2f((x) * texDetail, (y + 0) * texDetail);
			glVertex3f(x, 0, y + 0);
		}
		glEnd();
	}
	glDisable(GL_TEXTURE_2D);
#endif

	glColor3f(0, 0, 0);
	glDisable(GL_FOG);
	RenderString(-1.08, 1, "EYE_PLANE_ABSOLUTE_NV");
	RenderString(-1.08, .85, "(implementation specific default)");
	glEnable(GL_FOG);

	// Right Viewport
	glViewport(currentWidth >> 1, 0, currentWidth >> 1, currentHeight);
#endif
	glFogi((GLenum) GL_FOG_DISTANCE_MODE_NV, GL_EYE_RADIAL_NV);
#ifdef DISPLAY_LIST
	glCallList(1);
#else
	glEnable(GL_TEXTURE_2D);
	glColor3f(0.6, 1.0, 0.2);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	for (y = -size; y < size; y++)
	{
		glBegin(GL_QUAD_STRIP);
		for (x = -size; x <= size; x++)
		{
			glTexCoord2f((x) * texDetail, (y + 1) * texDetail);
			glVertex3f(x, 0, y + 1);
			glTexCoord2f((x) * texDetail, (y + 0) * texDetail);
			glVertex3f(x, 0, y + 0);
		}
		glEnd();
	}
	glDisable(GL_TEXTURE_2D);
#endif
	glColor3f(0, 0, 0);
	glDisable(GL_FOG);
	RenderString(-1.08, 1, "EYE_RADIAL_NV");
	glViewport(0, 0, currentWidth, currentHeight);
	crExtensionsDrawLogo(currentWidth, currentHeight);
	glEnable(GL_FOG);

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
	static GLboolean wireframe = GL_FALSE;

	switch (key)
	{
	case 'Q':
	case 'q':
		printf("User has quit. Exiting.\n");
		exit(0);
	case 'W':
	case 'w':
		wireframe = !wireframe;
		if (wireframe)
		{
			printf("Outputting wireframe mode...\n");
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glClearColor(0.2, 0.3, 0.8, 0.0);
		}
		else
		{
			printf("Outputting solid mode...\n");
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
		}
	default:
		break;
	}
	return;
}


static void
Special(int key, int x, int y)
{
	static GLfloat fogDistance = 15;

	switch (key)
	{
	case GLUT_KEY_UP:
		fogDistance += 1;
		if (fogDistance > 25)
			fogDistance = 25;
		else
			printf("Fog Distance: %f\n", fogDistance);
		glFogf(GL_FOG_END, fogDistance);
		break;
	case GLUT_KEY_DOWN:
		fogDistance -= 1;
		if (fogDistance < 3)
			fogDistance = 3;
		else
			printf("Fog Distance: %f\n", fogDistance);
		glFogf(GL_FOG_END, fogDistance);
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
	glutSpecialFunc(Special);
}


static void
InitSpecial(void)
{
#ifdef DISPLAY_LIST
	GLfloat texDetail = 2.0;
	GLint size = 30.0;
#endif
	GLint x, y;

	GLubyte textureData[32 * 32];

	// Create a noise texture (Grass).
	srand(2);
	for (x = 0; x < 32; x++)
	{
		for (y = 0; y < 32; y++)
		{
			textureData[y * 32 + x] = 255 - 32 + (rand() % 32);
		}
	}

	glEnable(GL_FOG);
	glFogfv(GL_FOG_COLOR, bgColor);
	glFogf(GL_FOG_START, 0.0);
	glFogf(GL_FOG_END, 15.0);
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glGetIntegerv((GLenum) GL_FOG_DISTANCE_MODE_NV, &defaultFogDistanceMode);

	switch (defaultFogDistanceMode)
	{
	case GL_EYE_RADIAL_NV:
		printf("FOG_DISTANCE_MODE_NV = EYE_RADIAL_NV\n");
		break;
	case GL_EYE_PLANE:
		printf("FOG_DISTANCE_MODE_NV = EYE_PLANE\n");
		break;
	case GL_EYE_PLANE_ABSOLUTE_NV:
		printf("FOG_DISTANCE_MODE_NV = EYE_PLANE_ABSOLUTE_NV\n");
		break;
	default:
		printf("FOG_DISTANCE_MODE_NV = 0x%x\n", defaultFogDistanceMode);
		break;
	}

	// Create the tile texture.
	glGenTextures(1, texture);
	// Create Trillenar MipMapped Texture
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
									GL_LINEAR_MIPMAP_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_LUMINANCE, 32, 32, GL_LUMINANCE,
										GL_UNSIGNED_BYTE, textureData);

#ifdef DISPLAY_LIST
	// Make the display list for the grass.
	glNewList(1, GL_COMPILE);
	glEnable(GL_TEXTURE_2D);
	glColor3f(0.6, 1.0, 0.2);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	for (y = -size; y < size; y++)
	{
		glBegin(GL_QUAD_STRIP);
		for (x = -size; x <= size; x++)
		{
			glTexCoord2f((x) * texDetail, (y + 1) * texDetail);
			glVertex3f(x, 0, y + 1);
			glTexCoord2f((x) * texDetail, (y + 0) * texDetail);
			glVertex3f(x, 0, y + 0);
		}
		glEnd();
	}
	glDisable(GL_TEXTURE_2D);
	glEndList();
#endif
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
