/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/* 1 = Use Chromium parallel API, 0 = use GLUT */
#ifndef USE_CHROMIUM
#define USE_CHROMIUM 1
#endif

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#if USE_CHROMIUM
#include "chromium.h"
#include "cr_string.h"
#include "cr_error.h"
#include "cr_timer.h"
#else
#define GL_GLEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#include <GL/glut.h>

#ifdef WINDOWS
#define crGetProcAddress(p) wglGetProcAddress(p)
#else
#define crGetProcAddress(p) glXGetProcAddressARB((const GLubyte *) p)
#endif

#endif /* USE_CHROMIUM */


#ifndef M_PI
# define M_PI		3.14159265358979323846	/* pi */
#endif


typedef struct
{
	/* from command line */
	const char *name;
	int size;
	int rank;
	int swapFlag;
	int clearFlag;
	int numSpheres;
	int numTris;
	int maxFrames;
	int useVBO;

	/* derived */
	float theta, zPos, radius;

} Options;


typedef struct
{
	GLuint NumVerts, NumIndices;
	GLuint NumTris;
	GLfloat *Vertices;
	GLbyte *Normals;
	GLuint *Indices;
	GLushort *Indices16;
	GLuint VertexBytes, NormalBytes, IndexBytes;
	GLuint VertexBufferObj;
	GLuint ElementBufferObj;
	GLuint DisplayList;
} TriangleMesh;


typedef enum
{
	DRAW_VERTEX_ARRAYS = 0,
	DRAW_VBO,
	DRAW_DISPLAY_LIST
} DrawMode;

static const char *DrawModeString[] = {
	"Conventional glDrawRangeElements",
	"VBO glDrawRangeElements",
	"Display Lists"
};


static GLint WinWidth = 400, WinHeight = 400;
static DrawMode DrawingMode = DRAW_VERTEX_ARRAYS;

static const GLfloat colors[7][4] = {
	{1,0,0,1},
	{0,1,0,1},
	{0,0,1,1},
	{0,1,1,1},
	{1,0,1,1},
	{1,1,0,1},
	{1,1,1,1}
};


static PFNGLGENBUFFERSARBPROC glGenBuffersARB_ptr;
static PFNGLBINDBUFFERARBPROC glBindBufferARB_ptr;
static PFNGLBUFFERDATAARBPROC glBufferDataARB_ptr;
static PFNGLBUFFERSUBDATAARBPROC glBufferSubDataARB_ptr;


#if USE_CHROMIUM
static const int MASTER_BARRIER = 100;

static crCreateContextProc crCreateContext_ptr;
static crMakeCurrentProc   crMakeCurrent_ptr;
static crSwapBuffersProc   crSwapBuffers_ptr;

static glChromiumParametervCRProc glChromiumParametervCR_ptr;
static glGetChromiumParametervCRProc glGetChromiumParametervCR_ptr;
static glBarrierCreateCRProc glBarrierCreateCR_ptr;
static glBarrierExecCRProc   glBarrierExecCR_ptr;
#endif


#define ASSIGN3V(V, I, X, Y, Z) \
	do { \
		V[(I) * 3 + 0] = X; \
		V[(I) * 3 + 1] = Y; \
		V[(I) * 3 + 2] = Z; \
	} while (0)

#define ASSIGN3N(N, I, X, Y, Z) \
	do { \
		N[(I) * 3 + 0] = (GLbyte) ((X) * 127.0); \
		N[(I) * 3 + 1] = (GLbyte) ((Y) * 127.0); \
		N[(I) * 3 + 2] = (GLbyte) ((Z) * 127.0); \
	} while (0)

#define STITCH(INDICES, ICOUNT) \
	do { \
		INDICES[ICOUNT] = INDICES[ICOUNT - 1]; \
		ICOUNT++; \
	} while (0)


/*
 * Generate geometry for a sphere.  Return a TriangleMesh object.
 * The sphere will be drawn with one, long triangle strip.
 */
