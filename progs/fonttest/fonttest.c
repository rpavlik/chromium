/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <string.h>
#include <stdlib.h>
#include <GL/glut.h>

static void myReshape( int w, int h )
{
	glViewport( 0, 0, w, h );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0, w, 0, h, 0, 1 );
}

static void printstring(void *font, char *string)
{
  int len,i;

  len=(int)strlen(string);
  for(i=0;i<len;i++)
    glutBitmapCharacter(font,string[i]);
}

static void display( void )
{
	glClear( GL_COLOR_BUFFER_BIT );

	glRasterPos2i( 100, 100 );
	printstring( GLUT_BITMAP_TIMES_ROMAN_24, "Welcome to Chromium!" );
	glutSwapBuffers();
}

static void keyboard( unsigned char key, int x, int y )
{
	(void) key;
	(void) x;
	(void) y;
	exit(0);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(500, 500);
    glutCreateWindow(argv[0]);
    glutReshapeFunc(myReshape);
    glutDisplayFunc(display);
		glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}
