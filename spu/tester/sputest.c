#include "cr_spu.h"
#include <stdio.h>
#include "cr_glwrapper.h"

#define ANGLE_STEP .1f

int main(int argc, char *argv[])
{
	SPU *spu;
	char *spuname = "renderspu";
	float angle = 0;

	if (argc > 1)
	{
		spuname = argv[1];
	}
	spu = crSPULoad( NULL, 1, spuname, NULL );

	spu->dispatch_table.ClearColor( 0,0,0,1 );
	for (;;)
	{
		angle += ANGLE_STEP;
		spu->dispatch_table.Clear( GL_COLOR_BUFFER_BIT );
		spu->dispatch_table.MatrixMode( GL_MODELVIEW );
		spu->dispatch_table.LoadIdentity();
		spu->dispatch_table.Rotatef( angle, 0, 0, 1 );
		spu->dispatch_table.Begin( GL_TRIANGLES );
		spu->dispatch_table.Vertex3f( 0,0,0 );
		spu->dispatch_table.Vertex3f( 0,1,0 );
		spu->dispatch_table.Vertex3f( 1,0,0 );
		spu->dispatch_table.End( );
		spu->dispatch_table.SwapBuffers();
	}
}
