/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include <stdio.h>
#include "cr_glwrapper.h"

#define ANGLE_STEP .5f

int main(int argc, char *argv[])
{
	SPU *spu;
	int ids[] = { 3 };
	char *spunames[] = { "tilesort" };
	float angle = 0;
	GLfloat v1[3] = { .25, .25, 0 }; 
	GLfloat v2[3] = { .25, .5, 0 }; 
	GLfloat v3[3] = { .5, .25, 0 }; 

	(void) argc;
	(void) argv;

	spu = crSPULoadChain( sizeof(spunames)/sizeof(spunames[0]), ids, spunames, NULL );

	spu->dispatch_table.NewList( 1, GL_COMPILE );
		spu->dispatch_table.Color3f( 1,1,0 );
		spu->dispatch_table.Begin( GL_TRIANGLES );
		spu->dispatch_table.Vertex3fv( v1 );
		spu->dispatch_table.Vertex3fv( v2 );
		spu->dispatch_table.Vertex3fv( v3 );
		spu->dispatch_table.End( );
		spu->dispatch_table.Rotatef( 30, 0, 1, 0 );
		spu->dispatch_table.Color3f( 1,0,0 );
		spu->dispatch_table.Begin( GL_TRIANGLES );
		spu->dispatch_table.Vertex3fv( v1 );
		spu->dispatch_table.Vertex3fv( v2 );
		spu->dispatch_table.Vertex3fv( v3 );
		spu->dispatch_table.End( );
	spu->dispatch_table.EndList( );
	/* spu->dispatch_table.Viewport( 0, 0, 256, 256 ); */
	spu->dispatch_table.ClearColor( 0,0,0,1 );
	for (;;)
	{
		angle += ANGLE_STEP;
		spu->dispatch_table.Clear( GL_COLOR_BUFFER_BIT );
		spu->dispatch_table.MatrixMode( GL_MODELVIEW );
		spu->dispatch_table.LoadIdentity();
		spu->dispatch_table.Rotatef( angle, 0, 0, 1 );
		spu->dispatch_table.CallList( 1 );
		spu->dispatch_table.SwapBuffers();
	}
}
