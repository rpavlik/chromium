
/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */



#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "chromium.h"
#include "cr_error.h"
#include "packertest.h"

extern int errChk;
extern int verbose;
void printError(char *name);


static int frontbuffer = 1;

/***************************************************************************/

static void init_test01(void)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-0.5, 639.5, -0.5, 479.5);
    glMatrixMode(GL_MODELVIEW);

    glShadeModel(GL_FLAT);
    glDisable(GL_DEPTH_TEST);

    glClearColor(0.0, 0.1, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0, 0.0, 0.0);
}

static int test01(int size, int num)
{
    int x, y;

    glBegin(GL_POINTS);
#if 0
    if (errChk)
	printError("glBegin(GL_POINTS)");
#endif
    if (verbose)
	crDebug("glBegin( GL_POINTS )");
    for (y = 0; y < num; y++)
	for (x = 0; x < 480; x++)
	    glVertex2i(x, x);
    glEnd();

    return 480 * num;
}

/***************************************************************************/

static void init_test02(void)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-0.5, 639.5, -0.5, 479.5);
    glMatrixMode(GL_MODELVIEW);

    glShadeModel(GL_SMOOTH);
    glDisable(GL_DEPTH_TEST);

    glClearColor(0.0, 0.1, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
}

static int test02(int size, int num)
{
    int x, y;

    glBegin(GL_LINES);
#if 0
    if (errChk)
	printError("glBegin(GL_LINES)");
#endif
    if (verbose)
	crDebug("glBegin( GL_LINES )");
    for (y = 0; y < num; y++)
	for (x = 0; x < size; x++) {
	    glColor3f(0.0, 1.0, y / (float) num);
	    glVertex2i(0, size - 1);
	    glColor3f(1.0, 0.0, x / (float) size);
	    glVertex2i(x, x);
	}
    glEnd();

    return num * size;
}

/***************************************************************************/

static void init_test03(void)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, 639.5, -0.5, 479.5, 1.0, -1000.0 * 480.0);
    glMatrixMode(GL_MODELVIEW);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0, 0.1, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static int test03(int size, int num)
{
    int x, y, z;

    glBegin(GL_TRIANGLES);
#if 0
    if (errChk)
	printError("glBegin(GL_TRIANGLES)");
#endif
    if (verbose)
	crDebug("glBegin( GL_TRIANGLES )");
    for (y = 0; y < num; y++)
	for (x = 0; x < size; x += 5) {
	    z = num * size - (y * size + x);
	    glColor3f(0.0, 1.0, 0.0);
	    glVertex3i(0, x, z);

	    glColor3f(1.0, 0.0, x / (float) size);
	    glVertex3i(size - 1 - x, 0, z);

	    glColor3f(1.0, x / (float) size, 0.0);
	    glVertex3i(x, size - 1 - x, z);
	}
    glEnd();

    return size * num / 5;
}

/***************************************************************************/

static void init_test04(void)
{

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, 639.5, -0.5, 479.5, 1.0, -1000.0 * 480.0);

    glMatrixMode(GL_MODELVIEW);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.0, 0.1, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static int test04(int size, int num)
{
    int x, y, z;

    glBegin(GL_TRIANGLES);
    for (y = 0; y < num; y++)
	for (x = 0; x < size; x += 5) {
	    z = num * size - (y * size + x);
	    glColor3f(1.0, 0.0, 0.0);
	    glVertex3i(0, x, z);

	    glColor3f(0.0, 1.0, 0.0);
	    glVertex3i(size - 1 - x, 0, z);

	    glColor3f(0.0, 0.0, 1.0);
	    glVertex3i(x, size - 1 - x, z);
	}
    glEnd();

    return num * size / 5;
}

/***************************************************************************/