static TriangleMesh *
MakeSphere(GLfloat radius, GLuint slices, GLuint stacks)
{
	TriangleMesh *mesh;
	GLuint iPhi, iTheta;
	GLuint iVert, iIndex;

	mesh = (TriangleMesh *) calloc(sizeof(*mesh), 1);

	mesh->NumVerts = 2 + slices * (stacks - 1);
	mesh->NumIndices = stacks * (slices + 1) * 2  /* vertices per stack */
                   + 2 * (stacks - 1);          /* stitching vertices */
	mesh->NumTris = slices * stacks * 2;

	mesh->VertexBytes = mesh->NumVerts * 3 * sizeof(GLfloat);
	mesh->NormalBytes = mesh->NumVerts * 3 * sizeof(GLbyte);

	mesh->Vertices = (GLfloat *) malloc(mesh->VertexBytes);
	mesh->Normals = (GLbyte *) malloc(mesh->NormalBytes);
	mesh->Indices = (GLuint *) malloc(mesh->NumIndices * sizeof(GLuint));
	if (mesh->NumIndices <= 65535)
		mesh->IndexBytes = mesh->NumIndices * sizeof(GLushort);
	else
		mesh->IndexBytes = mesh->NumIndices * sizeof(GLuint);

	/*
	 * Generate Vertices
	 */
	/* bottom (-Z) vertex */
	ASSIGN3V(mesh->Vertices, 0, 0.0, 0.0, radius);
	ASSIGN3N(mesh->Normals, 0, 0, 0, 1);
	/* top (+Z) vertex */
	ASSIGN3V(mesh->Vertices, 1, 0.0, 0.0, -radius);
	ASSIGN3N(mesh->Normals, 1, 0, 0, -1);
	iVert = 2;

	/* intermediate vertices */
	for (iPhi = 1; iPhi < stacks; iPhi++) {
		GLfloat phi = M_PI * (GLfloat) iPhi / (GLfloat) stacks;
		for (iTheta = 0; iTheta < slices; iTheta++) {
			GLfloat theta = 2.0 * M_PI * (GLfloat) iTheta / (GLfloat) slices;
			GLfloat nx = sin(phi) * cos(theta);
			GLfloat ny = sin(phi) * sin(theta);
			GLfloat nz = cos(phi);
			GLfloat vx = radius * nx;
			GLfloat vy = radius * ny;
			GLfloat vz = radius * nz;
			ASSIGN3V(mesh->Vertices, iVert, vx, vy, vz);
			ASSIGN3N(mesh->Normals, iVert, nx, ny, nz);
			iVert++;
		}
	}
	assert(iVert == mesh->NumVerts);

	/*
	 * Generate indices
	 */
	/* bottom (-Z) tri strip */
	iIndex = 0;
	for (iTheta = 0; iTheta <= slices; iTheta++) {
		mesh->Indices[iIndex++] = 0;
		if (iTheta == slices)
			/* last vert in strip == first */
			mesh->Indices[iIndex++] = 2;
		else
			mesh->Indices[iIndex++] = 2 + iTheta;
	}
	/* dummy verts to connect with next strip */
	STITCH(mesh->Indices, iIndex);
	STITCH(mesh->Indices, iIndex);

	/* intermediate tri strips */
	for (iPhi = 0; iPhi < stacks - 2; iPhi++) {
		GLuint k = 2 + iPhi * slices;
		for (iTheta = 0; iTheta <= slices; iTheta++) {
			if (iTheta == slices) {
				/* last verts in strip == first */
				mesh->Indices[iIndex++] = k;
				mesh->Indices[iIndex++] = k + slices;
			}
			else {
				mesh->Indices[iIndex++] = k + iTheta;
				mesh->Indices[iIndex++] = k + iTheta + slices;
			}
		}
		/* dummy verts to connect with next strip */
		STITCH(mesh->Indices, iIndex);
		STITCH(mesh->Indices, iIndex);
	}

	/* top (+Z) tri strip */
	for (iTheta = 0; iTheta <= slices; iTheta++) {
		mesh->Indices[iIndex++] = 1;
		if (iTheta == slices)
			/* last vert in strip == first */
			mesh->Indices[iIndex++] = mesh->NumVerts - slices;
		else
			mesh->Indices[iIndex++] = mesh->NumVerts - slices + iTheta;
	}

	assert(iIndex == mesh->NumIndices);

	if (mesh->NumIndices <= 65535) {
		GLuint i;
		mesh->Indices16 = (GLushort *) malloc(mesh->NumIndices * sizeof(GLushort));
		for (i = 0; i < mesh->NumIndices; i++) {
			mesh->Indices16[i] = mesh->Indices[i];
		}
	}

#if 0
	glGenBuffersARB_ptr(1, &mesh->VertexBufferObj);
	glGenBuffersARB_ptr(1, &mesh->ElementBufferObj);

	glBindBufferARB_ptr(GL_ARRAY_BUFFER_ARB, mesh->VertexBufferObj);
	glBufferDataARB_ptr(GL_ARRAY_BUFFER_ARB,
											mesh->VertexBytes + mesh->NormalBytes,
											NULL, GL_STATIC_DRAW_ARB);
	glBufferSubDataARB_ptr(GL_ARRAY_BUFFER_ARB, 0,
												 mesh->VertexBytes, mesh->Vertices);
	glBufferSubDataARB_ptr(GL_ARRAY_BUFFER_ARB, mesh->VertexBytes,
												 mesh->NormalBytes, mesh->Normals);

	glBindBufferARB_ptr(GL_ELEMENT_ARRAY_BUFFER_ARB, mesh->ElementBufferObj);
	glBufferDataARB_ptr(GL_ELEMENT_ARRAY_BUFFER_ARB, mesh->IndexBytes,
											mesh->Indices, GL_STATIC_DRAW_ARB);

	glBindBufferARB_ptr(GL_ARRAY_BUFFER_ARB, 0);
	glBindBufferARB_ptr(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
#endif

	return mesh;
}


/*
 * Create a vertex buffer object for storing the given mesh.
 */
static void
MeshToVBO(TriangleMesh *mesh)
{
	glGenBuffersARB_ptr(1, &mesh->VertexBufferObj);
	glGenBuffersARB_ptr(1, &mesh->ElementBufferObj);

	glBindBufferARB_ptr(GL_ARRAY_BUFFER_ARB, mesh->VertexBufferObj);
	glBufferDataARB_ptr(GL_ARRAY_BUFFER_ARB,
											mesh->VertexBytes + mesh->NormalBytes,
											NULL, GL_STATIC_DRAW_ARB);
	glBufferSubDataARB_ptr(GL_ARRAY_BUFFER_ARB, 0,
												 mesh->VertexBytes, mesh->Vertices);
	glBufferSubDataARB_ptr(GL_ARRAY_BUFFER_ARB, mesh->VertexBytes,
												 mesh->NormalBytes, mesh->Normals);

	glBindBufferARB_ptr(GL_ELEMENT_ARRAY_BUFFER_ARB, mesh->ElementBufferObj);
	if (mesh->NumIndices <= 65535)
		glBufferDataARB_ptr(GL_ELEMENT_ARRAY_BUFFER_ARB, mesh->IndexBytes,
												mesh->Indices16, GL_STATIC_DRAW_ARB);
	else
		glBufferDataARB_ptr(GL_ELEMENT_ARRAY_BUFFER_ARB, mesh->IndexBytes,
												mesh->Indices, GL_STATIC_DRAW_ARB);

	glBindBufferARB_ptr(GL_ARRAY_BUFFER_ARB, 0);
	glBindBufferARB_ptr(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}


/*
 * Create a display list which stores the given mesh.
 */
static void
MeshToDisplayList(TriangleMesh *mesh)
{
	glVertexPointer(3, GL_FLOAT, 0, mesh->Vertices);
	glNormalPointer(GL_BYTE, 0, mesh->Normals);
	glEnable(GL_VERTEX_ARRAY);
	glEnable(GL_NORMAL_ARRAY);

	mesh->DisplayList = glGenLists(1);
	glNewList(mesh->DisplayList, GL_COMPILE);
	glDrawElements(GL_TRIANGLE_STRIP, mesh->NumIndices,
								 GL_UNSIGNED_INT, mesh->Indices);
	glEndList();

	glDisable(GL_VERTEX_ARRAY);
	glDisable(GL_NORMAL_ARRAY);
}


/*
 * Render the given mesh with specified drawing mode.
 * Return number of triangles drawn.
 */
static GLuint
DrawMesh(const TriangleMesh *mesh, DrawMode drawMode)
{
	if (drawMode == DRAW_VBO) {
		static int first = 1;
		glBindBufferARB_ptr(GL_ARRAY_BUFFER_ARB, mesh->VertexBufferObj);
		if (first) {
			/* This is a bit of a hack.
			 * Apparently, calling glVertex/NormalPointer every time reduces
			 * performance by 25x with (some?) NVIDIA drivers!!!
			 */
			glVertexPointer(3, GL_FLOAT, 0, 0);
			glNormalPointer(GL_BYTE, 0, (void *) mesh->VertexBytes);
			glEnable(GL_VERTEX_ARRAY);
			glEnable(GL_NORMAL_ARRAY);
			first = 0;
		}
		glBindBufferARB_ptr(GL_ELEMENT_ARRAY_BUFFER_ARB, mesh->ElementBufferObj);
		if (mesh->NumIndices <= 65535)
			glDrawRangeElements(GL_TRIANGLE_STRIP, 0, mesh->NumIndices - 1,
													mesh->NumIndices, GL_UNSIGNED_SHORT, 0);
		else
			glDrawRangeElements(GL_TRIANGLE_STRIP, 0, mesh->NumIndices - 1,
													mesh->NumIndices, GL_UNSIGNED_INT, 0);
	}
	else if (drawMode == DRAW_DISPLAY_LIST) {
		glCallList(mesh->DisplayList);
	}
	else {
		assert(drawMode == DRAW_VERTEX_ARRAYS);

		glBindBufferARB_ptr(GL_ARRAY_BUFFER_ARB, 0);
		glBindBufferARB_ptr(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
		glVertexPointer(3, GL_FLOAT, 0, mesh->Vertices);
		glNormalPointer(GL_BYTE, 0, mesh->Normals);
		glEnable(GL_VERTEX_ARRAY);
		glEnable(GL_NORMAL_ARRAY);
		glDrawRangeElements(GL_TRIANGLE_STRIP, 0, mesh->NumIndices - 1,
												mesh->NumIndices, GL_UNSIGNED_INT, mesh->Indices);
	}

	return mesh->NumTris;
}


static TriangleMesh *
MakeDemoMesh(const Options *options)
{
	/* compute sphere parameters to approximate requested triangles/sphere */
	float k = sqrt(options->numTris * 0.5);
	int slices = (int) (k * 1.5);
	int stacks = slices / 2;
	TriangleMesh *mesh = NULL;

	if (slices < 4)
		slices = 4;
	if (stacks < 2)
		stacks = 2;

	mesh = MakeSphere(options->radius, slices, stacks);

	printf("spheres:  %d triangles / sphere\n", mesh->NumTris);
	printf("spheres:  %d indices / sphere\n", mesh->NumIndices);
	printf("spheres:  %d spheres / ring\n", options->numSpheres);
	printf("spheres:  %d triangles / ring\n", mesh->NumTris * options->numSpheres);

	MeshToVBO(mesh);
	MeshToDisplayList(mesh);

	return mesh;
}


static GLuint
DrawFrame(const TriangleMesh *mesh, const Options *options, int frame,
					DrawMode mode)
{
	GLuint tris = 0;
	int i;

	glPushMatrix();
		/* scene rotation */
		glRotatef((GLfloat)frame, 1, 0, 0);
		glRotatef((GLfloat)(-2 * frame), 0, 1, 0);
		glRotatef((GLfloat)frame, 0, 0, 1);

		/* draw a ring of spheres */
		for (i = 0; i < options->numSpheres; i++) {
			glPushMatrix();
				glRotatef(i * options->theta, 0, 0, 1);
				glTranslatef(0.65, 0, options->zPos);
				tris += DrawMesh(mesh, mode);
			glPopMatrix();
		}
	glPopMatrix();

	return tris;
}


static void
Reshape(int w, int h)
{
	WinWidth = w;
	WinHeight = h;

	glViewport(0, 0, WinWidth, WinHeight);

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glFrustum( -1.0, 1.0, -1.0, 1.0, 2.0, 13.0 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glTranslatef( 0.0, 0.0, -3.0 );
}


static void
InitGL(const Options *options)
{
	static const GLfloat white[4] = { 1, 1, 1, 1 };
	static const GLfloat gray[4] = { 0.25, 0.25, 0.25, 1 };
	static const GLfloat pos[4] = { -1, -1, 10, 0 };

	glGenBuffersARB_ptr = (PFNGLGENBUFFERSARBPROC) crGetProcAddress("glGenBuffersARB");
	glBindBufferARB_ptr = (PFNGLBINDBUFFERARBPROC) crGetProcAddress("glBindBufferARB");
	glBufferDataARB_ptr = (PFNGLBUFFERDATAARBPROC) crGetProcAddress("glBufferDataARB");
	glBufferSubDataARB_ptr = (PFNGLBUFFERSUBDATAARBPROC) crGetProcAddress("glBufferSubDataARB");

	glEnable( GL_DEPTH_TEST );
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, gray);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, gray);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10.0);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,
							 colors[options->rank % 7]);
	glEnable(GL_NORMALIZE);
	glClearColor(0.5, 0.5, 0.5, 0.0);

	Reshape(WinWidth, WinHeight);
}


