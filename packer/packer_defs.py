# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

# This script generates the packer/packer.def file.

import sys,os;
import cPickle;
import string;
sys.path.append( "../opengl_stub" )
import stub_common;

parsed_file = open( "../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

stub_common.CopyrightDef()

print "DESCRIPTION \"\""
print "EXPORTS"

keys = gl_mapping.keys()
keys.sort();
for func_name in keys:
    print "crPack" + func_name

print """
crPackVertex2dBBOX
crPackVertex2dvBBOX
crPackVertex2fBBOX
crPackVertex2fvBBOX
crPackVertex2iBBOX
crPackVertex2ivBBOX
crPackVertex2sBBOX
crPackVertex2svBBOX
crPackVertex3dBBOX
crPackVertex3dvBBOX
crPackVertex3fBBOX
crPackVertex3fvBBOX
crPackVertex3iBBOX
crPackVertex3ivBBOX
crPackVertex3sBBOX
crPackVertex3svBBOX
crPackVertex4dBBOX
crPackVertex4dvBBOX
crPackVertex4fBBOX
crPackVertex4fvBBOX
crPackVertex4iBBOX
crPackVertex4ivBBOX
crPackVertex4sBBOX
crPackVertex4svBBOX
crPackVertex2dBBOX_COUNT
crPackVertex2dvBBOX_COUNT
crPackVertex2fBBOX_COUNT
crPackVertex2fvBBOX_COUNT
crPackVertex2iBBOX_COUNT
crPackVertex2ivBBOX_COUNT
crPackVertex2sBBOX_COUNT
crPackVertex2svBBOX_COUNT
crPackVertex3dBBOX_COUNT
crPackVertex3dvBBOX_COUNT
crPackVertex3fBBOX_COUNT
crPackVertex3fvBBOX_COUNT
crPackVertex3iBBOX_COUNT
crPackVertex3ivBBOX_COUNT
crPackVertex3sBBOX_COUNT
crPackVertex3svBBOX_COUNT
crPackVertex4dBBOX_COUNT
crPackVertex4dvBBOX_COUNT
crPackVertex4fBBOX_COUNT
crPackVertex4fvBBOX_COUNT
crPackVertex4iBBOX_COUNT
crPackVertex4ivBBOX_COUNT
crPackVertex4sBBOX_COUNT
crPackVertex4svBBOX_COUNT
crPackInitBuffer
crPackResetPointers
crPackAppendBuffer
crPackAppendBoundedBuffer
crPackSetBuffer
crPackGetBuffer
crPackFlushFunc
crPackFlushArg
crPackSendHugeFunc
crPackBoundsInfo
crPackResetBBOX
crPackOffsetCurrentPointers
crPackNullCurrentPointers
crPackInit
"""
