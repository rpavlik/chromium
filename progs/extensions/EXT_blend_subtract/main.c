/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*

  main.cpp

  This is an example of GL_EXT_blend_subtract.

  Christopher Niederauer, ccn@graphics.stanford.edu, 6/25/2001

*/

#include "../common/logo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glext.h>

#define TEST_EXTENSION_STRING  "GL_EXT_blend_subtract"
#ifndef GL_EXT_blend_subtract
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
#define DISPLAY_LISTS
#define MULTIPLE_VIEWPORTS

#ifndef APIENTRY
#define APIENTRY
#endif

typedef void (APIENTRY * GLBLENDEQUATIONEXTPROC) (GLenum mode);

static GLBLENDEQUATIONEXTPROC glBlendEquation_ext;
static GLuint currentWidth, currentHeight;
static GLfloat bgColor[4] = { 0, 0, 1, 0.0 };


static void
Idle(void)
{
	glutPostRedisplay();
	return;
}


static void
Display(void)
{
	const float size = 1.0;
	static double theta = 0.0;

	glClear(GL_COLOR_BUFFER_BIT);

	theta += 0.05;

	glLoadIdentity();
	glRotated(90, 1, 0, 0);
	glTranslatef(0, -2, 0);
	glRotated(theta, 0, 1, 0);
	glColor3f(1, 1, 1);

#ifdef MULTIPLE_VIEWPORTS

	/* Left Viewport */
	glViewport(0, 0, currentWidth >> 1, currentHeight);

	glPushMatrix();
	glLoadIdentity();
	glRotated(90, 1, 0, 0);
	glTranslatef(0, -2.1, 0);
	glBegin(GL_QUADS);
	glColor3f(0, 1, 0);
	glVertex3f(-size, 0.0, size);
	glVertex3f(size, 0.0, size);
	glColor3f(1, 0, 0);
	glVertex3f(size, 0.0, -size);
	glVertex3f(-size, 0.0, -size);
	glEnd();
	glPopMatrix();

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0, -1);
	glBlendEquation_ext(GL_FUNC_SUBTRACT_EXT);
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_BLEND);
	glBegin(GL_QUADS);
	glColor3f(0, 0, 0);
	glVertex3f(-size, 0.0, size);
	glVertex3f(size, 0.0, size);

	glColor3f(1, 1, 1);
	glVertex3f(size, 0.0, -size);
	glVertex3f(-size, 0.0, -size);
	glEnd();
	glDisable(GL_BLEND);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glBlendEquation_ext(GL_FUNC_ADD_EXT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor3f(1, 1, 1);
	glPushMatrix();
	glLoadIdentity();
	RenderString(-1.1, 1, "FUNC_SUBTRACT_EXT");
	glPopMatrix();

	/* Upper Right Viewport */
	glViewport(currentWidth >> 1, 0, currentWidth >> 1, currentHeight);
#endif
	glPushMatrix();
	glLoadIdentity();
	glRotated(90, 1, 0, 0);
	glTranslatef(0, -2.1, 0);
	glBegin(GL_QUADS);
	glColor3f(0, 1, 0);
	glVertex3f(-size, 0.0, size);
	glVertex3f(size, 0.0, size);
	glColor3f(1, 0, 0);
	glVertex3f(size, 0.0, -size);
	glVertex3f(-size, 0.0, -size);
	glEnd();
	glPopMatrix();

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0, -1);
	glBlendEquation_ext(GL_FUNC_REVERSE_SUBTRACT_EXT);
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_BLEND);
	glBegin(GL_QUADS);
	glColor3f(0, 0, 0);
	glVertex3f(-size, 0.0, size);
	glVertex3f(size, 0.0, size);

	glColor3f(1, 1, 1);
	glVertex3f(size, 0.0, -size);
	glVertex3f(-size, 0.0, -size);
	glEnd();
	glDisable(GL_BLEND);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glBlendEquation_ext(GL_FUNC_ADD_EXT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor3f(1, 1, 1);
	glPushMatrix();
	glLoadIdentity();
	RenderString(-1.1, 1, "FUNC_REVERSE_SUBTRACT_EXT");
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
	glutMouseFunc(NULL);
	glutMotionFunc(NULL);
	glutSpecialFunc(NULL);
}


static void
InitSpecial(void)
{
#ifdef WIN32
	glBlendEquation_ext =
		(GLBLENDEQUATIONEXTPROC) wglGetProcAddress("glBlendEquationEXT");
#elif defined(IRIX) || defined (SunOS)
	glBlendEquation_ext = glBlendEquationEXT;
#else
	glBlendEquation_ext =
		(GLBLENDEQUATIONEXTPROC) glXGetProcAddressARB((const GLubyte *)
							      "glBlendEquationEXT");
#endif
	if (glBlendEquation_ext == NULL)
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
