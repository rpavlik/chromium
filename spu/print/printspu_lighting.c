/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "printspu.h"

void PRINT_APIENTRY printMaterialfv( GLenum face, GLenum mode, const GLfloat *params )
{
	int i;
	int num_params = 4;
	fprintf( print_spu.fp, "Materialfv( %s, %s, [ ", printspuEnumToStr( face ), printspuEnumToStr( mode ) );

	if (mode == GL_SHININESS)
	{
		num_params = 1;
	}

	for (i = 0 ; i < num_params ; i++)
	{
		fprintf( print_spu.fp, "%f", params[i] );
		if (i != num_params -1)
		{
			fprintf( print_spu.fp, ", " );
		}
	}
	fprintf( print_spu.fp, " ] )\n" );
	fflush( print_spu.fp );
	print_spu.passthrough.Materialfv( face, mode, params );
}

void PRINT_APIENTRY printMaterialiv( GLenum face, GLenum mode, const GLint *params )
{
	int i;
	int num_params = 4;
	fprintf( print_spu.fp, "Materialiv( %s, %s, [ ", printspuEnumToStr( face ), printspuEnumToStr( mode ) );

	if (mode == GL_SHININESS)
	{
		num_params = 1;
	}

	for (i = 0 ; i < num_params ; i++)
	{
		fprintf( print_spu.fp, "%d", params[i] );
		if (i != num_params -1)
		{
			fprintf( print_spu.fp, ", " );
		}
	}
	fprintf( print_spu.fp, " ] )\n" );
	fflush( print_spu.fp );
	print_spu.passthrough.Materialiv( face, mode, params );
}

void PRINT_APIENTRY printLightfv( GLenum light, GLenum pname, const GLfloat *params )
{
	int i;
	int num_params = 4;
	fprintf( print_spu.fp, "Lightfv( %s, %s, [ ", printspuEnumToStr( light ), printspuEnumToStr( pname ) );

	if (pname == GL_SPOT_EXPONENT || pname == GL_SPOT_CUTOFF || pname == GL_CONSTANT_ATTENUATION || pname == GL_LINEAR_ATTENUATION || pname == GL_QUADRATIC_ATTENUATION )
	{
		num_params = 1;
	}

	for (i = 0 ; i < num_params ; i++)
	{
		fprintf( print_spu.fp, "%f", params[i] );
		if (i != num_params -1)
		{
			fprintf( print_spu.fp, ", " );
		}
	}
	fprintf( print_spu.fp, " ] )\n" );
	fflush( print_spu.fp );
	print_spu.passthrough.Lightfv( light, pname, params );
}

void PRINT_APIENTRY printLightiv( GLenum light, GLenum pname, const GLint *params )
{
	int i;
	int num_params = 4;
	fprintf( print_spu.fp, "Lightiv( %s, %s, [ ", printspuEnumToStr( light ), printspuEnumToStr( pname ) );

	if (pname == GL_SPOT_EXPONENT || pname == GL_SPOT_CUTOFF || pname == GL_CONSTANT_ATTENUATION || pname == GL_LINEAR_ATTENUATION || pname == GL_QUADRATIC_ATTENUATION )
	{
		num_params = 1;
	}

	for (i = 0 ; i < num_params ; i++)
	{
		fprintf( print_spu.fp, "%d", params[i] );
		if (i != num_params -1)
		{
			fprintf( print_spu.fp, ", " );
		}
	}
	fprintf( print_spu.fp, " ] )\n" );
	fflush( print_spu.fp );
	print_spu.passthrough.Lightiv( light, pname, params );
}

