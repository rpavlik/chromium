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
		arg_types.append( "%s *" % return_type )
		arg_names.append( "return_value" )
	elif stub_common.FindSpecial( "packer_pixel", func_name ):	
		arg_types.append( "CRPackState *" )
		arg_names.append( "packstate" )
	print 'void PACK_APIENTRY crPack%s%s;' %( func_name, stub_common.ArgumentString( arg_names, arg_types ) )
print '\n#endif /* CR_PACKFUNCTIONS_H */'
