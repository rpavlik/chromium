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
#include "cr_net.h"
"""

for func_name in keys:
	(return_type, args, types) = gl_mapping[func_name]
	if stub_common.FindSpecial( "packspu", func_name ): continue
	if return_type != 'void' or stub_common.FindSpecial( "../../packer/packer_get", func_name):
		print '%s PACKSPU_APIENTRY packspu_%s%s' % ( return_type, func_name, stub_common.ArgumentString( args, types ) )
		print '{'
		print '\tint writeback = pack_spu.server.conn->type == CR_DROP_PACKETS ? 0 : 1;'
		if return_type != 'void':
			print '\t%s return_val = (%s) 0;' % (return_type, return_type)
			args.append( "&return_val" )
		args.append( "&writeback" )
		print '\tcrPack%s%s;' % (func_name, stub_common.CallString( args ) )
		print '\tpackspuFlush();'
		print '\twhile (writeback)'
		print '\t\tcrNetRecv();'
		if return_type != 'void':
			print '\treturn return_val;'
		print '}\n'
