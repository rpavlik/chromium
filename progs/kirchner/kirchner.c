#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>

//Test glColorMaterial():
//
//Draw three frames; use glColorMaterial to change
//the material color..
//
//press any key on the keyboard to switch between
//the frames
//
//June 20, 2001


void
draw(void)
{

  static int i = 0;


  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
   glShadeModel(GL_FLAT);

  glLoadIdentity();
  gluLookAt(0.5, 0.5, 1.2,
         0.5, 0.5, 0.0,
         1.0, 0.0, 0.0);

  switch(i){
  case 0:
  case 2:
     //green triangles

     glColor3f(0.0, 1.0, 0.0);
     glDisable(GL_COLOR_MATERIAL);
     glShadeModel(GL_FLAT);

     glBegin(GL_TRIANGLES);
     glNormal3f(0.0, 0.0, 1.0);
     glVertex3f(0.0, 0.0, 0.0);
     glVertex3f(1.0, 0.0, 0.0);
     glVertex3f(1.0, 1.0, 0.0);

     glNormal3f(0.0, 0.0, 1.0);
     glVertex3f(0.0, 0.0, 0.0);
     glVertex3f(1.0, 1.0, 0.0);
     glVertex3f(0.0, 1.0, 0.0);
     glEnd();
     i++;

     break;


  case 1:
    //a red triangle and a blue triangle
    glShadeModel(GL_FLAT);
    glEnable(GL_COLOR_MATERIAL);
    glColor3f(0.0, 1.0, 0.0);

    glBegin(GL_TRIANGLES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(1.0, 0.0, 0.0);
    glVertex3f(1.0, 1.0, 0.0);

    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(1.0, 1.0, 0.0);
    glVertex3f(0.0, 1.0, 0.0);
    glEnd();


    i++;

    break;



   default:
      exit(0);
  }


  glutSwapBuffers();
  // sleep(2);
  // glutPostRedisplay();
}



void
keyboard(unsigned char ch, int x, int y)
{
	// modified by Humper to not explode on Windows
	(void) ch;
	(void) x;
	(void) y;
  glutPostRedisplay();
}




int
main(int argc, char *argv[])
{
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH |
GLUT_MULTISAMPLE);
  glutCreateWindow("Test of glColorMaterial in WireGL");
  glutDisplayFunc(draw);
  glutKeyboardFunc(keyboard);


  glClearDepth(1.0);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glMatrixMode(GL_PROJECTION);
  glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 20);
  glMatrixMode(GL_MODELVIEW);

  // printf("my pid is %d\n", getpid());
  //sleep(10);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glDepthFunc(GL_LEQUAL);
  glEnable(GL_DEPTH_TEST);
  glColorMaterial(GL_FRONT, GL_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);

  glutMainLoop();
  return 0;
}
