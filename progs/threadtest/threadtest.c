/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*
 * Example of multi-thread rendering.
 * Brian Paul
 * 26 Feb 2002
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>  /* for sleep() */
#include <pthread.h>
#endif
#include "cr_applications.h"
#include "cr_string.h"
#include "cr_error.h"

#ifndef M_PI
# define M_PI		3.14159265358979323846	/* pi */
#endif

float colors[7][4] = {
	{1,0,0,1},
	{0,1,0,1},
	{0,0,1,1},
	{0,1,1,1},
	{1,0,1,1},
	{1,1,0,1},
	{1,1,1,1}
};

static const int MASTER_BARRIER = 100;


#define LOAD( x ) gl##x##CR = (cr##x##Proc) crGetProcAddress( "cr"#x )
#define LOAD2( x ) gl##x##CR = (gl##x##CRProc) crGetProcAddress( "gl"#x )

crCreateContextProc   glCreateContextCR;
crMakeCurrentProc     glMakeCurrentCR;
crSwapBuffersProc     glSwapBuffersCR;
crCreateWindowProc    glCreateWindowCR;
crWindowPositionProc  glWindowPositionCR;

glBarrierCreateCRProc glBarrierCreateCR;
glBarrierExecCRProc   glBarrierExecCR;

#define MAXIMUM_THREADS 10

struct context_t {
	int Context;
	int Window;
	int Rank;
	GLboolean Clear;
	GLint SwapFlags;
};

static int NumThreads = 1;
#ifdef WINDOWS
static DWORD ThreadID[MAXIMUM_THREADS];
#else
static pthread_t ThreadID[MAXIMUM_THREADS];
#endif
static struct context_t Context[MAXIMUM_THREADS];
static GLboolean ExitFlag = GL_FALSE;


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


static void *render_loop( void *threadData )
{
	static const GLfloat white[4] = { 1, 1, 1, 1 };
	static const GLfloat gray[4] = { 0.25, 0.25, 0.25, 1 };
	static const GLfloat pos[4] = { -1, -1, 10, 0 };
	float theta;
	struct context_t *context = (struct context_t *) threadData;
	int frame, rank;

	rank = context->Rank;

	glMakeCurrentCR(context->Window, context->Context);

	LOAD2( BarrierCreate );
	LOAD2( BarrierExec );
	if (!glBarrierCreateCR)
		 crError("BarrierCreateCR is NULL!");
	if (!glBarrierExecCR)
		 crError("BarrierExecCR is NULL!");
	/* It's OK for everyone to create this, as long as all the "size"s match */
	glBarrierCreateCR( MASTER_BARRIER, NumThreads );

	theta = (float) (360.0 / (float) NumThreads);

	glEnable(GL_DEPTH_TEST);
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
	glFrustum( -1.0, 1.0, -1.0, 1.0, 7.0, 13.0 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glTranslatef( 0.0, 0.0, -10.0 );
	/*	glClearColor(0.3, 0.3, 0.3, 0.0);*/

	for (frame = 0; frame < 1000; frame++) {
		if (context->Clear) {
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		}

		glBarrierExecCR( MASTER_BARRIER );

		glPushMatrix();
		glRotatef((GLfloat)frame, 1, 0, 0);
		glRotatef((GLfloat)(-2 * frame), 0, 1, 0);
		glRotatef((GLfloat)(frame + rank * theta), 0, 0, 1);

		glTranslatef(0.5, 0, 0);
		glRotatef(18, 1, 0, 0);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, colors[rank%7]);
		doughnut((GLfloat).15,(GLfloat)0.7, 15, 30);

		glPopMatrix();

		glBarrierExecCR( MASTER_BARRIER );

		/* The crserver only executes the SwapBuffers() for the 0th client.
		 * No need to test for rank==0 as we used to do.
		 */
		glSwapBuffersCR(context->Window, context->SwapFlags);
	}

	ExitFlag = GL_TRUE;
	return NULL;
}


extern void StubInit(void);

