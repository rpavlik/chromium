
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


print """
#include <stdio.h>
#include "cr_glwrapper.h"
#include "cr_error.h"
#include "cr_dll.h"
#include "cr_spu.h"
#include "cr_string.h"
#include "renderspu.h"
"""

print 'SPUNamedFunctionTable render_table[%d];' % (len(keys)+1)

print """
#define RENDER_UNUSED(x) ((void)x)

#if defined(WINDOWS)
#define SYSTEM_GL "opengl32.dll"
#elif defined(IRIX) || defined(IRIX64) || defined(Linux)
#define SYSTEM_GL "libGL.so"
#else
#error I don't know where your system's GL lives.  Too bad.
#endif

static void __fillin( int offset, char *name, SPUGenericFunction func )
{
	render_table[offset].name = crStrdup( name );
	render_table[offset].fn = func;
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
void renderspuLoadSystemGL( )
{
	CRDLL *dll = __findSystemGL();
"""

for index in range(len(keys)):
	func_name = keys[index]
	(return_type, names, types) = gl_mapping[func_name]
	if stub_common.FindSpecial( "render_nop", func_name ):
		print '\t__fillin( %3d, "%s", (SPUGenericFunction) __renderNop%s );' % (index, func_name, func_name )
	elif stub_common.FindSpecial( "render", func_name ): 
		print '\t__fillin( %3d, "%s", (SPUGenericFunction) renderspu%s );' % (index, func_name, func_name )
	else:
		print '\t__fillin( %3d, "%s", crDLLGet( dll, "gl%s" ) );' % (index, func_name, func_name )
print '\t__fillin( %3d, NULL, NULL );' % len(keys)

useful_wgl_functions = [
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
print '#ifdef WINDOWS'
for fun in useful_wgl_functions:
	print '\trender_spu.wgl%s = (wgl%sFunc_t) crDLLGet( dll, "wgl%s" );' % (fun,fun,fun)
print '#else'
for fun in useful_glx_functions:
	print '\trender_spu.%s = (%sFunc_t) crDLLGet( dll, "%s" );' % (fun, fun, fun)
print """#endif
}"""
