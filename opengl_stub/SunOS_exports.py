# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

import sys;
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

print """#include <stdio.h>
#include <stdlib.h>

#define SUN_OGL_NO_VERTEX_MACROS
#include <GL/gl.h>

#include "api_templates.h"

"""

for func_name in keys:
	if stub_common.FindSpecial( "noexport", func_name ):
		continue
	( return_type, arg_names, arg_types ) = gl_mapping[func_name]

	if func_name == "TexImage3D":
		# Ugly hack - Sun's gl.h has wrong type for internalFormat
		print "void glTexImage3D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels )"
		print "{"
		print "\tglim.TexImage3D( target, level, (GLint) internalformat, width, height, depth, border, format, type, pixels );"
		print "}"
	else:
		print "%s gl%s%s" % (return_type, func_name, stub_common.ArgumentString( arg_names, arg_types ) )
		print "{"
		print "\t",
		if return_type != "void":
			print "return ",
		print "glim.%s%s;" % (func_name, stub_common.CallString( arg_names ))
		print "}"
		print ""

	real_func_name = alias_exports.AliasMap(func_name);
	if real_func_name:
		print "%s gl%s%s" % (return_type, real_func_name, stub_common.ArgumentString( arg_names, arg_types ) )
		print "{"
		print "\t",
		if return_type != "void":
			print "return ",
		print "glim.%s%s;" % (func_name, stub_common.CallString( arg_names ))
		print "}"
		print ""
