/*
 * Use GL_NV_fragment_program and GL_NV_vertex_program to implement
 * simple per-pixel lighting.
 *
 * Brian Paul
 * 7 April 2003
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define GL_GLEXT_PROTOTYPES
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

typedef void (APIENTRY * glProgramNamedParameter4fvNV_t) (GLuint id, GLsizei len, const GLubyte *name, const GLfloat *v);
typedef void (APIENTRY * glGenProgramsNV_t) (GLsizei n, GLuint *programs);
typedef void (APIENTRY * glLoadProgramNV_t) (GLenum target, GLuint id, GLsizei len, const GLubyte *program);
typedef void (APIENTRY * glBindProgramNV_t) (GLenum target, GLuint id);
typedef void (APIENTRY * glTrackMatrixNV_t) (GLenum target, GLuint address, GLenum matrix, GLenum transform);
typedef GLboolean (APIENTRY * glIsProgramNV_t) (GLuint id);
typedef void (APIENTRY * glVertexAttrib3fNV_t) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * glGetProgramNamedParameterdvNV_t) (GLuint id, GLsizei len, const GLubyte *name, GLdouble *params);
typedef void (APIENTRY * glProgramLocalParameter4dARB_t) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void (APIENTRY * glGetProgramLocalParameterdvARB_t) (GLenum target, GLuint index, GLdouble *params);



static glProgramNamedParameter4fvNV_t glProgramNamedParameter4fvNV_func;
static glGenProgramsNV_t glGenProgramsNV_func;
static glLoadProgramNV_t glLoadProgramNV_func;
static glBindProgramNV_t glBindProgramNV_func;
static glTrackMatrixNV_t glTrackMatrixNV_func;
static glIsProgramNV_t glIsProgramNV_func;
static glVertexAttrib3fNV_t glVertexAttrib3fNV_func;
static glGetProgramNamedParameterdvNV_t glGetProgramNamedParameterdvNV_func;
static glProgramLocalParameter4dARB_t glProgramLocalParameter4dARB_func;
static glGetProgramLocalParameterdvARB_t glGetProgramLocalParameterdvARB_func;


#define NAMED_PARAMETER4FV(prog, name, v)        \
  glProgramNamedParameter4fvNV_func(prog, strlen(name), \
                                    (const GLubyte *) name, v)


/*
 * Draw a sphere with normal vectors.
 * Code borrowed from Mesa's GLU.
 */
static void sphere(GLdouble radius, GLint slices, GLint stacks)
{
	/* Either use conventional vertex attributes or generic attributes */
#if 1
#define VERTEX3F(x, y, z)  glVertex3f(x, y, z)
#define NORMAL3F(x, y, z)  glNormal3f(x, y, z)
#else
#define VERTEX3F(x, y, z)  glVertexAttrib3fNV_func(0, x, y, z)
#define NORMAL3F(x, y, z)  glVertexAttrib3fNV_func(2, x, y, z)
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
      NAMED_PARAMETER4FV(FragProg, "LightPos", LightPos);
      glEnable(GL_FRAGMENT_PROGRAM_NV);
      glEnable(GL_VERTEX_PROGRAM_NV);
      glDisable(GL_LIGHTING);
   }
   else {
      glLightfv(GL_LIGHT0, GL_POSITION, LightPos);
      glDisable(GL_FRAGMENT_PROGRAM_NV);
      glDisable(GL_VERTEX_PROGRAM_NV);
      glEnable(GL_LIGHTING);
   }

   glPushMatrix();
   glRotatef(Xrot, 1, 0, 0);
   glRotatef(Yrot, 0, 1, 0);

   glTexCoord2f(0, 0);
#if 1
   sphere(2.0, 10, 5);
#elif 0
   glutSolidSphere(2.0, 10, 5);
#else
   {
      GLUquadricObj *q = gluNewQuadric();
      gluQuadricNormals(q, GL_SMOOTH);
      gluQuadricTexture(q, GL_TRUE);
      glRotatef(90, 1, 0, 0);
      glTranslatef(0, 0, -1);
      gluCylinder(q, 1.0, 1.0, 2.0, 24, 1);
      gluDeleteQuadric(q);
   }
#endif

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
   /*glOrtho( -2.0, 2.0, -2.0, 2.0, 5.0, 25.0 );*/
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


