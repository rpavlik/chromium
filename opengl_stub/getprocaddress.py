import sys,os;
import cPickle;
import types;
import string;
import re;

import stub_common;

parsed_file = open( "../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

keys = gl_mapping.keys()
keys.sort();

print """#include "cr_glwrapper.h"
#include "api_templates.h"
#include "cr_applications.h"
#include "cr_string.h"
#include "cr_error.h"

#include <stdio.h>

#ifdef WINDOWS
#pragma warning( disable: 4055 )
#endif

CR_PROC CR_APIENTRY crGetProcAddress( const char *name )
{"""

for func_name in keys:
	if stub_common.FindSpecial( "noexport", func_name ):
		continue
	print '\tif (!crStrcmp( name, "%s" )) return (CR_PROC) %s;' % ( func_name, stub_common.DoImmediateMapping( func_name ) )

print '\treturn NULL;'
print '}'
