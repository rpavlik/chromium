
import sys;
import cPickle;
import types;
import string;
import re;

import stub_common;

parsed_file = open( "../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

keys = gl_mapping.keys()
keys.sort();

print """#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
"""

print "/* these pointers live in opengl_stub/api_templates.c */"
for func_name in keys:
    if stub_common.FindSpecial( "exports", func_name ): continue
    print "extern void *" + stub_common.DoImmediateMapping( func_name ) + ";"
print ""

for func_name in keys:
    if stub_common.FindSpecial( "exports", func_name ): continue
    ( return_type, arg_names, arg_types ) = gl_mapping[func_name]

    print "typedef " + return_type + " (*gl" + func_name + "_ptr)",
    print stub_common.ArgumentString( arg_names, arg_types ),
    print ";"
print ""

for func_name in keys:
    if stub_common.FindSpecial( "exports", func_name ): continue
    ( return_type, arg_names, arg_types ) = gl_mapping[func_name]

    print return_type + " gl" + func_name,
    print stub_common.ArgumentString( arg_names, arg_types )
    print "{"
    print "\t",

    if return_type != "void":
	print "return ",
    print "((gl" + func_name + "_ptr) __glim_" + func_name + ")",
    print "(",
    for index in range( 0, len( arg_names ) ):
	arg = arg_names[index]
	if arg != "":
	    print arg,
	    if index != len( arg_names ) - 1:
		print ",",
    print ");"

    print "}"
    print ""
