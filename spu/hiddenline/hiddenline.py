# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

import sys

sys.path.append( "../../glapi_parser" )
import apiutil


apiutil.CopyrightC()

print """
#include "cr_string.h"
#include "cr_spu.h"
#include "cr_packfunctions.h"
#include "hiddenlinespu.h"
#include "hiddenlinespu_proto.h"
"""

keys = apiutil.GetDispatchedFunctions("../../glapi_parser/APIspec.txt")

# Determine which functions to ignore
ignore_functions = []
for func_name in keys:
	if func_name == "CreateContext":
		continue
	if ("get" in apiutil.Properties(func_name) or
		"setclient" in apiutil.Properties(func_name) or
		"useclient" in apiutil.Properties(func_name) or
		func_name in apiutil.AllSpecials("hiddenline_ignore") or
		apiutil.Category(func_name) == "GL_chromium"):
		ignore_functions.append(func_name)

num_funcs = len(keys) - len(ignore_functions)

specials = apiutil.AllSpecials( "hiddenline" ) + apiutil.AllSpecials( "hiddenline_pixel" )

print 'SPUNamedFunctionTable _cr_hiddenline_table[%d];' % (num_funcs + 1)

print ''
print 'static void __fillin( int offset, char *name, SPUGenericFunction func )'
print '{'
print '\tCRASSERT( offset < %d);' % (num_funcs + 1)
print '\t_cr_hiddenline_table[offset].name = crStrdup( name );'
print '\t_cr_hiddenline_table[offset].fn = func;'
print '}'
print ''

print '\nvoid hiddenlinespuCreateFunctions( void )'
print '{'
table_index = 0
for func_name in keys:
	if func_name in ignore_functions:
		# don't implement any get-functions
		continue
	elif func_name in specials:
		print '\t__fillin( %3d, "%s", (SPUGenericFunction) hiddenlinespu_%s );' % (table_index, func_name, func_name )
	else:
		print '\t__fillin( %3d, "%s", (SPUGenericFunction) crPack%s );' % (table_index, func_name, func_name )
	table_index += 1
print '\t__fillin( %3d, NULL, NULL );' % num_funcs
print '}'

assert table_index < num_funcs + 1
