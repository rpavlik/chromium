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

print """#ifndef SERVER_DISPATCH_HEADER
#define SERVER_DISPATCH_HEADER

#ifdef WINDOWS
#define SERVER_DISPATCH_APIENTRY __stdcall
#else
#define SERVER_DISPATCH_APIENTRY
#endif

#include "cr_glwrapper.h"
#include "state/cr_statetypes.h"
"""

for func_name in keys:
	(return_type, names, types) = gl_mapping[func_name]
	if return_type != 'void' or stub_common.FindSpecial( "../packer/packer_get", func_name ) or stub_common.FindSpecial( "server", func_name ) or stub_common.FindSpecial( "../spu/state_tracker/state", func_name ):
		print '%s SERVER_DISPATCH_APIENTRY crServerDispatch%s%s;' % (return_type, func_name, stub_common.ArgumentString( names, types ))

print '#endif /* SERVER_DISPATCH_HEADER */'
