# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

import sys,os;
import cPickle;
import string;
import re;

parsed_file = open( "../../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

sys.path.append( "../../opengl_stub" )

import stub_common;

keys = gl_mapping.keys()
keys.sort();

stub_common.CopyrightC()
print """#include "cr_spu.h"

void crSPUCopyDispatchTable( SPUDispatchTable *dst, SPUDispatchTable *src )
{"""

for func_name in keys:
	(return_type, names, types) = gl_mapping[func_name]
	print '\tdst->%s = src->%s;' % (func_name,func_name)
print '}'
