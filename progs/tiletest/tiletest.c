/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*
 * Tilesort SPU verification program.
 *
 * This program uses the GL_CR_chromium_parameter and GL_CR_tilesort_info
 * extensions to verify that geometry sent through the tilesort SPU is
 * binned properly and not just broadcast to all servers.
 *
 * Author: Brian Paul
 */


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "chromium.h"
#include <GL/glut.h>


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
 * Description of the tilesort configuration
 */
#define MAX_SERVERS 64
#define MAX_TILES_PER_SERVER 64

typedef struct bounds {
	GLint x0, y0;
	GLint x1, y1;
} bounds_t;

typedef struct server {
	GLuint Width, Height;  /* size of server's frame buffer */
	GLuint NumTiles;       /* number of tiles on this server */
	bounds_t TileBounds[MAX_TILES_PER_SERVER];
	bounds_t ServerBounds[MAX_TILES_PER_SERVER];
} server_t;

typedef struct mural {
	GLuint NumServers;
	server_t *Servers;
	GLuint Width, Height;
} mural_t;

static mural_t *TheMural = NULL;

/* XXX we really need a way to query these from Chromium */
static GLuint DefaultServerWidth = 100, DefaultServerHeight = 300;

/* GLUT, misc stuff */
static GLint GlutWidth = 400, GlutHeight = 400;


/*
 * Dry run / test mode:
 */
static GLboolean UseChromium = GL_TRUE;
static GLboolean AllocTiles = GL_FALSE;
static GLuint FakeRows = 4, FakeCols = 4;
static GLuint FakeNumServers = 3;


typedef enum {
	PER_FACE,
	PER_CUBE,
	PER_SCENE
} frequency_t;

static const char *FrequencyString[] = {
	"Per-FACE",
	"Per-CUBE",
	"Per-SCENE"
};

static frequency_t BeginFrequency = PER_CUBE;

static GLboolean RunTests = GL_FALSE;


/*
 * For the given server, loop over the tiles defined by TileBounds[] and
 * allocate space for them in the server's frame buffer.  Put the results
 * in the ServerBounds[] array.
 * XXX this is kind of pointless at this time since we don't have all
 * the info we need (server size) to do it.
 */
static void
AllocTilesOnServer(server_t *server)
{
	GLuint freeX = 0, freeY = 0, tallestInRow = 0;
	GLuint t;
	
	for (t = 0; t < server->NumTiles; t++) {
		const GLuint w = server->TileBounds[t].x1 - server->TileBounds[t].x0;
		const GLuint h = server->TileBounds[t].y1 - server->TileBounds[t].y0;

		if (w > server->Width || h > server->Height) {
			fprintf(stderr, "Tile %d is too large for the server!!!\n", t);
			return;
		}

		if (freeX + w > server->Width) {
			freeX = 0;
			freeY += tallestInRow;
			tallestInRow = 0;
		}

		if (freeY + h > server->Height) {
			fprintf(stderr, "Ran out of room on server for tile %d\n", t);
			return;
		}

		server->ServerBounds[t].x0 = freeX;
		server->ServerBounds[t].y0 = freeY;
		server->ServerBounds[t].x1 = freeX + w;
		server->ServerBounds[t].y1 = freeY + h;

		if (h > tallestInRow)
			tallestInRow = h;
	}
}


static void
AllocTilesOnServers(mural_t *m)
{
	GLuint s;
	for (s = 0; s < m->NumServers; s++)
		AllocTilesOnServer(&(m->Servers[s]));
}


/*
 * Query the number of servers, tiles, the tile bounds, etc.
 */
static mural_t *
FetchMuralInformation(void)
{
	mural_t *m;
	GLuint v[4], s, t;

	m = malloc(sizeof(mural_t));
	assert(m);

	glGetChromiumParametervCRptr(GL_MURAL_SIZE_CR, 0, GL_INT, 2, v);
	m->Width = v[0];
	m->Height = v[1];

	glGetChromiumParametervCRptr(GL_NUM_SERVERS_CR, 0, GL_INT, 1,
															 &(m->NumServers));

	m->Servers = malloc(m->NumServers * sizeof(server_t));
	assert(m->Servers);

	for (s = 0; s < m->NumServers; s++) {
		server_t *server = m->Servers + s;
		GLuint tiles;

		server->Width = DefaultServerWidth;
		server->Height = DefaultServerHeight;

		glGetChromiumParametervCRptr(GL_NUM_TILES_CR, s, GL_INT, 1, &tiles);
		server->NumTiles = tiles;

		for (t = 0; t < tiles; t++) {
			GLuint index = (s << 16) | t;
			glGetChromiumParametervCRptr(GL_TILE_BOUNDS_CR, index, GL_INT, 4, v);
			server->TileBounds[t].x0 = v[0];
			server->TileBounds[t].y0 = v[1];
			server->TileBounds[t].x1 = v[2];
			server->TileBounds[t].y1 = v[3];
		}
	}

	if (AllocTiles)
		AllocTilesOnServers(m);

	return m;
}


