/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*

  main.

  This is an example of using GL_ARB_multitexture.

  Christopher Niederauer, ccn@graphics.stanford.edu, 5/30/2001

*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glext.h>
#include "../common/logo.h"

/*#define	CCN_DEBUG */
#define	DISPLAY_LISTS
#define	MULTIPLE_VIEWPORTS

#define	TEST_EXTENSION_STRING	"GL_ARB_multitexture"
#ifndef	GL_ARB_multitexture
#error	Please update your GL/glext.h header file.
#endif

enum
{
	eBrickTex = 0,
	eLightTex,
	eNumTextures
};


static GLuint currentWidth, currentHeight;
static GLuint textureID[eNumTextures];
static GLfloat bgColor[4] = { 0.1, 0.1, 0.1, 0.0 };

#ifdef WINDOWS
PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB;
PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
#endif

#ifdef IRIX
void
glActiveTextureARB(GLenum texture)
{
}

void
glMultiTexCoord2fARB(GLenum texture, GLfloat s, GLfloat t)
{
}
#endif


static void
Display(void)
{
	const float size = 1.0, texDetail = 0.5, texOffset = 0.5;
	static double theta = 0.0;

	glClear(GL_COLOR_BUFFER_BIT);

	/* theta += 0.05; */
	theta = glutGet((GLenum) GLUT_ELAPSED_TIME) / 100.0;

	glLoadIdentity();
	glRotated(90.0, 1.0, 0.0, 0.0);
	glTranslatef(0.0, -2.0, .0);
	glRotated(theta, 0.0, -1.0, 0.0);

#ifdef MULTIPLE_VIEWPORTS
	/* Left Viewport */
	glViewport(0, 0, currentWidth >> 1, currentHeight);

	glEnable(GL_TEXTURE_2D);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D, textureID[eBrickTex]);
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture(GL_TEXTURE_2D, textureID[eLightTex]);

	glBegin(GL_QUADS);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, -texDetail + texOffset,
											 texDetail + texOffset);
	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, -texDetail + texOffset,
											 texDetail + texOffset);
	glVertex3f(-size, 0.0, size);

	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, texDetail + texOffset,
											 texDetail + texOffset);
	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, texDetail + texOffset,
											 texDetail + texOffset);
	glVertex3f(size, 0.0, size);

	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, texDetail + texOffset,
											 -texDetail + texOffset);
	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, texDetail + texOffset,
											 -texDetail + texOffset);
	glVertex3f(size, 0.0, -size);

	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, -texDetail + texOffset,
											 -texDetail + texOffset);
	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, -texDetail + texOffset,
											 -texDetail + texOffset);
	glVertex3f(-size, 0.0, -size);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glDisable(GL_TEXTURE_2D);

	glColor3f(1, 1, 1);
	glPushMatrix();
	glLoadIdentity();
	RenderString(-1.1, 1, "GL_MODULATE");
	glPopMatrix();

	/* Upper Right Viewport */
	glViewport(currentWidth >> 1, 0, currentWidth >> 1, currentHeight);
#endif
	glEnable(GL_TEXTURE_2D);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D, textureID[eBrickTex]);
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
	glBindTexture(GL_TEXTURE_2D, textureID[eLightTex]);

	glBegin(GL_QUADS);
	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, -texDetail + texOffset,
											 texDetail + texOffset);
	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, -texDetail + texOffset,
											 texDetail + texOffset);
	glVertex3f(-size, 0.0, size);

	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, texDetail + texOffset,
											 texDetail + texOffset);
	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, texDetail + texOffset,
											 texDetail + texOffset);
	glVertex3f(size, 0.0, size);

	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, texDetail + texOffset,
											 -texDetail + texOffset);
	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, texDetail + texOffset,
											 -texDetail + texOffset);
	glVertex3f(size, 0.0, -size);

	glMultiTexCoord2fARB(GL_TEXTURE0_ARB, -texDetail + texOffset,
											 -texDetail + texOffset);
	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, -texDetail + texOffset,
											 -texDetail + texOffset);
	glVertex3f(-size, 0.0, -size);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisable(GL_TEXTURE_2D);

	glColor3f(1, 1, 1);
	glPushMatrix();
	glLoadIdentity();
	RenderString(-1.1, 1, "GL_ADD");
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
Idle(void)
{
	glutPostRedisplay();
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
	glutMouseFunc(NULL);
	glutMotionFunc(NULL);
	glutSpecialFunc(NULL);
}