static void init_test05(void)
{

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, 639.5, -0.5, 479.5, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDepthFunc(GL_ALWAYS);

    glClearColor(0.0, 0.1, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static int test05(int size, int num)
{
    int y;
    float v0[3], v1[3], v2[3], v3[3];
    float cv0[3], cv1[3], cv2[3], cv3[3];
    float tv0[3], tv1[3], tv2[3], tv3[3];

    v0[0] = 320 - size / 2;
    v0[1] = 240 - size / 2;
    v0[2] = 0.0;
    v1[0] = 320 + size / 2;
    v1[1] = 240 - size / 2;
    v1[2] = 0.0;
    v2[0] = 320 - size / 2;
    v2[1] = 240 + size / 2;
    v2[2] = 0.0;
    v3[0] = 320 + size / 2;
    v3[1] = 240 + size / 2;
    v3[2] = 0.0;
    cv0[0] = 1.0;
    cv0[1] = 0.0;
    cv0[2] = 0.0;
    cv1[0] = 1.0;
    cv1[1] = 1.0;
    cv1[2] = 0.0;
    cv2[0] = 1.0;
    cv2[1] = 0.0;
    cv2[2] = 1.0;
    cv3[0] = 1.0;
    cv3[1] = 1.0;
    cv3[2] = 1.0;
    tv0[0] = 0.0;
    tv0[1] = 0.0;
    tv0[2] = 0.0;
    tv1[0] = 1.0;
    tv1[1] = 0.0;
    tv1[2] = 0.0;
    tv2[0] = 0.0;
    tv2[1] = 1.0;
    tv2[2] = 0.0;
    tv3[0] = 1.0;
    tv3[1] = 1.0;
    tv3[2] = 0.0;
    glBegin(GL_TRIANGLE_STRIP);
#if 0
    if (errChk)
	printError("glBegin(GL_TRIANGLE_STRIP)");
#endif
    if (verbose)
	crDebug("glBegin( GL_TRIANGLE_STRIP )");

    for (y = 0; y < num; y++) {
	glColor3fv(cv0);
	glVertex3fv(v0);

	glColor3fv(cv1);
	glVertex3fv(v1);

	glColor3fv(cv2);
	glVertex3fv(v2);

	glColor3fv(cv3);
	glVertex3fv(v3);
    }
    glEnd();

    return 4 * num - 2;
}


/***************************************************************************/

static void init_test06(void)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, 639.5, -0.5, 479.5, 1.0, -1000.0 * 480.0);
    glMatrixMode(GL_MODELVIEW);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0, 0.1, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static float pntA[3] = {
    1.6, 0.0, 0.0
};
static float pntB[3] = {
    1.3, 0.0, 0.0
};

static int test06(int size, int num)
{
    int i, j;

    for (j = 0; j < num; j++)
	for (i = 0; i < 360; i += 5) {
	    glRotatef(5.0, 0, 0, 1);

	    glColor3f(1.0, 1.0, 0.0);
	    glBegin(GL_LINE_STRIP);
#if 0
	    if (errChk)
		printError("glBegin(GL_LINE_STRIP)");
#endif
	    if (verbose)
		crDebug("glBegin( GL_LINE_STRIP )");
	    pntA[0] = pntA[0] + size / 2;
	    pntB[0] = pntB[0] + size / 2;
	    glVertex3fv(pntA);
	    glVertex3fv(pntB);
	    glEnd();

	    glPointSize(1);

	    glBegin(GL_POINTS);
	    glVertex3fv(pntA);
	    glVertex3fv(pntB);
	    glEnd();
	}
    return size * num / 5;
}

/***************************************************************************/
#define MAXQUAD 6

typedef GLfloat Vertex[3];

typedef Vertex Quad[4];

/* data to define the six faces of a unit cube */
Quad quads[MAXQUAD] = {
    {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}},
    {{0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}},
    {{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}},
    {{0, 1, 0}, {1, 1, 0}, {1, 1, 1}, {0, 1, 1}},
    {{0, 0, 0}, {0, 0, 1}, {0, 1, 1}, {0, 1, 0}},
    {{1, 0, 0}, {1, 0, 1}, {1, 1, 1}, {1, 1, 0}}
};


static void init_test07(void)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, 639.5, -0.5, 479.5, 1.0, -1000.0 * 480.0);
    glMatrixMode(GL_MODELVIEW);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0, 0.1, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void outline(Quad quad, int size)
{
    glBegin(GL_LINE_LOOP);
#if 0
        if (errChk)
	    printError("glBegin(GL_LINE_LOOP)");
#endif
        if (verbose)
	    crDebug("glBegin( GL_LINE_LOOP )");
        glVertex3fv(quad[0] + size / 2);
        glVertex3fv(quad[1] + size / 2);
        glVertex3fv(quad[2] + size / 2);
        glVertex3fv(quad[3] + size / 2);
    glEnd();
}

