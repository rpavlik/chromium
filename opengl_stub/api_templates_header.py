# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

import sys,os;
import cPickle;
import string;
import re;

import stub_common;

parsed_file = open( "../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

keys = gl_mapping.keys()
keys.sort();

stub_common.CopyrightC()

print '#ifndef CR_API_TEMPLATES_H'
print '#define CR_API_TEMPLATES_H'

for func_name in keys:
	if stub_common.FindSpecial( "noexport", func_name ): 
		continue
	print "extern void *%s;" % stub_common.DoImmediateMapping( func_name )

print '#endif /* CR_API_TEMPLATES_H */'
