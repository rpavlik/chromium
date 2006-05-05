/*
 * Simple test program for dist_texture SPU.
 */


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glut.h>


static GLfloat Xrot = 0, Yrot = 0, Zrot = 0;
static GLboolean Anim = GL_TRUE;


static void
Idle(void)
{
   Zrot += 0.5;
   glutPostRedisplay();
}


static void
Draw(void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glPushMatrix();
   glRotatef(Xrot, 1, 0, 0);
   glRotatef(Yrot, 0, 1, 0);
   glRotatef(Zrot, 0, 0, 1);

   glBegin(GL_POLYGON);
   glTexCoord2f(0, 0);  glVertex2f(-1, -1);
   glTexCoord2f(1, 0);  glVertex2f( 1, -1);
   glTexCoord2f(1, 1);  glVertex2f( 1,  1);
   glTexCoord2f(0, 1);  glVertex2f(-1,  1);
   glEnd();

   glPopMatrix();

   glutSwapBuffers();
}


static void
Reshape(int width, int height)
{
   glViewport(0, 0, width, height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum(-1.0, 1.0, -1.0, 1.0, 5.0, 25.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(0.0, 0.0, -8.0);
}


static void
Key(unsigned char key, int x, int y)
{
   const GLfloat step = 3.0;
   (void) x;
   (void) y;
   switch (key) {
      case 'a':
         Anim = !Anim;
         if (Anim)
            glutIdleFunc(Idle);
         else
            glutIdleFunc(NULL);
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
SpecialKey(int key, int x, int y)
{
   const GLfloat step = 3.0;
   (void) x;
   (void) y;
   switch (key) {
      case GLUT_KEY_UP:
         Xrot -= step;
         break;
      case GLUT_KEY_DOWN:
         Xrot += step;
         break;
      case GLUT_KEY_LEFT:
         Yrot -= step;
         break;
      case GLUT_KEY_RIGHT:
         Yrot += step;
         break;
   }
   glutPostRedisplay();
}


static void
Init(int argc, char *argv[])
{
   const int width = 256, height = 256, border = 0;
   const char *filename = "distteximage.png";
   GLubyte *buffer;
   int readMode = 0, writeMode = 0;
   GLenum type;
   char *renderer = (char *) glGetString(GL_RENDERER);

   printf("Entering Init\n");

   if (strcmp(renderer, "Chromium") != 0) {
      printf("Error: disttexture program MUST be run with Chromium.\n");
      printf("GL_RENDERER = %s\n", renderer);
      exit(1);
   }

   if (argc < 2) {
      printf("Error: disttexture requires -read or -write option!\n");
      exit(1);
   }
   if (strcmp(argv[1], "-read") == 0) {
      readMode = 1;
   }
   else if (strcmp(argv[1], "-write") == 0) {
      writeMode = 1;
   }
   else {
      printf("Error: Invalid parameter: %s\n", argv[1]);
      exit(1);
   }

   if (writeMode) {
      /* make test texture */
      GLubyte image[256][256][3];
      int i, j;
      for (i = 0; i < height; i++) {
         for (j = 0; j < width; j++) {
            /* checker w/ gradient */
            if ((i & 0x20) ^ (j & 0x20)) {
               image[i][j][0] = i;
               image[i][j][1] = j;
               image[i][j][2] = 0;
            }
            else {
               image[i][j][0] = 0;
               image[i][j][1] = 255-i;
               image[i][j][2] = j;
            }
         }
      }
      /* allocate buffer for both filename and pixel data */
      buffer = (GLubyte *) malloc(width * height * 3 + strlen(filename) + 1);
      assert(buffer);
      /* filename comes first, with terminating zero */
      strcpy((char *) buffer, filename);
      /* image data follows */
      memcpy(buffer + strlen(filename) + 1, image, width * height * 3);
      type = GL_TRUE;
   }
   else {
      /* the texture will be read from file produced by the "write" step */
      assert(readMode);
      buffer = (GLubyte *) filename;
      type = GL_FALSE;
   }


   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, border,
                GL_RGB, type, buffer);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glEnable(GL_TEXTURE_2D);
}


int
main(int argc, char *argv[])
{
   printf("Entering main!\n");
   glutInit(&argc, argv);
   glutInitWindowPosition(700, 0);
   glutInitWindowSize(400, 400);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
   glutCreateWindow(argv[0]);
   glutReshapeFunc(Reshape);
   glutKeyboardFunc(Key);
   glutSpecialFunc(SpecialKey);
   glutDisplayFunc(Draw);
   if (Anim)
      glutIdleFunc(Idle);
   Init(argc, argv);
   glutMainLoop();
   return 0;
}
