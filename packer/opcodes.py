
import sys;
import cPickle;
import string;
import re;

sys.path.append( "../opengl_stub" )

import stub_common;

parsed_file = open( "../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

print ""
print "#ifndef CR_OPCODES_H"
print "#define CR_OPCODES_H"
print ""

keys = gl_mapping.keys()
keys.sort();

print "typedef enum {"

enum_index = 0
for index in range(0,len(keys)):
	func_name = keys[index]
	if not stub_common.FindSpecial( "opcode", func_name ):
		print "\t%s = %d," % ( stub_common.OpcodeName( func_name ), enum_index )
		enum_index = enum_index + 1
print "\tCR_EXTEND_OPCODE=%d" % enum_index
print "} CROpcode;\n"

num_extends = 0
for key in keys:
	if stub_common.FindSpecial( "packer_get", key ):
		num_extends += 1
	

print "typedef enum {"

enum_index = 0
for index in range(0,len(keys)):
	func_name = keys[index]
	if stub_common.FindSpecial( "packer_get", func_name ):
		if enum_index == num_extends-1:
			print "\t%s = %d" % ( stub_common.ExtendedOpcodeName( func_name ), enum_index )
		else:
			print "\t%s = %d," % ( stub_common.ExtendedOpcodeName( func_name ), enum_index )
		enum_index = enum_index + 1
print "} CRExtendOpcode;\n"
print "#endif /* CR_OPCODES_H */"
