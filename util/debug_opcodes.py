# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

import sys;
import cPickle;
import string;
import re;

sys.path.append( "../opengl_stub" )

import stub_common;

parsed_file = open( "../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

stub_common.CopyrightC()

print """
#include "cr_debugopcodes.h"
#include <stdio.h>
"""

print """void crDebugOpcodes( FILE *fp, unsigned char *ptr, unsigned int num_opcodes )
{
\tunsigned int i;
\tfor (i = 0 ; i < num_opcodes ; i++)
\t{
\t\tswitch(*(ptr--))
\t\t{
"""

keys = gl_mapping.keys()
keys.sort();

for func_name in keys:
	if not stub_common.FindSpecial( "../packer/opcode", func_name ) and not stub_common.FindSpecial( "../packer/opcode_extend", func_name ):
		print '\t\tcase %s:' % stub_common.OpcodeName( func_name )
		print '\t\t\tfprintf( fp, "%s\\n" ); ' % stub_common.OpcodeName( func_name )
		print '\t\t\tbreak;'

print """
\t\t}
\t}
}
"""
