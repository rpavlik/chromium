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



print """#include <stdio.h>
#include "cr_error.h"
#include "cr_string.h"
#include "cr_spu.h"
#include "packspu.h"
#include "cr_packfunctions.h"
#include <GL/gl.h>
"""

print 'SPUNamedFunctionTable pack_table[%d];' % len(keys)

print """
static void __fillin( int offset, char *name, SPUGenericFunction func )
{
	pack_table[offset].name = crStrdup( name );
	pack_table[offset].fn = func;
}

void packspuCreateFunctions( )
{"""
for index in range(len(keys)):
	func_name = keys[index]
	print '\t__fillin( %3d, "%s", (SPUGenericFunction) crPack%s );' % (index, func_name, func_name )
print '}'
