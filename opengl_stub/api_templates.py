import sys,os;
import cPickle;
import string;
import re;

import stub_common;

parsed_file = open( "../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

keys = gl_mapping.keys()
keys.sort();

print '#include <stdio.h>'

for func_name in keys:
	if stub_common.FindSpecial( 'noexport', func_name ):
		continue
	print "void *" + stub_common.DoImmediateMapping( func_name ) + " = NULL;"
print ""
