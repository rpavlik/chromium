
import sys,os;
import cPickle;
import string;
import re;

sys.path.append( "../../opengl_stub" )
parsed_file = open( "../../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

import stub_common;

print """DESCRIPTION ""
EXPORTS
"""

keys = gl_mapping.keys()
keys.sort();

for func_name in stub_common.AllSpecials( 'state' ):
	print "crState%s" % func_name

print """crStateInit
crStateCreateContext
crStateMakeCurrent
crStateFlushFunc
crStateDiffAPI
"""
