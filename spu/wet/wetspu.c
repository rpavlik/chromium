/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include <math.h>
#include "cr_spu.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_rand.h"
#include "wetspu.h"

#define MAX( a, b ) ((a) > (b) ? (a) : (b))
#define MIN( a, b ) ((a) < (b) ? (a) : (b))

static void wetMeshSetup( void )
{
	GLint viewport[4];
	int i;

	wet_spu.super.GetIntegerv( GL_VIEWPORT, viewport );
	if (viewport[2] > viewport[3])
	{
		wet_spu.mesh_width = wet_spu.mesh_dice;
		wet_spu.mesh_height = (wet_spu.mesh_dice * viewport[3]) / viewport[2];
	}
	else
	{
		wet_spu.mesh_width = (wet_spu.mesh_dice * viewport[2]) / viewport[3];
		wet_spu.mesh_height = wet_spu.mesh_dice;
	}
	wet_spu.mesh_width ++;
	wet_spu.mesh_height ++;

	wet_spu.mesh_displacement = (float **) crAlloc( wet_spu.mesh_height * sizeof( *(wet_spu.mesh_displacement) ) );
	for (i = 0 ; i < wet_spu.mesh_height ; i++)
	{
		wet_spu.mesh_displacement[i] = (float *) crCalloc( wet_spu.mesh_width * sizeof( *(wet_spu.mesh_displacement[i]) ) );
	}

	/* wet_spu.super.GenTextures( 1, &(wet_spu.tex_id ) ); */
	wet_spu.tex_id = 0x16969;
	wet_spu.super.BindTexture( GL_TEXTURE_2D, wet_spu.tex_id );
	wet_spu.super.TexImage2D( GL_TEXTURE_2D, 0, 4, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
}

static void GenerateNewDrop( void )
{
	static int num_drops_generated = 0;
	float rv = crRandFloat( 0, 100 );
	if (rv > wet_spu.density) 
	{
		WetSPURaindrop *drop;
		num_drops_generated++;
		drop = (WetSPURaindrop *) crAlloc( sizeof( *drop ) );
		drop->x = (int) crRandFloat( 0.f, (float) wet_spu.mesh_width );
		drop->y = (int) crRandFloat( 0.f, (float) wet_spu.mesh_height );
		drop->drop_frame = wet_spu.frame_counter;
		drop->next = wet_spu.drops;
		wet_spu.drops = drop;
	}
}

float Evaluate( float elapsed, float distance, float max_dist )
{
	float val, s_val;
	if (elapsed < distance)
		return 0;

	val =  elapsed + wet_spu.ripple_freq * distance;
	s_val = wet_spu.ripple_scale * (float) sin( val );
	s_val *= (1-(float) elapsed / max_dist );
	return s_val;
}

static void UpdateMesh( void )
{
	WetSPURaindrop *drop, *before_drop = NULL, *nextdrop;
	int x, y;
	int xmin, xmax, ymin, ymax;

	for ( y = 0 ; y < wet_spu.mesh_height ; y++ )
	{
		crMemset( wet_spu.mesh_displacement[y], 0, sizeof( float ) * wet_spu.mesh_width );
	}

	for (drop = wet_spu.drops ; drop ; before_drop = drop, drop = nextdrop)
	{
		float elapsed = (wet_spu.frame_counter - drop->drop_frame) * wet_spu.time_scale;
		xmin = MAX( 0, drop->x - wet_spu.raininess );
		xmax = MIN( wet_spu.mesh_width, drop->x + wet_spu.raininess );
		ymin = MAX( 0, drop->y - wet_spu.raininess );
		ymax = MIN( wet_spu.mesh_height, drop->y + wet_spu.raininess );
		for (x = xmin ; x < xmax ; x++)
		{
			for (y = ymin ; y < ymax ; y++)
			{
				int delx = drop->x-x;
				int dely = drop->y-y;
				float dist, disp;

				dist = (float) sqrt( delx*delx + dely*dely );
				disp = Evaluate( elapsed, dist, (float) wet_spu.raininess );
				wet_spu.mesh_displacement[y][x] += disp;
			}
		}
		nextdrop = drop->next;
		if (elapsed > wet_spu.raininess)
		{
			if (before_drop != NULL)
			{
				before_drop->next = drop->next;
			}
			else
			{
				wet_spu.drops = drop->next;
			}
			crFree( drop );
		}
	}
}

static void ComputeNormal( int x, int y, float n[3] )
{
	float len;
	float p1[3], p2[3], p3[3], p4[3];
	n[0] = n[1] = n[2] = 0;
	p1[0] = (float) x-1;
	p2[0] = (float) x;
	p3[0] = (float) x+1;
	p4[0] = (float) x;
	p1[1] = (float) y;
	p2[1] = (float) y-1;
	p3[1] = (float) y;
	p4[1] = (float) y+1;
	if (x > 0)
		p1[2] = wet_spu.mesh_displacement[(int) p1[1]][(int) p1[0]];
	if (y > 0)
		p2[2] = wet_spu.mesh_displacement[(int) p2[1]][(int) p2[0]];
	if (x < wet_spu.mesh_width-1)
		p3[2] = wet_spu.mesh_displacement[(int) p3[1]][(int) p3[0]];
	if (y < wet_spu.mesh_height-1)
		p4[2] = wet_spu.mesh_displacement[(int) p4[1]][(int) p4[0]];

/* Huh?  This looks like a cross product, but left-handed?!? */
#define ADD_VECTOR( a, b ) \
	do { \
		n[0] += b[1]*a[2] - b[2]*a[1]; \
		n[1] += b[0]*a[2] - b[2]*a[0]; \
		n[2] += b[0]*a[1] - b[1]*a[0]; \
	} while (0)

	if (x > 0 && y > 0)
		ADD_VECTOR( p1, p2 );
	if (y > 0 && x < wet_spu.mesh_width-1 )
		ADD_VECTOR( p2, p3 );
	if (x < wet_spu.mesh_width-1 && y < wet_spu.mesh_height-1 )
		ADD_VECTOR( p3, p4 );
	if (y < wet_spu.mesh_height-1 && x > 0)
		ADD_VECTOR( p4, p1 );

	len = (float) sqrt( n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
	n[0] /= len;
	n[1] /= len;
	n[2] /= len;
}

static void RefractRay( int x, int y, float n[3], float t[2] )
{
	/* the rays we're refracting are all 0,0,1, so that makes the math a lot simpler */

	float eta = wet_spu.ior;
	float c1 = -n[2]; /* wo.N */
	float c2 = 1-(eta*eta*(1.0f-c1));
	float D[3], O[3];

	if (c2 < 0) {
		/* Total internal reflection, seems unlikely in this system, but... */
		t[0] = -1;
		t[1] = -1;
	} else {
		float ray_t;
		c2 = (float) sqrt(c2);
		/*
		D[0] = (eta*c1-c2)*n[0];
		D[1] = (eta*c1-c2)*n[1];
		D[2] = -eta + (eta*c1-c2)*n[2];
		*/
		D[0] = -n[0];
		D[1] = -n[1];
		D[2] = -n[2]*eta;

		O[0] = (float) x;
		O[1] = (float) y;
		O[2] = wet_spu.mesh_displacement[y][x];

		/* okay, we have this thing, now we intersect it with the z=0 plane */
		ray_t = -O[2]/D[2];
		
		t[0] = O[0] + ray_t*D[0];
		t[1] = O[1] + ray_t*D[1];
	}
}

static void DrawMesh( void )
{
	int x, y;
	int viewport[4];
	int win_width, win_height;
	float s, t;

	wet_spu.super.GetIntegerv( GL_VIEWPORT, viewport );
	win_width = viewport[2];
	win_height = viewport[3];

	s = (float) win_width / 1024;
	t = (float) win_height / 1024;

	wet_spu.super.PushAttrib( GL_TRANSFORM_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT | GL_POLYGON_BIT | GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT );
	wet_spu.super.Enable( GL_TEXTURE_2D );
	wet_spu.super.BindTexture( GL_TEXTURE_2D, wet_spu.tex_id );
	wet_spu.super.PixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	wet_spu.super.TexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	wet_spu.super.TexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	wet_spu.super.TexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	wet_spu.super.TexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	wet_spu.super.TexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	wet_spu.super.CopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, win_width, win_height );
	/* wet_spu.super.TexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, CR_LOGO_H_WIDTH, CR_LOGO_H_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, raw_bytes ); */

	wet_spu.super.ClearColor(0,0,0,1);
	wet_spu.super.Clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	wet_spu.super.MatrixMode( GL_PROJECTION );
	wet_spu.super.PushMatrix();
	wet_spu.super.LoadIdentity();
	wet_spu.super.Ortho( 0, wet_spu.mesh_width-1, 0, wet_spu.mesh_height-1, -500, 500 );
	wet_spu.super.MatrixMode( GL_MODELVIEW );
	wet_spu.super.PushMatrix();
	wet_spu.super.LoadIdentity();
	/* wet_spu.super.Scalef(5,5,5); */
	/*wet_spu.super.Rotatef(60,1,0,0); */
	/*wet_spu.super.Rotatef(30,0,1,0); */
	wet_spu.super.MatrixMode( GL_TEXTURE );
	wet_spu.super.PushMatrix();
	wet_spu.super.LoadIdentity();
	wet_spu.super.Scalef( s/(wet_spu.mesh_width-1), t/(wet_spu.mesh_height-1), 1 );
	wet_spu.super.Disable( GL_LIGHTING );
	wet_spu.super.Color3f( 1, 1, 1 );

	/*wet_spu.super.PolygonMode( GL_FRONT_AND_BACK, GL_LINE ); */
	
	for (y = 0 ; y < wet_spu.mesh_height-1 ; y++)
	{
		wet_spu.super.Begin( GL_TRIANGLE_STRIP );
		for (x = 0 ; x < wet_spu.mesh_width ; x++)
		{
			float n[3];
			float t[2];
			ComputeNormal( x, y, n );
			RefractRay( x, y, n, t );
			wet_spu.super.TexCoord2fv( t );
			if (t[0] != -1)
			{
				wet_spu.super.Color3f( 1,1,1 );
			}
			else
			{
				wet_spu.super.Color3f(0,0,0);
			}
			wet_spu.super.Vertex3f( (float) x,  (float) y, wet_spu.mesh_displacement[y][x] );	
			ComputeNormal( x, y+1, n );
			RefractRay( x, y+1, n, t );
			if (t[0] != -1)
			{
				wet_spu.super.Color3f(1,1,1);
			}
			else
			{
				wet_spu.super.Color3f(0,0,0);
			}
			wet_spu.super.TexCoord2fv( t );
			wet_spu.super.Vertex3f( (float) x, (float) y+1, wet_spu.mesh_displacement[y+1][x] );	
		}
		wet_spu.super.End();
	}

#ifdef DEBUG_NORMALS
	wet_spu.super.Begin( GL_LINES );
	for (y = 0 ; y < wet_spu.mesh_height ; y++)
	{
		for (x = 0 ; x < wet_spu.mesh_width ; x++)
		{
			float n[3];
			ComputeNormal( x, y, n );
			wet_spu.super.Color3f( 0, 1, 0 );
			wet_spu.super.Vertex3f( (float) x, (float) y, wet_spu.mesh_displacement[y][x] );
			wet_spu.super.Vertex3f( x+3*n[0], y+3*n[1], wet_spu.mesh_displacement[y][x] + 3*n[2] );
		}
	}
	wet_spu.super.End();
#endif

	wet_spu.super.MatrixMode( GL_PROJECTION );
	wet_spu.super.PopMatrix();
	wet_spu.super.MatrixMode( GL_MODELVIEW );
	wet_spu.super.PopMatrix();
	wet_spu.super.MatrixMode( GL_TEXTURE );
	wet_spu.super.PopMatrix();
	wet_spu.super.PopAttrib();
}

void WETSPU_APIENTRY wetSwapBuffers( GLint window, GLint flags )
{
	/* CR_SUPPRESS_SWAP_BIT is typically only used with multi-threaded
	 * programs in which there are N threads rendering to the same window.
	 * We only want to do a SwapBuffers() once per N windows.  BUT, the
	 * app must still call SwapBuffers() so that the SPU can do its
	 * end-of-frame stuff (consider the readback SPU, or this SPU).
	 */
	if ((flags & CR_SUPPRESS_SWAP_BIT) == 0)
	{
		wet_spu.frame_counter++;

		GenerateNewDrop();
		UpdateMesh();
		DrawMesh();
	}
	wet_spu.super.SwapBuffers( window, flags );
}


/* Keep track of context numbers which we've seen before */
struct context_info {
	GLint ctx;
	struct context_info *next;
};
static struct context_info *knownContexts = NULL;


static void WETSPU_APIENTRY wetMakeCurrent(GLint window, GLint nativeWindow, GLint ctx)
{
	struct context_info *c;
	int found = 0;

	wet_spu.super.MakeCurrent(window, nativeWindow, ctx);

	/* have we seen this context before? */
	for (c = knownContexts; c; c = c->next) {
		if (c->ctx == ctx) {
			found = 1;
			break;
		}
	}
	if (!found) {
		/* this is the first time we've seen this context */
		/* save this context number */
		c = crAlloc(sizeof(struct context_info));
		if (c) {
			c->ctx = ctx;
			c->next = knownContexts;
			knownContexts = c;
		}
		/* do our per-context SPU initialization */
		wetMeshSetup();
	}
}


static void WETSPU_APIENTRY wetDestroyContext( GLint ctx )
{
	struct context_info *c, *prev;

	/* remove this context number from the list of known contexts */
	prev = NULL;
	for (c = knownContexts; c; c = c->next) {
		if (c->ctx == ctx) {
			/* remove c from the list */
			if (prev)
				prev->next = c->next;
			else
				knownContexts = c->next;
			crFree(c);
			return;
		}
	}
}


SPUNamedFunctionTable wet_table[] = {
	{ "SwapBuffers", (SPUGenericFunction) wetSwapBuffers },
	{ "MakeCurrent", (SPUGenericFunction) wetMakeCurrent },
	{ "DestroyContext", (SPUGenericFunction) wetDestroyContext },
	{ NULL, NULL }
};

