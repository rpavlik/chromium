# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

import sys

sys.path.append( "../../glapi_parser" )
import apiutil


apiutil.CopyrightC()

print """
#include "hiddenlinespu.h"
#include "hiddenlinespu_proto.h"
#include "cr_packfunctions.h"
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

# Need this since we're down two levels of subdirs
apiutil.GetAllFunctions("../../glapi_parser/APIspec.txt")

for func_name in apiutil.AllSpecials( "hiddenline_pixel" ):
	params = apiutil.Parameters(func_name)
	print 'void HIDDENLINESPU_APIENTRY hiddenlinespu_%s(%s)' % (func_name, apiutil.MakeDeclarationString(params))
	print '{'
	print '\tGET_CONTEXT(context);'
	args = apiutil.MakeCallString(params) + ', &(context->ctx->client.unpack)'
	print '\tcrPack%s(%s);' % (func_name, args)
	print '}'
