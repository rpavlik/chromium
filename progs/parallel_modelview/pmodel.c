#include <math.h>
#include <float.h>

#include "pmodel.h"
#include "chromium.h"
#include "cr_error.h"
#include "cr_string.h"

Globals globals;

static const int MASTER_BARRIER = 100;

crCreateContextProc glCreateContextCR;
crMakeCurrentProc   glMakeCurrentCR;
crSwapBuffersProc   glSwapBuffersCR;

glChromiumParametervCRProc glChromiumParametervCR;
glGetChromiumParametervCRProc glGetChromiumParametervCR;
glBarrierCreateCRProc glBarrierCreateCR;
glBarrierExecCRProc   glBarrierExecCR;

int DrawFrame( void );
void CreateGraphicsContext( void );
void SetupGraphicsState( void );
void SetupCamera( void );
void CreateDisplayLists( void );
void ParseArguments( int argc, char *argv[] );


int DrawFrame( void )
{
	int i;
	static int frame_count = 0;
	static float bogus_bounds[6] = {0,0,0,0,0,0};
	Model *model;
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	gluLookAt( globals.center.x, 
						 globals.center.y, 
						 globals.center.z+2*globals.radius, 
						 globals.center.x, 
						 globals.center.y, 
						 globals.center.z,
						 0, 1, 0 );
	glRotatef( (float) frame_count, 1 ,0, 0 );
	frame_count++;

	for (i = 0,model=globals.models ; 
			 model; 
			 i++,model = model->next)
	{
		glCallList( globals.dpy_list_base + i );
		if (globals.compositor != BINARYSWAP)
		{
			glChromiumParametervCR( GL_OBJECT_BBOX_CR, GL_FLOAT, 6, &(model->bounds) );
		}
		glFlush();
	}
	if (globals.compositor != BINARYSWAP)
	{
		glChromiumParametervCR( GL_OBJECT_BBOX_CR, GL_FLOAT, 6, bogus_bounds );
	}
	else
	{
		glChromiumParametervCR( GL_OBJECT_BBOX_CR, GL_FLOAT, 6, &(globals.local_bounds) );
	}
	glSwapBuffersCR( globals.window, 0 );

	return 1;
}

void CreateGraphicsContext( )
{
	int visual = CR_RGB_BIT | CR_DEPTH_BIT | CR_DOUBLE_BIT;
	const char *dpy = NULL;
	char *spu_name;

	globals.window = 0;
#define LOAD( x ) gl##x##CR = (cr##x##Proc) crGetProcAddress( "cr"#x )

	LOAD( CreateContext );
	LOAD( MakeCurrent );
	LOAD( SwapBuffers );

	globals.ctx = glCreateContextCR(dpy, visual);
	if (globals.ctx < 0) 
	{
		crError("glCreateContextCR() call failed!\n");
	}
	glMakeCurrentCR(globals.window, globals.ctx);

#define LOAD2( x ) gl##x##CR = (gl##x##CRProc) crGetProcAddress( "gl"#x"CR" )
  LOAD2( GetChromiumParameterv );
  LOAD2( ChromiumParameterv );
	LOAD2( BarrierCreate );
	LOAD2( BarrierExec );

	globals.compositor = OTHER;
	glGetChromiumParametervCR( GL_HEAD_SPU_NAME_CR, 0, GL_BYTE, 1, &(spu_name) );
	if (!crStrcmp( spu_name, "readback" ))
	{
		globals.compositor = READBACK;
		crDebug( "Detected the readback SPU!" );
	}
	else if (!crStrcmp( spu_name, "binaryswap" ))
	{
		globals.compositor = BINARYSWAP;
		crDebug( "Detected the binaryswap SPU!" );
	}
	else
	{
		crDebug( "Unknown head SPU: %s!", spu_name );
	}
}

