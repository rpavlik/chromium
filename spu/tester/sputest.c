/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include <stdlib.h>
#include "cr_spu.h"

#define ANGLE_STEP .5f

int main(int argc, char *argv[])
{
	SPU *spu;
	int ids[] = { 0, 1 };
	char *spunames[] = { "readback", "pack" };
	float angle = 0;
	GLfloat v1[3] = { .25, .25, 0 }; 
	GLfloat v2[3] = { .25, .5, 0 }; 
	GLfloat v3[3] = { .5, .25, 0 }; 
	GLint ctx, win;

	(void) argc;
	(void) argv;

	spu = crSPULoadChain( sizeof(spunames)/sizeof(spunames[0]), ids, spunames, NULL, NULL );

	ctx = spu->dispatch_table.CreateContext( NULL, CR_RGB_BIT | CR_DOUBLE_BIT, 0);
	if (!ctx) {
		fprintf(stderr, "CreateContext() failed!\n");
		exit(1);
	}

	win = spu->dispatch_table.WindowCreate( NULL, CR_RGB_BIT | CR_DOUBLE_BIT);
	spu->dispatch_table.MakeCurrent( win, 0, ctx );

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
		spu->dispatch_table.SwapBuffers( win, 0 );
	}
}
