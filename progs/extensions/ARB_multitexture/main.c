/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*

  main.cpp

  This is an example of GL_ARB_multitexture, described on
  page 18 of the NVidia OpenGL Extension Specifications.

  Christopher Niederauer, ccn@graphics.stanford.edu, 5/30/2001

*/


#include "../common/logo.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glut.h>
#include "chromium.h"

/* #define CCN_DEBUG */
#define DISPLAY_LISTS
#define MULTIPLE_VIEWPORTS

#define TEST_EXTENSION_STRING  "GL_ARB_multitexture"
#ifndef GL_ARB_multitexture
#error Please update your GL/glext.h header file.
#endif

#ifndef DISPLAY_LISTS
#error This program requires display lists to do anything :-(
#endif


enum
{
	eGrassTex = 0,
	eSandTex,
	numTextures
};

enum
{
	eGrassList = 1,
	eSandList,
	eShadowList,
	eMultiTexList
};


static GLuint currentWidth, currentHeight;
static GLuint textureID[numTextures];
static GLfloat bgColor[4] = { 0.4f, 0.7f, 1.0f, 0.0f };

/* NVIDIA's gl.h may not include glext.h or defined the PFNGL pointers,
 * so we define our own with unique names.
 */
typedef void (APIENTRY * glActiveTextureARB_t)(GLenum texture);
typedef void (APIENTRY * glMultiTexCoord2fARB_t) (GLenum target, GLfloat s, GLfloat t);

static glActiveTextureARB_t glActiveTextureARB_func;
static glMultiTexCoord2fARB_t glMultiTexCoord2fARB_func;


static void
Idle(void)
{
	glutPostRedisplay();
	return;
}


static void
Redisplay(void)
{
	static double theta = 0.0;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	theta += 0.1;

	glLoadIdentity();
	glTranslatef(0.0, -2.0, 0.0);
	glRotated(30.0, 1.0, 0.0, 0.0);
	glRotated(theta, 0.0, 1.0, 0.0);
	glColor3f(1.0, 1.0, 1.0);

#ifdef MULTIPLE_VIEWPORTS
	/* Left viewport */
	glViewport(0, 0, currentWidth >> 1, currentHeight);
	glEnable(GL_FOG);
	glEnable(GL_TEXTURE_2D);
	glColor3f(1, 1, 1);

	glBindTexture(GL_TEXTURE_2D, textureID[eGrassTex]);
	glCallList(eGrassList);

	glBindTexture(GL_TEXTURE_2D, textureID[eSandTex]);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glEnable(GL_BLEND);
	glPolygonOffset(0, -1);
	glCallList(eSandList);

	glPolygonOffset(0, -2);
	glDisable(GL_TEXTURE_2D);
	glCallList(eShadowList);
	glDisable(GL_POLYGON_OFFSET_FILL);

	glBegin(GL_QUADS);
	glColor4f(0.0f, 0.0f, 0.8f, 0.7f);
	glVertex3f(-50, -7, -50);
	glVertex3f(-50, -7, 50);
	glVertex3f(50, -7, 50);
	glVertex3f(50, -7, -50);
	glEnd();

	glDisable(GL_BLEND);
	glDisable(GL_FOG);
	glDisable(GL_TEXTURE_2D);

	glPushMatrix();
	glLoadIdentity();
	glColor3f(0, 0, 0);
	RenderString(-1.1f, 1, "Multiple Passes (3)");
	glPopMatrix();

	/* Right viewport */
	glViewport(currentWidth >> 1, 0, currentWidth - (currentWidth >> 1),
		   currentHeight);
#endif /* MULTIPLE_VIEWPORTS */

	glEnable(GL_FOG);
	glEnable(GL_TEXTURE_2D);
	glColor3f(1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, textureID[0]);
	glCallList(eMultiTexList);

	glEnable(GL_BLEND);
	glBegin(GL_QUADS);
	glColor4f(0.0f, 0.0f, 0.8f, 0.7f);
	glVertex3f(-50, -7, -50);
	glVertex3f(-50, -7, 50);
	glVertex3f(50, -7, 50);
	glVertex3f(50, -7, -50);
	glEnd();

	glDisable(GL_BLEND);
	glDisable(GL_FOG);
	glDisable(GL_TEXTURE_2D);

	glPushMatrix();
	glLoadIdentity();
	glColor3f(0, 0, 0);
	RenderString(-1.1f, 1, "Single Pass");
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
	glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 40.0);
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

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(5, 25);
	glutInitWindowSize(currentWidth, currentHeight);
	glutCreateWindow(TEST_EXTENSION_STRING);

	glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	glutIdleFunc(Idle);
	glutDisplayFunc(Redisplay);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(NULL);
	glutMotionFunc(NULL);
	glutSpecialFunc(NULL);
}


