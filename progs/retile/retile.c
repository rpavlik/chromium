/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*
 * Demonstrate dynamic tile reconfiguration.
 * Works with retile.conf script.
 *
 * Author: Brian Paul
 * July 2002
 */


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "chromium.h"
#include <GL/glut.h>


/*
 * Function pointers for the Chromium functions we need
 */
static glChromiumParameteriCRProc glChromiumParameteriCRptr;
static glChromiumParameterfCRProc glChromiumParameterfCRptr;
static glChromiumParametervCRProc glChromiumParametervCRptr;
static glGetChromiumParametervCRProc glGetChromiumParametervCRptr;


/*
 * Call this after the window's created so that we have a rendering
 * context and wglGetProcAddress will work.
 */
static void
GetCrExtensions(void)
{
	glChromiumParameteriCRptr = (glChromiumParameteriCRProc)
 		GET_PROC("glChromiumParameteriCR");
	glChromiumParameterfCRptr = (glChromiumParameterfCRProc)
		GET_PROC("glChromiumParameterfCR");
	glChromiumParametervCRptr = (glChromiumParametervCRProc)
		GET_PROC("glChromiumParametervCR");
	glGetChromiumParametervCRptr = (glGetChromiumParametervCRProc)
		GET_PROC("glGetChromiumParametervCR");

	if (!glChromiumParameteriCRptr || !glChromiumParameterfCRptr ||
			!glChromiumParametervCRptr || !glGetChromiumParametervCRptr) {
		fprintf(stderr, "Couldn't find glChromiumParameter[ifv]CR() functions.\n");
		fprintf(stderr, "Are you sure you're running with Chromium?\n");
		fprintf(stderr, "Run with -t if you want to try 'configure' mode.\n");
		exit(1);
	}
}


/*
 * Window & rendering
 */
static GLfloat Xrot = 0, Yrot = 0, Zrot = 0;
static GLint WinWidth = 512, WinHeight = 512;
static GLboolean Anim = GL_TRUE;

/*
 * Servers & tiles
 */
#define MAX_SERVERS 100
#define MAX_TILES_PER_SERVER 100

struct rect {
	int x, y, w, h;
};

static struct rect ServerTiles[MAX_SERVERS][MAX_TILES_PER_SERVER];
static int NumTiles[MAX_SERVERS];
static GLint NumServers = 2;
static GLint TileSize = 256;
static const GLint MinTileSize = 8, MaxTileSize = 512;
static GLint MuralWidth, MuralHeight;


/*
 * Compute tile bounds and assign them to servers.
 * Input:  muralWidth, muralHeight, tileSize, numServers
 */
static void
LayoutMural(int muralWidth, int muralHeight, int tileSize, int numServers)
{
	int i, j, k, rows, cols;
	GLboolean zigzag = GL_FALSE;

	assert(numServers >= 1);
	assert(tileSize >= MinTileSize);
	assert(tileSize <= MaxTileSize);
	assert(muralWidth >= tileSize);
	assert(muralHeight >= tileSize);

	for (i = 0; i < numServers; i++)
		NumTiles[i] = 0;

	/* keep it simple for now */
	if (muralWidth % tileSize != 0 || muralHeight % tileSize != 0) {
		printf("window size is not a multiple of tile size!");
		return;
	}

	/*
	 * Use simple raster-order tile layout/assignment.
	 */
	cols = muralWidth / tileSize;
	rows = muralHeight / tileSize;

	if (cols % numServers != 0)
		zigzag = GL_TRUE;

	k = 0;

	for (i = 0; i < rows; i++) {
		/* odd-numbered rows are right-to-left instead of left-to-right */
		if ((i & 1) && zigzag) {
			/* right to left */
			for (j = cols - 1; j >= 0; j--) {
				const int srv = k % numServers;
				struct rect *r = &(ServerTiles[srv][NumTiles[srv]]);
				r->x = j * tileSize;
				r->y = i * tileSize;
				r->w = tileSize;
				r->h = tileSize;
				NumTiles[srv]++;
				k++;
			}
		}
		else {
			/* left to right */
			for (j = 0; j < cols; j++) {
				const int srv = k % numServers;
				struct rect *r = &(ServerTiles[srv][NumTiles[srv]]);
				r->x = j * tileSize;
				r->y = i * tileSize;
				r->w = tileSize;
				r->h = tileSize;
				NumTiles[srv]++;
				k++;
			}
		}
	}
}


/*
 * Print mural parameters.
 */
static void
PrintMural(void)
{
	int i, j;

	printf("Window size: %d x %d\n", WinWidth, WinHeight);
	printf("Mural size: %d x %d\n", MuralWidth, MuralHeight);
	printf("Num servers: %d\n", NumServers);
	printf("Tile size: %d x %d\n", TileSize, TileSize);
	for (i = 0; i < NumServers; i++) {
		printf("Server %d, %d tiles:\n", i, NumTiles[i]);
		for (j = 0; j < NumTiles[i]; j++) {
			const struct rect *r = &(ServerTiles[i][j]);
			printf("  %d: %d, %d %d x %d\n", j, r->x, r->y, r->w, r->h);
		}
	}
}


/*
 * Give the new mural/tile information to Chromium.
 */
