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
#include "hiddenlinespu.h"
#include "cr_packfunctions.h"
#include "cr_glwrapper.h"
#include "cr_glstate.h"

void HIDDENLINESPU_APIENTRY hiddenlinespu_PixelStoref( GLenum pname, GLfloat param )
{
	crStatePixelStoref( pname, param );
	crPackPixelStoref( pname, param );
}

void HIDDENLINESPU_APIENTRY hiddenlinespu_PixelStorei( GLenum pname, GLint param )
{
	crStatePixelStorei( pname, param );
	crPackPixelStorei( pname, param );
}
"""

for func_name in stub_common.AllSpecials( "hiddenline_pixel" ):
	(return_type, args, types) = gl_mapping[func_name]
	print 'void HIDDENLINESPU_APIENTRY hiddenlinespu_%s%s' % ( func_name, stub_common.ArgumentString( args, types ) )
	print '{'
	args.append( '&(hiddenline_spu.ctx->client.unpack)' )
	print '\tcrPack%s%s;' % ( func_name, stub_common.CallString( args ) )
	print '}'
