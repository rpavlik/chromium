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
/* DO NOT EDIT - this file generated by replicatespu_flush.py script */

/* These are otherwise ordinary functions which require that the buffer be
 * flushed immediately after packing the function.
 */
#include "cr_glstate.h"
#include "cr_packfunctions.h"
#include "replicatespu.h"
#include "replicatespu_proto.h"
"""

for func_name in stub_common.AllSpecials( "replicatespu_flush" ):
	(return_type, args, types) = gl_mapping[func_name]
	print 'void REPLICATESPU_APIENTRY replicatespu_%s%s' % ( func_name, stub_common.ArgumentString( args, types ) )
	print '{'
	print '\tGET_THREAD(thread);'
	print '\tif (replicate_spu.swap)'
	print '\t{'
	print '\t\tcrPack%sSWAP%s;' % ( func_name, stub_common.CallString( args ) )
	print '\t}'
	print '\telse'
	print '\t{'
	print '\t\tcrPack%s%s;' % ( func_name, stub_common.CallString( args ) )
	print '\t}'
	print '\treplicatespuFlush( (void *) thread );'
	print '}\n'