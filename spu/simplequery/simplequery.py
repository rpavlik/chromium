# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.


import sys,os;
import cPickle;
import string;
import re;

sys.path.append( "../../opengl_stub" )
parsed_file = open( "../../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

import stub_common;

stub_common.CopyrightC()

print """#include <stdio.h>
#include "cr_spu.h"
#include "cr_glstate.h"
#include "state/cr_stateerror.h"
#include "simplequeryspu.h"

static void SIMPLEQUERYSPU_APIENTRY
simplequeryChromiumParameteriCR( GLenum target, GLint value )
{
	crStateError(__LINE__,__FILE__,GL_INVALID_ENUM,"This is not a simple query");
}

static void SIMPLEQUERYSPU_APIENTRY
simplequeryChromiumParameterfCR( GLenum target, GLfloat value )
{
	crStateError(__LINE__,__FILE__,GL_INVALID_ENUM,"This is not a simple query");
}

static void SIMPLEQUERYSPU_APIENTRY
simplequeryChromiumParametervCR( GLenum target, GLenum type, GLsizei count, const GLvoid *values )
{
	crStateError(__LINE__,__FILE__,GL_INVALID_ENUM,"This is not a simple query");
}

static void SIMPLEQUERYSPU_APIENTRY
simplequeryGetChromiumParametervCR( GLenum target, GLenum type, GLsizei count, const GLvoid *values )
{
	crStateError(__LINE__,__FILE__,GL_INVALID_ENUM,"This is not a simple query");
}

"""

keys = gl_mapping.keys()
keys.sort();

print 'SPUNamedFunctionTable _cr_simplequery_table[] = {'
for index in range(len(keys)):
	func_name = keys[index]
	if stub_common.FindSpecial( "simplequery", func_name ):
		print '\t{ "%s", (SPUGenericFunction) crState%s },' % (func_name, func_name )
print """	{ "ChromiumParameteriCR", (SPUGenericFunction) simplequeryChromiumParameteriCR },
	{ "ChromiumParameterfCR", (SPUGenericFunction) simplequeryChromiumParameterfCR },
	{ "ChromiumParametervCR", (SPUGenericFunction) simplequeryChromiumParametervCR },
	{ "GetChromiumParametervCR", (SPUGenericFunction) simplequeryGetChromiumParametervCR },
	{ NULL, NULL }
};"""