static void Init( void )
{
   static const char *fragProgramText =
      "!!FP1.0\n"
      "DECLARE Diffuse; \n"
      "DECLARE Specular; \n"
      "DECLARE LightPos; \n"

      "# Compute normalized LightPos, put it in R0\n"
      "DP3 R0.x, LightPos, LightPos;\n"
      "RSQ R0.y, R0.x;\n"
      "MUL R0, LightPos, R0.y;\n"

      "# Compute normalized normal, put it in R1\n"
      "DP3 R1, f[TEX0], f[TEX0]; \n"
      "RSQ R1.y, R1.x;\n"
      "MUL R1, f[TEX0], R1.y;\n"

      "# Compute dot product of light direction and normal vector\n"
      "DP3 R2, R0, R1;"

      "MUL R3, Diffuse, R2;    # diffuse attenuation\n"

      "POW R4, R2.x, {20.0}.x; # specular exponent\n"

      "MUL R5, Specular, R4;   # specular attenuation\n"

      "ADD o[COLR], R3, R5;    # add diffuse and specular colors\n"
      "END \n"
      ;

   static const char *vertProgramText =
      "!!VP1.0\n"
      "# typical modelview/projection transform\n"
      "DP4   o[HPOS].x, c[0], v[OPOS] ;\n"
      "DP4   o[HPOS].y, c[1], v[OPOS] ;\n"
      "DP4   o[HPOS].z, c[2], v[OPOS] ;\n"
      "DP4   o[HPOS].w, c[3], v[OPOS] ;\n"
      "# transform normal by inv transpose of modelview, put in tex0\n"
      "DP4   o[TEX0].x, c[4], v[NRML] ;\n"
      "DP4   o[TEX0].y, c[5], v[NRML] ;\n"
      "DP4   o[TEX0].z, c[6], v[NRML] ;\n"
      "DP4   o[TEX0].w, c[7], v[NRML] ;\n"
      "END\n";
   ;

   if (!glutExtensionSupported("GL_NV_vertex_program")) {
      printf("Sorry, this demo requires GL_NV_vertex_program\n");
      exit(1);
   }
   if (!glutExtensionSupported("GL_NV_fragment_program")) {
      printf("Sorry, this demo requires GL_NV_fragment_program\n");
      exit(1);
   }
         
   glProgramNamedParameter4fvNV_func = (glProgramNamedParameter4fvNV_t) crGetProcAddress("glProgramNamedParameter4fvNV");
   assert(glProgramNamedParameter4fvNV_func);

   glGenProgramsNV_func = (glGenProgramsNV_t) crGetProcAddress("glGenProgramsNV");
   assert(glGenProgramsNV_func);

   glLoadProgramNV_func = (glLoadProgramNV_t) crGetProcAddress("glLoadProgramNV");
   assert(glLoadProgramNV_func);

   glBindProgramNV_func = (glBindProgramNV_t) crGetProcAddress("glBindProgramNV");
   assert(glBindProgramNV_func);

   glIsProgramNV_func = (glIsProgramNV_t) crGetProcAddress("glIsProgramNV");
   assert(glIsProgramNV_func);

   glTrackMatrixNV_func = (glTrackMatrixNV_t) crGetProcAddress("glTrackMatrixNV");
   assert(glTrackMatrixNV_func);

   glVertexAttrib3fNV_func = (glVertexAttrib3fNV_t) crGetProcAddress("glVertexAttrib3fNV");
   assert(glVertexAttrib3fNV_func);

   glGetProgramNamedParameterdvNV_func = (glGetProgramNamedParameterdvNV_t) crGetProcAddress("glGetProgramNamedParameterdvNV");
   assert(glGetProgramNamedParameterdvNV_func);

   glProgramLocalParameter4dARB_func = (glProgramLocalParameter4dARB_t) crGetProcAddress("glProgramLocalParameter4dARB");
   assert(glProgramLocalParameter4dARB_func);

   glGetProgramLocalParameterdvARB_func = (glGetProgramLocalParameterdvARB_t) crGetProcAddress("glGetProgramLocalParameterdvARB");
   assert(glGetProgramLocalParameterdvARB_func);


   glGenProgramsNV_func(1, &FragProg);
   assert(FragProg > 0);
   glGenProgramsNV_func(1, &VertProg);
   assert(VertProg > 0);

   /*
    * Fragment program
    */
   glLoadProgramNV_func(GL_FRAGMENT_PROGRAM_NV, FragProg,
                   strlen(fragProgramText),
                   (const GLubyte *) fragProgramText);
   assert(glIsProgramNV_func(FragProg));
   glBindProgramNV_func(GL_FRAGMENT_PROGRAM_NV, FragProg);

   NAMED_PARAMETER4FV(FragProg, "Diffuse", Diffuse);
   NAMED_PARAMETER4FV(FragProg, "Specular", Specular);

   /*
    * Do some sanity tests
    */
   {
      GLdouble v[4];
      glProgramLocalParameter4dARB_func(GL_FRAGMENT_PROGRAM_NV, 1,
                                   10.0, 20.0, 30.0, 40.0);
      glGetProgramLocalParameterdvARB_func(GL_FRAGMENT_PROGRAM_NV, 1, v);
      assert(v[0] == 10.0);
      assert(v[1] == 20.0);
      assert(v[2] == 30.0);
      assert(v[3] == 40.0);
      glGetProgramNamedParameterdvNV_func(FragProg, 7, (GLubyte *) "Diffuse", v);
      assert(v[0] == Diffuse[0]);
      assert(v[1] == Diffuse[1]);
      assert(v[2] == Diffuse[2]);
      assert(v[3] == Diffuse[3]);
   }

   /*
    * Vertex program
    */
   glLoadProgramNV_func(GL_VERTEX_PROGRAM_NV, VertProg,
                   strlen(vertProgramText),
                   (const GLubyte *) vertProgramText);
   assert(glIsProgramNV_func(VertProg));
   glBindProgramNV_func(GL_VERTEX_PROGRAM_NV, VertProg);
   glTrackMatrixNV_func(GL_VERTEX_PROGRAM_NV, 0, GL_MODELVIEW_PROJECTION_NV, GL_IDENTITY_NV);
   glTrackMatrixNV_func(GL_VERTEX_PROGRAM_NV, 4, GL_MODELVIEW, GL_INVERSE_TRANSPOSE_NV);

   /*
    * Misc init
    */
   glClearColor(0.3, 0.3, 0.3, 0.0);
   glEnable(GL_ALPHA_TEST); /* temporary hack */
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
