
import sys;
import cPickle;
import types;
import string;
import re;

parsed_file = open( "../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

keys = gl_mapping.keys()
keys.sort();

for func_name in keys:
	( return_type, arg_names, arg_types ) = gl_mapping[func_name]

	print "\t.align 4"
	print ".globl gl" + func_name
	print "\t.type gl" + func_name + ",@function"
	print "gl" + func_name + ":"
	print "\t movl __glim_" + func_name + " , %eax"
	print "\tjmp *%eax"
	print ""