static int test07(int size, int num)
{
    int i,j;

    /*
     * draw an outlined polygon 
     */

    for ( j = 0; j < num; j++)
        for ( i = 0; i < MAXQUAD; i++)
	  outline(quads[i], size);


    return size * num / 5;
}

/***************************************************************************/

static void init_test08(void)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, 639.5, -0.5, 479.5, 1.0, -1000.0 * 480.0);
    glMatrixMode(GL_MODELVIEW);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0, 0.1, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void fill(Quad quad, int size)
{
    /*
     * draw an outlined polygon 
     */
    glBegin(GL_QUADS);
#if 0
        if (errChk)
	    printError("glBegin(GL_QUADS)");
#endif
        if (verbose)
	    crDebug("glBegin( GL_QUADS )");
        glVertex3fv(quad[0] + size / 2);
        glVertex3fv(quad[1] + size / 2);
        glVertex3fv(quad[2] + size / 2);
        glVertex3fv(quad[3] + size / 2);
    glEnd();
}

static int test08(int size, int num)
{
    int i,j;


    for ( j = 0; j < num; j++)
    	for ( i = 0; i < MAXQUAD; i++)
	  fill(quads[i], size);


    return size * num / 5;
}

/***************************************************************************/
static void init_test09(void)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, 639.5, -0.5, 479.5, 1.0, -1000.0 * 480.0);
    glMatrixMode(GL_MODELVIEW);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0, 0.1, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


#define GAP 10
#define ROWS 3
#define COLS 4

static int test09(int size, int num)
{
    GLint vx[8][2];
    GLint x0, y0, x1, y1, x2, y2, x3, y3;
    GLint i;
    GLint boxW, boxH;

    boxW = (24 - (COLS + 1) * GAP) / COLS;
    boxH = (24 - (ROWS + 1) * GAP) / ROWS;

    y0 = -boxH / 4;
    y1 = y0 + boxH / 2 / 3;
    y2 = y1 + boxH / 2 / 3;
    y3 = boxH / 4;
    x0 = -boxW / 4;
    x1 = x0 + boxW / 2 / 3;
    x2 = x1 + boxW / 2 / 3;
    x3 = boxW / 4;

    vx[0][0] = x0;
    vx[0][1] = y1;
    vx[1][0] = x0;
    vx[1][1] = y2;
    vx[2][0] = x1;
    vx[2][1] = y3;
    vx[3][0] = x2;
    vx[3][1] = y3;
    vx[4][0] = x3;
    vx[4][1] = y2;
    vx[5][0] = x3;
    vx[5][1] = y1;
    vx[6][0] = x2;
    vx[6][1] = y0;
    vx[7][0] = x1;
    vx[7][1] = y0;

    glBegin(GL_TRIANGLE_FAN);

#if 0
        if (errChk)
	    printError("glBegin(GL_TRIANGLE_FAN)");
#endif
        if (verbose)
    	    crDebug("glBegin( GL_TRIANGLE_FAN )");
        glVertex2i(0, 0);
        for (i = 0; i < 8; i++) {
	    glVertex2iv(vx[i]);
        }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
        glVertex2i(0, 0);
        glVertex2i(-100, 100);
    glEnd();




    return size * num / 5;
}

/***************************************************************************/


static void init_test10(void)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, 639.5, -0.5, 479.5, 1.0, -1000.0 * 480.0);
    glMatrixMode(GL_MODELVIEW);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0, 0.1, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void FillTorus(float rc, int numc, float rt, int numt)
{
    int i, j, k;
    double s, t;
    double x, y, z;
    double pi, twopi;

    pi = 3.14159265358979323846;
    twopi = 2 * pi;

    for (i = 0; i < numc; i++) {
	glBegin(GL_QUAD_STRIP);
#if 0
	if (errChk)
	    printError("glBegin(GL_QUAD_STRIP)");
#endif
	if (verbose)
	    crDebug("glBegin( GL_QUAD_STRIP )");
	for (j = 0; j <= numt; j++) {
	    for (k = 1; k >= 0; k--) {
		s = (i + k) % numc + 0.5;
		t = j % numt;

		x = cos(t * twopi / numt) * cos(s * twopi / numc);
		y = sin(t * twopi / numt) * cos(s * twopi / numc);
		z = sin(s * twopi / numc);
		glNormal3f(x, y, z);

		x = (rt +
		     rc * cos(s * twopi / numc)) * cos(t * twopi / numt);
		y = (rt +
		     rc * cos(s * twopi / numc)) * sin(t * twopi / numt);
		z = rc * sin(s * twopi / numc);
		glVertex3f(x, y, z);
	    }
	}
	glEnd();
    }
}


