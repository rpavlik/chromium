/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "cr_applications.h"
#include "cr_string.h"
#include "cr_error.h"

float verts[3][3] = {
	{ -1, 1, -1 },
	{ -1, -1, 0 },
	{ .5, 0, 1 }
};

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

crCreateContextProc crCreateContextCR;
crMakeCurrentProc   crMakeCurrentCR;
crSwapBuffersProc   crSwapBuffersCR;

glBarrierCreateProc glBarrierCreateCR;
glBarrierExecProc   glBarrierExecCR;

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



int main(int argc, char *argv[])
{
	static const GLfloat white[4] = { 1, 1, 1, 1 };
	static const GLfloat gray[4] = { 0.25, 0.25, 0.25, 1 };
	static const GLfloat pos[4] = { -1, -1, 10, 0 };
	int rank = -1, size=-1;
	int i, ctx, frame;
	float theta;

	if (argc < 5)
	{
		crError( "Usage: %s -rank <ID NUMBER> -size <SIZE>", argv[0] );
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
		if (!crStrcmp( argv[i], "-size" ))
		{
			if (i == argc - 1)
			{
				crError( "-size requires an argument" );
			}
			size = crStrToInt( argv[i+1] );
			i++;
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

#define LOAD( x ) x##CR = (x##Proc) crGetProcAddress( #x )

	LOAD( crCreateContext );
	LOAD( crMakeCurrent );
	LOAD( crSwapBuffers );

	ctx = crCreateContextCR(0, CR_RGB_BIT | CR_DEPTH_BIT | CR_DOUBLE_BIT);
	if (ctx <= 0) {
		crError("crCreateContextCR() call failed!\n");
		return 0;
	}
	crMakeCurrentCR(0, 0, ctx);

	LOAD( glBarrierCreate );
	LOAD( glBarrierExec );

	/* It's OK for everyone to create this, as long as all the "size"s match */
	glBarrierCreateCR( MASTER_BARRIER, size );

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
	glFrustum( -1.0, 1.0, -1.0, 1.0, 7.0, 13.0 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glTranslatef( 0.0, 0.0, -10.0 );


	for (frame = 0; ; frame++)
	{
		/* The GL_SINGLE_CLIENT_BIT_CR bit indicates that the glClear() should
		 * only be executed by the server for the 0th client.
		 * This really only matters for a sort-last configuration.
		 */
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
						 | GL_SINGLE_CLIENT_BIT_CR );

		glBarrierExecCR( MASTER_BARRIER );

		glPushMatrix();
		glRotatef((GLfloat)frame, 1, 0, 0);
		glRotatef((GLfloat)(-2 * frame), 0, 1, 0);
		glRotatef((GLfloat)(frame + rank * theta), 0, 0, 1);
#if 0
		/* old code */
		glBegin( GL_TRIANGLES );
		glColor3fv(colors[rank%7]);
		glVertex3fv(verts[0]);
		glVertex3fv(verts[1]);
		glVertex3fv(verts[2]);
		glEnd();
#else
		glTranslatef(0.5, 0, 0);
		glRotatef(18, 1, 0, 0);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, colors[rank%7]);
		doughnut((GLfloat).15,(GLfloat)0.7, 15, 30);
#endif
		glPopMatrix();

		glBarrierExecCR( MASTER_BARRIER );

		/* The crserver only executes the SwapBuffers() for the 0th client.
		 * No need to test for rank==0 as we used to do.
		 */
		crSwapBuffersCR(0, 0);

		/* ARGH -- need to trick out the compiler this sucks. */
		if (argv[0] == NULL)
		{
			return 0;
		}
	}
}
