#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>

//Test glMaterial()
//
// One red triangle and one blue triangle are rendered.
//
// ***** Chromium renders two blue triangles *****
//
//
// Press q to quit
//
//June 28, 2001
//

GLfloat Red[] = {1.0, 0.0, 0.0, 1.0};
GLfloat Blue [] = {0.0, 0.0, 1.0, 1.0};

void
draw(void)
{
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glLoadIdentity();
  gluLookAt(0.5, 0.5, 1.2,
         0.5, 0.5, 0.0,
         1.0, 0.0, 0.0);

  glShadeModel(GL_FLAT);

  //a red triangle and a blue triangle
  glBegin(GL_TRIANGLES);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, Red);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(1.0, 0.0, 0.0);
  glVertex3f(1.0, 1.0, 0.0);

  glMaterialfv(GL_FRONT, GL_DIFFUSE, Blue);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(1.0, 1.0, 0.0);
  glVertex3f(0.0, 1.0, 0.0);
  glEnd();

  glutSwapBuffers();
}



void
keyboard(unsigned char ch, int x, int y)
{
	(void) x;
	(void) y;
  if ((ch == 'q')||(ch == 'Q'))
    exit(0);
}


int
main(int argc, char *argv[])
{
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutCreateWindow("Test of glMaterial");
  glutDisplayFunc(draw);
  glutKeyboardFunc(keyboard);

  glClearDepth(1.0);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glMatrixMode(GL_PROJECTION);
  glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 20);
  glMatrixMode(GL_MODELVIEW);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glDepthFunc(GL_LEQUAL);
  glEnable(GL_DEPTH_TEST);

  glutMainLoop();
  return 0;
}