int main(int argc, char *argv[])
{
	const GLint visual = CR_RGB_BIT | CR_DEPTH_BIT | CR_DOUBLE_BIT;
	void *dpy = NULL;
	GLboolean multiWindow = GL_FALSE;
	GLboolean swapSingle = GL_FALSE;
	int i;

#ifndef WINDOWS
	XInitThreads();
#endif

	if (argc < 3)
	{
		crError( "Usage: %s [-t <num threads>] [-w] [-s1]", argv[0] );
	}

	for (i = 1 ; i < argc ; i++)
	{
		if (!crStrcmp( argv[i], "-t" ))
		{
			if (i == argc - 1)
			{
				crError( "-t requires an argument" );
			}
			NumThreads = crStrToInt( argv[i+1] );
			i++;
		}
		else if (!crStrcmp( argv[i], "-w" ))
		{
			multiWindow = GL_TRUE;
		}
		else if (!crStrcmp( argv[i], "-s1" ))
		{
			swapSingle = GL_TRUE;
		}
	}

	if (NumThreads < 1) {
		crError( "1 or more threads required" );
	}
	if (NumThreads > MAXIMUM_THREADS) {
		crError( "%d threads is the limit", MAXIMUM_THREADS);
	}

	LOAD( CreateContext );
	LOAD( MakeCurrent );
	LOAD( SwapBuffers );
	LOAD( CreateWindow );
	LOAD( WindowPosition );

	Context[0].Window = glCreateWindowCR(dpy, visual);
	/*	Context[0].Context = glCreateContextCR(dpy, visual);*/
	if (Context[0].Window < 0) {
		 crError("Failed to create 0th window!\n");
		 return 0;
	}
	Context[0].Clear = GL_TRUE;
	Context[0].SwapFlags = 0;
	glWindowPositionCR(Context[0].Window, 20, 20);

	/* Create a context for each thread */
	for (i = 0; i < NumThreads; i++) {
		if (i > 0) {
			if (multiWindow) {
				/* a new window */
				Context[i].Window = glCreateWindowCR(dpy, visual);
				if (Context[i].Window < 0) {
					crError("glCreateWindowCR() failed!\n");
					return 0;
				}
				glWindowPositionCR(Context[i].Window, 420*i, 20);
				Context[i].Clear = GL_TRUE;
			}
			else {
				/* share one window */
				Context[i].Window = Context[0].Window;
				Context[i].Clear = GL_FALSE;
			}

			if (swapSingle) {
				Context[i].SwapFlags = CR_SUPPRESS_SWAP_BIT;
			}
			else {
				Context[i].SwapFlags = 0;
			}
		}
		Context[i].Rank = i;
		Context[i].Context = glCreateContextCR(dpy, visual);
		if (Context[i].Context < 0) {
			crError("glCreateContextCR() call for thread %d failed!\n", i);
			return 0;
		}
	}

	printf("------- thread test ---------\n");
	for (i = 0; i < NumThreads; i++) {
		printf("Thread %d: win=%d ctx=%d clear=%d swap=%d\n", i, Context[i].Window,
					 Context[i].Context, Context[i].Clear, Context[i].SwapFlags);
	}
	printf("-----------------------------\n");

	/* create the threads! */
	for (i = 0; i < NumThreads; i++) {
#ifdef WINDOWS
		CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) render_loop, &(Context[i]), 0, &ThreadID[i] );
#else
		pthread_create( &ThreadID[i], NULL, render_loop, (void *) &(Context[i]) );
#endif
		printf("threadtest created thread %d (id=%d)\n", i, (int) ThreadID[i]);
	}

	/* main thread waits here until it's time to exit */
	while (!ExitFlag) {
#ifdef WINDOWS
		/* XXX */
#else
		sleep(1);
#endif
	}

	/* wait for threads to finish */
	for (i = 0; i < NumThreads; i++) {
#ifdef WINDOWS
		/* XXX */
#else
		pthread_join(ThreadID[i], NULL);
#endif
	}

	printf("threadtest exiting\n");

	return 0;
}
