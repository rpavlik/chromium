/*
 * Procedurally-generated cityscape.
 * This is a _really_ simplistic demo somewhat inspired by the
 * paper "Procedural Modeling of Cities" by Parish and Muller as
 * seen in the SIGGRAPH 2001 conference proceedings.
 * http://graphics.ethz.ch/Downloads/Publications/Papers/2001/p_Par01.pdf
 *
 * Possible improvements:
 *   - texturing
 *   - don't place buildings on the steets
 *   - skybox
 *
 * Copyright (C) 2002  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#define USE_CHROMIUM 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <memory.h>
#if USE_CHROMIUM
#include "chromium.h"
#include "multiview.h"
#endif
#include <GL/glut.h>

#ifdef WINDOWS
#pragma warning(disable : 4125)
#endif

#include "wall2.h"
#include "roof.h"

unsigned char *wall;

struct building
{
	float x, y, z; /* pos */
	float sx, sy, sz; /* size */
	float color[4];
	GLuint dlist;
};

#define WALL_WIDTH 128
#define WALL_HEIGHT 256
#define WALL_BYTES_PER_PIXEL 4

#define MAX_BUILDINGS 300

static struct building Buildings[MAX_BUILDINGS];
static GLuint NumBuildings = 150;

static GLint WinWidth = 400, WinHeight = 250;
static GLfloat Xrot = 0, Yrot = 0, Zrot = 0;
static GLfloat EyeDist = 0, EyeHeight = 0;
static GLboolean Anim = GL_TRUE;
static int StartTime = 0;
static GLfloat StartRot = 0;
static GLboolean ShowOverlay = GL_FALSE;

static GLint CheckerRows = 20, CheckerCols = 20;
static GLboolean UseDisplayLists = GL_FALSE;
static GLboolean Texture = GL_TRUE;
static GLuint GroundList = 1;
static GLuint wallTex;
static GLuint roofTex;

static GLboolean CaveMode = GL_FALSE;
static GLuint NumViews = 2, NumTilesPerView = 1;
static GLfloat EyeX = 0, EyeY = 0, EyeZ = 0;
#if USE_CHROMIUM
glChromiumParametervCRProc glChromiumParametervCR_ptr;
#endif



static GLfloat
Random(GLfloat min, GLfloat max)
{
	GLfloat x = (float) rand() / (float) RAND_MAX;
	return x * (max - min) + min;
}


static void
DrawBuilding(const struct building *b)
{
	GLfloat hc, xwc, zwc;

	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, b->color);

	glPushMatrix();
	glTranslatef(b->x, b->y, b->z);
	glScalef(b->sx, b->sy, b->sz);

	hc = floor(2.0 * b->sy);
	xwc = floor(5.0 * b->sx);
	zwc = floor(5.0 * b->sz);

	if (hc < 1.0)
		hc = 1.0;
	if (xwc < 1.0)
		xwc = 1.0;
	if (zwc < 1.0)
		zwc = 1.0;

	glBindTexture(GL_TEXTURE_2D, wallTex);

	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-0.5, -0.5, 0.5);
	glTexCoord2f(xwc, 0.0);
	glVertex3f(0.5, -0.5, 0.5);
	glTexCoord2f(xwc, hc);
	glVertex3f(0.5, 0.5, 0.5);
	glTexCoord2f(0.0, hc);
	glVertex3f(-0.5, 0.5, 0.5);

	glNormal3f(1.0, 0.0, 0.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(0.5, -0.5, 0.5);
	glTexCoord2f(zwc, 0.0);
	glVertex3f(0.5, -0.5, -0.5);
	glTexCoord2f(zwc, hc);
	glVertex3f(0.5, 0.5, -0.5);
	glTexCoord2f(0.0, hc);
	glVertex3f(0.5, 0.5, 0.5);

	glNormal3f(0.0, 0.0, -1.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(0.5, -0.5, -0.5);
	glTexCoord2f(xwc, 0.0);
	glVertex3f(-0.5, -0.5, -0.5);
	glTexCoord2f(xwc, hc);
	glVertex3f(-0.5, 0.5, -0.5);
	glTexCoord2f(0.0, hc);
	glVertex3f(0.5, 0.5, -0.5);

	glNormal3f(-1.0, 0.0, 0.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-0.5, 0.5, -0.5);
	glTexCoord2f(0.0, hc);
	glVertex3f(-0.5, -0.5, -0.5);
	glTexCoord2f(zwc, hc);
	glVertex3f(-0.5, -0.5, 0.5);
	glTexCoord2f(zwc, 0.0);
	glVertex3f(-0.5, 0.5, 0.5);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, roofTex);

	glBegin(GL_QUADS);
	glNormal3f(0.0, 1.0, 0.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-0.5, 0.5, -0.5);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-0.5, 0.5, 0.5);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(0.5, 0.5, 0.5);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(0.5, 0.5, -0.5);
	glEnd();

	glPopMatrix();
}


