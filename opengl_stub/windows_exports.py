
import sys,os;
import cPickle;
import types;
import string;
import re;

import stub_common;

parsed_file = open( "../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

keys = gl_mapping.keys()
keys.sort();

print """#include "cr_glwrapper.h"
#include "api_templates.h"

#define NAKED __declspec(naked)
#define UNUSED(x) ((void)(x))
"""

for func_name in keys:
	if stub_common.FindSpecial( "noexport", func_name ): continue
	( return_type, arg_names, arg_types ) = gl_mapping[func_name]

	print "NAKED " + return_type + " cr_gl" + func_name,
	print stub_common.ArgumentString( arg_names, arg_types )
	print "{"
	print "\t__asm jmp [" + stub_common.DoImmediateMapping( func_name ) + "]"
	for arg_name in arg_names:
		if arg_name: 
			print "\tUNUSED(" + arg_name + ");"
	print "}"
	print ""

