
import sys;
import cPickle;
import types;
import string;
import re;

sys.path.append( "../opengl_stub" )

import stub_common;

parsed_file = open( "../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

print """
#include "cr_opengl_types.h"
#include "cr_pack.h"
#include "cr_opcodes.h"
#include "cr_packfunctions.h"

#define PACK_UNUSED(x) ((void)(x))

CRPackGlobals __pack_globals;

char *__cr_opcode_names[] = {
"""

keys = gl_mapping.keys()
keys.sort()

for func_name in keys:
	if (stub_common.FindSpecial( "opcode", func_name ) ): continue
	print '    "' + func_name + '",'
print "};"
print ""

def SmackVector( func_name ):
		return re.sub( "v$", "", func_name )

def VectorLength( func_name ):
	m = re.search( r"([0-9])", func_name )
	return string.atoi( m.group(1) )

def WriteData( offset, arg_type, arg_name ):
	if string.find( arg_type, '*' ) != -1:
		retval = "\tWRITE_NETWORK_POINTER( %d, (void *) %s );" % (offset, arg_name )
	elif arg_type == "GLdouble" or arg_type == "GLclampd":
		retval = "\tWRITE_DOUBLE( %d, %s );" % (offset, arg_name)
	else:	
		retval = "\tWRITE_DATA( %d, %s, %s );" % (offset, arg_type, arg_name)
	return retval

def UpdateCurrentPointer( func_name ):
	m = re.search( r"^(Color|Normal|TexCoord)([1234])(ub|b|us|s|ui|i|f|d)$", func_name )
	if m :
		name = string.lower( m.group(1) )
		type = m.group(3) + m.group(2)
		print "\t__pack_globals.current.%s.%s = data_ptr;" % (name,type)
		return

	m = re.match( r"^(Index)(ub|b|us|s|ui|i|f|d)$", func_name )
	if m :
		name = string.lower( m.group(1) )
		type = m.group(2) + "1"
		print "\t__pack_globals.current.%s.%s = data_ptr;" % (name,type)
		return

	m = re.match( r"^(EdgeFlag)$", func_name )
	if m :
		name = string.lower( m.group(1) )
		type = "l1"
		print "\t__pack_globals.current.%s.%s = data_ptr;" % (name,type)
		return

	m = re.match( r"^(Begin)$", func_name )
	if m :
		name = string.lower( m.group(1) )
		type = ""
		print "\t__pack_globals.current.%s_data = data_ptr;" % name
		print "\t__pack_globals.current.%s_op = __pack_globals.buffer.opcode_current;" % name
		return

def IsVector ( func_name ) :
	m = re.search( r"^(Color|EdgeFlag|EvalCoord|Index|Normal|TexCoord|Vertex|RasterPos)([1234]?)(ub|b|us|s|ui|i|f|d|)v$", func_name )
	if m :
		if m.group(2) :
			return string.atoi( m.group(2) )
		else:
			return 1
	else:
		return 0

for func_name in keys:
	( return_type, arg_names, arg_types ) = gl_mapping[func_name]
	if stub_common.FindSpecial( "packer", func_name ): continue
	if stub_common.FindSpecial( "packer_get", func_name ):
		for type in arg_types:
			if string.find( type, '*' ) != -1:
				break;
		else:
			arg_types.append( "%s *" % return_type )
			arg_names.append( "return_value" )
	print 'void PACK_APIENTRY ' + stub_common.PackFunction( func_name ),
	print stub_common.ArgumentString( arg_names, arg_types )
	print '{'
	orig_func_name = func_name[:] #make copy
	vector_arg_type = ""
	vector_nelem = IsVector( func_name )
	if vector_nelem :
		func_name = SmackVector( func_name )
		vector_arg_type = re.sub( r"\*", "", arg_types[0] )
		vector_arg_type = re.sub( "const ", "", vector_arg_type )
		vector_arg_type = string.strip( vector_arg_type )
		packet_length = stub_common.WordAlign( vector_nelem * stub_common.lengths[vector_arg_type] )
	else:
		packet_length = stub_common.PacketLength( arg_types )

	if packet_length == -1:
		print '\tcrError ( "%s needs to be special cased!");' % orig_func_name
		for arg in arg_names:
			print "\tPACK_UNUSED( %s );" % arg
	else:
		print "\tunsigned char *data_ptr;"
		if packet_length == 0:
			print "\tGET_BUFFERED_POINTER_NO_ARGS( );"
		else:
			if stub_common.FindSpecial( "packer_get", func_name ):
				packet_length += 8
			print "\tGET_BUFFERED_POINTER( %d );" % packet_length

		UpdateCurrentPointer( func_name )

		counter = 0
		if stub_common.FindSpecial( "packer_get", func_name ):
			counter = 8
			print WriteData( 0, 'int', packet_length )
			print WriteData( 4, 'GLenum', stub_common.ExtendedOpcodeName( func_name ) )
		if vector_nelem :
			for index in range( 0, vector_nelem ):
				print WriteData( index*stub_common.lengths[vector_arg_type], vector_arg_type, arg_names[-1] + ("[%d]" %index) )
		else:
			for index in range(0,len(arg_names)):
				if arg_names[index] != '':
					print WriteData( counter, arg_types[index], arg_names[index] )
					if string.find( arg_types[index], '*' ) != -1:
						counter += stub_common.PointerSize()
					else:
						counter += stub_common.lengths[arg_types[index]]
		if stub_common.FindSpecial( "packer_get", func_name ):
			print "\tWRITE_OPCODE( CR_EXTEND_OPCODE );"
		else:
			print "\tWRITE_OPCODE( %s );" % stub_common.OpcodeName( func_name )
	print '}\n'