static int test10(int size, int num)
{

    FillTorus(0.1, 8, 1.0, 25);

    return size * num / 5;
}

/***************************************************************************/
static void init_test11(void)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, 639.5, -0.5, 479.5, 1.0, -1000.0 * 480.0);
    glMatrixMode(GL_MODELVIEW);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0, 0.1, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


static int test11(int size, int num)
{


/* clear all pixels  */
    glClear(GL_COLOR_BUFFER_BIT);

/* draw white polygon (rectangle) with corners at
 * (0.25, 0.25, 0.0) and (0.75, 0.75, 0.0)
 */
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_POLYGON);
#if 0
        if (errChk)
	    printError("glBegin(GL_POLYGON)");
#endif
        if (verbose)
	    crDebug("glBegin( GL_POLYGON )");
        glVertex3f(0.25, 0.25, 0.0);
        glVertex3f(0.75, 0.25, 0.0);
        glVertex3f(0.75, 0.75, 0.0);
        glVertex3f(0.25, 0.75, 0.0);
    glEnd();



    return size * num / 5;
}

/***************************************************************************/

#define BMARKS_TIME 5.0

#define NUM_BMARKS 11

/* 554 ~= sqrt(640*480) */

typedef struct {
    char *name;
    char *unit;
    void (*init) (void);
    int (*run) (int, int);
    int type;
    int numsize;
    int size[10];
} benchmark;