static void
InitSpecial(void)
{
	GLint numTexUnits, currentActiveUnit, maxTextureSize;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glFogfv(GL_FOG_COLOR, bgColor);
	glFogf(GL_FOG_START, 5);
	glFogf(GL_FOG_END, 30);
	glFogf(GL_FOG_MODE, GL_LINEAR);

	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &numTexUnits);	/* Up to 4 */
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	glGetIntegerv(GL_ACTIVE_TEXTURE_ARB, &currentActiveUnit);
	printf("MAX_TEXTURE_SIZE = %d\n", maxTextureSize);
	printf("MAX_TEXTURE_UNITS_ARB = %d\n", numTexUnits);
	printf("EXTENSIONS: %s\n", (char *) glGetString(GL_EXTENSIONS));
	if (numTexUnits < 2)
	{
		printf
			("Sorry, this program requires at least two texture units to work properly.\n");
		exit(0);
	}
	switch (currentActiveUnit)
	{
	case GL_TEXTURE0_ARB:
		printf("ACTIVE_TEXTURE_ARB = GL_TEXTURE0_ARB\n");
		break;
	case GL_TEXTURE1_ARB:
		printf("ACTIVE_TEXTURE_ARB = GL_TEXTURE1_ARB\n");
		break;
	case GL_TEXTURE2_ARB:
		printf("ACTIVE_TEXTURE_ARB = GL_TEXTURE2_ARB\n");
		break;
	case GL_TEXTURE3_ARB:
		printf("ACTIVE_TEXTURE_ARB = GL_TEXTURE3_ARB\n");
		break;
	default:
		printf("ACTIVE_TEXTURE_ARB = 0x%x\n", currentActiveUnit);
		break;
	}

#ifdef WIN32
	glMultiTexCoord2fARB =
		(PFNGLMULTITEXCOORD2FARBPROC) wglGetProcAddress("glMultiTexCoord2fARB");
	glActiveTextureARB =
		(PFNGLACTIVETEXTUREARBPROC) wglGetProcAddress("glActiveTextureARB");
	if (!glMultiTexCoord2fARB || !glActiveTextureARB)
	{
		printf("getProcAddress() failed\n");
		exit(0);
	}
#endif
	/* Gets of CURRENT_TEXTURE_COORDS return that of the active unit.
	 * ActiveTextureARB( enum texture ) changes active unit for:
	 *  - Seperate Texture Matrices
	 *  - TexEnv calls
	 *  - Get CURRENT_TEXTURE_COORDS
	 */
	/* Create the tile texture. */
	glGenTextures(eNumTextures, textureID);

	/* Load the textures. */
	{
		const int texmapX = 128,
			texmapY = 128, texmapSize = texmapX * texmapY * 3;
		FILE *file;
		GLubyte textureData[texmapSize];

		/* Load brick texture. */
		if ((file = fopen("brick.raw", "rb")) == NULL)
		{
			printf("Error opening file: brick.raw\n");
			exit(0);
		}
		else
		{
			fread(textureData, texmapSize, 1, file);
			fclose(file);

			/* Create Trilinear MipMapped Noise Texture */
			glBindTexture(GL_TEXTURE_2D, textureID[eBrickTex]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
											GL_LINEAR_MIPMAP_LINEAR);
			gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, texmapX, texmapY, GL_RGB,
												GL_UNSIGNED_BYTE, textureData);
		}

		/* Load light texture. */
		if ((file = fopen("light.raw", "rb")) == NULL)
		{
			printf("Error opening file: light.raw\n");
			exit(0);
		}
		else
		{
			fread(textureData, texmapSize / 3, 1, file);
			fclose(file);

			/* Create Trilinear MipMapped Circle Texture */
			glBindTexture(GL_TEXTURE_2D, textureID[eLightTex]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
											GL_LINEAR_MIPMAP_LINEAR);
			gluBuild2DMipmaps(GL_TEXTURE_2D, GL_LUMINANCE, texmapX, texmapY,
												GL_LUMINANCE, GL_UNSIGNED_BYTE, textureData);
		}
	}
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