static void
PrintHelp(void)
{
	printf("Usage: spheres [options]\n");
	printf("Options:\n");
	printf("  -size N   specify number of parallel instances\n");
	printf("  -rank I   specifies instance's rank in [0..N-1]\n");
	printf("  -swap     do SwapBuffers\n");
	printf("  -clear    do glClear\n");
	printf("  -s N      specifies number of spheres per frame\n");
	printf("  -t N      specifies number of triangles per sphere\n");
	printf("  -f N      specifies number of frames to render\n");
	printf("  -h        print this information\n");
}


static void
ParseOptions(int argc, char *argv[], Options *options)
{
	int i;

	options->name = argv[0];
	options->size = 1;
	options->rank = 0;
	options->swapFlag = 0;
	options->clearFlag = 0;
	options->numSpheres = 12;
	options->numTris = 12000;
	options->maxFrames = 1000/* * 1000 * 1000*/;
	options->useVBO = 1;

	for (i = 1 ; i < argc ; i++)
	{
		if (!strcmp( argv[i], "-rank" ))
		{
			if (i == argc - 1)
			{
				fprintf(stderr, "-rank requires an argument" );
				exit(1);
			}
			options->rank = atoi( argv[i+1] );
			i++;
		}
		else if (!strcmp( argv[i], "-size" ))
		{
			if (i == argc - 1)
			{
				fprintf(stderr, "-size requires an argument" );
				exit(1);
			}
			options->size = atoi( argv[i+1] );
			i++;
		}
		else if (!strcmp( argv[i], "-swap" ))
		{
			options->swapFlag = 1;
		}
		else if (!strcmp( argv[i], "-clear" ))
		{
			options->clearFlag = 1;
		}
		else if (!strcmp( argv[i], "-s" ))
		{
			if (i == argc - 1)
			{
				fprintf(stderr, "-t requires an argument" );
				exit(1);
			}
			options->numSpheres = atoi(argv[i+1]);
			i++;
		}
		else if (!strcmp( argv[i], "-t" ))
		{
			if (i == argc - 1)
			{
				fprintf(stderr, "-t requires an argument" );
				exit(1);
			}
			options->numTris = atoi(argv[i+1]);
			i++;
		}
		else if (!strcmp( argv[i], "-f" ))
		{
			if (i == argc - 1)
			{
				fprintf(stderr, "-f requires an argument" );
				exit(1);
			}
			options->maxFrames = atoi(argv[i+1]);
			i++;
		}
		else if (!strcmp( argv[i], "-v" )) {
			DrawingMode = DRAW_VBO;
		}
		else if (!strcmp( argv[i], "-d" )) {
			DrawingMode = DRAW_DISPLAY_LIST;
		}
		else if (!strcmp( argv[i], "-h" ) || !strcmp(argv[i], "--help"))
		{
			PrintHelp();
			exit(0);
		}
	} 

	if (options->size == 1) {
		options->swapFlag = 1;
		options->clearFlag = 1;
	}

	options->theta = 360.0 / (float) options->numSpheres;
	options->radius = options->theta * 0.005;
	if (options->radius > 0.3)
		options->radius = 0.3;
	options->zPos = 1.0 * (2.0 * options->rank * options->radius
												 - (options->size * options->radius));
}


