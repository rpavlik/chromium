/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_applications.h"
#include "cr_spu.h"
#include "cr_error.h" 
#include "stub.h"


int APIENTRY crCreateContext( const char *dpyName, GLint visBits )
{
	crDebug( "In CreateConetxt" );
	StubInit();
	return stub.spu->dispatch_table.CreateContext( dpyName, visBits );
}

void APIENTRY crDestroyContext( GLint context )
{
	stub.spu->dispatch_table.DestroyContext( context );
}

void APIENTRY crMakeCurrent( GLint window, GLint context )
{
	const GLint nativeWindow = 0;
	stub.spu->dispatch_table.MakeCurrent( window, nativeWindow, context );
}

void APIENTRY crSwapBuffers( GLint window, GLint flags )
{
	stub.spu->dispatch_table.SwapBuffers( window, flags );
}

GLint APIENTRY crCreateWindow( const char *dpyName, GLint visBits )
{
	StubInit();
	return stub.spu->dispatch_table.crCreateWindow( dpyName, visBits );
}

void APIENTRY crDestroyWindow( GLint window )
{
	stub.spu->dispatch_table.DestroyWindow( window );
}

void APIENTRY crWindowSize( GLint window, GLint w, GLint h )
{
	stub.spu->dispatch_table.WindowSize( window, w, h );
}

void APIENTRY crWindowPosition( GLint window, GLint x, GLint y )
{
	stub.spu->dispatch_table.WindowPosition( window, x, y );
}

