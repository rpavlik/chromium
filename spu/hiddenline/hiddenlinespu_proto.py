# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

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



stub_common.CopyrightC()

for func_name in keys:
	(return_type, args, types) = gl_mapping[func_name]
	if return_type != 'void' and not stub_common.FindSpecial( "hiddenline_ignore", func_name ):
		#print >> sys.stderr, func_name
		pass

print """
#ifndef HIDDENLINE_SPU_PROTO_H
#define HIDDENLINE_SPU_PROTO_H 1

"""

num_funcs = len(keys) - len( stub_common.AllSpecials( "hiddenline_ignore" ) )

for func_name in keys:
	(return_type, args, types) = gl_mapping[func_name]

specials = stub_common.AllSpecials( "hiddenline" ) + stub_common.AllSpecials( "hiddenline_pixel" )

for func_name in keys:
	(return_type, args, types) = gl_mapping[func_name]
	if func_name in specials:
		print 'extern %s HIDDENLINESPU_APIENTRY hiddenlinespu_%s%s;' % ( return_type, func_name, stub_common.ArgumentString( args, types ) )


print """
#endif
"""
