import sys;
import cPickle;
import types;
import string;
import re;

sys.path.append( "../opengl_stub" )

import stub_common;

parsed_file = open( "../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

print """#ifndef CR_PACKFUNCTIONS_H
#define CR_PACKFUNCTIONS_H

#include "cr_glwrapper.h"
#include "state/cr_pixel.h"
#include "cr_pack.h"
"""

keys = gl_mapping.keys()
keys.sort()


for func_name in keys:
	( return_type, arg_names, arg_types ) = gl_mapping[func_name]
	if return_type != 'void':
		if string.find( return_type, '*' ) != -1:
			arg_types.append( "%s" % return_type )
		else:
			arg_types.append( "%s *" % return_type )
		arg_names.append( "return_value" )
	elif stub_common.FindSpecial( "packer_pixel", func_name ):	
		arg_types.append( "CRPackState *" )
		arg_names.append( "packstate" )
	if return_type != 'void' or stub_common.FindSpecial( 'packer_get', func_name ):
		arg_types.append( "int *" )
		arg_names.append( "writeback" )
	print 'void PACK_APIENTRY crPack%s%s;' %( func_name, stub_common.ArgumentString( arg_names, arg_types ) )

for n in [2,3,4]:
	for t in ['d', 'f', 'i', 's']:
		for v in ['', 'v']:
			func_name = 'Vertex%d%s%s' % (n,t,v)
			( return_type, arg_names, arg_types ) = gl_mapping[func_name]
			print 'void PACK_APIENTRY crPack%sBBOX%s;' % (func_name, stub_common.ArgumentString( arg_names, arg_types ) )
			
print '\n#endif /* CR_PACKFUNCTIONS_H */'