static void
GenerateBuildings(void)
{
	int i;
	for (i = 0; i < MAX_BUILDINGS; i++)
	{
		struct building *b = Buildings + i;
		GLfloat atten;
		GLfloat g;

		/* x/y position and size */
		b->x = Random(-14, 14);
		b->z = Random(-14, 14);
		b->sx = Random(0.5, 2);
		b->sz = Random(0.5, 2);
		/* make buildings in middle generally taller than outlying buildings */
#if 1
		atten = 0.2 + cos(b->x / 12) * cos(b->z / 12);
#else
		atten = 0.2 + cos(b->x + b->z / 100);
#endif
		atten = atten * atten;
		/* height */
		b->sy = Random(0.2f, 8) * atten;
		b->y = 0.5 * b->sy;
		g = Random(0.3f, 0.7f);
		b->color[0] = g;
		b->color[1] = g;
		b->color[2] = g;
		b->color[3] = 1.0;

		/* make display list*/
		b->dlist = glGenLists(1);
		glNewList(b->dlist, GL_COMPILE);
		DrawBuilding(b);
		glEndList();
	}
}


static void
Idle(void)
{
	int time = glutGet(GLUT_ELAPSED_TIME);
	Yrot = StartRot + (time - StartTime) / 100.0;
	glutPostRedisplay();
}


static void
DrawGround(void)
{
	static const GLfloat colors[2][3] = {
		{0.3f, 0.6f, 0.3f},
		{0.4f, 0.4f, 0.4f}
	};
	int i, j;
	float stepx = 80.0 / CheckerCols;
	float stepy = 80.0 / CheckerRows;
	float x, y, z, dx, dy;

	glShadeModel(GL_FLAT);
	glDisable(GL_LIGHTING);
	z = 0.0;
	y = -20.0;
	for (i = 0; i < CheckerRows; i++)
	{
		x = -20.0;
		if (i & 1)
			dy = 0.95 * stepy;
		else
			dy = 0.05 * stepy;
		glBegin(GL_QUAD_STRIP);
		for (j = 0; j <= CheckerCols; j++)
		{
			int k = ((i & 1) == 0) || ((j & 1) == 1);
			if (j & 1)
				dx = 0.95 * stepx;
			else
				dx = 0.05 * stepx;

			glColor3fv(colors[k]);
			glVertex3f(x, z, y);
			glVertex3f(x, z, y + dy);
			x += dx;
		}
		glEnd();
		y += dy;
	}
}


static void
DrawCompass(void)
{
	glColor3f(1, 1, 1);

	glPushMatrix();
	glTranslatef(0, 3, 0);

	glPushMatrix();
	glScalef(2, 2, 1);

	/* N */
	glBegin(GL_LINE_STRIP);
	glVertex3f(-1, -1, -21);
	glVertex3f(-1, 1, -21);
	glVertex3f(1, -1, -21);
	glVertex3f(1, 1, -21);
	glEnd();

	/* S */
	glBegin(GL_LINE_STRIP);
	glVertex3f(1, -1, 21);
	glVertex3f(-1, -1, 21);
	glVertex3f(-1, 0, 21);
	glVertex3f(1, 0, 21);
	glVertex3f(1, 1, 21);
	glVertex3f(-1, 1, 21);
	glEnd();

	glPopMatrix();

	glPushMatrix();
	glScalef(1, 2, 2);

	/* E */
	glBegin(GL_LINE_STRIP);
	glVertex3f(21, 1, 1);
	glVertex3f(21, 1, -1);
	glVertex3f(21, 0, -1);
	glVertex3f(21, 0, 1);
	glVertex3f(21, 0, -1);
	glVertex3f(21, -1, -1);
	glVertex3f(21, -1, 1);
	glEnd();

	/* W */
	glBegin(GL_LINE_STRIP);
	glVertex3f(-21, 1, -1);
	glVertex3f(-21, -1, -1);
	glVertex3f(-21, 0, 0);
	glVertex3f(-21, -1, 1);
	glVertex3f(-21, 1, 1);
	glEnd();

	glPopMatrix();
	glPopMatrix();
}