static void
InitSpecial(void)
{
	GLint numTexUnits, currentActiveUnit;
	GLint x, y;
	GLfloat blackColor[4] = { 0, 0, 0, 0 };
	const GLfloat shade = 1.0, sandAlpha = 0;

	glBindTexture(GL_TEXTURE_2D, textureID[1]);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glEnable(GL_BLEND);

	glPolygonOffset(0, -1);
/* 	glCallList( 2 ); */

	glPolygonOffset(0, -2);
	glDisable(GL_TEXTURE_2D);
/* 	glCallList( 3 ); */
	glDisable(GL_POLYGON_OFFSET_FILL);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glFogfv(GL_FOG_COLOR, bgColor);
	glFogf(GL_FOG_START, 5);
	glFogf(GL_FOG_END, 30);
	glFogf(GL_FOG_MODE, GL_LINEAR);

	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &numTexUnits);	/* Up to 4 */
	glGetIntegerv(GL_ACTIVE_TEXTURE_ARB, &currentActiveUnit);
	printf("MAX_TEXTURE_UNITS_ARB = %d\n", numTexUnits);
	if (numTexUnits < 3)
	{
		printf("Sorry, this program requires at least three texture units to work properly.\n");
		exit(0);
	}
	switch (currentActiveUnit)
	{
	case GL_TEXTURE0_ARB:
		printf("ACTIVE_TEXTURE_ARB = TEXTURE0_ARB\n");
		break;
	case GL_TEXTURE1_ARB:
		printf("ACTIVE_TEXTURE_ARB = TEXTURE1_ARB\n");
		break;
	case GL_TEXTURE2_ARB:
		printf("ACTIVE_TEXTURE_ARB = TEXTURE2_ARB\n");
		break;
	case GL_TEXTURE3_ARB:
		printf("ACTIVE_TEXTURE_ARB = TEXTURE3_ARB\n");
		break;
	default:
		printf("ACTIVE_TETURE_ARB = 0x%x\n", currentActiveUnit);
		break;
	}

	glMultiTexCoord2fARB_func = (glMultiTexCoord2fARB_t) GET_PROC("glMultiTexCoord2fARB");
	glActiveTextureARB_func = (glActiveTextureARB_t) GET_PROC("glActiveTextureARB");
	if (!glMultiTexCoord2fARB_func || !glActiveTextureARB_func)
	{
		printf("Error trying to link to extensions!\n");
		exit(0);
	}
	/* Gets of CURRENT_TEXTURE_COORDS return that of the active unit.
	 * ActiveTextureARB( enum texture ) changes active unit for:
	 *  - Seperate Texture Matrices
	 *  - TexEnv calls
	 *  - Get CURRENT_TEXTURE_COORDS
	 */

	/* Load the textures. */
	glGenTextures(2, textureID);
	{
		const int texmapX = 128,
			texmapY = 128, texmapSize = texmapX * texmapY * 3;
		FILE *file;
		GLubyte *textureData;

		textureData = malloc(texmapSize);

		/* Load grass texture. */
		{
			if ((file = fopen("terrain1.raw", "rb")) == NULL)
			{
				printf("Error opening file: terrain1.raw\n");
				file = NULL;
			}
			else
			{
				fread(textureData, texmapSize, 1, file);
				fclose(file);

				/* Create Trilinear MipMapped Noise Texture */
				glBindTexture(GL_TEXTURE_2D, textureID[eGrassTex]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
						GL_LINEAR_MIPMAP_LINEAR);
				gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, texmapX, texmapY, GL_RGB,
						  GL_UNSIGNED_BYTE, textureData);
			}
		}
		/* Load sand texture. */
		{
			if ((file = fopen("terrain2.raw", "rb")) == NULL)
			{
				printf("Error opening file: terrain2.raw\n");
				file = NULL;
			}
			else
			{
				fread(textureData, texmapSize, 1, file);
				fclose(file);

				/* Create Trilinear MipMapped Circle Texture */
				glBindTexture(GL_TEXTURE_2D, textureID[eSandTex]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
						GL_LINEAR_MIPMAP_LINEAR);
				gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, texmapX, texmapY, GL_RGB,
						  GL_UNSIGNED_BYTE, textureData);
			}
		}
		free(textureData);
	}
	/* Load heightmap data. */
	{
		const int heightmapX = 128,
			heightmapY = 128, heightNormRow = 3 * heightmapX;
		const float size = 0.5f, sizeV = 0.05f, offsetV = -10, texScale = 0.15f;

		FILE *file;
		GLubyte *height;
		GLfloat *normals, vec1[3], vec2[3];

		height = (GLubyte *) malloc(heightmapX * heightmapY);
		normals = (GLfloat *) calloc(heightmapX * heightmapY * 3, sizeof(GLfloat));

		file = fopen("height.raw", "rb");
		if (file == NULL)
		{
			printf("Error opening file: height.raw!  Exiting.\n");
			exit(0);
		}
		fread(height, heightmapX * heightmapY, 1, file);
		fclose(file);

		/* Create the normals. */
		for (y = 0; y < heightmapY; y++)
		{
			for (x = 0; x < heightmapX; x++)
			{
				/* Sets all normals to 0,1,0 by default. */
				normals[y * heightNormRow + x * 3 + 1] = 1.0F;
			}
		}
		for (y = 1; y < heightmapY - 1; y++)
		{
			for (x = 1; x < heightmapX - 1; x++)
			{
				register float i, j, k;
				float magInv;

				vec1[0] = size * 2;
				vec1[1] = sizeV * (height[y * heightmapX + (x + 1)] -
						   height[y * heightmapX + (x - 1)]);
				vec1[2] = 0;

				vec2[0] = 0;
				vec2[1] = sizeV * (height[(y + 1) * heightmapX + x] -
						   height[(y - 1) * heightmapX + x]);
				vec2[2] = size * 2;

				i = -vec1[1] * vec2[2] + vec2[1] * vec1[2];
				j = -vec1[2] * vec2[0] + vec2[2] * vec1[0];
				k = -vec1[0] * vec2[1] + vec2[0] * vec1[1];
				magInv = 1.0 / sqrt(i * i + j * j + k * k);
				normals[y * heightNormRow + x * 3 + 0] = i * magInv;
				normals[y * heightNormRow + x * 3 + 1] = j * magInv;
				normals[y * heightNormRow + x * 3 + 2] = k * magInv;
			}
		}

#ifdef DISPLAY_LISTS
		glNewList(eGrassList, GL_COMPILE);
		for (y = 0; y < heightmapY - 1; y++)
		{
			glBegin(GL_QUAD_STRIP);
			for (x = 0; x < heightmapX; x++)
			{
				glTexCoord2f(x * texScale, y * texScale);
				glVertex3f(size * (x - (heightmapX >> 1)),
					   sizeV * height[y * heightmapX + x] + offsetV,
					   size * (y - (heightmapY >> 1)));

				glTexCoord2f(x * texScale, (y + 1) * texScale);
				glVertex3f(size * (x - (heightmapX >> 1)),
					   sizeV * height[(y + 1) * heightmapX + x] + offsetV,
					   size * (y + 1 - (heightmapY >> 1)));
			}
			glEnd();
		}
		glEndList();
		glNewList(eSandList, GL_COMPILE);
		for (y = 0; y < heightmapY - 1; y++)
		{
			glBegin(GL_QUAD_STRIP);
			for (x = 0; x < heightmapX; x++)
			{
				GLfloat normalThreshold1 =
					(normals[y * heightNormRow + x * 3 + 1] - 0.5) * 1.8,
					normalThreshold2 =
					(normals[(y + 1) * heightNormRow + x * 3 + 1] - 0.5) * 1.8;

				glColor4f(1, 1, 1,
					  1.0 - (normalThreshold1 > 1.0 ? 1 :
						 normalThreshold1 < 0 ? 0 : normalThreshold1));
				glTexCoord2f(x * texScale, y * texScale);
				glVertex3f(size * (x - (heightmapX >> 1)),
					   sizeV * height[y * heightmapX + x] + offsetV,
					   size * (y - (heightmapY >> 1)));

				glColor4f(1, 1, 1,
					  1.0 - (normalThreshold2 > 1.0 ? 1 :
						 normalThreshold2 < 0 ? 0 : normalThreshold2));
				glTexCoord2f(x * texScale, (y + 1) * texScale);
				glVertex3f(size * (x - (heightmapX >> 1)),
					   sizeV * height[(y + 1) * heightmapX + x] + offsetV,
					   size * (y + 1 - (heightmapY >> 1)));
			}
			glEnd();
		}
		glEndList();
		glNewList(eShadowList, GL_COMPILE);
		for (y = 0; y < heightmapY - 1; y++)
		{
			glBegin(GL_QUAD_STRIP);
			for (x = 0; x < heightmapX; x++)
			{
				glColor4f(0, 0, 0, (normals[y * heightNormRow + x * 3 + 2]));
				glVertex3f(size * (x - (heightmapX >> 1)),
					   sizeV * height[y * heightmapX + x] + offsetV,
					   size * (y - (heightmapY >> 1)));

				glColor4f(0, 0, 0, (normals[(y + 1) * heightNormRow + x * 3 + 2]));
				glVertex3f(size * (x - (heightmapX >> 1)),
					   sizeV * height[(y + 1) * heightmapX + x] + offsetV,
					   size * (y + 1 - (heightmapY >> 1)));
			}
			glEnd();
		}
		glEndList();

		/* Multitexturing */
		glNewList(eMultiTexList, GL_COMPILE);

		glActiveTextureARB_func(GL_TEXTURE0_ARB);
		glBindTexture(GL_TEXTURE_2D, textureID[eGrassTex]);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		glActiveTextureARB_func(GL_TEXTURE1_ARB);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textureID[eSandTex]);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		glActiveTextureARB_func(GL_TEXTURE2_ARB);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, blackColor);

		for (y = 0; y < heightmapY - 1; y++)
		{
			glBegin(GL_QUAD_STRIP);
			for (x = 0; x < heightmapX; x++)
			{
				glColor4f(shade, shade, shade, sandAlpha);
				glMultiTexCoord2fARB_func(GL_TEXTURE0_ARB, x * texScale, y * texScale);
				glMultiTexCoord2fARB_func(GL_TEXTURE1_ARB, x * texScale, y * texScale);
/* 				glMultiTexCoord2fARB_func( GL_TEXTURE2_ARB, x*texScale, y*texScale ); */
				glVertex3f(size * (x - (heightmapX >> 1)),
					   sizeV * height[y * heightmapX + x] + offsetV,
					   size * (y - (heightmapY >> 1)));

				glColor4f(shade, shade, shade, sandAlpha);
				glMultiTexCoord2fARB_func(GL_TEXTURE0_ARB, x * texScale,
						     (y + 1) * texScale);
				glMultiTexCoord2fARB_func(GL_TEXTURE1_ARB, x * texScale,
						     (y + 1) * texScale);
/* 				glMultiTexCoord2fARB_func( GL_TEXTURE2_ARB, x*texScale, (y+1)*texScale ); */
				glVertex3f(size * (x - (heightmapX >> 1)),
					   sizeV * height[(y + 1) * heightmapX + x] + offsetV,
					   size * (y + 1 - (heightmapY >> 1)));
			}
			glEnd();
		}
		glActiveTextureARB_func(GL_TEXTURE1_ARB);
		glDisable(GL_TEXTURE_2D);
		glActiveTextureARB_func(GL_TEXTURE0_ARB);
		glEndList();
#endif /* DISPLAY_LISTS */

		free(normals);
		free(height);
	}

	return;
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
