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

stub_common.CopyrightC()

print """#include "cr_spu.h"
#include "cr_packfunctions.h"
#include "tilesortspu.h"

#include <stdio.h>
"""

for func_name in stub_common.AllSpecials( "../../packer/packer_pixel" ):
	(return_type, names, types) = gl_mapping[func_name]
	print '%s TILESORTSPU_APIENTRY tilesortspuDiff%s%s' % (return_type, func_name, stub_common.ArgumentString( names, types ) )
	print '{'
	names.append( '&(tilesort_spu.ctx->pixel.unpack)' )
	print '\tcrPack%s%s;' % (func_name, stub_common.CallString( names ) )
	print '}'

print """
void tilesortspuCreateDiffAPI( void )
{
	SPUDispatchTable table;
"""

keys = gl_mapping.keys()
keys.sort();

for func_name in keys:
	(return_type, names, types) = gl_mapping[func_name]
	if stub_common.FindSpecial( "../../packer/packer_pixel", func_name ):
		print '\ttable.%s = (%sFunc_t) tilesortspuDiff%s;' % (func_name,func_name,func_name)
	else:
		print '\ttable.%s = (%sFunc_t) crPack%s;' % (func_name,func_name,func_name)
print '\tcrStateDiffAPI( &table );'
print '}'
