/*
 * Procedurally-generated cityscape.
 * This is a _really_ simplistic demo somewhat inspired by the
 * paper "Procedural Modeling of Cities" by Parish and Muller as
 * seen in the SIGGRAPH 2001 conference proceedings.
 * http://graphics.ethz.ch/Downloads/Publications/Papers/2001/p_Par01.pdf
 *
 * Possible improvements:
 *   - texturing
 *   - don't place buildings on the steets
 *   - skybox
 *
 * Copyright (C) 2002  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <memory.h>
#include <GL/glut.h>

#ifdef WINDOWS
#pragma warning(disable : 4125)
#endif

#include "wall2.h"
#include "roof.h"

unsigned char *wall;

struct building {
   float x, y, z;     /* pos */
   float sx, sy, sz;  /* size */
   float color[4];
};

#define WALL_WIDTH 128
#define WALL_HEIGHT 256
#define WALL_BYTES_PER_PIXEL 4

#define MAX_BUILDINGS 300

static struct building Buildings[MAX_BUILDINGS];
static int NumBuildings = 150;

static GLfloat Xrot = 30, Yrot = 0, Zrot = 0;
static GLfloat EyeDist = 30;
static GLboolean Anim = GL_TRUE;
static int StartTime = 0;
static GLfloat StartRot = 0;

static GLint CheckerRows = 20, CheckerCols = 20;
static GLboolean UseDisplayLists = GL_FALSE;
static GLboolean Texture = GL_TRUE;
static GLuint GroundList = 1;
static GLuint wallTex ;
static GLuint roofTex ;


static GLfloat
Random(GLfloat min, GLfloat max)
{
   GLfloat x = (float) rand() / (float) RAND_MAX;
   return x * (max - min) + min;
}


static void GenerateBuildings(void)
{
   int i;
   for (i = 0; i < MAX_BUILDINGS; i++) {
      GLfloat atten;
      GLfloat g;
      /* x/y position and size */
      Buildings[i].x = Random(-14, 14);
      Buildings[i].z = Random(-14, 14);
      Buildings[i].sx = Random(0.5, 2);
      Buildings[i].sz = Random(0.5, 2);
      /* make buildings in middle generally taller than outlying buildings */
#if 1
      atten = 0.2 + cos(Buildings[i].x / 12) * cos(Buildings[i].z / 12);
#else
      atten = 0.2 + cos(Buildings[i].x + Buildings[i].z / 100);
#endif
      atten = atten * atten;
      /* height */
      Buildings[i].sy = Random(0.2, 8) * atten;
      Buildings[i].y = 0.5 * Buildings[i].sy;
      g = Random(0.3, 0.7);
      Buildings[i].color[0] = g;
      Buildings[i].color[1] = g;
      Buildings[i].color[2] = g;
      Buildings[i].color[3] = 1.0;
   }
}