#if USE_CHROMIUM
static void
ChromiumMain(const Options *options)
{
	int ctx, frame;
	int window = 0;  /* default window */
	const char *dpy = NULL;
	int visual = CR_RGB_BIT | CR_DEPTH_BIT | CR_DOUBLE_BIT;
	CRTimer *timer;
	int drawnTris;
	double t0;
	int frameCount;
	TriangleMesh *mesh;

#define LOAD( x ) x##_ptr = (x##Proc) crGetProcAddress( #x )

	LOAD( crCreateContext );
	LOAD( crMakeCurrent );
	LOAD( crSwapBuffers );
	LOAD( glChromiumParametervCR );
	LOAD( glGetChromiumParametervCR );
	LOAD( glBarrierCreateCR );
	LOAD( glBarrierExecCR );

	printf("spheres:  node %d of %d total\n", options->rank, options->size);

	timer = crTimerNewTimer();

	ctx = crCreateContext_ptr(dpy, visual);
	if (ctx < 0) {
		crError("glCreateContextCR() call failed!\n");
	}
	crMakeCurrent_ptr(window, ctx);

	/* Test getting window size */
	{
		GLint winsize[2];
		glGetChromiumParametervCR_ptr(GL_WINDOW_SIZE_CR, 0, GL_INT, 2, winsize);
	}

	/* It's OK for everyone to create this, as long as all the "size"s match */
	glBarrierCreateCR_ptr( MASTER_BARRIER, options->size );

	InitGL(options);

	mesh = MakeDemoMesh(options);

	frameCount = 0;
	drawnTris = 0;
	crStartTimer(timer);
	t0 = crTimerTime(timer);

	printf("spheres:  Drawing Mode: %s\n", DrawModeString[DrawingMode]);

	for (frame = 0; frame < options->maxFrames; frame++)
	{
		double t;

		if (options->clearFlag)
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		if (options->size > 1)
			 glBarrierExecCR_ptr( MASTER_BARRIER );

		drawnTris += DrawFrame(mesh, options, frame, DrawingMode);

		if (options->size > 1)
			 glBarrierExecCR_ptr( MASTER_BARRIER );

		/* All clients need to call crSwapBuffers() to indicate end-of-frame.
		 * However, the server's window should only do one real swapbuffers for
		 * N clients (not N swaps for N clients!)
		 * We can achieve this in one of two ways:
		 * 1. Have all clients issue a normal SwapBuffers and set the
		 *    crserver's only_swap_once config flag.  The server will then ensure
		 *    that only one client's SwapBuffers message gets through to the
		 *    render SPU.
		 * 2. Don't set only_swap_once:  explicitly control swapping in the
		 *    clients with the CR_SUPPRESS_SWAP_BIT flag.  All but one client
		 *    should pass this flag to crSwapBuffers.  The CR_SUPPRESS_SWAP_BIT
		 *    tells the render SPU to no-op the swap.  Thus, crSwapBuffers can
		 *    be used to indicate end-of-frame without swapping the color buffers.
		 */

		if (options->swapFlag) {
			/* really swap */
			crSwapBuffers_ptr( window, 0 );
		}
		else {
			/* don't really swap, just mark end of frame */
			crSwapBuffers_ptr( window, CR_SUPPRESS_SWAP_BIT );
		}

		/* Compute performance figures */
		frameCount++;
		t = crTimerTime(timer);
		if (t - t0 > 3.0) {
			/* get current window size */
			GLint winsize[2];
			glGetChromiumParametervCR_ptr(GL_WINDOW_SIZE_CR, 0, GL_INT, 2, winsize);
			WinWidth = winsize[0];
			WinHeight = winsize[1];
			printf("spheres:  %.02g fps   %.04g tris/sec   %.04g pixels/sec\n",
						 frameCount / (t - t0),
						 drawnTris / (t - t0),
						 WinWidth * WinHeight * frameCount / (t - t0));
			t0 = t;
			drawnTris = 0;
			frameCount = 0;
		}
	}
}


