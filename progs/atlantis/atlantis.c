
/* Copyright (c) Mark J. Kilgard, 1994. */

/**
 * (c) Copyright 1993, 1994, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 * Permission to use, copy, modify, and distribute this software for
 * any purpose and without fee is hereby granted, provided that the above
 * copyright notice appear in all copies and that both the copyright notice
 * and this permission notice appear in supporting documentation, and that
 * the name of Silicon Graphics, Inc. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.
 *
 * THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
 * AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
 * FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL SILICON
 * GRAPHICS, INC.  BE LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT,
 * SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY
 * KIND, OR ANY DAMAGES WHATSOEVER, INCLUDING WITHOUT LIMITATION,
 * LOSS OF PROFIT, LOSS OF USE, SAVINGS OR REVENUE, OR THE CLAIMS OF
 * THIRD PARTIES, WHETHER OR NOT SILICON GRAPHICS, INC.  HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
 * POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * US Government Users Restricted Rights
 * Use, duplication, or disclosure by the Government is subject to
 * restrictions set forth in FAR 52.227.19(c)(2) or subparagraph
 * (c)(1)(ii) of the Rights in Technical Data and Computer Software
 * clause at DFARS 252.227-7013 and/or in similar or successor
 * clauses in the FAR or the DOD or NASA FAR Supplement.
 * Unpublished-- rights reserved under the copyright laws of the
 * United States.  Contractor/manufacturer is Silicon Graphics,
 * Inc., 2011 N.  Shoreline Blvd., Mountain View, CA 94039-7311.
 *
 * OpenGL(TM) is a trademark of Silicon Graphics, Inc.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include "atlantis.h"

fishRec sharks[NUM_SHARKS];
fishRec momWhale;
fishRec babyWhale;
fishRec dolph;

GLboolean moving;
int screenshot = 0;

static void
InitFishs(void)
{
    int i;

    for (i = 0; i < NUM_SHARKS; i++) {
        memset(sharks + i, 0, sizeof(fishRec));
        sharks[i].x = 70000.0 + rand() % 6000;
        sharks[i].y = rand() % 6000;
        sharks[i].z = rand() % 6000;
        sharks[i].psi = rand() % 360 - 180.0;
        sharks[i].v = 1.0;
    }

    memset(&dolph, 0, sizeof(fishRec));
    dolph.x = 30000.0;
    dolph.y = 0.0;
    dolph.z = 6000.0;
    dolph.psi = 90.0;
    dolph.theta = 0.0;
    dolph.v = 3.0;

    memset(&momWhale, 0, sizeof(fishRec));
    momWhale.x = 70000.0;
    momWhale.y = 0.0;
    momWhale.z = 0.0;
    momWhale.psi = 90.0;
    momWhale.theta = 0.0;
    momWhale.v = 3.0;

    memset(&babyWhale, 0, sizeof(fishRec));
    babyWhale.x = 60000.0;
    babyWhale.y = -2000.0;
    babyWhale.z = -2000.0;
    babyWhale.psi = 90.0;
    babyWhale.theta = 0.0;
    babyWhale.v = 3.0;
}

static void
Init(void)
{
    static float ambient[] =
    {0.1, 0.1, 0.1, 1.0};
    static float diffuse[] =
    {1.0, 1.0, 1.0, 1.0};
    static float position[] =
    {0.0, 1.0, 0.0, 0.0};
    static float mat_shininess[] =
    {90.0};
    static float mat_specular[] =
    {0.8, 0.8, 0.8, 1.0};
    static float mat_diffuse[] =
    {0.46, 0.66, 0.795, 1.0};
    static float mat_ambient[] =
    {0.0, 0.1, 0.2, 1.0};
    static float lmodel_ambient[] =
    {0.4, 0.4, 0.4, 1.0};
    static float lmodel_localviewer[] =
    {0.0};

    glFrontFace(GL_CW);

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, lmodel_localviewer);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);

    InitFishs();

    glClearColor(0.0, 0.5, 0.9, 0.0);
}

static void
Reshape(int width, int height)
{
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(4.0, 2.0, 10000.0, 400000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}

static void
Animate(void)
{
    int i;

    for (i = 0; i < NUM_SHARKS; i++) {
        SharkPilot(&sharks[i]);
        SharkMiss(i);
    }
    WhalePilot(&dolph);
    dolph.phi++;
    glutPostRedisplay();
    WhalePilot(&momWhale);
    momWhale.phi++;
    WhalePilot(&babyWhale);
    babyWhale.phi++;
}

/* ARGSUSED1 */
static void
Key(unsigned char key, int x, int y)
{
	(void) x;
	(void) y;
    switch (key) 
	{
	  case 27:           /* Esc will quit */
        exit(1);
        break;
	  case 's':
		screenshot++;
		glutPostRedisplay( );
		break;
	  case ' ':          /* space will advance frame */
        if (!moving) {
            Animate();
        }
    }
}

