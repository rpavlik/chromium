/*
 * Test tilesort SPU bucketing with display lists and bounding boxes.
 *
 * Brian Paul
 * 3 Feb 2003
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include "chromium.h"


static glChromiumParametervCRProc glChromiumParametervCR_ptr;
static glChromiumParameteriCRProc glChromiumParameteriCR_ptr;
static glGetChromiumParametervCRProc glGetChromiumParametervCR_ptr;

/* Icosahedron radius is 1 (according to comments in GLUT) */
static GLfloat ObjectBoundingBox[8] = {
	-1.0, -1.0, -1.0,							/* xmin, ymin, zmin */
	1.0, 1.0, 1.0,								/* xmax, ymax, zmax */
};

static const GLfloat Red[4] = { 1, .2, .2, 1 };
static const GLfloat Green[4] = { .2, 1, .2, 1 };

static GLuint Objects[2];
static GLboolean UseBoundingBox = 1;
static GLboolean UseDisplayLists = 1;
static GLboolean Anim = 0;
static GLfloat Zrot = 0;
static GLint NumServers = 0;


static void
Idle(void)
{
	Zrot = glutGet(GLUT_ELAPSED_TIME) * .03;
	glutPostRedisplay();
}


static void
Redisplay(void)
{
	if (!UseBoundingBox)
		glChromiumParametervCR_ptr(GL_DEFAULT_BBOX_CR, GL_FLOAT, 0, NULL);

	if (NumServers == 2 && !UseDisplayLists)
	{
		glChromiumParameteriCR_ptr(GL_RESET_VERTEX_COUNTERS_CR, 0);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();
	glRotatef(Zrot, 0, 0, 1);

	/* first object */
	glPushMatrix();
	glTranslatef(-2, 0, 0);
	if (UseBoundingBox)
		glChromiumParametervCR_ptr(GL_OBJECT_BBOX_CR, GL_FLOAT, 6,
															 ObjectBoundingBox);
	if (UseDisplayLists)
	{
		glCallList(Objects[0]);
	}
	else
	{
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, Red);
		glutSolidIcosahedron();
	}
	glPopMatrix();

	/* second object */
	glPushMatrix();
	glTranslatef(2, 0, 0);
	if (UseBoundingBox)
		glChromiumParametervCR_ptr(GL_OBJECT_BBOX_CR, GL_FLOAT, 6,
															 ObjectBoundingBox);
	if (UseDisplayLists)
	{
		glCallList(Objects[1]);
	}
	else
	{
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, Green);
		glutSolidIcosahedron();
	}
	glPopMatrix();

	/* disable bounding box */
	glChromiumParametervCR_ptr(GL_DEFAULT_BBOX_CR, GL_FLOAT, 0, NULL);

	glPopMatrix();

	glutSwapBuffers();

	if (NumServers == 2 && !UseDisplayLists)
	{
		GLint vertices[2];
		/* report vertices sent to each server */
		glGetChromiumParametervCR_ptr(GL_VERTEX_COUNTS_CR, 0,
																	GL_INT, NumServers, vertices);
		printf("Vertices drawn:  server[0]: %d  server[1]: %d\n",
					 vertices[0], vertices[1]);
	}
}


static void
Reshape(int width, int height)
{
	GLfloat ar = (float) width / (float) height;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-ar, ar, -1.0, 1.0, 5.0, 25.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -15.0);
}


static void
Key(unsigned char key, int x, int y)
{
	const GLfloat step = 3.0;
	(void) x;
	(void) y;
	switch (key)
	{
	case ' ':
		Anim = !Anim;
		if (Anim)
			glutIdleFunc(Idle);
		else
			glutIdleFunc(NULL);
		break;
	case 'b':
		UseBoundingBox = !UseBoundingBox;
		printf("Use bounding box? %c\n", "NY"[UseBoundingBox]);
		break;
	case 'd':
		UseDisplayLists = !UseDisplayLists;
		printf("Use display lists? %c\n", "NY"[UseDisplayLists]);
		break;
	case 'z':
		Zrot -= step;
		break;
	case 'Z':
		Zrot += step;
		break;
	case 27:
		exit(0);
		break;
	}
	glutPostRedisplay();
}


static void
Init(void)
{
	/* Get chromium extension function pointers */
	glChromiumParametervCR_ptr =
		(glChromiumParametervCRProc) crGetProcAddress("glChromiumParametervCR");
	assert(glChromiumParametervCR_ptr);

	glChromiumParameteriCR_ptr =
		(glChromiumParameteriCRProc) crGetProcAddress("glChromiumParameteriCR");
	assert(glChromiumParameteriCR_ptr);

	glGetChromiumParametervCR_ptr =
		(glGetChromiumParametervCRProc)
		crGetProcAddress("glGetChromiumParametervCR");
	assert(glGetChromiumParametervCR_ptr);

	/* setup lighting, etc */
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	Objects[0] = glGenLists(1);
	glNewList(Objects[0], GL_COMPILE);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, Red);
	glutSolidIcosahedron();
	glEndList();

	Objects[1] = glGenLists(1);
	glNewList(Objects[1], GL_COMPILE);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, Green);
	glutSolidIcosahedron();
	glEndList();

	/* check if we're running with a 2-server tilesort */
	glGetChromiumParametervCR_ptr(GL_NUM_SERVERS_CR, 0,
																GL_INT, 1, (void *) &NumServers);
	printf("Num servers: %d\n", NumServers);

	printf("Press 'z'/'Z' to rotate.\n");
	printf("Press 'b' to toggle bounding boxes.\n");
	printf("Press 'd' to toggle dislay lists.\n");
}


int
main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitWindowPosition(100, 200);
	glutInitWindowSize(300, 200);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow(argv[0]);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Key);
	if (Anim)
		glutIdleFunc(Idle);
	glutDisplayFunc(Redisplay);
	Init();
	glutMainLoop();
	return 0;
}
