import string;
import re;

def DecoderName( glName ):
	return "crUnpack" + glName

def OpcodeName( glName ):
	return "CR_" + string.upper( glName ) + "_OPCODE"

def ExtendedOpcodeName( glName ):
	return "CR_" + string.upper( glName ) + "_EXTEND_OPCODE"

def PackFunction( glName ):
	return "crPack" + glName

def DoPackFunctionMapping( glName ):
	return "__glpack_" + glName

def DoStateFunctionMapping( glName ):
	return "__glstate_" + glName

def DoImmediateMapping( glName ):
	return "__glim_" + glName

specials = {}

def LoadSpecials( filename ):
	table = {}
	try:
		f = open( filename, "r" )
	except:
		specials[filename] = {}
		return {}
	
	for line in f.readlines():
		table[string.strip(line)] = 1
	
	specials[filename] = table
	return table

def FindSpecial( table_file, glName ):
	table = {}
	filename = table_file + "_special"
	try:
		table = specials[filename]
	except KeyError:
		table = LoadSpecials( filename )
	
	try:
		if (table[glName] == 1):
			return 1
		else:
			return 0 #should never happen
	except KeyError:
		return 0

def AllSpecials( table_file ):
	table = {}
	filename = table_file + "_special"
	try:
		table = specials[filename]
	except KeyError:
		table = LoadSpecials( filename )
	
	keys = table.keys()
	keys.sort()
	return keys

def AllSpecials( table_file ):
	filename = table_file + "_special"

	table = {}
	try:
		table = specials[filename]
	except KeyError:
		table = LoadSpecials(filename)
	
	ret = table.keys()
	ret.sort()
	return ret
	
def NumSpecials( table_file ):
	filename = table_file + "_special"

	table = {}
	try:
		table = specials[filename]
	except KeyError:
		table = LoadSpecials(filename)

	return len(table.keys())

lengths = {}
lengths['GLbyte'] = 1
lengths['GLubyte'] = 1
lengths['GLshort'] = 2
lengths['GLushort'] = 2
lengths['GLint'] = 4
lengths['GLuint'] = 4
lengths['GLfloat'] = 4
lengths['GLclampf'] = 4
lengths['GLdouble'] = 8
lengths['GLclampd'] = 8

lengths['GLenum'] = 4
lengths['GLboolean'] = 1
lengths['GLsizei'] = 4
lengths['GLbitfield'] = 4

lengths['void'] = 0

align_types = 1

def FixAlignment( pos, alignment ):
	# if we want double-alignment take word-alignment instead,
	# yes, this is super-lame, but we know what we are doing
	if alignment > 4:
		alignment = 4
	if align_types and alignment and ( pos % alignment ):
		pos += alignment - ( pos % alignment )
	return pos

def WordAlign( pos ):
	return FixAlignment( pos, 4 )

def PointerSize():
	return 8 # Leave room for a 64 bit pointer

def PacketLength( arg_types ):
	len = 0
	for arg in arg_types:
		if string.find( arg, '*') != -1:
			size = PointerSize()
		else:
			temp_arg = re.sub("const ", "", arg)
			size = lengths[temp_arg]
		len = FixAlignment( len, size ) + size
	len = WordAlign( len )
	return len

def ArgumentString( arg_names, arg_types ):
	output = '( '
	for index in range(0,len(arg_names)):
		if len(arg_names) != 1 and arg_names[index] == '':
			continue
		output += arg_types[index]
		if arg_types[index][-1:] != '*' and arg_names[index] != '':
			output += " ";
		output += arg_names[index]
		if index != len(arg_names) - 1:
			output += ", "
	output += " )"
	return output
	
def CallString( arg_names ):
	output = '( '
	for index in range(0,len(arg_names)):
		output += arg_names[index]
		if arg_names[index] != '' and index != len(arg_names) - 1:
			output += ", "
	output += " )"
	return output
