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
#include "packspu.h"
#include "cr_packfunctions.h"
#include "cr_glwrapper.h"
"""

for func_name in keys:
	(return_type, args, types) = gl_mapping[func_name]
	if return_type != 'void' or stub_common.FindSpecial( "../../packer/packer_get", func_name):
		print '%s packspu_%s%s' % ( return_type, func_name, stub_common.ArgumentString( args, types ) )
		print '{'
		if return_type != 'void':
			print '\t%s return_val;' % return_type
			args.append( "&return_val" )
		print '\tcrPack%s%s;' % (func_name, stub_common.CallString( args ) )
		if return_type != 'void':
			print '\treturn (%s)0;' % return_type
		print '}\n'