static void
UpdateMural(void)
{
	int params[MAX_TILES_PER_SERVER * 4 + 8];
	int i, j, count;

	for (i = 0; i < NumServers; i++) {
		params[0] = i;           /* server index */
		params[1] = WinWidth;    /* mural width */
		params[2] = WinHeight;   /* mural height */
		params[3] = NumTiles[i]; /* num tiles on this server */
		for (j = 0; j < NumTiles[i]; j++) {
			const struct rect *r = &(ServerTiles[i][j]);
			params[4 + j * 4 + 0] = r->x;
			params[4 + j * 4 + 1] = r->y;
			params[4 + j * 4 + 2] = r->w;
			params[4 + j * 4 + 3] = r->h;
		}
		count = 4 + NumTiles[i] * 4;
		glChromiumParametervCRptr(GL_TILE_INFO_CR, GL_INT, count, params);
	}
}


static void
DrawMolecule(void)
{
	static const GLfloat red[4]  = {1.0, 0.2, 0.2, 1.0};
	static const GLfloat blue[4] = {0.2, 0.2, 1.0, 1.0};
	const float radius = 5.0;
	const float sphereRad = 0.5;
	int angle;
	/* just draw a bunch of spheres in a pretty pattern around a circle */

	for (angle = 0; angle < 720; angle += 5) {
		glPushMatrix();
			glTranslatef(0, 0, angle * 0.01 - 3.6);
			glRotatef(angle, 0, 0, 1);
			glTranslatef(radius, 0, 0);  /* position around ring */
			glPushMatrix();
				glRotatef(-9.0 * angle, 0, 1, 0);
				glPushMatrix();
					glTranslatef(-0.7, 0, 0);
					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, red);
					glutSolidSphere(sphereRad, 20, 10);
				glPopMatrix();
				glPushMatrix();
					glTranslatef(0.7, 0, 0);
					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, blue);
					glutSolidSphere(sphereRad, 20, 10);
				glPopMatrix();
			glPopMatrix();
		glPopMatrix();
	}
}


static void
Redraw( void )
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
		glRotatef(Xrot, 1, 0, 0);
		glRotatef(Yrot, 0, 1, 0);
		glRotatef(Zrot, 0, 0, 1);
		DrawMolecule();
	glPopMatrix();
	glutSwapBuffers();

	/* fps */
	{
		static GLint frames = 0, t0 = 0;
		GLint t = glutGet(GLUT_ELAPSED_TIME);
		frames++;
		if (t - t0 >= 5000) {
			GLfloat seconds = (t - t0) / 1000.0;
			GLfloat fps = frames / seconds;
			printf("%d frames in %6.3f seconds = %6.3f FPS\n",
				   frames, seconds, fps);
			t0 = t;
			frames = 0;
		}
	}

	/* XXX In addition to computing fps we should vary the tile size,
	 * render some number of frames, then determine which tile size is
	 * optimal.
	 * But this sort of thing is rather application-specific.
	 * Furthermore, the best arrangement of tiles is application-specific.
	 */
}


static void
Reshape( int width, int height )
{
	glViewport( 0, 0, width, height );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glFrustum( -1.0, 1.0, -1.0, 1.0, 4.0, 100.0 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glTranslatef( 0.0, 0.0, -40.0 );
}


static void
Idle( void )
{
	Xrot += 0.0;
	Yrot += 3.0;
	Zrot += 0.0;
	glutPostRedisplay();
}

static void
Key( unsigned char key, int x, int y )
{
	GLboolean newLayout = GL_FALSE;
	(void) x;
	(void) y;
	switch (key) {
	case 'a':
		Anim = !Anim;
		if (Anim)
            glutIdleFunc(Idle);
		else
            glutIdleFunc(NULL);
		break;
	case 't':
		TileSize /= 2;
		if (TileSize < MinTileSize)
			TileSize = MinTileSize;
		newLayout = GL_TRUE;
		break;
	case 'T':
		TileSize *= 2;
		if (TileSize > MaxTileSize)
			TileSize = MaxTileSize;
		newLayout = GL_TRUE;
		break;
	case 27:
		exit(0);
		break;
	}

	if (newLayout) {
		/* compute mural size to be at least the window size */
		MuralWidth = ((WinWidth + TileSize - 1) / TileSize) * TileSize;
		MuralHeight = ((WinHeight + TileSize - 1) / TileSize) * TileSize;
		LayoutMural(MuralWidth, MuralHeight, TileSize, NumServers);
		PrintMural();
		UpdateMural();
	}

	glutPostRedisplay();
}



static void
SpecialKey( int key, int x, int y )
{
	const GLfloat step = 3.0;
	(void) x;
	(void) y;
	switch (key) {
	case GLUT_KEY_UP:
		Xrot -= step;
		break;
	case GLUT_KEY_DOWN:
		Xrot += step;
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
Init( int argc, char *argv[] )
{
	if (argc > 1)
		NumServers = atoi(argv[1]);
	if (NumServers < 1)
		NumServers = 1;

	GetCrExtensions();

	/* setup lighting, etc */
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	/* initial mural config */
	MuralWidth = WinWidth;
	MuralHeight = WinHeight;
	LayoutMural(MuralWidth, MuralHeight, TileSize, NumServers);
	PrintMural();
	UpdateMural();
}


int
main( int argc, char *argv[] )
{
	glutInit( &argc, argv );
	glutInitWindowPosition( 600, 300 );
	glutInitWindowSize( WinWidth, WinHeight );
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
	glutCreateWindow( argv[0] );
	glutReshapeFunc( Reshape );
	glutKeyboardFunc( Key );
	glutSpecialFunc( SpecialKey );
	glutDisplayFunc( Redraw );
	if (Anim)
		glutIdleFunc(Idle);

	Init(argc, argv);

	printf("Keystrokes:\n");
	printf("  t - decrease tile size to 1/2\n");
	printf("  T - increase tile size by 2\n");
	printf("  a - toggle animation\n");

	glutMainLoop();
	return 0;
}


