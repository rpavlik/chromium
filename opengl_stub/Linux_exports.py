# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.


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

stub_common.CopyrightC()

for index in range(len(keys)):
	func_name = keys[index]
	if stub_common.FindSpecial( "noexport", func_name ): continue
	( return_type, arg_names, arg_types ) = gl_mapping[func_name]

	print "\t.align 4"
	print ".globl gl%s" % func_name
	print "\t.type gl%s,@function" % func_name
	print "gl%s:" % func_name
	print "\tmovl glim+%d, %%eax" % (4*index)
	print "\tjmp *%eax"
	print ""

# Deal with NVIDIA driver lossage.  These functions will never be called, but
# the symbols need to exist.  Fooey.

nvidia_symbols = 1

if nvidia_symbols:
	GLcore_crap = [
		'__glTLSCXIndex',
		'__glXthreadGlxc',
		'__glNVTLSIndex',
		'NvRmConfigGet',
		'NvRmArchHeap',
		'NvRmConfigGetEx',
		'NvRmAllocMemory',
		'NvRmFree',
		'NvRmAllocChannelDma',
		'NvRmAllocRoot',
		'NvRmAllocDevice',
		'NvRmAllocContextDma',
		'NvRmAlloc',
		'NvRmIoFlush'
	]

	for crap in GLcore_crap:
		print "\t.align 4"
		print ".globl %s" % crap
		print "\t.type %s,@function" % crap
		print "%s:" % crap
		print "\tret"
		