/*
 * Set up fake server/tile information for testing purposes.
 */
static mural_t *
FakeMuralInformation(GLuint muralWidth, GLuint muralHeight,
										 GLuint numServers,
										 GLuint tileWidth, GLuint tileHeight)
{
	mural_t *m;
	GLuint rows, cols;
	GLuint i, j, s, t;

	m = malloc(sizeof(mural_t));
	assert(m);

	m->Width = muralWidth;
	m->Height = muralHeight;
	m->NumServers = numServers;

	m->Servers = malloc(m->NumServers * sizeof(server_t));
	assert(m->Servers);

	/* make sure the mural is a nice multiple of tile size */
	assert(muralWidth % tileWidth == 0);
	assert(muralHeight % tileHeight == 0);

	cols = muralWidth / tileWidth;
	rows = muralHeight / tileHeight;

	for (s = 0; s < m->NumServers; s++) {
		m->Servers[s].Width = DefaultServerWidth;
		m->Servers[s].Height = DefaultServerHeight;
		m->Servers[s].NumTiles = 0;
	}

	/* Compute tile bounds */
	s = t = 0;
	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			m->Servers[s].TileBounds[t].x0 = j * tileWidth;
			m->Servers[s].TileBounds[t].y0 = i * tileHeight;
			m->Servers[s].TileBounds[t].x1 = j * tileWidth + tileWidth;
			m->Servers[s].TileBounds[t].y1 = i * tileHeight + tileHeight;
			m->Servers[s].NumTiles++;
			s++;
			if (s >= numServers) {
				s = 0;
				t++;
			}
		}
	}

	if (AllocTiles)
		AllocTilesOnServers(m);

	return m;
}


/*
 * Used during 'dry-run' testing via the keyboard params
 */
static mural_t *
SetupFakeMuralInformation(GLuint numServers, GLuint rows, GLuint cols)
{
	GLuint muralWidth = 1000;
	GLuint muralHeight = 1000;
	GLuint tileWidth, tileHeight;

	tileWidth = muralWidth / cols;
	tileHeight = muralHeight / rows;

	muralWidth = tileWidth * cols;
	muralHeight = tileHeight * rows;

	return FakeMuralInformation(muralWidth, muralHeight, numServers,
															tileWidth, tileHeight);
}



static void
PrintMuralInformation(const mural_t *m)
{
	GLuint s, t;

	printf("\nMural parameters:\n");
	printf("  Mural size: %d x %d\n", m->Width, m->Height);
	printf("  Number of servers: %d\n", m->NumServers);
	for (s = 0; s < m->NumServers; s++) {
		const server_t *server = m->Servers + s;
		printf("    Server %d has %d tiles:\n", s, server->NumTiles);
		for (t = 0; t < server->NumTiles; t++) {
			printf("      Tile %d: (%d, %d) .. (%d, %d)\n", t,
						 server->TileBounds[t].x0, server->TileBounds[t].y0,
						 server->TileBounds[t].x1, server->TileBounds[t].y1);
		}
	}
}


static void
DrawString(GLint x, GLint y, const char *s, float scale)
{
	 int i;
	 glPushMatrix();
	 glTranslatef(x, y, 0);
	 glScalef(scale, scale, 1);
	 for (i = 0; s[i]; i++) {
			int w;
			glutStrokeCharacter(GLUT_STROKE_ROMAN, s[i]);
			w = glutStrokeWidth(GLUT_STROKE_ROMAN, s[i]);
			glTranslatef(w * scale, 0, 0);
	 }
	 glPopMatrix();
}


/*
 * Draw a test pattern that draws color coded tiles (according to the server)
 * with tile information.
 */