static void DrawBuildings(void)
{
   int i;
   GLfloat hc, xwc, zwc ;
   for (i = 0; i < NumBuildings; i++) {
      glPushMatrix();
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Buildings[i].color);

      glTranslatef(Buildings[i].x, Buildings[i].y, Buildings[i].z);
      glScalef(Buildings[i].sx, Buildings[i].sy, Buildings[i].sz);

      hc = floor( 2.*Buildings[i].sy ) ;
      xwc = floor( 5.*Buildings[i].sx ) ;
      zwc = floor( 5.*Buildings[i].sz ) ;
      if ( hc < 1 ) hc = 1. ;
      if ( xwc < 1 ) xwc = 1. ;
      if ( zwc < 1 ) zwc = 1. ;

      if (Texture) {
         glEnable( GL_TEXTURE_2D ) ;
         glBindTexture( GL_TEXTURE_2D, wallTex ) ;
      }

      glBegin( GL_QUADS ) ;

      glNormal3f(  0.0,  0.0,  1.0 ) ;
      glTexCoord2f( 0.0, 0.0 ) ; glVertex3f( -0.5, -0.5,  0.5 ) ;
      glTexCoord2f( xwc, 0.0 ) ; glVertex3f(  0.5, -0.5,  0.5 ) ;
      glTexCoord2f( xwc, hc  ) ; glVertex3f(  0.5,  0.5,  0.5 ) ;
      glTexCoord2f( 0.0, hc  ) ; glVertex3f( -0.5,  0.5,  0.5 ) ;

      glNormal3f(  1.0,  0.0,  0.0 ) ;
      glTexCoord2f( 0.0, 0.0 ) ; glVertex3f(  0.5, -0.5,  0.5 ) ;
      glTexCoord2f( zwc, 0.0 ) ; glVertex3f(  0.5, -0.5, -0.5 ) ;
      glTexCoord2f( zwc, hc  ) ; glVertex3f(  0.5,  0.5, -0.5 ) ;
      glTexCoord2f( 0.0, hc  ) ; glVertex3f(  0.5,  0.5,  0.5 ) ;

      glNormal3f(  0.0,  0.0, -1.0 ) ;
      glTexCoord2f( 0.0, 0.0 ) ; glVertex3f(  0.5, -0.5, -0.5 ) ;
      glTexCoord2f( xwc, 0.0 ) ; glVertex3f( -0.5, -0.5, -0.5 ) ;
      glTexCoord2f( xwc, hc  ) ; glVertex3f( -0.5,  0.5, -0.5 ) ;
      glTexCoord2f( 0.0, hc  ) ; glVertex3f(  0.5,  0.5, -0.5 ) ;

      glNormal3f( -1.0,  0.0,  0.0 ) ;
      glTexCoord2f( 0.0, 0.0 ) ; glVertex3f( -0.5,  0.5, -0.5 ) ;
      glTexCoord2f( 0.0, hc  ) ; glVertex3f( -0.5, -0.5, -0.5 ) ;
      glTexCoord2f( zwc, hc  ) ; glVertex3f( -0.5, -0.5,  0.5 ) ;
      glTexCoord2f( zwc, 0.0 ) ; glVertex3f( -0.5,  0.5,  0.5 ) ;

      glEnd() ;

      if (Texture)
         glBindTexture( GL_TEXTURE_2D, roofTex ) ;
      glBegin( GL_QUADS ) ;

      glNormal3f(  0.0,  1.0,  0.0 ) ;
      glTexCoord2f( 0.0, 0.0 ) ; glVertex3f( -0.5,  0.5, -0.5 ) ;
      glTexCoord2f( 1.0, 0.0 ) ; glVertex3f( -0.5,  0.5,  0.5 ) ;
      glTexCoord2f( 1.0, 1.0 ) ; glVertex3f(  0.5,  0.5,  0.5 ) ;
      glTexCoord2f( 0.0, 1.0 ) ; glVertex3f(  0.5,  0.5, -0.5 ) ;

      glEnd() ;
      glDisable( GL_TEXTURE_2D ) ;
      /*glutSolidCube(1.0);*/
      glPopMatrix();
   }
}


static void Idle( void )
{
   int time = glutGet(GLUT_ELAPSED_TIME);
   Yrot = StartRot + (time - StartTime) / 100.0;
   glutPostRedisplay();
}


static void DrawGround(void)
{
   static const GLfloat colors[2][3] = {
      { 0.3, 0.6, 0.3 },
      { 0.4, 0.4, 0.4 }
   };
   int i, j;
   float stepx = 80.0 / CheckerCols;
   float stepy = 80.0 / CheckerRows;
   float x, y, z, dx, dy;

   glShadeModel(GL_FLAT);
   glDisable(GL_LIGHTING);
   z = 0.0;
   y = -20.0;
   for (i = 0; i < CheckerRows; i++) {
      x = -20.0;
      if (i & 1)
         dy = 0.95 * stepy;
      else
         dy = 0.05 * stepy;
      glBegin(GL_QUAD_STRIP);
      for (j = 0; j <= CheckerCols; j++) {
         int k = ((i & 1) == 0) || ((j & 1) == 1);
         if (j & 1)
            dx = 0.95 * stepx;
         else
            dx = 0.05 * stepx;

         glColor3fv(colors[k]);
         glVertex3f(x, z, y);
         glVertex3f(x, z, y + dy);
         x += dx;
      }
      glEnd();
      y += dy;
   }
}


static void DrawScene( void )
{
   GLfloat lightPos[4] = { 20, 90, 90, 0 };
   GLfloat lightPos1[4] = { -40, 90, -90, 0 };

   glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
   glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);

   glShadeModel(GL_FLAT);
   glDisable(GL_LIGHTING);
   if (UseDisplayLists)
      glCallList(GroundList);
   else
      DrawGround();

   glEnable(GL_LIGHTING);
   glEnable(GL_CULL_FACE);
   DrawBuildings();
   glDisable(GL_CULL_FACE);
}


static void Display( void )
{
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   glPushMatrix();
   glRotatef(Xrot, 1, 0, 0);
   glRotatef(Yrot, 0, 1, 0);
   glRotatef(Zrot, 0, 0, 1);

   DrawScene();

   glPopMatrix();

   glutSwapBuffers();

  {
     static GLint T0 = 0;
     static GLint Frames = 0;
     GLint t = glutGet(GLUT_ELAPSED_TIME);
     Frames++;
     if (t - T0 >= 5000) {
        GLfloat seconds = (t - T0) / 1000.0;
        GLfloat fps = Frames / seconds;
        printf("%d frames in %6.3f seconds = %6.3f FPS\n", Frames, seconds, fps);
        T0 = t;
        Frames = 0;
     }
  }
}


