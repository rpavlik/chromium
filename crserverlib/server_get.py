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

print """
#include "cr_spu.h"
#include "chromium.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_net.h"
#include "server_dispatch.h"
#include "server.h"
"""

keys = gl_mapping.keys()
keys.sort()

max_components = {
	'GetClipPlane': 4,
	'GetCombinerStageParameterfvNV': 4,
	'GetCombinerStageParameterivNV': 4,
	'GetCombinerOutputParameterfvNV': 4,
	'GetCombinerOutputParameterivNV': 4,
	'GetCombinerInputParameterfvNV': 4,
	'GetCombinerInputParameterivNV': 4,
	'GetFinalCombinerInputParameterfvNV': 4,
	'GetFinalCombinerInputParameterivNV': 4,
	'GetLightfv': 4,
	'GetLightiv': 4,
	'GetMaterialfv': 4, 
	'GetMaterialiv': 4, 
	'GetPolygonStipple': 32*32/8,
	'GetTexEnvfv': 4,
	'GetTexEnviv': 4,
	'GetTexGendv': 4,
	'GetTexGenfv': 4,
	'GetTexGeniv': 4,
	'GetTexLevelParameterfv': 1,
	'GetTexLevelParameteriv': 1,
	'GetTexParameterfv': 4,
	'GetTexParameteriv': 4,
	'GetProgramParameterdvNV': 4,
	'GetProgramParameterfvNV': 4,
	'GetProgramivNV': 1,
	'GetTrackMatrixivNV': 1,
	'GetVertexAttribPointervNV': 1,
	'GetVertexAttribdvNV': 4,
	'GetVertexAttribfvNV': 4,
	'GetVertexAttribivNV': 4,
	'GetFenceivNV': 1,
}

no_pnames = [ 'GetClipPlane', 'GetPolygonStipple' ];

from get_components import *;

for func_name in keys:
	(return_type, arg_names, arg_types) = gl_mapping[func_name]
	if stub_common.FindSpecial( "../packer/packer_get", func_name ) and not stub_common.FindSpecial( "server", func_name ):
		print 'void SERVER_DISPATCH_APIENTRY crServerDispatch%s%s' % (func_name, stub_common.ArgumentString( arg_names, arg_types ) )
		print '{'
		local_argtype = string.replace( arg_types[len(arg_types)-1], '*', '' )
		local_argname = 'local_%s' % arg_names[len(arg_names)-1]
		print '\t%s %s[%d];' % ( local_argtype, local_argname, max_components[func_name] )
		print '\t(void) %s;' % arg_names[len(arg_names)-1]
		arg_names[len(arg_names)-1] = local_argname
		print '\tcr_server.head_spu->dispatch_table.%s%s;' % ( func_name, stub_common.CallString(arg_names) )
		if func_name in no_pnames:
			print '\tcrServerReturnValue( &(%s[0]), %d*sizeof(%s) );' % (local_argname, max_components[func_name], local_argtype );
		else:
			print '\tcrServerReturnValue( &(%s[0]), __lookupComponents(pname)*sizeof(%s) );' % (local_argname, local_argtype );
		print '}\n'