/* Setup modelview and draw the 3D scene. */
static void
DrawScene(void)
{
	GLfloat lightPos[4] = { 20, 90, 90, 0 };
	GLfloat lightPos1[4] = { -40, 90, -90, 0 };

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(EyeX, EyeY - EyeHeight, EyeZ - EyeDist);

	glPushMatrix();

	glRotatef(Xrot, 1, 0, 0);
	glRotatef(Yrot, 0, 1, 0);
	glRotatef(Zrot, 0, 0, 1);

	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);

	glShadeModel(GL_FLAT);
	glDisable(GL_LIGHTING);
	if (UseDisplayLists)
		glCallList(GroundList);
	else
		DrawGround();

	DrawCompass();

	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	if (Texture)
		glEnable(GL_TEXTURE_2D);

	if (UseDisplayLists) {
		GLuint i;
		for (i = 0; i < NumBuildings; i++)
			glCallList(Buildings[i].dlist);
	}
	else {
		GLuint i;
		for (i = 0; i < NumBuildings; i++)
			DrawBuilding(Buildings + i);
	}

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);

	glPopMatrix();
}


static void
StrokeString(const char *s)
{
	for (; *s; s++)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *s);
}


static void
BitmapString(const char *s)
{
	for (; *s; s++)
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *s);
}


static void
DrawOverlay(void)
{
	char s[100];

	/* setup matrices */
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glColor3f(1, 1, 0);
	/* rectangle */
	glBegin(GL_LINE_LOOP);
	glVertex2f(-0.9f, -0.9f);
	glVertex2f(0.9f, -0.9f);
	glVertex2f(0.9f, 0.9f);
	glVertex2f(-0.9f, 0.9f);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex2f(-0.95f, -0.95f);
	glVertex2f(0.95f, -0.95f);
	glVertex2f(0.95f, 0.95f);
	glVertex2f(-0.95f, 0.95f);
	glEnd();

	/* text info */
	/*
	   glRasterPos2f(-0.85, 0.8);
	   BitmapString("City demo");
	 */
	(void) BitmapString;
	glPushMatrix();
	glTranslatef(-0.8f, 0.8f, 0);
	glScalef(0.0005f, 0.0005f, 1);
	StrokeString("City demo");
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-0.8f, -0.8f, 0);
	glScalef(0.0005f, 0.0005f, 1);
	sprintf(s, "%d buildings", NumBuildings);
	StrokeString(s);
	glPopMatrix();

	/* restore matrices */
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
}


/*
 * This is called when operating in "CAVE mode".  Basically we specify
 * a new viewing frustum and view matrix for each server/view.
 */
static void
MultiFrustum(void)
{
#if USE_CHROMIUM
	unsigned int i, j;
	for (i = 0; i < NumViews; i++)
	{
		for (j = 0; j < NumTilesPerView; j++)
		{
			const float upX = 0, upY = 1, upZ = 0;
			float dirX = 0, dirY = 0, dirZ = 0;
			if (i == 0)
				dirZ = -1;
			else if (i == 1)
				dirX = 1;
			else if (i == 2)
				dirZ = 1;
			else
				dirX = -1;
			MultiviewFrustum(i * NumTilesPerView + j,
											 1.1f, 100.0f,
											 EyeX, EyeY, EyeZ, dirX, dirY, dirZ, upX, upY, upZ);
		}
	}
#endif
}


/*
 * This is called when operating in "CAVE mode" prior to rendering the
 * overlay graphics.  Just reset the per-server projection and viewing
 * matrices to the identity.
 */
static void
ResetMultiFrustum(void)
{
#if USE_CHROMIUM
	MultiviewLoadIdentity(GL_SERVER_PROJECTION_MATRIX_CR,
												NumViews * NumTilesPerView);
	MultiviewLoadIdentity(GL_SERVER_VIEW_MATRIX_CR, NumViews * NumTilesPerView);
#endif
}