void SetupGraphicsState( void )
{
	glClearColor( 0, 0, 0, 1 );
	glEnable( GL_DEPTH_TEST );

	/* Set up lighting */

	if (globals.has_nx)
	{
		float position[4] = {0,0,1,0};
		float intensity[4] = {1,1,1,1};
		glEnable( GL_COLOR_MATERIAL );
		glEnable( GL_LIGHTING );
		glEnable( GL_LIGHT0 );
		glLightfv( GL_LIGHT0, GL_POSITION, position );
		glLightfv( GL_LIGHT0, GL_DIFFUSE, intensity );
	}
}

void SetupCamera( void )
{
	GLint viewport[4];
	globals.center.x = (globals.global_bounds.max.x+globals.global_bounds.min.x)/2;
	globals.center.y = (globals.global_bounds.max.y+globals.global_bounds.min.y)/2;
	globals.center.z = (globals.global_bounds.max.z+globals.global_bounds.min.z)/2;
	globals.width    = (globals.global_bounds.max.x-globals.global_bounds.min.x);
	globals.height   = (globals.global_bounds.max.y-globals.global_bounds.min.y);
	globals.depth    = (globals.global_bounds.max.z-globals.global_bounds.min.z);
 
	globals.radius= (float) sqrt( globals.width*globals.width/4 + 
			        globals.height*globals.height/4 + 
							globals.depth*globals.depth/4 );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glGetIntegerv( GL_VIEWPORT, viewport );
	gluPerspective( 45, (float) viewport[2]/(float) viewport[3], globals.radius, 4*globals.radius );
}

void CreateDisplayLists()
{
	Model *model;
	int dpy_list;
	int i;

	globals.dpy_list_base = glGenLists( globals.num_models );
	for (model = globals.models, dpy_list = globals.dpy_list_base;
			 model;
			 model = model->next, dpy_list++)
	{
		crDebug( "Making display list for model %d of %d", dpy_list, globals.num_models );
		glNewList( dpy_list, GL_COMPILE );
		glBegin( GL_TRIANGLES );
		for (i = 0 ; i < model->nfaces*3 ; i++)
		{
			Vertex *v = &(model->vlist[model->faces[i]]);
			if (globals.has_r) glColor3f( v->r, v->g, v->b );
			if (globals.has_nx) glNormal3f( v->nx, v->ny, v->nz );
			glVertex3f( v->x, v->y, v->z );
		}
		glEnd();

		glEndList();
	}
	crDebug( "Done!" );

}

void ParseArguments( int argc, char *argv[] )
{
	int i;

	globals.ply_root = NULL;
	for (i = 1 ; i < argc ; i++)
	{
		if (!crStrcmp(argv[i], "-plyroot"))
		{
			if (i == argc-1)
			{
				crError( "-plyroot requires an argument" );
			}
			globals.ply_root = strdup( argv[i+1] );
			i++;
		}
		else if (!crStrcmp(argv[i], "-nodes" ) )
		{
			if (i == argc-1)
			{
				crError( "-nodes requires an argument" );
			}
			globals.num_nodes = atoi( argv[i+1] );
			i++;
		}
		else if (!crStrcmp(argv[i], "-nodeid" ) )
		{
			if (i == argc-1)
			{
				crError( "-nodeid requires an argument" );
			}
			globals.node_id = atoi( argv[i+1] );
			i++;
		}
		else
		{
			crError( "Unknown argument: %s", argv[i] );
		}
	}
	if (!globals.ply_root)
	{
		crWarning( "No ply root specified, defaulting to /home/dale/plys." );
		crWarning( "It's good to be the king." );
		globals.ply_root = crStrdup( "/home/dale/plys" );
	}
}

int main(int argc, char *argv[])
{

	globals.node_id = 0;
	globals.num_nodes = 1;

	ParseArguments( argc, argv );

	CreateGraphicsContext();

	ReadFiles();
	CreateDisplayLists();
	SetupGraphicsState();
	SetupCamera();

	while (DrawFrame()) /* Empty */
		;

	return 0;
}
