import sys,os;
import cPickle;
import string;
import re;

parsed_file = open( "../../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

sys.path.append( "../../opengl_stub" )

import stub_common;

print """#include "cr_spu.h"
#include "cr_packfunctions.h"
#include "tilesortspu.h"

#include <stdio.h>

void tilesortspuCreateDiffAPI( void )
{
	SPUDispatchTable table;
"""

keys = gl_mapping.keys()
keys.sort();

for func_name in keys:
	(return_type, names, types) = gl_mapping[func_name]
	print '\ttable.%s = (%sFunc_t) crPack%s;' % (func_name,func_name,func_name)
print '\tcrStateDiffAPI( &table );'
print '}'