static void Reshape( int width, int height )
{
   GLfloat ar = 0.5 * (float) width / (float) height;
   glViewport( 0, 0, width, height );
   glMatrixMode( GL_PROJECTION );
   glLoadIdentity();
   glFrustum( -ar, ar, -0.5, 0.5, 1.5, 150.0 );
   glMatrixMode( GL_MODELVIEW );
   glLoadIdentity();
   glTranslatef( 0.0, 0.0, -EyeDist );
}


static void Key( unsigned char key, int x, int y )
{
   (void) x;
   (void) y;
   switch (key) {
      case 'a':
         Anim = !Anim;
         if (Anim) {
            glutIdleFunc(Idle);
            StartRot = Yrot;
            StartTime = glutGet(GLUT_ELAPSED_TIME);
         }
         else {
            glutIdleFunc(NULL);
         }
         break;
      case 'b':
         if (NumBuildings > 1)
            NumBuildings--;
         break;
      case 'B':
         if (NumBuildings < MAX_BUILDINGS)
            NumBuildings++;
         break;
      case 'd':
         UseDisplayLists = !UseDisplayLists;
         printf("Use display lists: %d\n", (int) UseDisplayLists);
         break;
      case 't':
         Texture = !Texture;
         break;
      case 'z':
         EyeDist -= 1;
         glMatrixMode( GL_MODELVIEW );
         glLoadIdentity();
         glTranslatef( 0.0, 0.0, -EyeDist );
         break;
      case 'Z':
         EyeDist += 1;
         glMatrixMode( GL_MODELVIEW );
         glLoadIdentity();
         glTranslatef( 0.0, 0.0, -EyeDist );
         break;
      case ' ':
         GenerateBuildings();
         break;
      case 'q':
      case 27:
         exit(0);
         break;
   }
   glutPostRedisplay();
}


static void SpecialKey( int key, int x, int y )
{
   const GLfloat step = 3.0;
   (void) x;
   (void) y;
   switch (key) {
      case GLUT_KEY_UP:
         if (Xrot < 90)
            Xrot += step;
         break;
      case GLUT_KEY_DOWN:
         if (Xrot > step*2)
            Xrot -= step;
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


static void Init( void )
{
   GLfloat one[4] = { 1, 1, 1, 1 };
   GLfloat amb[4] = { 0.4, 0.4, 0.4, 1.0 };

   glEnable(GL_NORMALIZE);
   glEnable(GL_DEPTH_TEST);

   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
   glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
   glLightfv(GL_LIGHT1, GL_DIFFUSE, one);

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_LIGHT1);

   GenerateBuildings();

   glNewList(GroundList, GL_COMPILE);
   DrawGround();
   glEndList();

   glGenTextures( 1, &wallTex ) ;
   glBindTexture( GL_TEXTURE_2D, wallTex ) ;
   glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL ) ;
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ) ;
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ) ;
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) ;
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ) ;
   glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, WALL_WIDTH, WALL_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, wall ) ;

   glGenTextures( 1, &roofTex ) ;
   glBindTexture( GL_TEXTURE_2D, roofTex ) ;
   glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL ) ;
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ) ;
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ) ;
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) ;
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ) ;
   glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, roof.width, roof.height, 0, GL_RGB, GL_UNSIGNED_BYTE, roof.pixel_data ) ;
}


int main( int argc, char *argv[] )
{
   int mode;

   /* Work around compiler issues that have 64k limits */
   wall = malloc(WALL_WIDTH * WALL_HEIGHT * WALL_BYTES_PER_PIXEL);
   memcpy(wall, walldata1, sizeof(walldata1) );
   memcpy(wall + sizeof(walldata1), walldata2, sizeof(walldata2) );
   memcpy(wall + sizeof(walldata1) + sizeof(walldata2), walldata3, sizeof(walldata3) );

   glutInit( &argc, argv );
   mode = GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH;
   if (argc > 1 && strcmp(argv[1], "-ms") == 0)
      mode |= GLUT_MULTISAMPLE;

   glutInitWindowSize( 400, 250 );
   glutInitDisplayMode( mode );
   glutCreateWindow(argv[0]);
   glutReshapeFunc( Reshape );
   glutKeyboardFunc( Key );
   glutSpecialFunc( SpecialKey );
   glutDisplayFunc( Display );
   if (Anim) {
      glutIdleFunc(Idle);
      StartRot = Yrot;
      StartTime = glutGet(GLUT_ELAPSED_TIME);
   }
   Init();
   glutMainLoop();
   return 0;
}
