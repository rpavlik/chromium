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
#include "server_dispatch.h"
#include "server.h"
#include "cr_unpack.h"

CRCurrentStatePointers crServerCurrent;
"""

for func_name in stub_common.AllSpecials( "../spu/state_tracker/state" ):
	(return_type, names, types) = gl_mapping[func_name]
	if stub_common.FindSpecial( "server", func_name ) or stub_common.FindSpecial( "../packer/packer_get", func_name ) or return_type != 'void':
		continue
	print 'void SERVER_DISPATCH_APIENTRY crServerDispatch%s%s' % ( func_name, stub_common.ArgumentString( names, types ) )
	print '{'
	print '\tcrState%s%s;' % (func_name, stub_common.CallString( names ) )
	print '\tcr_server.head_spu->dispatch_table.%s%s;' % (func_name, stub_common.CallString( names ) )
	print '}'

keys = gl_mapping.keys()
keys.sort()

for func_name in keys:
	current = 0
	array = ""
	m = re.search( r"^(Color|Normal)([1234])(ub|b|us|s|ui|i|f|d)$", func_name )
	if m :
		current = 1
		name = string.lower( m.group(1)[:1] ) + m.group(1)[1:]
		type = m.group(3) + m.group(2)
	m = re.search( r"^(TexCoord)([1234])(ub|b|us|s|ui|i|f|d)$", func_name )
	if m :
		current = 1
		name = string.lower( m.group(1)[:1] ) + m.group(1)[1:]
		type = m.group(3) + m.group(2)
		array = "[0]"
	m = re.search( r"^(MultiTexCoord)([1234])(ub|b|us|s|ui|i|f|d)ARB$", func_name )
	if m :
		current = 1
		name = "texCoord"
		type = m.group(3) + m.group(2)
		array = "[texture-GL_TEXTURE0_ARB]"
	m = re.match( r"^(Index)(ub|b|us|s|ui|i|f|d)$", func_name )
	if m :
		current = 1
		name = string.lower( m.group(1)[:1] ) + m.group(1)[1:]
		type = m.group(2) + "1"
	m = re.match( r"^(EdgeFlag)$", func_name )
	if m :
		current = 1
		name = string.lower( m.group(1)[:1] ) + m.group(1)[1:]
		type = "l1"

	if current:
		(return_type, names, types) = gl_mapping[func_name]
		print 'void SERVER_DISPATCH_APIENTRY crServerDispatch%s%s' % ( func_name, stub_common.ArgumentString( names, types ) )
		print '{'
		print '\tcr_server.head_spu->dispatch_table.%s%s;' % (func_name, stub_common.CallString( names ) )
		print "\tcr_server.current.%s.%s%s = cr_unpackData;" % (name,type,array)
		print '}\n'	

print """
void crServerInitDispatch(void)
{
	crSPUInitDispatchTable( &(cr_server.dispatch) );
	crSPUCopyDispatchTable( &(cr_server.dispatch), &(cr_server.head_spu->dispatch_table ) );
"""

for func_name in keys:
	(return_type, names, types) = gl_mapping[func_name]
	if return_type != 'void' or stub_common.FindSpecial( "../packer/packer_get", func_name ) or stub_common.FindSpecial( "server", func_name ) or stub_common.FindSpecial( "../spu/state_tracker/state", func_name ):
		print '\tcr_server.dispatch.%s = crServerDispatch%s;' % (func_name, func_name)
print '}'
