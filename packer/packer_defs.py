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

print "crPackResetPointers"
print "crPackSetBuffer"
print "crPackGetBuffer"
print "crPackFlushFunc"
print "crPackSendHugeFunc"
