# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

import sys, re, string
sys.path.append( "../../opengl_stub" )
import stub_common

line_re = re.compile(r'^(\S+)\s+(GL_\S+)\s+(.*)\s*$')
input = open( "state_isenabled.txt", 'r' )

params = {}

for line in input.readlines():
	match = line_re.match( line )
	if match:
		type = match.group(1)
		pname = match.group(2)
		fields = string.split( match.group(3) )
		params[pname] = ( type, fields )

stub_common.CopyrightC()

print """
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "cr_glstate.h"
#include "state/cr_statetypes.h"

GLboolean STATE_APIENTRY crStateIsEnabled( GLenum pname )
{
    CRContext   *g = GetCurrentContext( );

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glGet called in Begin/End");
		return 0;
	}

    switch ( pname ) {
"""

keys = params.keys()
keys.sort();

for pname in keys:
	print "\tcase %s:" % pname
	print "\t\treturn %s;" % params[pname][1][0]
print "\tdefault:"
print "\t\tcrStateError(__LINE__, __FILE__, GL_INVALID_ENUM, \"glIsEnabled: Unknown enum: %d\", pname);"
print "\t\treturn 0;"
print "\t}"
print "}"
