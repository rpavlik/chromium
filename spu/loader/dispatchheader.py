import sys,os;
import cPickle;
import string;
import re;

parsed_file = open( "../../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

sys.path.append( "../../opengl_stub" )

import stub_common;

print """#ifndef CR_SPU_DISPATCH_TABLE_H
#define CR_SPU_DISPATCH_TABLE_H

#ifdef WINDOWS
#define SPU_APIENTRY __stdcall
#else
#define SPU_APIENTRY
#endif

#include <GL/gl.h>
"""

keys = gl_mapping.keys()
keys.sort();

for func_name in keys:
	(return_type, names, types) = gl_mapping[func_name]
	print 'typedef %s (SPU_APIENTRY *%sFunc_t)(%s);' % (return_type, func_name, string.join(types, ', '))

print 'typedef struct {'

for i in range(len(keys)):
	func_name = keys[i]
	(return_type, names, types) = gl_mapping[func_name]

	print "\t%sFunc_t %s; " % ( func_name, func_name )

print '} SPUDispatchTable;'

print '#endif /* CR_SPU_DISPATCH_TABLE_H */'
