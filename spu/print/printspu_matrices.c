#include "cr_glwrapper.h"
#include "printspu.h"

void PRINT_APIENTRY printLoadMatrixf( GLfloat *m )
{
	int i;
	fprintf( print_spu.fp, "LoadMatrixf( [" );
	for (i = 0; i < 16 ; i++)
	{
		fprintf( print_spu.fp, "%10.2f  ", m[i] );
		if ( i != 15 && (i+1)%4 == 0 )
		{
			fprintf( print_spu.fp, "\n              " );
		}
	}
	fprintf( print_spu.fp, "] )\n" );
	fflush( print_spu.fp );
	print_spu.passthrough.LoadMatrixf( m );
}

void PRINT_APIENTRY printLoadMatrixd( GLdouble *m )
{
	int i;
	fprintf( print_spu.fp, "LoadMatrixd( [" );
	for (i = 0; i < 16 ; i++)
	{
		fprintf( print_spu.fp, "%10.2f  ", m[i] );
		if ( i != 15 && (i+1)%4 == 0 )
		{
			fprintf( print_spu.fp, "\n              " );
		}
	}
	fprintf( print_spu.fp, "] )\n" );
	fflush( print_spu.fp );
	print_spu.passthrough.LoadMatrixd( m );
}

void PRINT_APIENTRY printMultMatrixf( GLfloat *m )
{
	int i;
	fprintf( print_spu.fp, "MultMatrixf( [" );
	for (i = 0; i < 16 ; i++)
	{
		fprintf( print_spu.fp, "%10.2f  ", m[i] );
		if ( i != 15 && (i+1)%4 == 0 )
		{
			fprintf( print_spu.fp, "\n              " );
		}
	}
	fprintf( print_spu.fp, "] )\n" );
	fflush( print_spu.fp );
	print_spu.passthrough.MultMatrixf( m );
}

void PRINT_APIENTRY printMultMatrixd( GLdouble *m )
{
	int i;
	fprintf( print_spu.fp, "MultMatrixd( [" );
	for (i = 0; i < 16 ; i++)
	{
		fprintf( print_spu.fp, "%10.2f  ", m[i] );
		if ( i != 15 && (i+1)%4 == 0 )
		{
			fprintf( print_spu.fp, "\n              " );
		}
	}
	fprintf( print_spu.fp, "] )\n" );
	fflush( print_spu.fp );
	print_spu.passthrough.MultMatrixd( m );
}
