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
#include "cr_error.h"
#include "cr_spu.h"
#include "cr_glstate.h"
#include "state/cr_statetypes.h"

#if defined(WINDOWS)
#define NOP_APIENTRY __stdcall
#else
#define NOP_APIENTRY
#endif

#define NOP_UNUSED(x) ((void)x)"""

keys = gl_mapping.keys()
keys.sort();

for func_name in keys:
	(return_type, names, types) = gl_mapping[func_name]
	print '\n%s NOP_APIENTRY nop%s%s' % (return_type, func_name, stub_common.ArgumentString( names, types ))
	print '{'
	for name in names:
		# Handle the void parameter list
		if name:
			print '\tNOP_UNUSED(%s);' % name
	if return_type != "void":
		print '\treturn (%s)0;' % return_type
	print '}'

print 'SPUNamedFunctionTable nop_table[] = {'
for index in range(len(keys)):
	func_name = keys[index]
	if stub_common.FindSpecial( "nop_state", func_name ):
		print '\t{ "%s", (SPUGenericFunction) crState%s },' % (func_name, func_name )
	else:
		print '\t{ "%s", (SPUGenericFunction) nop%s },' % (func_name, func_name )
print '\t{ NULL, NULL }'
print '};'
