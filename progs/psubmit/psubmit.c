/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "chromium.h"
#include "cr_string.h"
#include "cr_error.h"

static GLfloat colors[7][4] = {
	{1,0,0,1},
	{0,1,0,1},
	{0,0,1,1},
	{0,1,1,1},
	{1,0,1,1},
	{1,1,0,1},
	{1,1,1,1}
};

static const int MASTER_BARRIER = 100;

static crCreateContextProc crCreateContext_ptr;
static crMakeCurrentProc   crMakeCurrent_ptr;
static crSwapBuffersProc   crSwapBuffers_ptr;

static glChromiumParametervCRProc glChromiumParametervCR_ptr;
static glGetChromiumParametervCRProc glGetChromiumParametervCR_ptr;
static glBarrierCreateCRProc glBarrierCreateCR_ptr;
static glBarrierExecCRProc   glBarrierExecCR_ptr;

#ifndef M_PI
# define M_PI		3.14159265358979323846	/* pi */
#endif

/* Borrowed from GLUT */
static void
doughnut(GLfloat r, GLfloat R, GLint nsides, GLint rings)
{
	int i, j;
	GLfloat theta, phi, theta1;
	GLfloat cosTheta, sinTheta;
	GLfloat cosTheta1, sinTheta1;
	GLfloat ringDelta, sideDelta;

	ringDelta = (GLfloat)(2.0 * M_PI / rings);
	sideDelta = (GLfloat)(2.0 * M_PI / nsides);

	theta = 0.0;
	cosTheta = 1.0;
	sinTheta = 0.0;
	for (i = rings - 1; i >= 0; i--) {
		theta1 = theta + ringDelta;
		cosTheta1 = (GLfloat)cos(theta1);
		sinTheta1 = (GLfloat)sin(theta1);
		glBegin(GL_QUAD_STRIP);
		phi = 0.0;
		for (j = nsides; j >= 0; j--) {
			GLfloat cosPhi, sinPhi, dist;

			phi += sideDelta;
			cosPhi = (GLfloat)cos(phi);
			sinPhi = (GLfloat)sin(phi);
			dist = R + r * cosPhi;

			glNormal3f(cosTheta1 * cosPhi, -sinTheta1 * cosPhi, sinPhi);
			glVertex3f(cosTheta1 * dist, -sinTheta1 * dist, r * sinPhi);
			glNormal3f(cosTheta * cosPhi, -sinTheta * cosPhi, sinPhi);
			glVertex3f(cosTheta * dist, -sinTheta * dist,  r * sinPhi);
		}
		glEnd();
		theta = theta1;
		cosTheta = cosTheta1;
		sinTheta = sinTheta1;
	}
}


static void
drawBBox(const GLfloat bbox[6])
{
	const GLfloat x0 = bbox[0], y0 = bbox[1], z0 = bbox[2];
	const GLfloat x1 = bbox[3], y1 = bbox[4], z1 = bbox[5];
	 
	glColor3f(1, 1, 1);
	glBegin(GL_LINE_LOOP);
	glVertex3f(x0, y0, z0);
	glVertex3f(x1, y0, z0);
	glVertex3f(x1, y1, z0);
	glVertex3f(x0, y1, z0);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3f(x0, y0, z1);
	glVertex3f(x1, y0, z1);
	glVertex3f(x1, y1, z1);
	glVertex3f(x0, y1, z1);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(x0, y0, z0);	 glVertex3f(x0, y0, z1);
	glVertex3f(x1, y0, z0);	 glVertex3f(x1, y0, z1);
	glVertex3f(x1, y1, z0);	 glVertex3f(x1, y1, z1);
	glVertex3f(x0, y1, z0);	 glVertex3f(x0, y1, z1);
	glEnd();
}



