/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "cr_error.h" 
#include "cr_mem.h" 
#include "stub.h"
#include "api_templates.h"

/*
 * Returns -1 on error
 */
int APIENTRY crCreateContext( const char *dpyName, GLint visBits )
{
	ContextInfo *context;
	stubInit();
	context = stubNewContext(dpyName, visBits, CHROMIUM);
	return context ? context->id : -1;
}

void APIENTRY crDestroyContext( GLint context )
{
  	stubDestroyContext(context);
}

void APIENTRY crMakeCurrent( GLint window, GLint context )
{
	WindowInfo *winInfo = (WindowInfo *)
		crHashtableSearch(stub.windowTable, (unsigned int) window);
	ContextInfo *contextInfo = (ContextInfo *)
		crHashtableSearch(stub.contextTable, context);
	if (contextInfo && contextInfo->type == NATIVE) {
		crWarning("Can't call crMakeCurrent with native GL context");
		return;
	}
	stubMakeCurrent(winInfo, contextInfo);
}

GLint APIENTRY crGetCurrentContext( void )
{
	stubInit();
	if (stub.currentContext)
	  return stub.currentContext->id;
	else
	  return 0;
}

GLint APIENTRY crGetCurrentWindow( void )
{
	stubInit();
	if (stub.currentContext && stub.currentContext->currentDrawable)
	  return stub.currentContext->currentDrawable->spuWindow;
	else
	  return -1;
}

void APIENTRY crSwapBuffers( GLint window, GLint flags )
{
	const WindowInfo *winInfo = (const WindowInfo *)
		crHashtableSearch(stub.windowTable, (unsigned int) window);
	if (winInfo)
		stubSwapBuffers(winInfo, flags);
}

/*
 * Returns -1 on error
 */
GLint APIENTRY crWindowCreate( const char *dpyName, GLint visBits )
{
	stubInit();
	return stubNewWindow( dpyName, visBits );
}

void APIENTRY crWindowDestroy( GLint window )
{
	WindowInfo *winInfo = (WindowInfo *)
		crHashtableSearch(stub.windowTable, (unsigned int) window);
	if (winInfo && winInfo->type == CHROMIUM) {
		stub.spu->dispatch_table.WindowDestroy( winInfo->spuWindow );
		crHashtableDelete(stub.windowTable, window, crFree);
	}
}

void APIENTRY crWindowSize( GLint window, GLint w, GLint h )
{
	const WindowInfo *winInfo = (const WindowInfo *)
		crHashtableSearch(stub.windowTable, (unsigned int) window);
	if (winInfo && winInfo->type == CHROMIUM)
		stub.spu->dispatch_table.WindowSize( window, w, h );
}

void APIENTRY crWindowPosition( GLint window, GLint x, GLint y )
{
	const WindowInfo *winInfo = (const WindowInfo *)
		crHashtableSearch(stub.windowTable, (unsigned int) window);
	if (winInfo && winInfo->type == CHROMIUM)
		stub.spu->dispatch_table.WindowPosition( window, x, y );
}

void APIENTRY stub_GetChromiumParametervCR( GLenum target, GLuint index, GLenum type, GLsizei count, GLvoid *values )
{
	char **ret;
	switch( target )
	{
		case GL_HEAD_SPU_NAME_CR:
			ret = (char **) values;
			*ret = stub.spu->name;
			return;
		default:
			stub.spu->dispatch_table.GetChromiumParametervCR( target, index, type, count, values );
			break;
	}
}
