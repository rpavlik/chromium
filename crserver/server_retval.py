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

#include <memory.h>

void crServerReturnValue( const void *payload, unsigned int payload_len )
{
	CRMessageReadback *rb;
	int msg_len = sizeof( *rb ) + payload_len;
	rb = (CRMessageReadback *) crAlloc( msg_len );

	rb->type = CR_MESSAGE_READBACK;
	memcpy( &(rb->writeback_ptr), &(cr_server.writeback_ptr), sizeof( rb->writeback_ptr ) );
	memcpy( &(rb->readback_ptr), &(cr_server.return_ptr), sizeof( rb->readback_ptr ) );
	memcpy( rb+1, payload, payload_len );
	crNetSend( cr_server.curClient->conn, NULL, rb, msg_len );
	crFree( rb );
}
"""

keys = gl_mapping.keys()
keys.sort();

for func_name in keys:
	(return_type, names, types) = gl_mapping[func_name]
	if stub_common.FindSpecial( "server", func_name ): continue
	if return_type != 'void':
		print '%s SERVER_DISPATCH_APIENTRY crServerDispatch%s%s' % ( return_type, func_name, stub_common.ArgumentString( names, types ))
		print '{'
		print '\t%s retval;' % return_type
		print '\tretval = cr_server.head_spu->dispatch_table.%s%s;' % (func_name, stub_common.CallString( names ) );
		print '\tcrServerReturnValue( &retval, sizeof(retval) );'
		print '\treturn retval; /* WILL PROBABLY BE IGNORED */'
		print '}'