static void
DrawTileMapping(const mural_t *m)
{
	/* enough to uniquely color 16 servers */
	static GLfloat serverColors[][3] = {
		{1, 0, 0},
		{0, 1, 0},
		{0, 0, 1},
		{0, 1, 1},
		{1, 0, 1},
		{1, 1, 0},
		{.5f, 0, 0},
		{0, .5f, 0},
		{0, 0, .7f},
		{1, .7f, .7f},
		{.7f, 1, .7f},
		{.7f, .7f, 1},
		{.5f, .5f, 1},
		{1, .5f, .5f},
		{.5f, 1, .5f},
		{1, 1, 1}
	};
	const GLuint numColors = sizeof(serverColors) / sizeof(serverColors[0]);
	GLuint s, t;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, m->Width, 0, m->Height, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLineWidth(2);

	for (s = 0; s < m->NumServers; s++) {
		const server_t *server = m->Servers + s;
		for (t = 0; t < server->NumTiles; t++) {
			const bounds_t *b = &(server->TileBounds[t]);
			char name[10];
			float scale;

			glColor3fv(serverColors[s % numColors]);
#if 0
			/* glRect seems flakey with the tilesort SPU */
			glRectf(b->x0, b->y0, b->x1, b->y1);
#else
			glBegin(GL_POLYGON);
			glVertex2f(b->x0, b->y0);
			glVertex2f(b->x1, b->y0);
			glVertex2f(b->x1, b->y1);
			glVertex2f(b->x0, b->y1);
			glEnd();
#endif
			glColor3f(0.5, 0.5, 0.5);
			sprintf(name, "s%dt%d", s, t);

			scale = .002 * (b->x1 - b->x0);
			DrawString(b->x0 + 5, b->y0 + 5, name, scale);
		}
	}
	glutSwapBuffers();
}


static void
DrawCubeFaces(GLfloat size, GLfloat x, GLfloat y, GLfloat z,
							frequency_t beginFrequency)
{
	if (beginFrequency == PER_CUBE)
		glBegin(GL_QUADS);

	/* +X face */
	if (beginFrequency == PER_FACE)
		glBegin(GL_QUADS);
	glNormal3f(1, 0, 0);
	glVertex3f(x + size, y - size, z + size);
	glVertex3f(x + size, y - size, z - size);
	glVertex3f(x + size, y + size, z - size);
	glVertex3f(x + size, y + size, z + size);
	if (beginFrequency == PER_FACE)
		glEnd();

	/* -X face */
	if (beginFrequency == PER_FACE)
		glBegin(GL_QUADS);
	glNormal3f(-1, 0, 0);
	glVertex3f(x - size, y - size, z + size);
	glVertex3f(x - size, y + size, z + size);
	glVertex3f(x - size, y + size, z - size);
	glVertex3f(x - size, y - size, z - size);
	if (beginFrequency == PER_FACE)
		glEnd();

	/* +Y face */
	if (beginFrequency == PER_FACE)
		glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);
	glVertex3f(x - size, y + size, z + size);
	glVertex3f(x + size, y + size, z + size);
	glVertex3f(x + size, y + size, z - size);
	glVertex3f(x - size, y + size, z - size);
	if (beginFrequency == PER_FACE)
		glEnd();

	/* -Y face */
	if (beginFrequency == PER_FACE)
		glBegin(GL_QUADS);
	glNormal3f(0, -1, 0);
	glVertex3f(x - size, y + size, z + size);
	glVertex3f(x - size, y + size, z - size);
	glVertex3f(x + size, y + size, z - size);
	glVertex3f(x + size, y + size, z + size);
	if (beginFrequency == PER_FACE)
		glEnd();

	/* +Z face */
	if (beginFrequency == PER_FACE)
		glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	glVertex3f(x - size, y - size, z + size);
	glVertex3f(x + size, y - size, z + size);
	glVertex3f(x + size, y + size, z + size);
	glVertex3f(x - size, y + size, z + size);
	if (beginFrequency == PER_FACE)
		glEnd();

	/* -Z face */
	if (beginFrequency == PER_FACE)
		glBegin(GL_QUADS);
	glNormal3f(0, 0, -1);
	glVertex3f(x - size, y - size, z - size);
	glVertex3f(x - size, y + size, z - size);
	glVertex3f(x + size, y + size, z - size);
	glVertex3f(x + size, y - size, z - size);
	if (beginFrequency == PER_FACE)
		glEnd();

	if (beginFrequency == PER_CUBE)
		glEnd();
}


