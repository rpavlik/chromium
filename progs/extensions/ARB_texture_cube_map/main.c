/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*

  exec.cpp

  This is an example of GL_ARB_texture_cube_map, described
  on page 48 of the NVidia OpenGL Extension Specifications.

  Christopher Niederauer, ccn@graphics.stanford.edu, 7/2/2001

*/


#include "../common/logo.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glext.h>

/* #define CCN_DEBUG */
#define DISPLAY_LISTS
#define MULTIPLE_VIEWPORTS

#define TEST_EXTENSION_STRING  "GL_ARB_texture_cube_map"
#ifndef GL_ARB_texture_cube_map
#error Please update your GL/glext.h header file.
#endif


static GLuint currentWidth, currentHeight;
static GLenum envMapMode = (GLenum) GL_REFLECTION_MAP_ARB;
static short object = 1;
static GLuint textureID[1];
static GLfloat bgColor[4] = { 0.2, 0.3, 0.8, 0.0 };

static GLint mX, mY, mButton;
static GLfloat mPitch = 0, mYaw = 0, oPitch = 0, oYaw = 0;


static void
Idle(void)
{
	glutPostRedisplay();
}


static void
Display(void)
{
	const GLfloat skybox = 0.5;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (object == 0)
		mYaw = 0.7 * glutGet((GLenum) GLUT_ELAPSED_TIME) * 0.1;
	else
		mYaw = 0.06 * glutGet((GLenum) GLUT_ELAPSED_TIME) * 0.1;

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glRotatef(mYaw, 0, 1, 0);
	glRotatef(mPitch, 1, 0, 0);
	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	glTranslatef(0, 0, -15);
	glRotatef(oYaw, 0, 1, 0);
	glRotatef(oPitch, 1, 0, 0);
	glColor3f(1, 1, 1);

	glDisable(GL_TEXTURE_2D);
	glEnable((GLenum) GL_TEXTURE_CUBE_MAP_ARB);
	glBindTexture((GLenum) GL_TEXTURE_CUBE_MAP_ARB, textureID[0]);

	glPushMatrix();
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glBegin(GL_QUADS);
	glTexCoord3f(-1, 1, -1);
	glVertex3f(-skybox, skybox, -skybox);
	glTexCoord3f(1, 1, -1);
	glVertex3f(skybox, skybox, -skybox);
	glTexCoord3f(1, -1, -1);
	glVertex3f(skybox, -skybox, -skybox);
	glTexCoord3f(-1, -1, -1);
	glVertex3f(-skybox, -skybox, -skybox);

	glTexCoord3f(-1, 1, -1);
	glVertex3f(-skybox, skybox, -skybox);
	glTexCoord3f(-1, 1, 1);
	glVertex3f(-skybox, skybox, skybox);
	glTexCoord3f(-1, -1, 1);
	glVertex3f(-skybox, -skybox, skybox);
	glTexCoord3f(-1, -1, -1);
	glVertex3f(-skybox, -skybox, -skybox);

	glTexCoord3f(1, 1, -1);
	glVertex3f(skybox, skybox, -skybox);
	glTexCoord3f(1, 1, 1);
	glVertex3f(skybox, skybox, skybox);
	glTexCoord3f(1, -1, 1);
	glVertex3f(skybox, -skybox, skybox);
	glTexCoord3f(1, -1, -1);
	glVertex3f(skybox, -skybox, -skybox);

	glTexCoord3f(-1, -1, 1);
	glVertex3f(-skybox, -skybox, skybox);
	glTexCoord3f(1, -1, 1);
	glVertex3f(skybox, -skybox, skybox);
	glTexCoord3f(1, -1, -1);
	glVertex3f(skybox, -skybox, -skybox);
	glTexCoord3f(-1, -1, -1);
	glVertex3f(-skybox, -skybox, -skybox);

	glTexCoord3f(-1, 1, 1);
	glVertex3f(-skybox, skybox, skybox);
	glTexCoord3f(1, 1, 1);
	glVertex3f(skybox, skybox, skybox);
	glTexCoord3f(1, 1, -1);
	glVertex3f(skybox, skybox, -skybox);
	glTexCoord3f(-1, 1, -1);
	glVertex3f(-skybox, skybox, -skybox);
	glEnd();
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glPopMatrix();

	if (object == 0)
		glCullFace(GL_FRONT);
	else
		glCullFace(GL_BACK);

	glEnable(GL_CULL_FACE);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, envMapMode);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, envMapMode);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, envMapMode);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);
	if (object == 0)
		glutSolidTeapot(3.0);
	else if (object == 1)
		glutSolidSphere(3.0, 32, 20);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
	glDisable(GL_CULL_FACE);

	glDisable((GLenum) GL_TEXTURE_CUBE_MAP_ARB);

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
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
	gluPerspective(40, (GLfloat) currentWidth / currentHeight, 0.05, 30);
