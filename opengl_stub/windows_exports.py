# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

import sys,os;
import cPickle;
import types;
import string;
import re;
import alias_exports;
import stub_common;

parsed_file = open( "../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

keys = gl_mapping.keys()
keys.sort();

stub_common.CopyrightC()

print """#include "chromium.h"
#include "api_templates.h"

#define NAKED __declspec(naked)
#define UNUSED(x) ((void)(x))
"""

for func_name in keys:
	if stub_common.FindSpecial( "noexport", func_name ): continue
	( return_type, arg_names, arg_types ) = gl_mapping[func_name]

	print "NAKED %s cr_gl%s" % (return_type, func_name),
	print stub_common.ArgumentString( arg_names, arg_types )
	print "{"
	print "\t__asm jmp [glim.%s]" % func_name
	for arg_name in arg_names:
		if arg_name: 
			print "\tUNUSED( %s );" % arg_name
	print "}"
	print ""
    	real_func_name = alias_exports.AliasMap(func_name);
    	if real_func_name:
		print "NAKED %s cr_gl%s" % (return_type, real_func_name),
		print stub_common.ArgumentString( arg_names, arg_types )
		print "{"
		print "\t__asm jmp [glim.%s]" % func_name
		for arg_name in arg_names:
			if arg_name: 
				print "\tUNUSED( %s );" % arg_name
		print "}"
		print ""