static benchmark bmarks[NUM_BMARKS] = {
    {"Simple Points", "Pnts", init_test01, test01, 			0, 0,
     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {"Smooth Lines", "Lins", init_test02, test02, 			1, 5,
     {480, 250, 100, 50, 25, 0, 0, 0, 0, 0}},
    {"ZSmooth Triangles", "Tris", init_test03, test03, 			1, 5,
     {480, 250, 100, 50, 25, 0, 0, 0, 0, 0}},
    {"ZSmooth Tex Blend Triangles", "Tris", init_test04, test04, 	1, 5,
     {480, 250, 100, 50, 25, 0, 0, 0, 0, 0}},
    {"ZSmooth Tex Blend TMesh Triangles", "Tris", init_test05, test05, 	2, 8,
     {400, 250, 100, 50, 25, 10, 5, 2, 0, 0}},
    {"Line Strips", "Linestr", init_test06, test06, 			1, 5,
     {480, 250, 100, 50, 25, 0, 0, 0, 0, 0}},
    {"Line Loop", "Lineloop", init_test07, test07, 			1, 5,
     {480, 250, 100, 50, 25, 0, 0, 0, 0, 0}},
    {"Quads", "Quads", init_test08, test08, 				1, 5,
     {480, 250, 100, 50, 25, 0, 0, 0, 0, 0}},
    {"TriangleFan", "TriFan", init_test09, test09, 			1, 5,
     {480, 250, 100, 50, 25, 0, 0, 0, 0, 0}},
    {"QuadStrip", "QuadSt", init_test10, test10, 			1, 5,
     {480, 250, 100, 50, 25, 0, 0, 0, 0, 0}},
    {"Polygon", "Polygon", init_test11, test11, 			1, 5,
     {480, 250, 100, 50, 25, 0, 0, 0, 0, 0}},
};

/***************************************************************************/

static void dotest0param(benchmark * bmark)
{
    float stime, etime, dtime, tottime, maxtime, mintime;
    int num, numelem, calibnum, j;

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    bmark->init();

    stime = glutGet(GLUT_ELAPSED_TIME);

    dtime = 0.0;
    calibnum = 0;
    while (dtime < 2.0) {
	bmark->run(0, 1);
	glFinish();
	etime = glutGet(GLUT_ELAPSED_TIME);
	dtime = (etime - stime) / 1000.0;
	calibnum++;
    }
    glPopAttrib();

    crDebug( "Elapsed time for the calibration test (%d): %f",
	    calibnum, dtime);

    num = (int) ((BMARKS_TIME / dtime) * calibnum);
	
    num = num / 50;

    if (num < 1)
	num = 1;

    crDebug( "Selected number of benchmark iterations: %d", num);

    mintime = HUGE_VAL;
    maxtime = -HUGE_VAL;

    for (tottime = 0.0, j = 0; j < 5; j++) {
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	bmark->init();

	stime = glutGet(GLUT_ELAPSED_TIME);
	numelem = bmark->run(0, num);
	glFinish();
	etime = glutGet(GLUT_ELAPSED_TIME);

	glPopAttrib();

	dtime = (etime - stime) / 1000.0;
	tottime += dtime;

	crDebug( "Elapsed time for run %d: %f", j, dtime);

	if (dtime < mintime)
	    mintime = dtime;
	if (dtime > maxtime)
	    maxtime = dtime;
    }

    tottime -= mintime + maxtime;

    crDebug( "%s%f %s/sec", bmark->name,
	    numelem / (tottime / 3.0), bmark->unit);

    if (bmark->type == 3)
	crDebug( ", MPixel Fill/sec: %f",
		(numelem * bmark->size[0] * (float) bmark->size[0]) /
		(1000000.0 * tottime / 3.0));
    else
	crDebug( " ");
}

/***************************************************************************/

static void dotest1param(benchmark * bmark)
{
    float stime, etime, dtime, tottime, maxtime, mintime;
    int num, numelem, calibnum, j, k;

    crDebug( "%s", bmark->name);

    for (j = 0; j < bmark->numsize; j++) {
	crDebug( "Current size: %d", bmark->size[j]);

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	bmark->init();

	stime = glutGet(GLUT_ELAPSED_TIME);

	dtime = 0.0;
	calibnum = 0;
	while (dtime < 2.0) {
	    bmark->run(bmark->size[j], 1);
	    glFinish();
	    etime = glutGet(GLUT_ELAPSED_TIME);
	    dtime = (etime - stime) / 1000.0;
	    calibnum++;
	}
	glPopAttrib();

	crDebug( "Elapsed time for the calibration test (%d): %f",
		calibnum, dtime);

	num = (int) ((BMARKS_TIME / dtime) * calibnum);

	if (num < 1)
	    num = 1;

	crDebug( "Selected number of benchmark iterations: %d", num);

	mintime = HUGE_VAL;
	maxtime = -HUGE_VAL;

	for (numelem = 1, tottime = 0.0, k = 0; k < 5; k++) {
	    glPushAttrib(GL_ALL_ATTRIB_BITS);
	    bmark->init();

	    stime = glutGet(GLUT_ELAPSED_TIME);
	    numelem = bmark->run(bmark->size[j], num);
	    glFinish();
	    etime = glutGet(GLUT_ELAPSED_TIME);

	    glPopAttrib();

	    dtime = (etime - stime) / 1000.0;
	    tottime += dtime;

	    crDebug( "Elapsed time for run %d: %f", k, dtime);

	    if (dtime < mintime)
		mintime = dtime;
	    if (dtime > maxtime)
		maxtime = dtime;
	}

	tottime -= mintime + maxtime;

	crDebug( "SIZE=%03d => %f %s/sec", bmark->size[j],
		numelem / (tottime / 3.0), bmark->unit);
	if (bmark->type == 2)
	    crDebug( ", MPixel Fill/sec: %f",
		    (numelem * bmark->size[j] * bmark->size[j] / 2) /
		    (1000000.0 * tottime / 3.0));
	else
	    crDebug( " ");
    }

    crDebug( " ");
}

/***************************************************************************/

void crPackTestBegin(void)
{
    int i;

    if (frontbuffer)
	glDrawBuffer(GL_FRONT);
    else
	glDrawBuffer(GL_BACK);

    for (i = 0; i < NUM_BMARKS; i++) {
	crDebug( "Benchmark: %d", i);

	switch (bmarks[i].type) {
	case 0:
	case 3:
            dotest0param(&bmarks[i]);
	    break;
	case 1:
	case 2:
	    dotest1param(&bmarks[i]);
	    break;
	}
    }
}

void crPackTestEnd(void)
{
    int x, y, size, num;

    size = 25;
    num = 64;
    glBegin(GL_POINTS);
    for (y = 0; y < num; y++)
	for (x = 0; x < 480; x++)
	    glVertex2i(x, x);

    glEnd();
    if (errChk)
	printError("gl(End)");
    if (verbose)
	crDebug("glEnd( )");
}
