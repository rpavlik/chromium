import sys,os;
import cPickle;
import string;
import re;

sys.path.append( "../../opengl_stub" )
parsed_file = open( "../../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

import stub_common;

print """#include <stdio.h>
#include "cr_opengl_types.h"
#include "cr_error.h"
#include "cr_string.h"
#include "cr_spu.h" 

static void __fillin( SPUNamedFunctionTable *table, char *name, SPUGenericFunction func )
{
	table->name = crStrdup( name );
	table->fn = func;
}
"""

keys = gl_mapping.keys()
keys.sort();

print 'SPUNamedFunctionTable passthrough_table[%d];' % len(keys)

print """void BuildPassthroughTable( SPU *child )
{"""
for index in range(len(keys)):
	func_name = keys[index]
	print '\t__fillin( passthrough_table + %3d, "%s", (SPUGenericFunction) child->dispatch_table.%s );' % (index, func_name, func_name )
print '}'