/* 	glFrustum( -(GLfloat)currentWidth/currentHeight, (GLfloat)currentWidth/currentHeight, -1.0, 1.0, 0.05, 30.0 ); */
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
	case 'E':
	case 'e':
		envMapMode =
			(envMapMode ==
			 (GLenum) GL_REFLECTION_MAP_ARB) ? (GLenum) GL_NORMAL_MAP_ARB : (GLenum)
			GL_REFLECTION_MAP_ARB;
		break;
	case ' ':
		object++;
		if (object > 1)
			object = 0;
		break;
	default:
		break;
	}
}


static void
Mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		mButton = button;
		mX = x;
		mY = y;
	}
}


static void
Motion(int x, int y)
{
	if (mButton == GLUT_LEFT_BUTTON || mButton == GLUT_MIDDLE_BUTTON)
	{
		mPitch -= y - mY;
		mYaw -= x - mX;
		if (mPitch > 90)
			mPitch = 90;
		if (mPitch < -90)
			mPitch = -90;
	}
	if (mButton == GLUT_RIGHT_BUTTON || mButton == GLUT_MIDDLE_BUTTON)
	{
		oPitch += y - mY;
		oYaw += x - mX;
	}
	mX = x;
	mY = y;
}



static void
InitGL(void)
{
	currentWidth = 320;
	currentHeight = 240;

#ifndef MULTIPLE_VIEWPORTS
	currentWidth <<= 1;
#endif

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(5, 25);
	glutInitWindowSize(currentWidth, currentHeight);
	glutCreateWindow(TEST_EXTENSION_STRING);

	glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	glutIdleFunc(Idle);
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	glutSpecialFunc(NULL);
}


static void
InitSpecial(void)
{
	/* Create the cubemap textures. */
	glGenTextures(1, textureID);

	/* Load the textures. */
	{
		const int texmapX = 128, texmapY = 128, texmapSize = texmapX * texmapY * 3;
		FILE *file;
		int i;
		GLubyte *textureData[6];

		for (i = 0; i < 6; i++)
			textureData[i] = malloc(texmapSize);

		/* PosX: */
		{
			if ((file = fopen("cubemap/posx.raw", "rb")) == NULL)
			{
				printf("Error opening file: cubemap/posx.raw\n");
				file = NULL;
			}
			else
			{
				fread(textureData[0], texmapSize, 1, file);
				fclose(file);
			}
		}
		/* NegX: */
		{
			if ((file = fopen("cubemap/negx.raw", "rb")) == NULL)
			{
				printf("Error opening file: cubemap/negx.raw\n");
				file = NULL;
			}
			else
			{
				fread(textureData[1], texmapSize, 1, file);
				fclose(file);
			}
		}
		/* PosY: */
		{
			if ((file = fopen("cubemap/posy.raw", "rb")) == NULL)
			{
				printf("Error opening file: cubemap/posy.raw\n");
				file = NULL;
			}
			else
			{
				fread(textureData[2], texmapSize, 1, file);
				fclose(file);
			}
		}
		/* NegY: */
		{
			if ((file = fopen("cubemap/negy.raw", "rb")) == NULL)
			{
				printf("Error opening file: cubemap/negy.raw\n");
				file = NULL;
			}
			else
			{
				fread(textureData[3], texmapSize, 1, file);
				fclose(file);
			}
		}
		/* PosZ: */
		{
			if ((file = fopen("cubemap/posz.raw", "rb")) == NULL)
			{
				printf("Error opening file: cubemap/posz.raw\n");
				file = NULL;
			}
			else
			{
				fread(textureData[4], texmapSize, 1, file);
				fclose(file);
			}
		}
		/* NegZ: */
		{
			if ((file = fopen("cubemap/negz.raw", "rb")) == NULL)
			{
				printf("Error opening file: cubemap/negz.raw\n");
				file = NULL;
			}
			else
			{
				fread(textureData[5], texmapSize, 1, file);
				fclose(file);
			}
		}

		/* Create the Cubemap. */
		glBindTexture((GLenum) GL_TEXTURE_CUBE_MAP_ARB, textureID[0]);
		glTexParameteri((GLenum) GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri((GLenum) GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri((GLenum) GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri((GLenum) GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D((GLenum) GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, 0, GL_RGB8,
			     texmapX, texmapY, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData[0]);
		glTexImage2D((GLenum) GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB, 0, GL_RGB8,
			     texmapX, texmapY, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData[1]);
		glTexImage2D((GLenum) GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB, 0, GL_RGB8,
			     texmapX, texmapY, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData[2]);
		glTexImage2D((GLenum) GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB, 0, GL_RGB8,
			     texmapX, texmapY, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData[3]);
		glTexImage2D((GLenum) GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB, 0, GL_RGB8,
			     texmapX, texmapY, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData[4]);
		glTexImage2D((GLenum) GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB, 0, GL_RGB8,
			     texmapX, texmapY, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData[5]);
		for (i = 0; i < 6; i++)
			free(textureData[i]);
	}

	glEnable(GL_DEPTH_TEST);
/* 	glEnable( GL_NORMALIZE ); */
	glCullFace(GL_BACK);
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
