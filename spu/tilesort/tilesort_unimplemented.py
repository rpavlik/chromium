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

print """
#include "tilesortspu.h"
#include "cr_error.h"    
"""

for func_name in keys:
	(return_type, args, types) = gl_mapping[func_name]
	if stub_common.FindSpecial( "tilesort_unimplemented", func_name ):
		print '%s TILESORTSPU_APIENTRY tilesortspu_%s%s' % ( return_type, func_name, stub_common.ArgumentString( args, types ) )	
		print '{'
		for a in args:
			if a:
				print '\t(void)%s;' % a
		print ''
		print '\tcrWarning("Unimplemented tilesort function %s\\n");' % func_name
		if return_type != 'void':
			print ''
			print '\treturn 0;'
		print '}'
		print ''