/*
 * Draw a cube in the region specified by server s and tile t.
 */
static void
DrawCubeInTile(const mural_t *m, const bounds_t *b,
							 GLboolean usePerspective, frequency_t beginFrequency)
{
	GLfloat xmin, xmax, ymin, ymax, boxSize, xCenter, yCenter, zCenter;

	xmin = ((float) b->x0 / (float) m->Width) * 2.0 - 1.0;
	xmax = ((float) b->x1 / (float) m->Width) * 2.0 - 1.0;
	ymin = ((float) b->y0 / (float) m->Height) * 2.0 - 1.0;
	ymax = ((float) b->y1 / (float) m->Height) * 2.0 - 1.0;

	if (usePerspective) {
		xmin *= 2.0;
		xmax *= 2.0;
		ymin *= 2.0;
		ymax *= 2.0;
	}

	boxSize = (xmax - xmin) / 6.0;

	xCenter = (xmin + xmax) / 2.0;
	yCenter = (ymin + ymax) / 2.0;
	zCenter = 0;

	if (beginFrequency != PER_SCENE) {
		glPushMatrix();
		glTranslatef(xCenter, yCenter, 0);
		glRotatef(-10 * yCenter, 1, 0, 0);
		glRotatef( 10 * xCenter, 0, 1, 0);
		xCenter = yCenter = 0;
	}

	DrawCubeFaces(boxSize, xCenter, yCenter, zCenter, beginFrequency);

	if (beginFrequency != PER_SCENE) {
		glPopMatrix();
	}
}


/*
 * This is the interesting function - it exercises the tilesort SPU
 * to be sure it's really sorting.  We do this by drawing cubes in
 * locations which will get bucketed into particular servers, then
 * counting the number of vertices sent to each server and comparing
 * to expected values.
 */
static void
TestTileSorting(const mural_t *m, GLboolean usePerspective,
								frequency_t beginFrequency, GLboolean printResults)
{
	static const GLfloat white[4] = {1, 1, 1, 1};
	const GLint verticesPerCube = 24;
	GLuint counts[MAX_SERVERS], expectedCounts[MAX_SERVERS], grandTotal;
	GLuint s, t, i;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (usePerspective)
		glFrustum(-1, 1, -1, 1, 4, 12);
	else
		glOrtho(-1, 1, -1, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (usePerspective)
		glTranslatef(0, 0, -8);

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, white);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);


	for (i = 0; i < m->NumServers; i++)
		expectedCounts[i] = 0;
	grandTotal = 0;

	/* reset the vertex counters to zero */
	if (UseChromium)
		glChromiumParameteriCRptr(GL_RESET_VERTEX_COUNTERS_CR, 0);

	if (beginFrequency == PER_SCENE)
		glBegin(GL_QUADS);

	for (s = 0; s < m->NumServers; s++) {
		for (t = 0; t < m->Servers[s].NumTiles; t++) {
			const bounds_t *b = &(m->Servers[s].TileBounds[t]);
			DrawCubeInTile(m, b, usePerspective, beginFrequency);
			expectedCounts[s] += verticesPerCube;
			grandTotal += verticesPerCube;
		}
	}

	if (beginFrequency == PER_SCENE)
		glEnd();

	glFlush();

	/* Get the vertex counters */
	if (UseChromium)
		glGetChromiumParametervCRptr(GL_VERTEX_COUNTS_CR, 0, GL_INT, m->NumServers,
																 counts);

	if (printResults) {
		GLenum passed, broadcast;
		/* Print summary and check for pass/fail */
		printf("\n%s glBegin/glEnd test:\n", FrequencyString[beginFrequency]);
		passed = GL_TRUE;
		broadcast = GL_FALSE;
		if (beginFrequency == PER_SCENE) {
			/* All the cubes were drawn inside one glBegin/glEnd pair.  Since
			 * Chromium buckets primitives at that level, all vertices should have
			 * been sent to all servers.
			 */
			for (i = 0; i < m->NumServers; i++) {
				printf("  server %d: got %d vertices, expected %d vertices\n",
							 i, counts[i], grandTotal);
				if (counts[i] != grandTotal) {
					passed = GL_FALSE;
					break;
				}
			}
		}
		else {
			/* Either there was one glBegin/glEnd per cube, or a glBegin/glEnd per
			 * cube face.  In either case, each server should have been sent just
			 * one set of cube vertices per tile.
			 */
			for (i = 0; i < m->NumServers; i++) {
				printf("  server %d: got %d vertices, expected %d vertices\n",
								i, counts[i], expectedCounts[i]);
				if (counts[i] != expectedCounts[i]) {
					passed = GL_FALSE;
					if (counts[i] == grandTotal)
						broadcast = GL_TRUE;
				}
			}
		}

		if (passed)
			printf("  PASSED.\n");
		else {
			if (broadcast)
				printf("  FAILED:  check if tilesort broadcast option is enabled.");
			else
				printf("  FAILED.");
		}

		RunTests = GL_FALSE;
	}

	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
}


