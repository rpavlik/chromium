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

stub_common.CopyrightC()

print """#include <stdio.h>
#include "cr_spu.h"
#include "cr_glstate.h"
"""

keys = gl_mapping.keys()
keys.sort();

print 'SPUNamedFunctionTable simplequery_table[] = {'
for index in range(len(keys)):
	func_name = keys[index]
	if stub_common.FindSpecial( "simplequery", func_name ):
		print '\t{ "%s", (SPUGenericFunction) crState%s },' % (func_name, func_name )
print '\t{ NULL, NULL }'
print '};'
