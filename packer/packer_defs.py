import sys,os;
import cPickle;
import string;

parsed_file = open( "../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

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
"""
