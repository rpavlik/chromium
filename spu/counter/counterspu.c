/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_spu.h"
#include "counterspu.h"
#include "cr_error.h"


/*
 * The idea I had for the counter SPU is to count the number of glVertex,
 * glBegin calls, etc and reset / print the counters at strategic
 * places like in glClear() and SwapBuffers(), respectively.
 * We should probably also use the new glChromiumParameterCR() functions
 * to let the user explicitly reset and query counters from within a
 * program.
 *
 * We might also write the counters to a file when we exit.
 *
 * The motivation behind this SPU is to use it for testing that the
 * tilesort SPU really sends vertices only to the appropriate render
 * servers.
 *
 * -Brian
 */

void COUNTERSPU_APIENTRY counterspuSwapBuffers( )
{
	printf("Counter: (glSwapBuffers) Vertex3fv calls %d\n", counter_spu.v3fv);

	counter_spu.v3fv = 0; /* reset it */

	counter_spu.super.SwapBuffers();
}

void COUNTERSPU_APIENTRY counterspuVertex3fv( GLfloat *v0)
{
	counter_spu.v3fv++;

	counter_spu.super.Vertex3fv( v0 );
}

SPUNamedFunctionTable counter_table[] = {
	{ "Vertex3fv", (SPUGenericFunction) counterspuVertex3fv },
	{ "SwapBuffers", (SPUGenericFunction) counterspuSwapBuffers },
	{ NULL, NULL }
};
