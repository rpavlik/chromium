#include "OGLwin.h"
#include "Timer.h"
#include <stdio.h>
#include <stdlib.h>


static void draw_cube(void)
{
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_POLYGON);
    glVertex3f(-1.0, -1.0, -1.0);
    glVertex3f(1.0, -1.0, -1.0);
    glVertex3f(1.0, 1.0, -1.0);
    glVertex3f(-1.0, 1.0, -1.0);
    glEnd();

    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_POLYGON);
    glVertex3f(-1.0, -1.0, 1.0);
    glVertex3f(1.0, -1.0, 1.0);
    glVertex3f(1.0, 1.0, 1.0);
    glVertex3f(-1.0, 1.0, 1.0);
    glEnd();

    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_POLYGON);
    glVertex3f(-1.0, -1.0, -1.0);
    glVertex3f(1.0, -1.0, -1.0);
    glVertex3f(1.0, -1.0, 1.0);
    glVertex3f(-1.0, -1.0, 1.0);
    glEnd();

    glColor3f(1.0, 0.0, 1.0);
    glBegin(GL_POLYGON);
    glVertex3f(-1.0, 1.0, -1.0);
    glVertex3f(1.0, 1.0, -1.0);
    glVertex3f(1.0, 1.0, 1.0);
    glVertex3f(-1.0, 1.0, 1.0);
    glEnd();

    glColor3f(0.0, 1.0, 1.0);
    glBegin(GL_POLYGON);
    glVertex3f(1.0, -1.0, -1.0);
    glVertex3f(1.0, 1.0, -1.0);
    glVertex3f(1.0, 1.0, 1.0);
    glVertex3f(1.0, -1.0, 1.0);
    glEnd();

    glColor3f(1.0, 1.0, 0.0);
    glBegin(GL_POLYGON);
    glVertex3f(-1.0, -1.0, -1.0);
    glVertex3f(-1.0, 1.0, -1.0);
    glVertex3f(-1.0, 1.0, 1.0);
    glVertex3f(-1.0, -1.0, 1.0);
    glEnd();

    GLfloat c[3] = { 1.0, 1.0, 1.0 };

    glColor3fv(c);
}

static void gen(int list_base, float sz)
{
    glNewList(list_base, GL_COMPILE);

    glPushMatrix();
    glScalef(sz, sz, sz);
    draw_cube();
    glPopMatrix();
    glEndList();
}

int main()
{
    int i;
    int val[2];
    int done = 0;
    int frame_no = 0;
    float sz = 0.0;
    GLuint list_base;
    double sec;

    OGLwin_OGLwin(512, 512, RGBA_DOUBLE);
    OGLwin_SetWin();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-10.0, 10.0, -10.0, 10.0, -10.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    list_base = glGenLists(2);
    gen(list_base, 1.0);

    glEnable(GL_DEPTH_TEST);

    Timer_Timer();
    Start();

    for (i = 0; i < 5 ;) {

	if (OGLwin_QTest()) {
	    switch (OGLwin_Qread(val)) {
	    case OGL_KEYBOARD:
		if (val[0] == XK_Escape)
		    done = 1;
		break;
	     default:
		break;
	    }
	}

	sz = 1.0 - sz;
	gen(list_base, sz + 1.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glRotatef(1.0, 1.0, 1.0, 1.0);
	glCallList(list_base);

	OGLwin_SwapBuffers();
	glFinish();

	frame_no++;

	sec = GetSeconds();
	if (sec >= 5.0) {
	    fprintf(stderr, "%f FPS\n", frame_no / sec);
	    i++;
	    frame_no = 0;
	    Start();
	}
    }
    exit(0);
}
