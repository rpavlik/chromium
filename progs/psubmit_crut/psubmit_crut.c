/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <math.h>
#include <stdlib.h>
#include "chromium.h"
#include "crut_api.h"
#include "crut_clientapi.h"
#include "cr_error.h"
#include "cr_string.h"


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

static crCreateContextProc crCreateContext_ptr;
static crMakeCurrentProc   crMakeCurrent_ptr;
static crSwapBuffersProc   crSwapBuffers_ptr;

static glChromiumParametervCRProc glChromiumParametervCR_ptr;
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

static const GLfloat white[4] = { 1, 1, 1, 1 };
static const GLfloat gray[4] = { 0.25, 0.25, 0.25, 1 };
static const GLfloat pos[4] = { -1, -1, 10, 0 };
static int rank = -1, size=-1;
static int i, ctx;
static int frame = 0;
static int window;
static float theta;
static int swapFlag = 0, clearFlag = 0;
static int visual = CR_RGB_BIT | CR_DEPTH_BIT | CR_DOUBLE_BIT;
static GLfloat r = (GLfloat) 0.15;
static GLfloat R = (GLfloat) 0.7;
static GLint sides = 15;
static GLint rings = 30;
static int events;

static void 
mouse( int button, int state, int x, int y ) 
{

    if (button == CRUT_LEFT_BUTTON && state == CRUT_DOWN)
	r += 0.02f;
    if (button == CRUT_RIGHT_BUTTON && state == CRUT_DOWN)
	r -= 0.02f;
    events++;
    printf("events: %i\n", events);
}

static void
keyboard( unsigned char key, int x, int y )
{

    if (key == 'a')
	sides++;
    else if (key == 'z')
	sides--;
    else if (key == 's')
	size++;
    else if (key == 'x')
	size--;
    else if (key == 27)
        exit(0);
    events++;
    printf("events: %i\n", events);
}

static void 
motion( int x, int y )
{
  events++;
  printf("events: %i\n", events);
}

#if 0
static void 
passive_motion( int x, int y )
{
  events++;
  printf("events: %i\n", events); 
}
#endif

static void
DisplayRings(void)
{
 
    /*
      static const GLfloat boundingBox[6] = {
      -0.85, 0.85, -0.85, 0.85, -0.7, 0.7
      };
    */
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
    /*glChromiumParametervCR_ptr(GL_OBJECT_BBOX_CR, GL_FLOAT, 6, boundingBox);*/
    doughnut(r,R, sides, rings);
    
    glPopMatrix();
    
    glBarrierExecCR_ptr( MASTER_BARRIER );
    
    /* The crserver only executes the SwapBuffers() for the 0th client.
     * No need to test for rank==0 as we used to do.
     */
    
    if (swapFlag) {
	/* really swap */
	crSwapBuffers_ptr( window, 0 );
    }
    else {
	/* don't really swap, just mark end of frame */
	crSwapBuffers_ptr( window, CR_SUPPRESS_SWAP_BIT );
    }	
    frame++;
    crutPostRedisplay();
}

static void changeRank( int value ) 
{
    switch (value)
    {
    case 1:
	rank++;
	break;
    case 2:
	rank--;
	break;
    }
}

static void changeSize( int value ) 
{
    switch (value)
    {
    case 1:
	r += 0.02f;
	break;
    case 2:
	r -= 0.02f;
	break;
    }
}

static void changeSides( int value ) 
{
    crDebug(" func 1 being called, value is %i", value);
    switch (value)
    {
    case 1:
	sides++;
	break;
    case 2:
	sides--;
	break;
    }
}

static void menu_func(int value)
{
 
}

int main(int argc, char *argv[])
{
    int menu1, menu2, menu3;
    
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
		else if (!crStrcmp( argv[i], "-swap" ))
		{
			swapFlag = 1;
		}
		else if (!crStrcmp( argv[i], "-clear" ))
		{
			clearFlag = 1;
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

	crDebug("initializing");
		
	ctx = crutCreateContext(visual);  
	window = crutCreateWindow(visual);
	
	crutDisplayFunc(DisplayRings);
	crutMouseFunc(mouse);
	crutKeyboardFunc(keyboard);
	crutMotionFunc(motion);
	
	menu1 = crutCreateMenu(changeRank);
	crutAddMenuEntry("Rank++", 1);
	crutAddMenuEntry("Rank--", 2);
	
	menu2 = crutCreateMenu(changeSize);
	crutAddMenuEntry("Size++", 1);
	crutAddMenuEntry("Size--", 2);

	menu3 = crutCreateMenu(changeSides);
	crutAddMenuEntry("Sides++", 1);
	crutAddMenuEntry("Sides--", 2);
	
	crutCreateMenu(menu_func);
	crutAddSubMenu("Change Rank", menu1);
	crutAddSubMenu("Change Size", menu2);
	crutAddSubMenu("Change Vertices", menu3);
	crutAttachMenu(CRUT_RIGHT_BUTTON);
	

	/* crutPassiveMotionFunc(passive_motion); */
	crutInitClient();

#define LOAD( x ) x##_ptr = (x##Proc) crGetProcAddress( #x )

	LOAD( crCreateContext );
	LOAD( crMakeCurrent );
	LOAD( crSwapBuffers );
	LOAD( glChromiumParametervCR );
	LOAD( glBarrierCreateCR );
	LOAD( glBarrierExecCR );

	crMakeCurrent_ptr(window, ctx);

	/* It's OK for everyone to create this, as long as all the "size"s match */
	glBarrierCreateCR_ptr( MASTER_BARRIER, size );

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

	glClearColor(0.0, 0.0, 0.0, 1.0);

	crutMainLoop();
    
	/* ARGH -- need to trick out the compiler this sucks. */
	if (argv[0] == NULL)
	{
	    return 0;
	}

	return 0;

}
