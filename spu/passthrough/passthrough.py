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

print """#include <stdio.h>
#include "cr_error.h"
#include "cr_string.h"
#include "cr_spu.h"
#include "passthroughspu.h"
"""

print 'SPUNamedFunctionTable _cr_passthrough_table[%d];' % ( len(keys) + 1 )

print """
static void __fillin( int offset, char *name, SPUGenericFunction func )
{
	_cr_passthrough_table[offset].name = crStrdup( name );
	_cr_passthrough_table[offset].fn = func;
}

void BuildPassthroughTable( SPU *child )
{"""
for index in range(len(keys)):
	func_name = keys[index]
	print '\t__fillin( %3d, "%s", (SPUGenericFunction) child->dispatch_table.%s );' % (index, func_name, func_name )
print '\t__fillin( %3d, NULL, NULL );' % len(keys)
print '}'
