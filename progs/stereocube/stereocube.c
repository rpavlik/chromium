/*
 * Simple stereo rendering demo.
 * Brian Paul
 * 21 Jan 2004
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <memory.h>
#include "chromium.h"
#include <GL/glut.h>


#ifdef WINDOWS
#pragma warning(disable : 4125)
#endif

static GLint WinWidth = 300, WinHeight = 300;
static GLboolean Anim = GL_TRUE;
static GLboolean Stereo = GL_FALSE;
static GLfloat CubeXrot = 0, CubeYrot = 0, CubeZrot = 0;
static GLfloat ViewXrot = 30, ViewYrot = 0, ViewZrot = 0;
static GLfloat CameraRot = 0.0;
static GLfloat ViewDist = 5.0;
static GLfloat FocalDist = 5.0;
static GLboolean ShowOverlay = GL_FALSE;



/* From GLUT's scube.c demo */
static void
myShadowMatrix(const float ground[4], const float light[4])
{
  float dot;
  float shadowMat[4][4];

  dot = ground[0] * light[0] +
    ground[1] * light[1] +
    ground[2] * light[2] +
    ground[3] * light[3];

  shadowMat[0][0] = dot - light[0] * ground[0];
  shadowMat[1][0] = 0.0 - light[0] * ground[1];
  shadowMat[2][0] = 0.0 - light[0] * ground[2];
  shadowMat[3][0] = 0.0 - light[0] * ground[3];

  shadowMat[0][1] = 0.0 - light[1] * ground[0];
  shadowMat[1][1] = dot - light[1] * ground[1];
  shadowMat[2][1] = 0.0 - light[1] * ground[2];
  shadowMat[3][1] = 0.0 - light[1] * ground[3];

  shadowMat[0][2] = 0.0 - light[2] * ground[0];
  shadowMat[1][2] = 0.0 - light[2] * ground[1];
  shadowMat[2][2] = dot - light[2] * ground[2];
  shadowMat[3][2] = 0.0 - light[2] * ground[3];

  shadowMat[0][3] = 0.0 - light[3] * ground[0];
  shadowMat[1][3] = 0.0 - light[3] * ground[1];
  shadowMat[2][3] = 0.0 - light[3] * ground[2];
  shadowMat[3][3] = dot - light[3] * ground[3];

  glMultMatrixf((const GLfloat *) shadowMat);
}

static void DrawScene(void)
{
	static const GLfloat ground[4] = {0, 1, 0, -0.05f}; /* plane eq for ground */
	static const GLfloat light[4] = {-5, 20, -5, 1};
	int x, y, z;

	/* checkboard ground */
	y = 0;
	for (x = -10; x <= 10; x += 1) {
		glBegin(GL_TRIANGLE_STRIP);
		for (z = -10; z <= 10; z += 1) {
			if ((x + z) & 1)
				glColor3f(0, 0.5f, 0);
			else
				glColor3f(0, 0.4f, 0);
			glVertex3i(x, y, z);
			glVertex3i(x + 1, y, z);
		}
		glEnd();
	}

#if 1
	/* draw wire cube */
	glPushMatrix();
	glTranslatef(0, 1.7f, 0);
	glRotatef(CubeXrot, 1, 0, 0);
	glRotatef(CubeYrot, 0, 1, 0);
	glRotatef(CubeZrot, 0, 0, 1);
	glColor3f(1, 1, 1);
	glutWireCube(2.0);
	glPopMatrix();

	/* draw cube's shadow */
	glPushMatrix();
	myShadowMatrix(ground, light);
	glTranslatef(0, 1.7f, 0);
	glRotatef(CubeXrot, 1, 0, 0);
	glRotatef(CubeYrot, 0, 1, 0);
	glRotatef(CubeZrot, 0, 0, 1);
	glLineWidth(3);
	glEnable(GL_BLEND);
	glColor4f(0,0,0,0.3f);
	glutWireCube(2.0);
	glDisable(GL_BLEND);
	glLineWidth(1);
	glPopMatrix();
#else
	(void) ground;
	(void) light;
	(void) CubeZrot;
	(void) myShadowMatrix;
#endif
}


static void Idle( void )
{
	int t = glutGet(GLUT_ELAPSED_TIME);
	CubeXrot = 10 * 0.01f;
	CubeYrot = t * 0.05f;
	/*CubeZrot = t * 0.003;*/
	glutPostRedisplay();
}


static void StrokeString(const char *s)
{
   for (; *s; s++)
      glutStrokeCharacter(GLUT_STROKE_ROMAN, *s);
}


