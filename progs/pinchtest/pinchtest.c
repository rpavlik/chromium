/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>

int num_tris = 100;

void display( void )
{
	int i;
	glBegin( GL_TRIANGLES );
	for (i = 0 ; i < num_tris ; i++)
	{
		glVertex2f( 0,0 );
		glVertex2f( .5, 0 );
		glVertex2f( 0, .5 );
	}
	glEnd();
	glutSwapBuffers();
} 

void keyboard( unsigned char key, int x, int y )
{
	(void) key;
	(void) x;
	(void) y;
	exit(0);
}

int main(int argc, char **argv)
{
	if (argc == 2)
	{
		sscanf( argv[1], "%d", &num_tris );
	}
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(500, 500);
	glutCreateWindow(argv[0]);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMainLoop();
	return 0;
}
