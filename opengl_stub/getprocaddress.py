# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

import sys,os;
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

print """#include "cr_glwrapper.h"
#include "api_templates.h"
#include "cr_applications.h"
#include "cr_string.h"
#include "cr_error.h"

#include <stdio.h>

#ifdef WINDOWS
#pragma warning( disable: 4055 )
#endif

extern crCreateContextProc crCreateContext;
extern crMakeCurrentProc crMakeCurrent;
extern crSwapBuffersProc crSwapBuffers;

CR_PROC CR_APIENTRY crGetProcAddress( const char *name )
{"""

bonus_functions = [
	"crCreateContext",
	"crMakeCurrent",
	"crSwapBuffers"
]

for func_name in keys:
	if stub_common.FindSpecial( "noexport", func_name ):
		continue
	print '\tif (!crStrcmp( name, "gl%s" )) return (CR_PROC) glim.%s;' % ( func_name, func_name )

for func_name in bonus_functions:
	print '\tif (!crStrcmp( name, "%s" )) return (CR_PROC) %s;' % ( func_name, func_name )

print '\treturn NULL;'
print '}'
