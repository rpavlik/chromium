/*
 * Test glViewport by drawing four icosahedrons in the four quadrants
 * of the window.  Draw a colored quad behind each.
 *
 * Brian Paul
 * 13 Feb 2003
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>

static int TheWindow = 0;
static int Width = 300, Height = 300;
static GLboolean DrawExtraViewport = GL_TRUE;
static int VPx = 100, VPy = 100, VPw = 100, VPh = 100;


static void DrawObject( int x, int y, int w, int h, const GLfloat *color )
{
   const GLfloat z = -1;

   glViewport(x, y, w, h);

   glDisable(GL_LIGHTING);

   glColor3fv(color);
   glBegin(GL_POLYGON);
   glVertex3f(-1, -1, z);
   glVertex3f( 1, -1, z);
   glVertex3f( 1,  1, z);
   glVertex3f(-1,  1, z);
   glEnd();

   glEnable(GL_LIGHTING);

   glPushMatrix();
   glRotatef(30, 1, 0, 0);
   glRotatef(30, 0, 1, 0);
   glutSolidIcosahedron();
   glPopMatrix();
}


static void Display( void )
{
   static const GLfloat red[3] = { 0.9, 0.2, 0.2 };
   static const GLfloat green[3] = { 0.2, 0.9, 0.2 };
   static const GLfloat blue[3] = { 0.2, 0.2, 0.9 };
   static const GLfloat gray[3] = { 0.3, 0.3, 0.3 };
   static const GLfloat white[3] = { 0.9, 0.9, 0.9 };

   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   DrawObject(0, 0, Width/2, Height/2, red);
   DrawObject(Width/2, 0, Width - Width/2, Height/2, green);
   DrawObject(0, Height/2, Width/2, Height - Height/2, blue);
   DrawObject(Width/2, Height/2, Width - Width/2, Height - Height/2, gray);

   if (DrawExtraViewport) {
     DrawObject(VPx, VPy, VPw, VPh, white);
   }

   glutSwapBuffers();
}


static void Reshape( int width, int height )
{
   Width = width;
   Height = height;
   glMatrixMode( GL_PROJECTION );
   glLoadIdentity();
   glOrtho(-1, 1, -1, 1, -1, 1);
   glMatrixMode( GL_MODELVIEW );
   glLoadIdentity();
}


static void Key( unsigned char key, int x, int y )
{
   (void) x;
   (void) y;
   switch (key) {
      case 'w':
         VPw--;
         if (VPw < 0)
            VPw = 0;
         break;
      case 'W':
         VPw++;
         break;
      case 'h':
         VPh--;
         if (VPh < 0)
            VPh = 0;
         break;
      case 'H':
         VPh++;
         break;
      case 'v':
         DrawExtraViewport = !DrawExtraViewport;
         break;
      case 27:
         glutDestroyWindow(TheWindow);
         exit(0);
         break;
   }
   printf("Viewport %d, %d  %d x %d\n", VPx, VPy, VPw, VPh);
   glutPostRedisplay();
}


static void SpecialKey( int key, int x, int y )
{
   (void) x;
   (void) y;
   switch (key) {
      case GLUT_KEY_UP:
         VPy++;
         break;
      case GLUT_KEY_DOWN:
         VPy--;
         break;
      case GLUT_KEY_LEFT:
         VPx--;
         break;
      case GLUT_KEY_RIGHT:
         VPx++;
         break;
   }
   printf("Viewport %d, %d  %d x %d\n", VPx, VPy, VPw, VPh);
   glutPostRedisplay();
}

static void Init( void )
{
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_CULL_FACE); /* no depth test, just BF culling */
}


static void Usage( void )
{
   printf("Keys:\n");
   printf("  v - toggle drawing movable object/viewport.\n");
   printf("  w/W - decrease/increase movable viewport width.\n");
   printf("  h/H - decrease/increase movable viewport height.\n");
   printf("  left/right - move viewport left/right.\n");
   printf("  up/down - move viewport up/down.\n");
}


int main( int argc, char *argv[] )
{
   glutInit( &argc, argv );
   glutInitWindowSize( Width, Height );
   glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
   TheWindow = glutCreateWindow(argv[0]);
   glutReshapeFunc( Reshape );
   glutKeyboardFunc( Key );
   glutSpecialFunc( SpecialKey );
   glutDisplayFunc( Display );
   Init();
   Usage();
   glutMainLoop();
   return 0;
}