static void Draw( void )
{
	const GLfloat lookAtY = 1.0;
	const GLfloat aspect = (float) WinWidth / (float) WinHeight;
	/* Note: these values are also in stereo1.conf */
	const GLfloat eyeSep = .3f;  /* half of interocular distance */
	const GLfloat width = 12.0; /* width of frustum at FocalDist */
	const GLfloat hither = 1.0, yon = 25;  /* clipping planes */

        const GLfloat halfWidth = 0.5 * width;
	const GLfloat s = hither / FocalDist;  /* similar triangle ratio */
	GLfloat left, right, bottom, top;

	if (Stereo) {

		/* frustum top/bottom are same for both eyes */
		top = s * halfWidth;
		bottom = -top;

		/* left eye */
		glDrawBuffer(GL_BACK_LEFT);
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		left = s * ((halfWidth * -aspect) - eyeSep);
		right = s * ((halfWidth * aspect) - eyeSep);
		glFrustum( left, right, bottom, top, hither, yon );

		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();
                glRotatef(CameraRot, 0, 1, 0);
		glTranslatef(+eyeSep, -lookAtY, -ViewDist);

		glPushMatrix();
		glRotatef(ViewXrot, 1, 0, 0);
		glRotatef(ViewYrot, 0, 1, 0);
		glRotatef(ViewZrot, 0, 0, 1);
		DrawScene();
		glPopMatrix();

		/* right eye */
		glDrawBuffer(GL_BACK_RIGHT);
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		left = s * (halfWidth * -aspect + eyeSep);
		right = s * (halfWidth * aspect + eyeSep);
		glFrustum( left, right, bottom, top, hither, yon );

		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();
                glRotatef(CameraRot, 0, 1, 0);
		glTranslatef(-eyeSep, -lookAtY, -ViewDist);

		glPushMatrix();
		glRotatef(ViewXrot, 1, 0, 0);
		glRotatef(ViewYrot, 0, 1, 0);
		glRotatef(ViewZrot, 0, 0, 1);
		DrawScene();
		glPopMatrix();

		if (ShowOverlay)
			glDrawBuffer(GL_BACK); /* left and right back buffers*/
	}
	else {
		/* monoscopic */
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		right = s * halfWidth * aspect;
		left = -right;
		top = s * halfWidth;
		bottom = -top;
		glFrustum( left, right, bottom, top, hither, yon );

		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();
                glRotatef(CameraRot, 0, 1, 0);
		glTranslatef(0, -lookAtY, -ViewDist);

		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		glPushMatrix();
		glRotatef(ViewXrot, 1, 0, 0);
		glRotatef(ViewYrot, 0, 1, 0);
		glRotatef(ViewZrot, 0, 0, 1);
		DrawScene();
		glPopMatrix();
	}

	if (ShowOverlay) {
		glDisable(GL_DEPTH_TEST);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glColor3f(1, 1, 0);
		glBegin(GL_LINE_LOOP);
		glVertex2f(-.9f, -.9f);
		glVertex2f( .9f, -.9f);
		glVertex2f( .9f,  .9f);
		glVertex2f(-.9f,  .9f);
		glEnd();
#if 1
		glTranslatef(-0.8f, -0.8f, 0.0);
		glScalef(0.001f, 0.001f, 1);
		StrokeString("Stereocube demo");
#else
		(void) StrokeString;
#endif
		glEnable(GL_DEPTH_TEST);
	}

	glutSwapBuffers();
}


static void Reshape( int width, int height )
{
   WinWidth = width;
   WinHeight = height;
   glViewport( 0, 0, width, height );
}


static void Key( unsigned char key, int x, int y )
{
	const GLfloat step = 0.5;
	(void) x;
	(void) y;
	switch (key) {
	case 'a':
		Anim = !Anim;
		if (Anim) {
			glutIdleFunc(Idle);
		}
		else {
			glutIdleFunc(NULL);
		}
		break;
	case 'f':
		FocalDist -= step;
		break;
	case 'F':
		FocalDist += step;
		break;
	case 'v':
		ViewDist -= step;
		break;
	case 'V':
		ViewDist += step;
		break;
	case 'o':
		ShowOverlay = !ShowOverlay;
		break;
	case '0':
		ViewXrot = 30;
		ViewYrot = 0;
		ViewZrot = 0;
		ViewDist = 10.0;
		break;
	case 'y':
		CameraRot -= 2.0;
		break;
	case 'Y':
		CameraRot += 2.0;
		break;
	case 'q':
	case 27:
		exit(0);
		break;
	}
	glutPostRedisplay();
	printf("ViewDist = %f  FocalDist = %f\n", ViewDist, FocalDist);
}



static void SpecialKey( int key, int x, int y )
{
   const GLfloat step = 3.0;
   (void) x;
   (void) y;
   switch (key) {
      case GLUT_KEY_UP:
         if (ViewXrot < 90)
            ViewXrot += step;
         break;
      case GLUT_KEY_DOWN:
         if (ViewXrot > -step*2)
            ViewXrot -= step;
         break;
      case GLUT_KEY_LEFT:
         ViewYrot -= step;
         break;
      case GLUT_KEY_RIGHT:
         ViewYrot += step;
         break;
   }
   glutPostRedisplay();
}


static void Init( void )
{
	printf("GL_RENDERER: %s\n", (char *) glGetString(GL_RENDERER));
	printf("GL_VERSION: %s\n", (char *) glGetString(GL_VERSION));
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_FLAT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


int main( int argc, char *argv[] )
{
	int mode, i;
	int help = 0;

	glutInit( &argc, argv );
	mode = GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-ms") == 0) {
			mode |= GLUT_MULTISAMPLE;
		}
		else if (strcmp(argv[i], "-s") == 0) {
			Stereo = GL_TRUE;
			mode |= GLUT_STEREO;
		}
		else if (strcmp(argv[i], "-h") == 0) {
			help = 1;
		}
		else {
			printf("Bad option: %s\n", argv[i]);
			help = 1;
			break;
		}
	}

	if (help) {
		printf("Valid options:\n");
		printf("  -ms   enable multisample antialiasing\n");
		exit(0);
	}

	glutInitWindowSize( WinWidth, WinHeight );
	glutInitDisplayMode( mode );
	glutCreateWindow(argv[0]);
	glutReshapeFunc( Reshape );
	glutKeyboardFunc( Key );
	glutSpecialFunc( SpecialKey );
	glutDisplayFunc( Draw );
	if (Anim)
		glutIdleFunc(Idle);
	Init();
	glutMainLoop();
	return 0;
}
