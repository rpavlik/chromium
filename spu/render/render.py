# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.


import sys,os;
import cPickle;
import string;
import re;

sys.path.append( "../../opengl_stub" )
parsed_file = open( "../../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

import stub_common;

keys = gl_mapping.keys()
keys.sort();


stub_common.CopyrightC()

print """
#include <stdio.h>
#include "cr_glwrapper.h"
#include "cr_error.h"
#include "cr_dll.h"
#include "cr_spu.h"
#include "cr_string.h"
#include "cr_error.h"
#include "renderspu.h"
"""

print 'SPUNamedFunctionTable render_table[%d];' % (len(keys)+1)

print """
#define RENDER_UNUSED(x) ((void)x)

#if defined(WINDOWS)
#define SYSTEM_GL "opengl32.dll"
#elif defined(IRIX) || defined(IRIX64) || defined(Linux)
#define SYSTEM_GL "libGL.so"
typedef void (*glxfuncptr)();
extern glxfuncptr glxGetProcAddressARB( const GLubyte *name );
#else
#error I don't know where your system's GL lives.  Too bad.
#endif

#define FILLIN( n, f ) if (__fillin( function_index, (n), (f) ) == 1 ) function_index++;

int function_index = 0;

static int __fillin( int offset, char *name, SPUGenericFunction func )
{
	if (name != NULL && func == NULL)
	{
		crDebug( "NULL function for %s", name );
		return 0; /* The function will come from error */
	}
	render_table[offset].name = crStrdup( name );
	render_table[offset].fn = func;
	return 1;
}

static CRDLL *__findSystemGL( void )
{
	CRDLL *dll;
	char system_path[8096];
#if defined(WINDOWS)
	GetSystemDirectory(system_path, MAX_PATH);
#elif defined(IRIX) || defined(IRIX64)
	crStrcpy( system_path, "/usr/lib32" );
#else
	crStrcpy( system_path, "/usr/lib" );
#endif
	crStrcat( system_path, "/" );
	crStrcat( system_path, SYSTEM_GL );
	dll = crDLLOpen( system_path );
	return dll;
}
"""

for func_name in stub_common.AllSpecials( "render_nop" ):
	(return_type, names, types) = gl_mapping[func_name]
	print 'void SPU_APIENTRY __renderNop%s%s' % (func_name, stub_common.ArgumentString( names, types ) )
	print '{'
	for name in names:
		if name != "":
			print '\t(void) %s;' % name
	print '}'

print """
void renderspuLoadSystemGL( void )
{
	CRDLL *dll = __findSystemGL();
"""

useful_wgl_functions = [
	"GetProcAddress",
	"MakeCurrent",
	"SwapBuffers",
	"CreateContext",
	"GetCurrentContext",
	"ChoosePixelFormat",
	"SetPixelFormat"
]
useful_glx_functions = [
	"glXGetConfig",
	"glXQueryExtension",
	"glXChooseVisual",
	"glXCreateContext",
	"glXIsDirect",
	"glXMakeCurrent",
	"glGetString",
	"glXSwapBuffers"
]
possibly_useful_glx_functions = [
	"glXGetProcAddressARB"
]

print '#ifdef WINDOWS'
for fun in useful_wgl_functions:
	print '\trender_spu.wgl%s = (wgl%sFunc_t) crDLLGet( dll, "wgl%s" );' % (fun,fun,fun)
print '#else'
for fun in useful_glx_functions:
	print '\trender_spu.%s = (%sFunc_t) crDLLGet( dll, "%s" );' % (fun, fun, fun)
for fun in possibly_useful_glx_functions:
	print '\trender_spu.%s = (%sFunc_t) crDLLGetNoError( dll, "%s" );' % (fun, fun, fun)
print '#endif'

for func_name in keys:
	(return_type, names, types) = gl_mapping[func_name]
	if stub_common.FindSpecial( "render_nop", func_name ):
		print '\tFILLIN( "%s", (SPUGenericFunction) __renderNop%s );' % (func_name, func_name )
	elif stub_common.FindSpecial( "render", func_name ): 
		print '\tFILLIN( "%s", (SPUGenericFunction) renderspu%s );' % (func_name, func_name )
	elif stub_common.FindSpecial( "render_extensions", func_name ):
		continue;
	else:
		print '\tFILLIN( "%s", crDLLGet( dll, "gl%s" ) );' % (func_name, func_name )
print '}'

print """
void renderspuLoadSystemExtensions( void )
{"""
print '#ifdef WINDOWS'
for func_name in stub_common.AllSpecials( 'render_extensions' ):
	print '\tFILLIN( "%s", (SPUGenericFunction) render_spu.wglGetProcAddress( "gl%s" ) );' % (func_name, func_name )
print '#else'
print '\tif (render_spu.glXGetProcAddressARB == NULL)'
print '\t{'
print '\t\tFILLIN( NULL, NULL );'
print '\t\treturn;'
print '\t}'
print '\telse'
print '\t{'
for func_name in stub_common.AllSpecials( 'render_extensions' ):
	print '\t\tFILLIN( "%s", (SPUGenericFunction) render_spu.glXGetProcAddressARB( "gl%s" ) );' % (func_name, func_name )
print '\t}'
print '#endif'
print '\tFILLIN( NULL, NULL );'
print '}'
