/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_applications.h"
#include "cr_spu.h"
#include "stub.h"


int APIENTRY crCreateContext( void *display, GLint visBits )
{
	StubInit();
	return stub_spu->dispatch_table.CreateContext( display, visBits );
}

void APIENTRY crDestroyContext( void *display, GLint context )
{
	stub_spu->dispatch_table.DestroyContext( display, context );
}

void APIENTRY crMakeCurrent( void *display, GLint drawable, GLint context )
{
	stub_spu->dispatch_table.MakeCurrent( display, drawable, context );
}

void APIENTRY crSwapBuffers( void *display, GLint drawable )
{
	(void) display;
	(void) drawable;
	/* XXX these should get passed through */
	stub_spu->dispatch_table.SwapBuffers( );
}
