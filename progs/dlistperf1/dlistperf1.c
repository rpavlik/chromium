#include "OGLwin.h"
#include <math.h>
#ifndef DARWIN
#include <malloc.h>
#endif
#include "Timer.h"

#include <stdio.h>
#include <stdlib.h>

static int SphereInitialized = 0;

/* ******************************** Build a Ball *************************** */

#define LAT_DIVISIONS 16
#define LON_DIVISIONS 16

#define NUM_TRIANGLES (LON_DIVISIONS + LON_DIVISIONS + (LAT_DIVISIONS - 2)*LON_DIVISIONS*2)

struct vtxData {
    float sphereVtx[LON_DIVISIONS + 1][LAT_DIVISIONS + 1][3];
    float sphereNormal[LON_DIVISIONS + 1][LAT_DIVISIONS + 1][3];
    float sphereTexVtx[LON_DIVISIONS + 1][LAT_DIVISIONS + 1][2];
}      *theSphere = NULL, *theSphereLocal = NULL, *theSphereAGP = NULL;


static void DrawSphere(int Filled, int Textured)
{
    double sinlong, coslong, sinlat, coslat;
    double x, y, z;
    double dia = 1.3;
    int i, j;

    if (!SphereInitialized) {
	SphereInitialized = 1;
	theSphere = (struct vtxData *) malloc(sizeof(struct vtxData));
	for (i = 0; i < LON_DIVISIONS; i++) {
	    coslong = cos(i * 2.0 * M_PI / LON_DIVISIONS);
	    sinlong = sin(i * 2.0 * M_PI / LON_DIVISIONS);
	    for (j = 0; j <= LAT_DIVISIONS; j++) {
		coslat =
		    cos((j - LAT_DIVISIONS * 0.5) * M_PI / LAT_DIVISIONS);
		sinlat =
		    sin((j - LAT_DIVISIONS * 0.5) * M_PI / LAT_DIVISIONS);
		x = coslong * coslat;
		y = sinlong * coslat;
		z = sinlat;
		if ((x < 0.001) && (x > -0.001))
		    x = 0.0;
		if ((y < 0.001) && (y > -0.001))
		    y = 0.0;
		if ((z < 0.001) && (z > -0.001))
		    z = 0.0;
		theSphere->sphereVtx[i][j][0] = (float) x *dia;
		theSphere->sphereVtx[i][j][1] = (float) y *dia;
		theSphere->sphereVtx[i][j][2] = (float) z *dia;

		theSphere->sphereNormal[i][j][0] = (float) x;
		theSphere->sphereNormal[i][j][1] = (float) y;
		theSphere->sphereNormal[i][j][2] = (float) z;

		theSphere->sphereTexVtx[i][j][0] =
		    j * 1.0f / LAT_DIVISIONS;
		theSphere->sphereTexVtx[i][j][1] =
		    i * 1.0f / LON_DIVISIONS;

		if (!i) {
		    theSphere->sphereVtx[LON_DIVISIONS][j][0] =
			(float) x *dia;
		    theSphere->sphereVtx[LON_DIVISIONS][j][1] =
			(float) y *dia;
		    theSphere->sphereVtx[LON_DIVISIONS][j][2] =
			(float) z *dia;

		    theSphere->sphereNormal[LON_DIVISIONS][j][0] =
			(float) x;
		    theSphere->sphereNormal[LON_DIVISIONS][j][1] =
			(float) y;
		    theSphere->sphereNormal[LON_DIVISIONS][j][2] =
			(float) z;

		    theSphere->sphereTexVtx[LON_DIVISIONS][j][0] =
			theSphere->sphereTexVtx[0][j][0];
		    theSphere->sphereTexVtx[LON_DIVISIONS][j][1] = 1.0f;
		}
	    }
	}
    }

    if (Filled) {

	// Build the north (back) polar cap:

	glBegin(GL_TRIANGLE_FAN);
	glNormal3fv(theSphere->sphereNormal[0][0]);
	glVertex3fv(theSphere->sphereVtx[0][0]);
	for (i = LON_DIVISIONS; i >= 0; i--) {
	    glNormal3fv(theSphere->sphereNormal[i][1]);
	    glVertex3fv(theSphere->sphereVtx[i][1]);
	}
	glEnd();

	// Build the south (front) polar cap:

	glBegin(GL_TRIANGLE_FAN);
	glNormal3fv(theSphere->sphereNormal[0][LAT_DIVISIONS]);
	glVertex3fv(theSphere->sphereVtx[0][LAT_DIVISIONS]);
	for (i = 0; i <= LON_DIVISIONS; i++) {
	    glNormal3fv(theSphere->sphereNormal[i][LAT_DIVISIONS - 1]);
	    glVertex3fv(theSphere->sphereVtx[i][LAT_DIVISIONS - 1]);
	}
	glEnd();

	// Build the middle:

	for (i = 0; i < LON_DIVISIONS; i++) {
	    glBegin(GL_TRIANGLE_STRIP);
	    if (Textured) {
		for (j = 1; j <= LAT_DIVISIONS - 1; j++) {
		    glNormal3fv(theSphere->sphereNormal[i][j]);
		    glTexCoord2f(i * 1.0f / LON_DIVISIONS,
				 j * 1.0f / LAT_DIVISIONS);
		    glVertex3fv(theSphere->sphereVtx[i][j]);
		    glNormal3fv(theSphere->sphereNormal[i + 1][j]);
		    glTexCoord2f((i + 1) * 1.0f / LON_DIVISIONS,
				 j * 1.0f / LAT_DIVISIONS);
		    glVertex3fv(theSphere->sphereVtx[i + 1][j]);
		}
	    } else {
		for (j = 1; j <= LAT_DIVISIONS - 1; j++) {
		    glNormal3fv(theSphere->sphereNormal[i][j]);
		    glVertex3fv(theSphere->sphereVtx[i][j]);
		    glNormal3fv(theSphere->sphereNormal[i + 1][j]);
		    glVertex3fv(theSphere->sphereVtx[i + 1][j]);
		}
	    }
	    glEnd();
	}
    } else {
	// Wireframe
	// Draw longitude lines:
	for (i = 0; i < LON_DIVISIONS; i++) {
	    glBegin(GL_LINE_STRIP);
	    for (j = 0; j <= LAT_DIVISIONS; j++) {
		glNormal3fv(theSphere->sphereNormal[i][j]);
		glVertex3fv(theSphere->sphereVtx[i][j]);
	    }
	    glEnd();
	}
	for (j = 1; j < LAT_DIVISIONS; j++) {
	    glBegin(GL_LINE_STRIP);
	    for (i = 0; i <= LON_DIVISIONS; i++) {
		glNormal3fv(theSphere->sphereNormal[i][j]);
		glVertex3fv(theSphere->sphereVtx[i][j]);
	    }
	    glEnd();
	}
    }
}				// DrawSphere

int main(int argc, char *argv[])
{
    int f, x, y;
    int frames = 5000;
    GLfloat ang = 0.0;
    double sec;

    OGLwin_OGLwin(512, 512, RGBA_DOUBLE);
    OGLwin_SetWin();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 27.0, 0.0, 27.0, -10.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glNewList(1, GL_COMPILE);
    DrawSphere(0, 0);
    glEndList();

    Timer_Timer();

    Start();

    for (f = 0; f < frames; f++) {
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0f, 1.0f, 0.0f);
	ang += 1.0;
	if (ang >= 360.0)
	    ang -= 360.0;
	for (x = 1; x <= 8; x++) {
	    for (y = 1; y <= 8; y++) {
		glPushMatrix();
		glTranslatef(x * 3.0, y * 3.0, 0.0);
		glRotatef(ang, 0.0, 1.0, 0.0);
		glCallList(1);
		glPopMatrix();
	    }
	}
	OGLwin_SwapBuffers();
    }

    sec = GetSeconds();
    fprintf(stderr, "%f secs\n", sec);
    fprintf(stderr, "%f FPS\n", frames / sec);

    glFinish();
	
    exit(0);
}
