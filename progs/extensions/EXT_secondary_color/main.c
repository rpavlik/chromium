/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*

  main.cpp

  This is an example of GL_EXT_secondary_color.

  Christopher Niederauer, ccn@graphics.stanford.edu, 5/30/2001

*/


#include "../common/logo.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glext.h>

#define TEST_EXTENSION_STRING  "GL_EXT_secondary_color"
#ifndef GL_EXT_secondary_color
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

#define CCN_DEBUG
#define MULTIPLE_VIEWPORTS

static PFNGLSECONDARYCOLORPOINTEREXTPROC glSecondaryColorPointerEXT;
static PFNGLSECONDARYCOLOR3FEXTPROC glSecondaryColor3fEXT;

static GLuint currentWidth, currentHeight;
static GLfloat bgColor[4] = { 0.1, 0.2, 0.4, 0.0 };


static void
Idle(void)
{
	glutPostRedisplay();
	return;
}


static void
Display(void)
{
#define size 1.3
	const int stride = sizeof(GLfloat) * (2 + 3 + 3);
	static float theta;
	static GLfloat vertexArray[] = {
		/* Vertex2f */
		/* Color3f */
		/* SecondaryColor3fEXT */

		-size, -size,
		1.0, 1.0, 1.0,
		1.0, 0.0, 1.0,

		-size, size,
		1.0, 1.0, 1.0,
		1.0, 0.0, 0.0,

		size, size,
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0,

		size, -size,
		0.0, 0.0, 0.0,
		0.0, 0.0, 1.0
	};

	glClear(GL_COLOR_BUFFER_BIT);

	glLoadIdentity();
	glTranslatef(0, 0, -2);
	glRotatef(theta, 0, 0, 1);

	theta += 0.01;

#ifdef MULTIPLE_VIEWPORTS
	/* Left Viewport */
	glViewport(0, 0, currentWidth >> 1, currentHeight);

	glEnable((GLenum) GL_COLOR_SUM_EXT);
	glBegin(GL_QUADS);
	{
		glColor3f(1, 1, 1);
		glSecondaryColor3fEXT(1, 0, 1);
		glVertex2f(-size, -size);

		glSecondaryColor3fEXT(1, 0, 0);
		glVertex2f(-size, size);

		glColor3f(0, 0, 0);
		glSecondaryColor3fEXT(0, 1, 0);
		glVertex2f(size, size);

		glSecondaryColor3fEXT(0, 0, 1);
		glVertex2f(size, -size);
	}
	glEnd();
	glDisable((GLenum) GL_COLOR_SUM_EXT);

	glColor3f(1, 1, 1);
	glPushMatrix();
	glLoadIdentity();
	RenderString(-1.1, 1, "SecondaryColor3fEXT");
	glPopMatrix();

	/* Upper Right Viewport */
	glViewport(currentWidth >> 1, 0, currentWidth >> 1, currentHeight);
#endif /* MULTIPLE_VIEWPORTS */

	glEnable((GLenum) GL_COLOR_SUM_EXT);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState((GLenum) GL_SECONDARY_COLOR_ARRAY_EXT);
	glVertexPointer(2, GL_FLOAT, stride, vertexArray);
	glColorPointer(3, GL_FLOAT, stride, vertexArray + 2);
	glSecondaryColorPointerEXT(3, GL_FLOAT, stride, vertexArray + 5);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState((GLenum) GL_SECONDARY_COLOR_ARRAY_EXT);
	glDisable((GLenum) GL_COLOR_SUM_EXT);

	glColor3f(1, 1, 1);
	glPushMatrix();
	glLoadIdentity();
	RenderString(-1.1, 1, "SecondaryColorPointerEXT");
	glPopMatrix();

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





void
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
	glutMouseFunc(NULL);
	glutMotionFunc(NULL);
	glutSpecialFunc(NULL);
}


void
InitSpecial(void)
{
#ifdef WIN32
	glSecondaryColorPointerEXT =
		(PFNGLSECONDARYCOLORPOINTEREXTPROC)
		wglGetProcAddress("glSecondaryColorPointerEXT");
	glSecondaryColor3fEXT =
		(PFNGLSECONDARYCOLOR3FEXTPROC) wglGetProcAddress("glSecondaryColor3fEXT");
#else
	glSecondaryColorPointerEXT =
		(PFNGLSECONDARYCOLORPOINTEREXTPROC)
		glXGetProcAddressARB((const GLubyte *) "glSecondaryColorPointerEXT");
	glSecondaryColor3fEXT =
		(PFNGLSECONDARYCOLOR3FEXTPROC) glXGetProcAddressARB((const GLubyte *)
								    "glSecondaryColor3fEXT");
#endif
	if (!glSecondaryColor3fEXT || !glSecondaryColorPointerEXT)
	{
		printf("Error trying to link to extensions!\n");
		exit(0);
	}
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
