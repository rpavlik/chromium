# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.


import sys,os;
import cPickle;
import string;
import re;

sys.path.append( "../opengl_stub" )
parsed_file = open( "../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

import stub_common;

keys = gl_mapping.keys()
keys.sort();


stub_common.CopyrightC()

print """
/* DO NOT EDIT - THIS FILE GENERATED BY THE glloader.py SCRIPT */
#include "cr_error.h"
#include "cr_dll.h"
#include "cr_spu.h"
#include "cr_string.h"
#include "cr_error.h"
#include "cr_environment.h"


#if defined(WINDOWS)
#define SYSTEM_GL "opengl32.dll"
#elif defined(IRIX) || defined(IRIX64) || defined(Linux) || defined(FreeBSD) || defined(DARWIN) || defined(AIX) || defined(SunOS) || defined(OSF1)
#if defined(AIX)
#define SYSTEM_GL "libGL.o"
#else
#define SYSTEM_GL "libGL.so"
#endif
typedef void (*glxfuncptr)();
extern glxfuncptr glxGetProcAddressARB( const GLubyte *name );
#else
#error I don't know where your system's GL lives.  Too bad.
#endif

static CRDLL *glDll = NULL;


static int
__fillin( SPUNamedFunctionTable *entry, const char *funcName, SPUGenericFunction funcPtr )
{
	if (funcPtr) {
		entry->name = crStrdup( funcName );
		entry->fn = funcPtr;
		return 1;
	}
	return 0;
}


#define FILLIN( funcName, funcPtr ) \\
	if (__fillin(entry, funcName, funcPtr)) \\
		entry++;

#define FILLIN_OPT( funcName, funcPtr, nopName ) \\
	f = funcPtr; \\
	if (!f) \\
		f = nopName; \\
	(void) __fillin(entry, funcName, f); \\
	entry++;


static CRDLL *
__findSystemGL( char *provided_system_path )
{
	CRDLL *dll;
	char system_path[8096];
	if (provided_system_path && (crStrlen(provided_system_path) > 0) )
	{
		crStrcpy( system_path, provided_system_path );
	}
	else
	{
#if defined(WINDOWS)
		GetSystemDirectory(system_path, MAX_PATH);
#elif defined(IRIX) || defined(IRIX64)
#ifdef IRIX_64BIT
		crStrcpy( system_path, "/usr/lib64" );
#else
		crStrcpy( system_path, "/usr/lib32" );
#endif
#elif defined(PLAYSTATION2)
		crStrcpy( system_path, "/usr/X11R6/lib" );
#else
		crStrcpy( system_path, "/usr/lib" ); 
#endif
	}
	crStrcat( system_path, "/" );
	crStrcat( system_path, SYSTEM_GL );
	dll = crDLLOpen( system_path );
	return dll;
}

static SPUGenericFunction
__findExtFunction( const crOpenGLInterface *interface, const char *funcName )
{
#ifdef WINDOWS
	if (interface->wglGetProcAddress)
		return (SPUGenericFunction) interface->wglGetProcAddress( funcName );
	else
		return (SPUGenericFunction) NULL;
#else
	/* XXX for some reason, the NVIDIA glXGetProcAddressARB() function
	 * returns pointers that cause Chromium to crash.  If we use the
	 * pointer returned by crDLLGetNoError() instead, we're OK.
	 */
	SPUGenericFunction f = crDLLGetNoError(glDll, funcName);
	if (f)
		return f;
	else if (interface->glXGetProcAddressARB)
		return interface->glXGetProcAddressARB( (const GLubyte *) funcName );
	else
		return NULL;
#endif
}
"""


#
# Generate a no-op function.
#
def GenerateNop(name, return_type, names, types):
	print 'static %s Nop%s%s' % (return_type, func_name, stub_common.ArgumentString( names, types ) )
	print '{'
	for name in names:
		if name != "":
			print '\t(void) %s;' % name
	if return_type != 'void':
		print '\treturn 0;'
	print '}'
	print ''



#
# Make no-op funcs for all OpenGL extension functions
#
for func_name in stub_common.AllSpecials( "glloader_extensions" ):
	(return_type, names, types) = gl_mapping[func_name]
	GenerateNop(func_name, return_type, names, types)

#
# Make no-op funcs for all Chromium special functions
#
#for func_name in stub_common.AllSpecials( "glloader_nop" ):
#	(return_type, names, types) = gl_mapping[func_name]
#	GenerateNop(func_name, return_type, names, types)


#
# Generate the crLoadOpenGL() function
#
print """
/*
 * Initialize the 'interface' structure with the WGL or GLX window system
 * interface functions.
 * Then, fill in the table with (name, pointer) pairs for all the core
 * OpenGL entrypoint functions.
 */
int
crLoadOpenGL( crOpenGLInterface *interface, SPUNamedFunctionTable table[] )
{
	SPUNamedFunctionTable *entry = table;
	
	crDebug( "Looking for the system's OpenGL library..." );
	glDll = __findSystemGL( crGetenv( "CR_SYSTEM_GL_PATH" ) );
	if (!glDll)
	{
		crError("Unable to find system OpenGL!");
		return 0;
	}
	crDebug( "Found it in %s.", crGetenv("CR_SYSTEM_GL_PATH") );
"""

useful_wgl_functions = [
	"wglGetProcAddress",
	"wglMakeCurrent",
	"wglSwapBuffers",
	"wglCreateContext",
	"wglDeleteContext",
	"wglGetCurrentContext",
	"wglChoosePixelFormat",
	"wglDescribePixelFormat",
	"wglSetPixelFormat",
	"glGetString"
]
useful_glx_functions = [
	"glXGetConfig",
	"glXQueryExtension",
	"glXChooseVisual",
	"glXCreateContext",
	"glXDestroyContext",
	"glXIsDirect",
	"glXMakeCurrent",
	"glGetString",
	"glXSwapBuffers",
	"glXGetCurrentDisplay"
]
possibly_useful_glx_functions = [
	"glXGetProcAddressARB"
]

print '#ifdef WINDOWS'
for fun in useful_wgl_functions:
	print '\tinterface->%s = (%sFunc_t) crDLLGetNoError( glDll, "%s" );' % (fun,fun,fun)
print '#else'
for fun in useful_glx_functions:
	print '\tinterface->%s = (%sFunc_t) crDLLGetNoError( glDll, "%s" );' % (fun, fun, fun)
for fun in possibly_useful_glx_functions:
	print '\tinterface->%s = (%sFunc_t) crDLLGetNoError( glDll, "%s" );' % (fun, fun, fun)
print '#endif'

for func_name in keys:
	(return_type, names, types) = gl_mapping[func_name]
	# Do extensions later
	if stub_common.FindSpecial( "glloader_extensions", func_name ):
		# extensions done in crLoadOpenGLExtensions() below
		continue
#	elif stub_common.FindSpecial( "glloader_nop", func_name ):
#		print '\tFILLIN_OPT( "%s", crDLLGetNoError(glDll, "gl%s"), (SPUGenericFunction) Nop%s );' % (func_name, func_name, func_name )
	else:
		print '\tFILLIN( "%s", crDLLGetNoError(glDll, "gl%s"));' % (func_name, func_name)

print '\tentry->name = NULL;'
print '\tentry->fn = NULL;'
print '\treturn entry - table;  /* number of entries filled */'
print '}'


print """
int
crLoadOpenGLExtensions( const crOpenGLInterface *interface, SPUNamedFunctionTable table[] )
{
	SPUNamedFunctionTable *entry = table;
	SPUGenericFunction f;

#ifdef WINDOWS
	if (interface->wglGetProcAddress == NULL)
		crWarning("Unable to find wglGetProcAddress() in system GL library");
#else
	if (interface->glXGetProcAddressARB == NULL)
		crWarning("Unable to find glXGetProcAddressARB() in system GL library");
#endif
"""

for func_name in stub_common.AllSpecials( 'glloader_extensions' ):
	print '\tFILLIN_OPT( "%s", __findExtFunction(interface, "gl%s"), (SPUGenericFunction) Nop%s );' % (func_name, func_name, func_name )

print """
	/* end of list */
	entry->name = NULL;
	entry->fn = NULL;
	return entry - table;  /* number of entries filled */
}
"""

