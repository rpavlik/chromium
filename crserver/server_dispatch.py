import sys,os;
import cPickle;
import string;
import re;

sys.path.append( "../opengl_stub" )
parsed_file = open( "../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

import stub_common;

print """#include "cr_spu.h"
#include "cr_glwrapper.h"
#include "cr_error.h"
#include "server_dispatch.h"
#include "server.h"

void crServerInitDispatch(void)
{
	crSPUCopyDispatchTable( &(cr_server.dispatch), &(cr_server.head_spu->dispatch_table ) );
"""

keys = gl_mapping.keys()
keys.sort();

for func_name in keys:
	(return_type, names, types) = gl_mapping[func_name]
	if return_type != 'void' or stub_common.FindSpecial( "../packer/packer_get", func_name ) or stub_common.FindSpecial( "server", func_name ):
		print '\tcr_server.dispatch.%s = crServerDispatch%s;' % (func_name, func_name)
print '}'