#else /* USE_CHROMIUM */

static GLuint CurrentFrame = 0;
static TriangleMesh *TheMesh = NULL;
static const Options *TheOptions = NULL;


static void Idle(void)
{
	CurrentFrame++;
	glutPostRedisplay();
}


static void DisplayFrame(void)
{
	static double t0 = 0, t1 = 0;
	static int drawnTris = 0;

	if (t0 == 0) {
		t0 = glutGet(GLUT_ELAPSED_TIME) * 0.001;
	}

	assert(TheMesh);

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	drawnTris += DrawFrame(TheMesh, TheOptions, CurrentFrame, DrawingMode);

	glutSwapBuffers();

	t1 = glutGet(GLUT_ELAPSED_TIME) * 0.001;
	if (t1 - t0 > 3.0) {
		printf("spheres:  %.02g fps   %.04g tris/sec   %.04g pixels/sec\n",
					 frameCount / (t1 - t0),
					 drawnTris / (t1 - t0),
					 WinWidth * WinHeight * frameCount / (t1 - t0));
		t0 = t1;
		drawnTris = 0;
	}
}


static void
Key( unsigned char key, int x, int y )
{
	(void) x;
	(void) y;
	switch (key) {
	case 'm':
		DrawingMode = (DrawingMode + 1) % 3;
		printf("spheres:  DrawMode: %s\n", DrawModeString[DrawingMode]);
		break;
	case 27:
		exit(0);
		break;
	}
	glutPostRedisplay();
}


static void
GlutMain(const Options *options)
{
	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( WinWidth, WinHeight );
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
	glutCreateWindow(options->name);
	glutReshapeFunc( Reshape );
	glutKeyboardFunc( Key );
	/*glutSpecialFunc( SpecialKey );*/
	glutDisplayFunc( DisplayFrame );
	glutIdleFunc( Idle );

	InitGL(options);

	TheMesh = MakeDemoMesh(options);
	TheOptions = options;

	printf("spheres:  Press 'm' to change rendering mode\n");
	glutMainLoop();
}



#endif /* USE_CHROMIUM */



int
main(int argc, char *argv[])
{
	Options options;

#if USE_CHROMIUM
	ParseOptions(argc, argv, &options);
	ChromiumMain(&options);
#else
	glutInit(&argc, argv);
	ParseOptions(argc, argv, &options);
	GlutMain(&options);
#endif

	return 0;
}
