
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
#include "cr_opengl_types.h"
#include "cr_error.h"
#include "cr_dll.h"
#include "cr_spu.h"
#include "cr_string.h"
#include "renderspu.h"
"""

print 'SPUNamedFunctionTable render_table[%d];' % len(keys)

print """
#define RENDER_UNUSED(x) ((void)x)

#if defined(WINDOWS)
#define SYSTEM_GL "opengl32.dll"
#elif defined(IRIX) || defined(IRIX64) || defined(Linux)
#define SYSTEM_GL "libGL.so"
#else
#error I don't know where your system's GL lives.  Too bad.
#endif

static void __fillin( SPUNamedFunctionTable *table, char *name, SPUGenericFunction func )
{
	table->name = crStrdup( name );
	table->fn = func;
}

static CRDLL *__findSystemGL( void )
{
	CRDLL *dll;
	char system_path[8096];
#if defined(WINDOWS)
	GetSystemDirectory(system_path, MAX_PATH);
#else
	crStrcpy( system_path, "/usr/lib" )
#endif
	crStrcat( system_path, "/" );
	crStrcat( system_path, SYSTEM_GL );
	dll = crDLLOpen( system_path );
	return dll;
}
"""

for func_name in keys:
	if stub_common.FindSpecial( "render", func_name ):
		print 'void SPU_APIENTRY __renderSpecial%s(void) {}' % func_name

print """
void LoadSystemGL( SPUNamedFunctionTable *table )
{
	CRDLL *dll = __findSystemGL();
"""

index = 0
for func_name in keys:
	(return_type, names, types) = gl_mapping[func_name]
	if stub_common.FindSpecial( "render", func_name ):
		print '\t__fillin( table + %3d, "%s", (SPUGenericFunction) __renderSpecial%s );' % (index, func_name, func_name )
	elif stub_common.FindSpecial( "render_system", func_name ): 
		print '\t__fillin( table + %3d, "%s", (SPUGenericFunction) renderspu%s );' % (index, func_name, func_name )
	else:
		print '\t__fillin( table + %3d, "%s", crDLLGet( dll, "gl%s" ) );' % (index, func_name, func_name )
	index += 1
print """#ifdef WINDOWS
	render_spu.wglMakeCurrent = (wglMakeCurrentFunc_t) crDLLGet( dll, "wglMakeCurrent" );
	render_spu.wglSwapBuffers = (wglSwapBuffersFunc_t) crDLLGet( dll, "wglSwapBuffers" );
	render_spu.wglCreateContext = (wglCreateContextFunc_t) crDLLGet( dll, "wglCreateContext" );
#else
#error WORK ON IT
#endif
}"""
