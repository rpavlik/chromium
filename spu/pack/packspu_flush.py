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

print """#include <stdio.h>
#include "packspu.h"
#include "cr_packfunctions.h"
#include "cr_glwrapper.h"
#include "cr_glstate.h"
"""

for func_name in stub_common.AllSpecials( "packspu_flush" ):
	(return_type, args, types) = gl_mapping[func_name]
	print 'void PACKSPU_APIENTRY packspu_%s%s' % ( func_name, stub_common.ArgumentString( args, types ) )
	print '{'
	print '\tif (pack_spu.swap)'
	print '\t{'
	print '\t\tcrPack%sSWAP%s;' % ( func_name, stub_common.CallString( args ) )
	print '\t}'
	print '\telse'
	print '\t{'
	print '\t\tcrPack%s%s;' % ( func_name, stub_common.CallString( args ) )
	print '\t}'
	print '\tpackspuFlush( NULL );'
	print '}\n'
