# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

#!/usr/common/bin/python

# This script reads the GL.H and CR_GL.H header files and produces
# the gl_header.parsed file.  gl_header.parsed describes all the
# OpenGL functions and is used by several other Python scripts.

import sys, os, re, string, cPickle

system_gl = open("system_header/GL.H", 'r')
chromium_gl = open("system_header/CR_GL.H", 'r')
output_mapping_file = open("gl_header.parsed", 'wb')
output_mapping = {}

for line in system_gl.readlines() + chromium_gl.readlines():
	return_end = string.find( line, "gl" )
	if return_end == -1:
		continue
	return_type = line[:return_end-1]

	func_start = string.find( line, "gl" );
	func_end = string.find( line, " (", func_start )

	func_name = line[func_start+2:func_end] # strip off the "gl"

	args_start = func_end + 2
	args_end = string.find( line, ")", args_start )

	args = string.split( line[args_start:args_end], "," )
	arg_types = [];
	arg_names = [];
	for arg in args:
		arg_name_start = string.rfind( arg, "*" );
		if arg_name_start == -1:
			arg_name_start = string.rfind( arg, " ");
			if arg_name_start == -1:
				arg_name_start = len(arg)
		arg_names = arg_names + [string.strip(arg[arg_name_start+1:])]
		arg_types = arg_types + [string.strip(arg[:arg_name_start+1])]

	output_mapping[func_name] = ( return_type, arg_names, arg_types )

output_mapping['CreateContext'] = ( 'GLint', ['dpyName', 'visual'], ['const char *', 'GLint'] )
output_mapping['DestroyContext'] = ( 'void', ['ctx'], ['GLint'] )
output_mapping['MakeCurrent'] = ( 'void', ['window', 'nativeWindow', 'ctx'], ['GLint', 'GLint', 'GLint'] )
output_mapping['WindowCreate'] = ( 'GLint', ['dpyName', 'visBits'], ['const char *', 'GLint'] )
output_mapping['WindowDestroy'] = ( 'void', ['window'], ['GLint'] )
output_mapping['WindowSize'] = ( 'void', ['window', 'x', 'y'], ['GLint', 'GLint', 'GLint'] )
output_mapping['WindowPosition'] = ( 'void', ['window', 'w', 'h'], ['GLint', 'GLint', 'GLint'] )
output_mapping['WindowShow'] = ( 'void', ['window', 'flag'], ['GLint', 'GLint'] )
output_mapping['SwapBuffers'] = ( 'void', ['window', 'flags'], ['GLint', 'GLint'] )
output_mapping['Writeback'] = ( 'void', ['writeback'], ['GLint *'] )

cPickle.dump( output_mapping, output_mapping_file, 1 )