static void
Draw(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (CaveMode)
	{
		MultiFrustum();
	}
	DrawScene();

	if (ShowOverlay)
	{
		if (CaveMode)
			ResetMultiFrustum();
		DrawOverlay();
	}

	glutSwapBuffers();

	if (Anim)
	{
		static GLint T0 = 0;
		static GLint Frames = 0;
		GLint t = glutGet(GLUT_ELAPSED_TIME);
		Frames++;
		if (t - T0 >= 5000)
		{
			GLfloat seconds = (t - T0) / 1000.0;
			GLfloat fps = Frames / seconds;
			printf("%d frames in %6.3f seconds = %6.3f FPS\n", Frames, seconds,
						 fps);
			T0 = t;
			Frames = 0;
		}
	}
}


static void
Reshape(int width, int height)
{
	GLfloat ar = 0.5 * (float) width / (float) height;
	WinWidth = width;
	WinHeight = height;
	glViewport(0, 0, width, height);

	if (CaveMode)
	{
		MultiFrustum();
	}
	else
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glFrustum(-ar, ar, -0.5, 0.5, 1.5, 150.0);
	}
}


static void
ResetView(void)
{
	EyeX = EyeY = EyeZ = 0.0;
	Xrot = Yrot = Zrot = 0.0;
	if (CaveMode)
	{
		EyeDist = 0.0;
		EyeHeight = 5.0;
		MultiFrustum();
	}
	else
	{
		Xrot = 20.0;
		EyeDist = 50.0;
		EyeHeight = 0.0;
	}
}


static void
Key(unsigned char key, int x, int y)
{
	(void) x;
	(void) y;
	switch (key)
	{
	case 'a':
		Anim = !Anim;
		if (Anim)
		{
			glutIdleFunc(Idle);
			StartRot = Yrot;
			StartTime = glutGet(GLUT_ELAPSED_TIME);
		}
		else
		{
			glutIdleFunc(NULL);
		}
		break;
	case 'b':
		if (NumBuildings > 1)
			NumBuildings--;
		break;
	case 'B':
		if (NumBuildings < MAX_BUILDINGS)
			NumBuildings++;
		break;
	case 'd':
		UseDisplayLists = !UseDisplayLists;
		printf("Use display lists: %d\n", (int) UseDisplayLists);
		break;
	case 't':
		Texture = !Texture;
		break;
	case 'v':
		EyeDist -= 1;
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		break;
	case 'V':
		EyeDist += 1;
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		break;
	case ' ':
		GenerateBuildings();
		break;
	case '0':
		ResetView();
		break;
	case 'x':
		if (EyeX > -1)
			EyeX -= 0.1f;
		if (CaveMode)
			MultiFrustum();
		break;
	case 'X':
		if (EyeX < 1)
			EyeX += 0.1f;
		if (CaveMode)
			MultiFrustum();
		break;
	case 'y':
		if (EyeY > -1)
			EyeY -= 0.1f;
		if (CaveMode)
			MultiFrustum();
		break;
	case 'Y':
		if (EyeY < 1)
			EyeY += 0.1f;
		if (CaveMode)
			MultiFrustum();
		break;
	case 'z':
		if (EyeZ > -1)
			EyeZ -= 0.1f;
		if (CaveMode)
			MultiFrustum();
		break;
	case 'Z':
		if (EyeZ < 1)
			EyeZ += 0.1f;
		if (CaveMode)
			MultiFrustum();
		break;

	case 'o':
		ShowOverlay = !ShowOverlay;
		break;

	case 'q':
	case 27:
		exit(0);
		break;
	}
	glutPostRedisplay();
}


static void
SpecialKey(int key, int x, int y)
{
	const GLfloat step = 3.0;
	(void) x;
	(void) y;
	switch (key)
	{
	case GLUT_KEY_UP:
		if (Xrot < 90)
			Xrot += step;
		break;
	case GLUT_KEY_DOWN:
		if (Xrot > -step * 2)
			Xrot -= step;
		break;
	case GLUT_KEY_LEFT:
		Yrot -= step;
		break;
	case GLUT_KEY_RIGHT:
		Yrot += step;
		break;
	}
	glutPostRedisplay();
}


