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

print """#include <stdio.h>
#include "packspu.h"
#include "cr_packfunctions.h"
#include "cr_glwrapper.h"
#include "cr_glstate.h"

void packspu_PixelStoref( GLenum pname, GLfloat param )
{
	crStatePixelStoref( pname, param );
	crPackPixelStoref( pname, param );
}

void packspu_PixelStorei( GLenum pname, GLint param )
{
	crStatePixelStorei( pname, param );
	crPackPixelStorei( pname, param );
}
"""

for func_name in stub_common.AllSpecials( "packspu_pixel" ):
	(return_type, args, types) = gl_mapping[func_name]
	print 'void packspu_%s%s' % ( func_name, stub_common.ArgumentString( args, types ) )
	print '{'
	args.append( '&(GetCurrentContext()->pixel.unpack)' )
	print '\tcrPack%s%s;' % ( func_name, stub_common.CallString( args ) )
	print '}'
