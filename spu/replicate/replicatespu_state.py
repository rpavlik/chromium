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
#include "cr_packfunctions.h"
#include "replicatespu.h"
"""

for func_name in keys:
	(return_type, args, types) = gl_mapping[func_name]
	if (stub_common.FindSpecial( "replicatespu_state", func_name ) or stub_common.FindSpecial( "replicatespu_get", func_name)):
		print 'extern %s REPLICATESPU_APIENTRY replicatespu_%s%s;' % ( return_type, func_name, stub_common.ArgumentString( args, types ) )

for func_name in keys:
	(return_type, args, types) = gl_mapping[func_name]
	if (stub_common.FindSpecial( "replicatespu_state", func_name ) or stub_common.FindSpecial( "replicatespu_get", func_name)):
		print '%s REPLICATESPU_APIENTRY replicatespu_%s%s' % ( return_type, func_name, stub_common.ArgumentString( args, types ) )
		print '{'
		if stub_common.FindSpecial( "replicatespu_state", func_name ):
			print '\tif (replicate_spu.swap)'
			print '\t{'
			print '\t\tcrPack%sSWAP%s;' % (func_name, stub_common.CallString( args ) )
			print '\t}'
			print '\telse'
			print '\t{'
			print '\t\tcrPack%s%s;' % (func_name, stub_common.CallString( args ) )
			print '\t}'
		print ''
		if stub_common.FindSpecial( "replicatespu_get", func_name ):
			print '\treturn crState%s%s;' % ( func_name, stub_common.CallString( args ) )
		else:
			print '\tcrState%s%s;' % ( func_name, stub_common.CallString( args ) )
		print '}'
