/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*

  main.cpp

  This is an example of GL_EXT_separate_specular_color.

  Christopher Niederauer, ccn@graphics.stanford.edu, 5/30/2001

*/


#include "../common/logo.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glext.h>

#define TEST_EXTENSION_STRING  "GL_EXT_separate_specular_color"
#ifndef GL_EXT_separate_specular_color
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

static GLuint currentWidth, currentHeight;
static GLuint textureID, sphereList;
static GLfloat bgColor[4] = { 0.1f, 0.2f, 0.4f, 0.0f };


static void
Idle(void)
{
	glutPostRedisplay();
	return;
}


static void
Display(void)
{
	static float theta;

	glClear(GL_COLOR_BUFFER_BIT);

	glLoadIdentity();
	glTranslatef(0, 0, -4);
	glRotatef(theta, 0, 1, 0);
	glRotatef(90, 1, 0, 0);

	theta += 0.1f;
	if (theta > 90)
		theta -= 90;

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glScalef(16, 8, 1);
	glMatrixMode(GL_MODELVIEW);

#ifdef MULTIPLE_VIEWPORTS
	/* Left Viewport */
	glViewport(0, 0, currentWidth >> 1, currentHeight);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glEnable(GL_LIGHTING);
	glLightModelf((GLenum) GL_LIGHT_MODEL_COLOR_CONTROL_EXT,
		      GL_SINGLE_COLOR_EXT);
	glCallList(sphereList);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	glPushMatrix();
	glLoadIdentity();
	RenderString(-1.1f, 1, "SINGLE_COLOR_EXT");
	glPopMatrix();

	/* Upper Right Viewport */
	glViewport(currentWidth >> 1, 0, currentWidth >> 1, currentHeight);
#endif /* MULTIPLE_VIEWPORTS */

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glEnable(GL_LIGHTING);
	glLightModelf((GLenum) GL_LIGHT_MODEL_COLOR_CONTROL_EXT,
		      GL_SEPARATE_SPECULAR_COLOR_EXT);
	glCallList(sphereList);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	glPushMatrix();
	glLoadIdentity();
	RenderString(-1.1f, 1, "SEPARATE_SPECULAR_COLOR_EXT");
	glPopMatrix();

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glViewport(0, 0, currentWidth, currentHeight);
	crExtensionsDrawLogo(currentWidth, currentHeight);
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
	glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 10.0);
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
	currentWidth = 240;
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
	const int texWidth = 16, texHeight = 16;
	GLfloat position[4] = { 1, 1, 0, 1 };
	GLfloat specular[4] = {	1, 1, 1, 1 };
	GLubyte textureData[16 * 16];
	GLUquadricObj *q;
	int x, y;

	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 40);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glColor3f(1, 1, 1);
	glEnable(GL_LIGHT0);

	q = gluNewQuadric();
	gluQuadricDrawStyle(q, (GLenum) GLU_FILL);
	gluQuadricNormals(q, (GLenum) GLU_SMOOTH);
	gluQuadricTexture(q, GL_TRUE);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	sphereList = glGenLists(1);
	glNewList(sphereList, GL_COMPILE);
	gluSphere(q, 2.5, 128, 64);
	glEndList();

	for (x = 0; x < texWidth; x++)
	{
		for (y = 0; y < texHeight; y++)
		{
			if ((x < (texWidth >> 1) && y < (texHeight >> 1)) ||
			    (x >= (texWidth >> 1) && y >= (texHeight >> 1)))
				textureData[y * texWidth + x] = (GLubyte) 255;
			else
				textureData[y * texWidth + x] = (GLubyte) 0;
		}
	}

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 16, 16, 0, GL_LUMINANCE,
		     GL_UNSIGNED_BYTE, textureData);

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
}


int
main(int argc, char *argv[])
{
	glutInit(&argc, argv);

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