static void
Init(void)
{
	GLfloat one[4] = { 1, 1, 1, 1 };
	GLfloat amb[4] = { 0.4f, 0.4f, 0.4f, 1.0f };

	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, one);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	glNewList(GroundList, GL_COMPILE);
	DrawGround();
	glEndList();

	glGenTextures(1, &wallTex);
	glBindTexture(GL_TEXTURE_2D, wallTex);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WALL_WIDTH, WALL_HEIGHT, 0, GL_RGBA,
							 GL_UNSIGNED_BYTE, wall);

	glGenTextures(1, &roofTex);
	glBindTexture(GL_TEXTURE_2D, roofTex);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, roof.width, roof.height, 0, GL_RGB,
							 GL_UNSIGNED_BYTE, roof.pixel_data);

	GenerateBuildings();

	if (CaveMode)
	{
#if USE_CHROMIUM
		glChromiumParametervCR_ptr =
			(glChromiumParametervCRProc) GET_PROC("glChromiumParametervCR");
		if (!glChromiumParametervCR_ptr
				|| !glutExtensionSupported("GL_CR_server_matrix"))
		{
			printf("Warning: glChromiumParametervCR function not found ");
			printf("or GL_CR_server_matrix not supported.\n");
			printf("Turning off cave mode\n");
			CaveMode = 0;
		}
#endif

#if 00
		/* XXX sample code */
		{
			int i, j;
			/* set the per-server view matrix */
			for (i = 0; i < NumViews; i++)
			{
				for (j = 0; j < NumTilesPerView; j++)
				{
					GLfloat m[18];
					glPushMatrix();
					glLoadIdentity();
					glRotatef(i * 90, 0, 1, 0);
					glGetFloatv(GL_MODELVIEW_MATRIX, m + 2);
					glPopMatrix();
					m[0] = (GLfloat) i * 2 + j;  /* the server */
					m[1] = 0; /* the eye */
					glChromiumParametervCR_ptr(GL_SERVER_VIEW_MATRIX_CR, GL_FLOAT, 18, m);
				}
			}
		}
#endif
	}

	ResetView();
	if (CaveMode)
		Draw();											/* not sure why this is needed */
}


int
main(int argc, char *argv[])
{
	int mode, i;
	int help = 0;

	/* Work around compiler issues that have 64k limits */
	wall = malloc(sizeof(walldata1) + sizeof(walldata2) + sizeof(walldata3));
	memcpy(wall, walldata1, sizeof(walldata1));
	memcpy(wall + sizeof(walldata1), walldata2, sizeof(walldata2));
	memcpy(wall + sizeof(walldata1) + sizeof(walldata2), walldata3,
				 sizeof(walldata3));

	glutInit(&argc, argv);
	mode = GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-ms") == 0)
		{
			mode |= GLUT_MULTISAMPLE;
		}
		else if (strcmp(argv[i], "-b") == 0 && i + 1 < argc)
		{
			NumBuildings = atoi(argv[i + 1]);
			i++;
		}
		else if (strcmp(argv[i], "-c") == 0)
		{
			CaveMode = GL_TRUE;
		}
		else if (strcmp(argv[i], "-h") == 0)
		{
			help = 1;
		}
		else if (strcmp(argv[i], "-v") == 0 && i + 1 < argc)
		{
			NumViews = atoi(argv[i + 1]);
			i++;
		}
		else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc)
		{
			NumTilesPerView = atoi(argv[i + 1]);
			i++;
		}
		else
		{
			printf("Bad option: %s\n", argv[i]);
			help = 1;
			break;
		}
	}

	if (help)
	{
		printf("Valid options:\n");
		printf("  -b N  specify initial number of buildings\n");
		printf("  -ms   enable multisample antialiasing\n");
		printf("  -c    enable CAVE mode (for cavetest1.conf)\n");
		printf("  -v N  specify number of views for CAVE mode\n");
		printf("  -t N  specify number of tiles per view for CAVE mode\n");
		exit(0);
	}

	printf("City keyboard controls:\n");
	printf("  Up/Down/Left/Right Arrows - rotate scene\n");
	printf("  v/V                       - translate eye forward/backward\n");
	printf("  x/X/y/Y/z/Z               - translate eye position\n");
	printf("  0                         - reset view parameters\n");
	printf("  a                         - toggle animation\n");
	printf
		("  b/B                       - decrease/increase number of buildings\n");
	printf("  t                         - toggle textures on/off\n");
	printf("  SPACE                     - generate new, random buildings\n");
	printf("  q/ESC                     - exit\n");

	glutInitWindowSize(WinWidth, WinHeight);
	glutInitDisplayMode(mode);
	glutCreateWindow(argv[0]);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Key);
	glutSpecialFunc(SpecialKey);
	glutDisplayFunc(Draw);
	if (Anim)
	{
		glutIdleFunc(Idle);
		StartRot = Yrot;
		StartTime = glutGet(GLUT_ELAPSED_TIME);
	}
	Init();
	glutMainLoop();
	return 0;
}