int main(int argc, char *argv[])
{
	static const GLfloat white[4] = { 1, 1, 1, 1 };
	static const GLfloat gray[4] = { 0.25, 0.25, 0.25, 1 };
	static const GLfloat pos[4] = { -1, -1, 10, 0 };
	int rank = -1, size = -1, barrierSize = -1;
	int i, ctx, frame;
	int window = 0;  /* default window */
	float theta;
	int swapFlag = 0, clearFlag = 0;
	const char *dpy = NULL;
	int visual = CR_RGB_BIT | CR_DEPTH_BIT | CR_DOUBLE_BIT;
	int triangles = 15 * 30 * 2;
	int sides, rings;
	float aspectRatio;
	int useBBox = 0;

	if (argc < 5)
	{
		crError( "Usage: %s -rank <ID NUMBER> -size <SIZE> [-swap]", argv[0] );
	}

	for (i = 1 ; i < argc ; i++)
	{
		if (!crStrcmp( argv[i], "-rank" ))
		{
			if (i == argc - 1)
			{
				crError( "-rank requires an argument" );
			}
			rank = crStrToInt( argv[i+1] );
			i++;
		}
		else if (!crStrcmp( argv[i], "-size" ))
		{
			if (i == argc - 1)
			{
				crError( "-size requires an argument" );
			}
			size = crStrToInt( argv[i+1] );
			i++;
		}
		else if (!crStrcmp( argv[i], "-barrier" ))
		{
			if (i == argc - 1)
			{
				crError( "-barrier requires an argument" );
			}
			barrierSize = crStrToInt( argv[i+1] );
			i++;
		}
		else if (!crStrcmp( argv[i], "-swap" ))
		{
			swapFlag = 1;
		}
		else if (!crStrcmp( argv[i], "-clear" ))
		{
			clearFlag = 1;
		}
		else if (!crStrcmp( argv[i], "-t")) {
			if (i == argc - 1)
			{
				crError( "-t (triangles) requires an argument" );
			}
			triangles = crStrToInt( argv[i+1] );
			i++;
		}
		else if (!crStrcmp( argv[i], "-bbox" ))
		{
			useBBox = 1;
		}
	} 

	if (rank == -1)
	{
		crError( "Rank not specified" );
	}
	if (size == -1)
	{
		crError( "Size not specified" );
	}
	if (rank >= size || rank < 0)
	{
		crError( "Bogus rank: %d (size = %d)", rank, size );
	}
	if (barrierSize < 0)
		barrierSize = size;

	printf("psubmit: %d triangles/ring\n", triangles);

	/* Note: rings * sides * 2 ~= triangles */
	rings = (int) sqrt(triangles / 4);
	sides = rings * 2;

#define LOAD( x ) x##_ptr = (x##Proc) crGetProcAddress( #x )

	LOAD( crCreateContext );
	LOAD( crMakeCurrent );
	LOAD( crSwapBuffers );
	LOAD( glChromiumParametervCR );
	LOAD( glGetChromiumParametervCR );
	LOAD( glBarrierCreateCR );
	LOAD( glBarrierExecCR );

	ctx = crCreateContext_ptr(dpy, visual);
	if (ctx < 0) {
		crError("glCreateContextCR() call failed!\n");
		return 0;
	}

	crMakeCurrent_ptr(window, ctx);

	/* Test getting window size */
	{
		GLint winsize[2];
		glGetChromiumParametervCR_ptr(GL_WINDOW_SIZE_CR, 0, GL_INT, 2, winsize);
		printf("psubmit using window size: %d x %d\n", winsize[0], winsize[1]);
		glViewport(0, 0, winsize[0], winsize[1]);
		aspectRatio = (float) winsize[0] / (float) winsize[1];
	}

	/* It's OK for everyone to create this, as long as all the "size"s match */
	glBarrierCreateCR_ptr( MASTER_BARRIER, barrierSize );

	theta = (float)(360.0 / (float) size);

	glEnable( GL_DEPTH_TEST );
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, gray);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, gray);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10.0);
	glEnable(GL_NORMALIZE);

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glFrustum( -aspectRatio, aspectRatio, -1.0, 1.0, 7.0, 13.0 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glTranslatef( 0.0, 0.0, -10.0 );

	for (frame = 0; ; frame++)
	{
		const GLfloat innerRadius = 0.15;
		const GLfloat outerRadius = 0.70;
		GLfloat bbox[6];

		if (clearFlag)
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		glBarrierExecCR_ptr( MASTER_BARRIER );

		glPushMatrix();
		glRotatef((GLfloat)frame, 1, 0, 0);
		glRotatef((GLfloat)(-2 * frame), 0, 1, 0);
		glRotatef((GLfloat)(frame + rank * theta), 0, 0, 1);

		glTranslatef(0.5, 0, 0);
		glRotatef(18, 1, 0, 0);

		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, colors[rank%7]);

		if (useBBox) {
			bbox[0] = -(innerRadius + outerRadius);
			bbox[1] = -(innerRadius + outerRadius);
			bbox[2] = -innerRadius;
			bbox[3] = +(innerRadius + outerRadius);
			bbox[4] = +(innerRadius + outerRadius);
			bbox[5] = +innerRadius;
			glChromiumParametervCR_ptr(GL_OBJECT_BBOX_CR, GL_FLOAT, 6, bbox);
			drawBBox(bbox);
		}
		doughnut(innerRadius, outerRadius, sides, rings);

		glPopMatrix();

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

		if (swapFlag) {
			/* really swap */
			crSwapBuffers_ptr( window, 0 );
		}
		else {
			/* don't really swap, just mark end of frame */
			crSwapBuffers_ptr( window, CR_SUPPRESS_SWAP_BIT );
		}

		/* ARGH -- need to trick out the compiler this sucks. */
		if (argv[0] == NULL)
		{
			return 0;
		}
	}
}
