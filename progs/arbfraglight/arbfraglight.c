/*
 * Use GL_ARB_fragment_program and GL_ARB_vertex_program to implement
 * simple per-pixel lighting.
 *
 * Brian Paul
 * 17 April 2003
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include "chromium.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static GLfloat Diffuse[4] = { 0.5, 0.5, 1.0, 1.0 };
static GLfloat Specular[4] = { 0.8, 0.8, 0.8, 1.0 };
static GLfloat LightPos[4] = { 0.0, 10.0, 20.0, 1.0 };
static GLfloat Delta = 1.0;

static GLuint FragProg;
static GLuint VertProg;
static GLboolean Anim = GL_TRUE;
static GLboolean Wire = GL_FALSE;
static GLboolean PixelLight = GL_TRUE;

static GLfloat Xrot = 0, Yrot = 0;

/* We're making our own typedefs here since NVIDIA's gl.h is lacking them. */
typedef void (APIENTRY * glProgramLocalParameter4dARB_t) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY * glProgramLocalParameter4dvARB_t) (GLenum target, GLuint index, const GLdouble *params);
typedef void (APIENTRY * glProgramLocalParameter4fvARB_t) (GLenum target, GLuint index, const GLfloat *params);
typedef void (APIENTRY * glGetProgramLocalParameterdvARB_t) (GLenum target, GLuint index, GLdouble *params);
typedef void (APIENTRY * glProgramEnvParameter4dARB_t) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY * glProgramEnvParameter4dvARB_t) (GLenum target, GLuint index, const GLdouble *params);
typedef void (APIENTRY * glProgramEnvParameter4fARB_t) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY * glProgramEnvParameter4fvARB_t) (GLenum target, GLuint index, const GLfloat *params);
typedef void (APIENTRY * glGetProgramEnvParameterdvARB_t) (GLenum target, GLuint index, GLdouble *params);
typedef void (APIENTRY * glGetProgramEnvParameterfvARB_t) (GLenum target, GLuint index, GLfloat *params);
typedef void (APIENTRY * glGenProgramsARB_t) (GLsizei n, GLuint *programs);
typedef void (APIENTRY * glProgramStringARB_t) (GLenum target, GLenum format, GLsizei len, const GLvoid *string);
typedef void (APIENTRY * glBindProgramARB_t) (GLenum target, GLuint program);
typedef GLboolean (APIENTRY * glIsProgramARB_t) (GLuint program);
typedef void (APIENTRY * glVertexAttrib3fARB_t) (GLuint index, GLfloat x, GLfloat y, GLfloat z);

static glProgramLocalParameter4fvARB_t glProgramLocalParameter4fvARB_func;
static glProgramLocalParameter4dvARB_t glProgramLocalParameter4dvARB_func;
static glProgramLocalParameter4dARB_t glProgramLocalParameter4dARB_func;
static glGetProgramLocalParameterdvARB_t glGetProgramLocalParameterdvARB_func;

static glProgramEnvParameter4fvARB_t glProgramEnvParameter4fvARB_func;
static glProgramEnvParameter4fARB_t glProgramEnvParameter4fARB_func;
static glProgramEnvParameter4dvARB_t glProgramEnvParameter4dvARB_func;
static glProgramEnvParameter4dARB_t glProgramEnvParameter4dARB_func;
static glGetProgramEnvParameterdvARB_t glGetProgramEnvParameterdvARB_func;
static glGetProgramEnvParameterfvARB_t glGetProgramEnvParameterfvARB_func;

static glGenProgramsARB_t glGenProgramsARB_func;
static glProgramStringARB_t glProgramStringARB_func;
static glBindProgramARB_t glBindProgramARB_func;
static glIsProgramARB_t glIsProgramARB_func;

static glVertexAttrib3fARB_t glVertexAttrib3fARB_func;


/* These must match the indexes used in the fragment program */
#define DIFFUSE 1
#define SPECULAR 2
#define LIGHTPOS 3


/*
 * Draw a sphere with normal vectors.
 * Code borrowed from Mesa's GLU.
 */
