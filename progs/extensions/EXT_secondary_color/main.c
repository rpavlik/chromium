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
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

/* Ugly hack to work around problem with NVIDIA's gl.h file.
 * NVIDIA's gl.h doesn't define function pointers like
 * PFNGLSECONDARYCOLOR3FEXTPROC.  By undef'ing GL_EXT_secondary_color
 * we're sure to pick it up from glext.h.
 */
#ifndef GL_GLEXT_VERSION  /* glext.h wasn't already included */
#undef GL_EXT_secondary_color
#endif
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
#endif /* GLX_ARB_get_proc_address */

#define CCN_DEBUG
#define MULTIPLE_VIEWPORTS

#ifndef APIENTRY
#define APIENTRY
#endif

typedef void (APIENTRY * glSecondaryColor3fEXT_t) (GLfloat red, GLfloat green, GLfloat blue);
typedef void (APIENTRY * glSecondaryColorPointerEXT_t) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);

static glSecondaryColorPointerEXT_t glSecondaryColorPointerEXTptr;
static glSecondaryColor3fEXT_t glSecondaryColor3fEXTptr;

static GLuint currentWidth, currentHeight;
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
#define size 1.3
	const int stride = sizeof(GLfloat) * (2 + 3 + 3);
	static float theta;
	static GLfloat vertexArray[] = {
		/* Vertex2f */
		/* Color3f */
		/* SecondaryColor3fEXT */

		(float)-size, (float)-size,
		1.0, 1.0, 1.0,
		1.0, 0.0, 1.0,

		(float)-size, (float)size,
		1.0, 1.0, 1.0,
		1.0, 0.0, 0.0,

		(float)size, (float)size,
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0,

		(float)size, (float)-size,
		0.0, 0.0, 0.0,
		0.0, 0.0, 1.0
	};

	glClear(GL_COLOR_BUFFER_BIT);

	glLoadIdentity();
	glTranslatef(0, 0, -2);
	glRotatef(theta, 0, 0, 1);

	theta += 0.01f;

#ifdef MULTIPLE_VIEWPORTS
	/* Left Viewport */
	glViewport(0, 0, currentWidth >> 1, currentHeight);

	glEnable((GLenum) GL_COLOR_SUM_EXT);
	glBegin(GL_QUADS);
	{
		glColor3f(1, 1, 1);
		glSecondaryColor3fEXTptr(1, 0, 1);
		glVertex2f((float)-size, (float)-size);

		glSecondaryColor3fEXTptr(1, 0, 0);
		glVertex2f((float)-size, (float)size);

		glColor3f(0, 0, 0);
		glSecondaryColor3fEXTptr(0, 1, 0);
		glVertex2f((float)size, (float)size);

		glSecondaryColor3fEXTptr(0, 0, 1);
		glVertex2f((float)size, (float)-size);
	}
	glEnd();
	glDisable((GLenum) GL_COLOR_SUM_EXT);

	glColor3f(1, 1, 1);
	glPushMatrix();
	glLoadIdentity();
	RenderString(-1.1f, 1, "SecondaryColor3fEXT");
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
	glSecondaryColorPointerEXTptr(3, GL_FLOAT, stride, vertexArray + 5);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState((GLenum) GL_SECONDARY_COLOR_ARRAY_EXT);
	glDisable((GLenum) GL_COLOR_SUM_EXT);

	glColor3f(1, 1, 1);
	glPushMatrix();
	glLoadIdentity();
	RenderString(-1.1f, 1, "SecondaryColorPointerEXT");
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
	glutMouseFunc(NULL);
	glutMotionFunc(NULL);
	glutSpecialFunc(NULL);
}


static void
InitSpecial(void)
{
#ifdef WIN32
	glSecondaryColorPointerEXTptr =
		(glSecondaryColorPointerEXT_t)
		wglGetProcAddress("glSecondaryColorPointerEXT");
	glSecondaryColor3fEXTptr =
		(glSecondaryColor3fEXT_t) wglGetProcAddress("glSecondaryColor3fEXT");
#else
	glSecondaryColorPointerEXTptr =
		(glSecondaryColorPointerEXT_t)
		glXGetProcAddressARB((const GLubyte *) "glSecondaryColorPointerEXT");
	glSecondaryColor3fEXTptr =
		(glSecondaryColor3fEXT_t) glXGetProcAddressARB((const GLubyte *)
								    "glSecondaryColor3fEXT");
#endif
	if (!glSecondaryColor3fEXTptr || !glSecondaryColorPointerEXTptr)
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
