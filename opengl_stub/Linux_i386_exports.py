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

aliases = [ ('ActiveTexture','ActiveTextureARB'),
			('ClientActiveTexture','ClientActiveTextureARB'),
			('MultiTexCoord1d', 'MultiTexCoord1dARB'),
			('MultiTexCoord1dv','MultiTexCoord1dvARB'),
			('MultiTexCoord1f', 'MultiTexCoord1fARB'),
			('MultiTexCoord1fv','MultiTexCoord1fvARB'),
			('MultiTexCoord1i', 'MultiTexCoord1iARB'),
			('MultiTexCoord1iv','MultiTexCoord1ivARB'),
			('MultiTexCoord1s', 'MultiTexCoord1sARB'),
			('MultiTexCoord1sv','MultiTexCoord1svARB'),
			('MultiTexCoord2d', 'MultiTexCoord2dARB'),
			('MultiTexCoord2dv','MultiTexCoord2dvARB'),
			('MultiTexCoord2f', 'MultiTexCoord2fARB'),
			('MultiTexCoord2fv','MultiTexCoord2fvARB'),
			('MultiTexCoord2i', 'MultiTexCoord2iARB'),
			('MultiTexCoord2iv','MultiTexCoord2ivARB'),
			('MultiTexCoord2s', 'MultiTexCoord2sARB'),
			('MultiTexCoord2sv','MultiTexCoord2svARB'),
			('MultiTexCoord3d', 'MultiTexCoord3dARB'),
			('MultiTexCoord3dv','MultiTexCoord3dvARB'),
			('MultiTexCoord3f', 'MultiTexCoord3fARB'),
			('MultiTexCoord3fv','MultiTexCoord3fvARB'),
			('MultiTexCoord3i', 'MultiTexCoord3iARB'),
			('MultiTexCoord3iv','MultiTexCoord3ivARB'),
			('MultiTexCoord3s', 'MultiTexCoord3sARB'),
			('MultiTexCoord3sv','MultiTexCoord3svARB'),
			('MultiTexCoord4d', 'MultiTexCoord4dARB'),
			('MultiTexCoord4dv','MultiTexCoord4dvARB'),
			('MultiTexCoord4f', 'MultiTexCoord4fARB'),
			('MultiTexCoord4fv','MultiTexCoord4fvARB'),
			('MultiTexCoord4i', 'MultiTexCoord4iARB'),
			('MultiTexCoord4iv','MultiTexCoord4ivARB'),
			('MultiTexCoord4s', 'MultiTexCoord4sARB'),
			('MultiTexCoord4sv','MultiTexCoord4svARB') ]

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
	for aliased_index in range(len(aliases)):
		(aliased_func_name, real_func_name) = aliases[aliased_index]
		if real_func_name == func_name:
			print "\t.align 4"
			print ".globl gl%s" % aliased_func_name
			print "\t.type gl%s,@function" % aliased_func_name
			print "gl%s:" % aliased_func_name
			print "\tmovl glim+%d, %%eax" % (4*index)
			print "\tjmp *%eax"
			print ""

# Deal with NVIDIA driver lossage.  These functions will never be called, but
# the symbols need to exist.  Fooey.

nvidia_symbols = 0

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
		

