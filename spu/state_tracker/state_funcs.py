
import sys,os;
import cPickle;
import string;
import re;

sys.path.append( "../../opengl_stub" )
parsed_file = open( "../../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

import stub_common;

print """#ifndef CR_STATE_FUNCS_H
#define CR_STATE_FUNCS_H

#include "cr_glwrapper.h"
#include "cr_error.h"

#if defined(WINDOWS)
#define STATE_APIENTRY __stdcall
#else
#define STATE_APIENTRY
#endif

#define STATE_UNUSED(x) ((void)x)"""

keys = gl_mapping.keys()
keys.sort();

for func_name in stub_common.AllSpecials( "state" ):
	(return_type, names, types) = gl_mapping[func_name]
	print '%s STATE_APIENTRY crState%s%s;' % (return_type, func_name, stub_common.ArgumentString( names, types ))
print '\n#endif /* CR_STATE_FUNCS_H */'