static void
save_frame( void )
{
	GLint viewport[4];
	int x, y, width, height;
	void *pixels;
	FILE *f;

	glGetIntegerv( GL_VIEWPORT, viewport );

	x = viewport[0];
	y = viewport[1];
	width = viewport[2];
	height = viewport[3];

	printf( "saving %dx%d screenshot\n", width, height );

	pixels = malloc( width * height * 3 );
	assert( pixels );
	memset( pixels, 0, width * height * 3 );

	glReadPixels( x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels );

	f = fopen( "screenshot.ppm", "wb" );
	fprintf( f, "P6\n%d %d\n255\n", width, height );
	for ( y = height - 1; y >= 0; y-- )
		fwrite( (char *) pixels + y * width * 3, width, 3, f );
	fclose( f );

	free( pixels );

	printf( "done\n" );
}

static void
print_performance( void )
{
	static int first = 1;
	static int start, num_frames;
	int current;

	if ( first )
	{
		start = glutGet( GLUT_ELAPSED_TIME );
		num_frames = 0;
		first = 0;
	}

	num_frames++;
	current = glutGet( GLUT_ELAPSED_TIME );

	if ( current - start > 1000 )
	{
		double elapsed = 1e-3 * (double) ( current - start );
		double rate = (double) num_frames / elapsed;
		printf( "%5.1f fps\n", rate );

		num_frames = 0;
		start = current;
	}
}

static void
Display( void )
{
    float ambient[] = {0.1, 0.1, 0.1, 1.0};
    float diffuse[] = {1.0, 1.0, 1.0, 1.0};
    float position[] = {0.0, 1.0, 0.0, 0.0};
    float mat_shininess[] = {90.0};
    float mat_specular[] = {0.8, 0.8, 0.8, 1.0};
    float mat_diffuse[] = {0.46, 0.66, 0.795, 1.0};
    float mat_ambient[] = {0.0, 0.1, 0.2, 1.0};
    float lmodel_ambient[] = {0.4, 0.4, 0.4, 1.0};
    float lmodel_localviewer[] = {0.0};

    int i;

#if 0
	/* stupid WireGL */
	glViewport (0, 0, 450*3, 170*3);
#endif

    glFrontFace(GL_CCW);


    glDepthFunc(GL_LEQUAL);

    glEnable(GL_DEPTH_TEST);

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

    glLightfv(GL_LIGHT0, GL_POSITION, position);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

    glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, lmodel_localviewer);

    glEnable(GL_LIGHTING);

    glEnable(GL_LIGHT0);

	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);

    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);

    glClearColor(0.0, 0.5, 0.9, 0.0);

    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();

    gluPerspective(40.0, 2.0, 10000.0, 400000.0);

    glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  	for (i = 0; i < NUM_SHARKS; i++) {
        glPushMatrix();
        FishTransform(&sharks[i]);
        DrawShark(&sharks[i]);
        glPopMatrix();
    }

    glPushMatrix();
    FishTransform(&dolph);
    DrawDolphin(&dolph);
    glPopMatrix();

    glPushMatrix();
    FishTransform(&momWhale);
    DrawWhale(&momWhale);
    glPopMatrix();

    glPushMatrix();
    FishTransform(&babyWhale);
    glScalef(0.45, 0.45, 0.3);
    DrawWhale(&babyWhale);
    glPopMatrix();

	if ( screenshot )
		save_frame( );
	screenshot = 0;

    glutSwapBuffers( );

	print_performance( );
}

static void
Visible(int state)
{
    if (state == GLUT_VISIBLE) {
        if (moving)
            glutIdleFunc(Animate);
    } else {
        if (moving)
            glutIdleFunc(NULL);
    }
}

static void
menuSelect(int value)
{
    switch (value) {
    case 1:
        moving = GL_TRUE;
        glutIdleFunc(Animate);
        break;
    case 2:
        moving = GL_FALSE;;
        glutIdleFunc(NULL);
        break;
    case 3:
        exit(0);
        break;
    }
}

int
main(int argc, char **argv)
{
    int mode = GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH;
    glutInitWindowSize(512, 192);
    glutInit(&argc, argv);
    if (argc > 1 && strcmp(argv[1], "-ms") == 0)
       mode |= GLUT_MULTISAMPLE;
    glutInitDisplayMode(mode);
    glutCreateWindow("GLUT Atlantis Demo");
    Init();
    glutDisplayFunc(Display);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Key);
    moving = GL_TRUE;
    glutIdleFunc(Animate);
    glutVisibilityFunc(Visible);
    glutCreateMenu(menuSelect);
    glutAddMenuEntry("Start motion", 1);
    glutAddMenuEntry("Stop motion", 2);
    glutAddMenuEntry("Quit", 3);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    glutMainLoop();
    return 0;             /* ANSI C requires main to return int. */
}
