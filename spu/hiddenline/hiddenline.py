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
#include <stdio.h>
#include "cr_string.h"
#include "cr_spu.h"
#include "cr_packfunctions.h"
#include "hiddenlinespu.h"
#include "hiddenlinespu_proto.h"
"""

num_funcs = len(keys) - len( stub_common.AllSpecials( "hiddenline_ignore" ) )

for func_name in keys:
	(return_type, args, types) = gl_mapping[func_name]

specials = stub_common.AllSpecials( "hiddenline" ) + stub_common.AllSpecials( "hiddenline_pixel" )
print 'SPUNamedFunctionTable _cr_hiddenline_table[%d];' % (num_funcs+1)

print ''
print 'static void __fillin( int offset, char *name, SPUGenericFunction func )'
print '{'
print '\tCRASSERT( offset < %d);' % (num_funcs + 1)
print '\t_cr_hiddenline_table[offset].name = crStrdup( name );'
print '\t_cr_hiddenline_table[offset].fn = func;'
print '}'
print ''

#for func_name in keys:
#	(return_type, args, types) = gl_mapping[func_name]
#	if func_name in specials:
#		print 'extern %s HIDDENLINESPU_APIENTRY hiddenlinespu_%s%s;' % ( return_type, func_name, stub_common.ArgumentString( args, types ) )

print '\nvoid hiddenlinespuCreateFunctions( void )'
print '{'
table_index = 0
for index in range(len(keys)):
	func_name = keys[index]
	if stub_common.FindSpecial( "hiddenline_ignore", func_name ):
		continue
	(return_type, args, types) = gl_mapping[func_name]
	if func_name in specials:
		print '\t__fillin( %3d, "%s", (SPUGenericFunction) hiddenlinespu_%s );' % (table_index, func_name, func_name )
	else:
		print '\t__fillin( %3d, "%s", (SPUGenericFunction) crPack%s );' % (table_index, func_name, func_name )
	table_index += 1
print '\t__fillin( %3d, NULL, NULL );' % num_funcs
print '}'
