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

keys = gl_mapping.keys()
keys.sort();

stub_common.CopyrightC()

print """
#include <stdio.h>
#include "cr_server.h"
#include "feedbackspu.h"
"""

for func_name in keys:
	(return_type, args, types) = gl_mapping[func_name]
	if stub_common.FindSpecial( "feedback_state", func_name ):
		print '%s FEEDBACKSPU_APIENTRY feedbackspu_%s%s' % ( return_type, func_name, stub_common.ArgumentString( args, types ) )
		print '{'
		print '\tcrState%s%s;' % ( func_name, stub_common.CallString( args ) )
		print ''
		print '\tfeedback_spu.super.%s%s;' % ( func_name, stub_common.CallString( args ) )
			
		print '}'
