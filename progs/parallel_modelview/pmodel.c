#include "pmodel.h"
#include "ball.h"

#include "float.h"
#include "cr_error.h"
#include "GL/glut.h"
#include <math.h>

BBOX bounds;
float w,h,d,cx,cy,cz,rad;

#define RADIUS 0.75
static BallData oBall;
static int winWidth = -1, winHeight = -1;

void Display( void )
{
	int i;
	HMatrix hm;
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	gluLookAt( cx, cy, cz+2*rad, 
		       cx, cy, cz,
			   0, 1, 0 );
	
	Ball_Value( &oBall, hm );
	glTranslatef( cx, cy, cz );
	glMultMatrixf((GLfloat *) hm );
	glTranslatef( -cx, -cy, -cz );

	glBegin( GL_TRIANGLES );
	for (i = 0 ; i < nfaces*3 ; i++)
	{
		Vertex *v = &(vlist[faces[i]]);
		if (has_r) glColor3f( v->r, v->g, v->b );
		if (has_nx) glNormal3f( v->nx, v->ny, v->nz );
		glVertex3f( v->x, v->y, v->z );
	}
	glEnd();

#if 0
	glDisable(GL_LIGHTING);
	glBegin( GL_LINES );
	for (i = 0 ; i < nfaces*3 ; i++)
	{
		Vertex *v = &(vlist[faces[i]]);
		if (has_nx) {
			glVertex3f( v->x, v->y, v->z );
			glVertex3f( v->x+2000*v->nx, v->y+2000*v->ny, v->z+2000*v->nz );
		}
	}
	glEnd();
	glEnable(GL_LIGHTING);
#endif

	glMatrixMode( GL_MODELVIEW );
	glutPostRedisplay();
	glutSwapBuffers();
}

void Reshape( int w, int h )
{
	winWidth = w; winHeight = h;
	glViewport( 0, 0, w, h );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45, (float) w/(float)h, rad, 4*rad );
	glutPostRedisplay();
}

void Mouse( int button, int state, int x, int y )
{
	HVect vNow;
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_UP)
		{
			Ball_EndDrag(&oBall);
			glutPostRedisplay();
		}
		else
		{
			vNow.x = 2.0*x/winWidth - 1.0;
			vNow.y = 2.0*(winHeight-y)/winHeight - 1.0;
			Ball_Mouse(&oBall,vNow);
			Ball_BeginDrag(&oBall);
		}
	}
}

void Motion( int x, int y ) 
{
	HVect vNow;
	vNow.x = 2.0*x/winWidth - 1.0;
	vNow.y = 2.0*(winHeight-y)/winHeight - 1.0;
	Ball_Mouse(&oBall, vNow);
	Ball_Update(&oBall);
	glutPostRedisplay();
}

void Keyboard( unsigned char key, int x, int y )
{
	if (key == 27)
	{
		exit(0);
	}
	(void) x;
	(void) y;
}

int main(int argc, char *argv[])
{
	int i;

	if (argc == 1)
	{
		crError( "Usage: %s <ply_file>", argv[0] );
	}

	/* Set up GLUT */
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
	glutInitWindowSize( 300, 300 );
	glutCreateWindow( "PLY" );
	glutDisplayFunc(Display);
	glutMouseFunc( Mouse );
	glutMotionFunc( Motion );
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);

	Ball_Init(&oBall);
	Ball_Place(&oBall,qOne,RADIUS);

	/* Set up persistent OpenGL state */
	glClearColor( 0, 0, 0, 1 );
	glEnable( GL_DEPTH_TEST );

	/* Set up the vertex array */
	ReadFile( argv[1] );
	glInterleavedArrays( GL_C4F_N3F_V3F, 0, vlist );

	/* Set up lighting */

	if (has_nx)
	{
		float position[4] = {0,0,1,0};
		float intensity[4] = {1,1,1,1};
		glEnable( GL_COLOR_MATERIAL );
		glEnable( GL_LIGHTING );
		glEnable( GL_LIGHT0 );
		glLightfv( GL_LIGHT0, GL_POSITION, position );
		glLightfv( GL_LIGHT0, GL_DIFFUSE, intensity );
	}

	/* Set up the camera */
	bounds.maxx = -FLT_MAX;
	bounds.maxy = -FLT_MAX;
	bounds.maxz = -FLT_MAX;
	bounds.minx = FLT_MAX;
	bounds.miny = FLT_MAX;
	bounds.minz = FLT_MAX;
	for( i = 0 ; i < nfaces ; i++ )
	{
		Vertex *v = &(vlist[faces[i]]);
		if (v->x > bounds.maxx) bounds.maxx = v->x;
		if (v->y > bounds.maxy) bounds.maxy = v->y;
		if (v->z > bounds.maxz) bounds.maxz = v->z;
		if (v->x < bounds.minx) bounds.minx = v->x;
		if (v->y < bounds.miny) bounds.miny = v->y;
		if (v->z < bounds.minz) bounds.minz = v->z;
	}

	cx = (bounds.maxx+bounds.minx)/2;
	cy = (bounds.maxy+bounds.miny)/2;
	cz = (bounds.maxz+bounds.minz)/2;
	w = (bounds.maxx-bounds.minx);
	h = (bounds.maxy-bounds.miny);
	d = (bounds.maxz-bounds.minz);

	rad = sqrt( w*w/4 + h*h/4 + d*d/4 );

	crDebug( "radius of bounding sphere: %f", rad );
	crDebug( "w, h, d = (%f,%f,%f)", w, h, d );
	crDebug( "bounds: (%f,%f,%f)->(%f,%f,%f)", bounds.minx, bounds.miny, bounds.minz, bounds.maxx, bounds.maxy, bounds.maxz );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	gluLookAt( cx, cy, cz+2*rad, 
		       cx, cy, cz,
			   0, 1, 0 );
	
	/* Do it! */
	glutMainLoop();

	return 0;
}