static void
display( void )
{
	if (RunTests) {
		DrawTileMapping(TheMural);
		TestTileSorting(TheMural, GL_TRUE, PER_FACE, GL_TRUE);
		DrawTileMapping(TheMural);
		TestTileSorting(TheMural, GL_TRUE, PER_CUBE, GL_TRUE);
		DrawTileMapping(TheMural);
		TestTileSorting(TheMural, GL_TRUE, PER_SCENE, GL_TRUE);
		RunTests = GL_FALSE;
	}
	else {
		DrawTileMapping(TheMural);
		TestTileSorting(TheMural, GL_TRUE, BeginFrequency, GL_FALSE);
	}
	glutSwapBuffers();
}


static void
Reshape( int width, int height )
{
	GlutWidth = width;
	GlutHeight = height;
	glViewport( 0, 0, width, height );
}


static void
Key( unsigned char key, int x, int y )
{
	GLboolean recomputeTest = GL_FALSE;
	(void) x;
	(void) y;
	switch (key) {
	case 't':
		if (UseChromium)
			RunTests = GL_TRUE;
		break;
	case 'p':
		PrintMuralInformation(TheMural);
		break;
	case 'f':
		if (BeginFrequency == PER_FACE) {
			BeginFrequency = PER_CUBE;
		}
		else if (BeginFrequency == PER_CUBE) {
			BeginFrequency = PER_SCENE;
		}
		else {
			BeginFrequency = PER_FACE;
		}
		printf("glBegin %s\n", FrequencyString[BeginFrequency]);
		break;

	case 's':
		if (!UseChromium) {
			FakeNumServers--;
			if (FakeNumServers <= 0)
				FakeNumServers = 1;
			recomputeTest = GL_TRUE;
		}
		break;
	case 'S':
		if (!UseChromium) {
			FakeNumServers++;
			recomputeTest = GL_TRUE;
		}
		break;
	case 'r':
		if (!UseChromium) {
			FakeRows--;
			if (FakeRows < 1)
				FakeRows = 1;
			recomputeTest = GL_TRUE;
		}
		break;
	case 'R':
		if (!UseChromium) {
			FakeRows++;
			recomputeTest = GL_TRUE;
		}
		break;
	case 'c':
		if (!UseChromium) {
			FakeCols--;
			if (FakeCols < 1)
				FakeCols = 1;
			recomputeTest = GL_TRUE;
		}
		break;
	case 'C':
		if (!UseChromium) {
			FakeCols++;
			recomputeTest = GL_TRUE;
		}
		break;
	case 27:
		exit(0);
		break;
	}

	if (recomputeTest) {
		printf("%d servers, %d rows, %d columns\n",
					 FakeNumServers, FakeRows, FakeCols);
		TheMural = SetupFakeMuralInformation(FakeNumServers, FakeRows, FakeCols);
	}

	glutPostRedisplay();
}



static void
Init( int argc, char *argv[] )
{
	if (argc > 1 && strcmp(argv[1], "-t")==0) {
		UseChromium = GL_FALSE;
		RunTests = GL_FALSE;
	}
	else {
		RunTests = GL_TRUE;
	}
}


int
main( int argc, char *argv[] )
{
	glutInit( &argc, argv );
	glutInitWindowPosition( 600, 5 );
	glutInitWindowSize( GlutWidth, GlutHeight );
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
	glutCreateWindow(argv[0]);
	glutReshapeFunc( Reshape );
	glutKeyboardFunc( Key );
	glutDisplayFunc( display );

	Init(argc, argv);

	if (UseChromium) {
		GetCrExtensions();
		TheMural = FetchMuralInformation();
	}
	else {
		TheMural = SetupFakeMuralInformation(FakeNumServers, FakeRows, FakeCols);
	}
	PrintMuralInformation(TheMural);
	glutMainLoop();
	return 0;
}
