# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

import sys,os;
import cPickle;
import string;
import re;

sys.path.append( "../opengl_stub" )
parsed_file = open( "../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

import stub_common;

stub_common.CopyrightC()

print """#include "cr_spu.h"
#include "cr_glwrapper.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_net.h"
#include "server_dispatch.h"
#include "server.h"
"""

from get_sizes import *;


funcs = [ 'GetIntegerv', 'GetFloatv', 'GetDoublev', 'GetBooleanv' ]
types = [ 'GLint', 'GLfloat', 'GLdouble', 'GLboolean' ]

for index in range(len(funcs)):
	func_name = funcs[index]
	(return_type, arg_names, arg_types) = gl_mapping[func_name]
	print 'void SERVER_DISPATCH_APIENTRY crServerDispatch%s%s' % ( func_name, stub_common.ArgumentString( arg_names, arg_types ))
	print '{'
	print '\t%s get_values[16]; // Be safe' % types[index]
	print '\tint num_values;'
	print '\t(void) params;'
	print '\tcr_server.head_spu->dispatch_table.%s( pname, get_values );' % func_name
	print '\tnum_values = __numValues( pname);'
	print '\tcrServerReturnValue( get_values, num_values*sizeof(get_values[0]) );'
	print '}\n'