static void sphere(GLdouble radius, GLint slices, GLint stacks)
{
	/* Either use conventional vertex attributes or generic attributes */
#if 0  /* XXX this must match the vertex program below! */
#define VERTEX3F(x, y, z)  glVertex3f(x, y, z)
#define NORMAL3F(x, y, z)  glNormal3f(x, y, z)
#else
#define VERTEX3F(x, y, z)  glVertexAttrib3fARB_func(0, x, y, z)
#define NORMAL3F(x, y, z)  glVertexAttrib3fARB_func(2, x, y, z)
#endif
	GLfloat rho, drho, theta, dtheta;
	GLfloat x, y, z;
	GLfloat s, t, ds, dt;
	GLint i, j, imin, imax;
	GLfloat nsign = 1;

	drho = M_PI / (GLfloat) stacks;
	dtheta = 2.0 * M_PI / (GLfloat) slices;

	/* draw +Z end as a triangle fan */
	glBegin(GL_TRIANGLE_FAN);
	NORMAL3F(0.0, 0.0, 1.0);
	VERTEX3F(0.0, 0.0, nsign * radius);
	for (j = 0; j <= slices; j++) {
		theta = (j == slices) ? 0.0 : j * dtheta;
		x = -sin(theta) * sin(drho);
		y = cos(theta) * sin(drho);
		z = nsign * cos(drho);
		NORMAL3F(x * nsign, y * nsign, z * nsign);
		VERTEX3F(x * radius, y * radius, z * radius);
	}
	glEnd();

	ds = 1.0 / slices;
	dt = 1.0 / stacks;
	t = 1.0;			/* because loop now runs from 0 */
	imin = 1;
	imax = stacks - 1;

	/* draw intermediate stacks as quad strips */
	for (i = imin; i < imax; i++) {
		rho = i * drho;
		glBegin(GL_QUAD_STRIP);
		s = 0.0;
		for (j = 0; j <= slices; j++) {
			theta = (j == slices) ? 0.0 : j * dtheta;
			x = -sin(theta) * sin(rho);
			y = cos(theta) * sin(rho);
			z = nsign * cos(rho);
			NORMAL3F(x * nsign, y * nsign, z * nsign);
			VERTEX3F(x * radius, y * radius, z * radius);
			x = -sin(theta) * sin(rho + drho);
			y = cos(theta) * sin(rho + drho);
			z = nsign * cos(rho + drho);
			NORMAL3F(x * nsign, y * nsign, z * nsign);
			VERTEX3F(x * radius, y * radius, z * radius);
			s += ds;
		}
		glEnd();
		t -= dt;
	}

	/* draw -Z end as a triangle fan */
	glBegin(GL_TRIANGLE_FAN);
	NORMAL3F(0.0, 0.0, -1.0);
	VERTEX3F(0.0, 0.0, -radius * nsign);
	rho = M_PI - drho;
	s = 1.0;
	t = dt;
	for (j = slices; j >= 0; j--) {
	    theta = (j == slices) ? 0.0 : j * dtheta;
	    x = -sin(theta) * sin(rho);
	    y = cos(theta) * sin(rho);
	    z = nsign * cos(rho);
		NORMAL3F(x * nsign, y * nsign, z * nsign);
	    VERTEX3F(x * radius, y * radius, z * radius);
	    s -= ds;
	}
	glEnd();
}


static void Redisplay( void )
{
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   if (PixelLight) {
      glProgramLocalParameter4fvARB_func(GL_FRAGMENT_PROGRAM_ARB,
                                         LIGHTPOS, LightPos);
      glEnable(GL_FRAGMENT_PROGRAM_ARB);
      glEnable(GL_VERTEX_PROGRAM_ARB);
      glDisable(GL_LIGHTING);
   }
   else {
      glLightfv(GL_LIGHT0, GL_POSITION, LightPos);
      glDisable(GL_FRAGMENT_PROGRAM_ARB);
      glDisable(GL_VERTEX_PROGRAM_ARB);
      glEnable(GL_LIGHTING);
   }

   glPushMatrix();
   glRotatef(Xrot, 1, 0, 0);
   glRotatef(Yrot, 0, 1, 0);
   sphere(2.0, 10, 5);
   glPopMatrix();

   glutSwapBuffers();
}


static void Idle(void)
{
   LightPos[0] += Delta;
   if (LightPos[0] > 25.0)
      Delta = -1.0;
   else if (LightPos[0] <- 25.0)
      Delta = 1.0;
   glutPostRedisplay();
}


static void Reshape( int width, int height )
{
   glViewport( 0, 0, width, height );
   glMatrixMode( GL_PROJECTION );
   glLoadIdentity();
   glFrustum( -1.0, 1.0, -1.0, 1.0, 5.0, 25.0 );
   glMatrixMode( GL_MODELVIEW );
   glLoadIdentity();
   glTranslatef( 0.0, 0.0, -15.0 );
}


static void Key( unsigned char key, int x, int y )
{
   (void) x;
   (void) y;
   switch (key) {
     case ' ':
        Anim = !Anim;
        if (Anim)
           glutIdleFunc(Idle);
        else
           glutIdleFunc(NULL);
        break;
      case 'x':
         LightPos[0] -= 1.0;
         break;
      case 'X':
         LightPos[0] += 1.0;
         break;
      case 'w':
         Wire = !Wire;
         if (Wire)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
         else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
         break;
      case 'p':
         PixelLight = !PixelLight;
         if (PixelLight) {
            printf("Per-pixel lighting\n");
         }
         else {
            printf("Conventional lighting\n");
         }
         break;
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


/* A helper for finding errors in program strings */
static int FindLine( const char *program, int position )
{
   int i, line = 1;
   for (i = 0; i < position; i++) {
      if (program[i] == '\n')
         line++;
   }
   return line;
}


static void Init( void )
{
   GLint errorPos;
   GLenum err;

   /* Yes, this could be expressed more efficiently */
   static const char *fragProgramText =
      "!!ARBfp1.0\n"
      "PARAM Diffuse = program.local[1]; \n"
      "PARAM Specular = program.local[2]; \n"
      "PARAM LightPos = program.local[3]; \n"
      "TEMP lightDir, normal, len; \n"
      "TEMP dotProd, specAtten; \n"
      "TEMP diffuseColor, specularColor; \n"

      "# Compute normalized light direction \n"
      "DP3 len.x, LightPos, LightPos; \n"
      "RSQ len.y, len.x; \n"
      "MUL lightDir, LightPos, len.y; \n"

      "# Compute normalized normal \n"
      "DP3 len.x, fragment.texcoord[0], fragment.texcoord[0]; \n"
      "RSQ len.y, len.x; \n"
      "MUL normal, fragment.texcoord[0], len.y; \n"

      "# Compute dot product of light direction and normal vector\n"
      "DP3 dotProd, lightDir, normal;"

      "MUL diffuseColor, Diffuse, dotProd;            # diffuse attenuation\n"

      "POW specAtten.x, dotProd.x, {20.0}.x;          # specular exponent\n"

      "MUL specularColor, Specular, specAtten.x;      # specular attenuation\n"

      "ADD result.color, diffuseColor, specularColor; # add colors\n"
      "END \n"
      ;

   static const char *vertProgramText =
      "!!ARBvp1.0\n"
      "ATTRIB pos = vertex.position; \n"
      "#ATTRIB norm = vertex.normal; \n"
      "ATTRIB norm = vertex.attrib[2]; \n" /* XXX must match sphere function */
      "PARAM modelviewProj[4] = { state.matrix.mvp }; \n"
      "PARAM invModelview[4] = { state.matrix.modelview.invtrans }; \n"

      "# typical modelview/projection transform \n"
      "DP4 result.position.x, pos, modelviewProj[0]; \n"
      "DP4 result.position.y, pos, modelviewProj[1]; \n"
      "DP4 result.position.z, pos, modelviewProj[2]; \n"
      "DP4 result.position.w, pos, modelviewProj[3]; \n"

      "# transform normal by inv transpose of modelview, put in tex0 \n"
      "DP4 result.texcoord[0].x, norm, invModelview[0]; \n"
      "DP4 result.texcoord[0].y, norm, invModelview[1]; \n"
      "DP4 result.texcoord[0].z, norm, invModelview[2]; \n"
      "DP4 result.texcoord[0].w, norm, invModelview[3]; \n"

      "END\n";
      ;

   if (!glutExtensionSupported("GL_ARB_vertex_program")) {
      printf("Sorry, this demo requires GL_ARB_vertex_program\n");
      exit(1);
   }
   if (!glutExtensionSupported("GL_ARB_fragment_program")) {
      printf("Sorry, this demo requires GL_ARB_fragment_program\n");
      exit(1);
   }
         
   /*
    * Get extension function pointers.
    */
   glProgramLocalParameter4fvARB_func = (glProgramLocalParameter4fvARB_t) crGetProcAddress("glProgramLocalParameter4fvARB");
   assert(glProgramLocalParameter4fvARB_func);

   glProgramLocalParameter4dvARB_func = (glProgramLocalParameter4dvARB_t) crGetProcAddress("glProgramLocalParameter4dvARB");
   assert(glProgramLocalParameter4dvARB_func);

   glProgramLocalParameter4dARB_func = (glProgramLocalParameter4dARB_t) crGetProcAddress("glProgramLocalParameter4dARB");
   assert(glProgramLocalParameter4dARB_func);

   glGetProgramLocalParameterdvARB_func = (glGetProgramLocalParameterdvARB_t) crGetProcAddress("glGetProgramLocalParameterdvARB");
   assert(glGetProgramLocalParameterdvARB_func);

   glProgramEnvParameter4fvARB_func = (glProgramEnvParameter4fvARB_t) crGetProcAddress("glProgramEnvParameter4fvARB");
   assert(glProgramEnvParameter4fvARB_func);

   glProgramEnvParameter4fARB_func = (glProgramEnvParameter4fARB_t) crGetProcAddress("glProgramEnvParameter4fARB");
   assert(glProgramEnvParameter4fARB_func);

   glProgramEnvParameter4dvARB_func = (glProgramEnvParameter4dvARB_t) crGetProcAddress("glProgramEnvParameter4dvARB");
   assert(glProgramEnvParameter4dvARB_func);

   glProgramEnvParameter4dARB_func = (glProgramEnvParameter4dARB_t) crGetProcAddress("glProgramEnvParameter4dARB");
   assert(glProgramEnvParameter4dARB_func);

   glGetProgramEnvParameterdvARB_func = (glGetProgramEnvParameterdvARB_t) crGetProcAddress("glGetProgramEnvParameterdvARB");
   assert(glGetProgramEnvParameterdvARB_func);

   glGetProgramEnvParameterfvARB_func = (glGetProgramEnvParameterfvARB_t) crGetProcAddress("glGetProgramEnvParameterfvARB");
   assert(glGetProgramEnvParameterfvARB_func);

   glGenProgramsARB_func = (glGenProgramsARB_t) crGetProcAddress("glGenProgramsARB");
   assert(glGenProgramsARB_func);

   glProgramStringARB_func = (glProgramStringARB_t) crGetProcAddress("glProgramStringARB");
   assert(glProgramStringARB_func);

   glBindProgramARB_func = (glBindProgramARB_t) crGetProcAddress("glBindProgramARB");
   assert(glBindProgramARB_func);

   glIsProgramARB_func = (glIsProgramARB_t) crGetProcAddress("glIsProgramARB");
   assert(glIsProgramARB_func);

   glVertexAttrib3fARB_func = (glVertexAttrib3fARB_t) crGetProcAddress("glVertexAttrib3fARB");
   assert(glVertexAttrib3fARB_func);

   /*
    * Fragment program
    */
   glGenProgramsARB_func(1, &FragProg);
   assert(FragProg > 0);
   glBindProgramARB_func(GL_FRAGMENT_PROGRAM_ARB, FragProg);
   glProgramStringARB_func(GL_FRAGMENT_PROGRAM_ARB,
                           GL_PROGRAM_FORMAT_ASCII_ARB,
                           strlen(fragProgramText),
                           (const GLubyte *) fragProgramText);
   glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
   err = glGetError();
   if (err != GL_NO_ERROR || errorPos != -1) {
      int l = FindLine(fragProgramText, errorPos);
      printf("Fragment Program Error (err=%d pos=%d line=%d): %s\n",
             err, errorPos, l,
             (char *) glGetString(GL_PROGRAM_ERROR_STRING_ARB));
      exit(0);
   }
   assert(glIsProgramARB_func(FragProg));

   glProgramLocalParameter4fvARB_func(GL_FRAGMENT_PROGRAM_ARB, DIFFUSE, Diffuse);
   glProgramLocalParameter4fvARB_func(GL_FRAGMENT_PROGRAM_ARB, SPECULAR, Specular);

   /*
    * Do some sanity tests
    */
   {
      GLdouble v[4];
      GLfloat fv[4];
      glProgramLocalParameter4dARB_func(GL_FRAGMENT_PROGRAM_ARB, 8,
                                   10.0, 20.0, 30.0, 40.0);
      glGetProgramLocalParameterdvARB_func(GL_FRAGMENT_PROGRAM_ARB, 8, v);
      assert(v[0] == 10.0);
      assert(v[1] == 20.0);
      assert(v[2] == 30.0);
      assert(v[3] == 40.0);
      glGetProgramLocalParameterdvARB_func(GL_FRAGMENT_PROGRAM_ARB, DIFFUSE, v);
      assert(v[0] == Diffuse[0]);
      assert(v[1] == Diffuse[1]);
      assert(v[2] == Diffuse[2]);
      assert(v[3] == Diffuse[3]);

      glProgramEnvParameter4fARB_func(GL_FRAGMENT_PROGRAM_ARB, 4,
                                      91, 92, 93, 94);
      glGetProgramEnvParameterfvARB_func(GL_FRAGMENT_PROGRAM_ARB, 4, fv);
      assert(fv[0] == 91);
      assert(fv[1] == 92);
      assert(fv[2] == 93);
      assert(fv[3] == 94);
   }

   /*
    * Vertex program
    */
   glGenProgramsARB_func(1, &VertProg);
   assert(VertProg > 0);
   glBindProgramARB_func(GL_VERTEX_PROGRAM_ARB, VertProg);
   glProgramStringARB_func(GL_VERTEX_PROGRAM_ARB,
                           GL_PROGRAM_FORMAT_ASCII_ARB,
                           strlen(vertProgramText),
                           (const GLubyte *) vertProgramText);
   glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
   if (glGetError() != GL_NO_ERROR || errorPos != -1) {
      int l = FindLine(fragProgramText, errorPos);
      printf("Vertex Program Error (pos=%d line=%d): %s\n", errorPos, l,
             (char *) glGetString(GL_PROGRAM_ERROR_STRING_ARB));
      exit(0);
   }
   assert(glIsProgramARB_func(VertProg));

   /*
    * Misc init
    */
   glClearColor(0.3, 0.3, 0.3, 0.0);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHT0);
   glEnable(GL_LIGHTING);
   glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Diffuse);
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Specular);
   glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 20.0);

   printf("GL_RENDERER = %s\n", (char *) glGetString(GL_RENDERER));
   printf("Press p to toggle between per-pixel and per-vertex lighting\n");
}


int main( int argc, char *argv[] )
{
   glutInit( &argc, argv );
   glutInitWindowPosition( 0, 0 );
   glutInitWindowSize( 200, 200 );
   glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
   glutCreateWindow(argv[0]);
   glutReshapeFunc( Reshape );
   glutKeyboardFunc( Key );
   glutSpecialFunc( SpecialKey );
   glutDisplayFunc( Redisplay );
   if (Anim)
      glutIdleFunc(Idle);
   Init();
   glutMainLoop();
   return 0;
}
