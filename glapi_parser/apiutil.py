#!/usr/common/bin/python

# apiutil.py
#
# This file defines a bunch of utility functions for OpenGL API code
# generation.

import sys, string


#======================================================================

def CopyrightC( ):
	print """/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */
	"""

def CopyrightDef( ):
	print """; Copyright (c) 2001, Stanford University
	; All rights reserved.
	;
	; See the file LICENSE.txt for information on redistributing this software.
	"""



#======================================================================

class APIFunction:
	"""Class to represent a GL API function (name, return type,
	parameters, etc)."""
	def __init__(self):
		self.name = ''
		self.returnType = ''
		self.category = ''
		self.offset = -1
		self.alias = ''
		self.vectoralias = ''
		self.params = []
		self.props = []
		self.chromium = []



def ProcessSpecFile(filename, userFunc):
	"""Open the named API spec file and call userFunc(record) for each record
	processed."""
	specFile = open(filename, "r")
	if not specFile:
		print "Error: couldn't open %s file!" % filename
		sys.exit()

	record = APIFunction()

	for line in specFile.readlines():

		# split line into tokens
		tokens = string.split(line)

		if len(tokens) > 0 and line[0] != '#':

			if tokens[0] == 'name':
				if record.name != '':
					# process the function now
					userFunc(record)
					# reset the record
					record = APIFunction()

				record.name = tokens[1]

			elif tokens[0] == 'return':
				record.returnType = string.join(tokens[1:], ' ')
			
			elif tokens[0] == 'param':
				name = tokens[1]
				type = string.join(tokens[2:], ' ')
				vecSize = 0
				record.params.append((name, type, vecSize))

			elif tokens[0] == 'category':
				record.category = tokens[1]

			elif tokens[0] == 'offset':
				if tokens[1] == '?':
					record.offset = -2
				else:
					record.offset = int(tokens[1])

			elif tokens[0] == 'alias':
				record.alias = tokens[1]

			elif tokens[0] == 'vectoralias':
				record.vectoralias = tokens[1]

			elif tokens[0] == 'props':
				record.props = tokens[1:]

			elif tokens[0] == 'chromium':
				record.chromium = tokens[1:]

			elif tokens[0] == 'vector':
				vecName = tokens[1]
				vecSize = int(tokens[2])
				for i in range(len(record.params)):
					(name, type, oldSize) = record.params[i]
					if name == vecName:
						record.params[i] = (name, type, vecSize)
						break

			else:
				print 'Invalid token %s after function %s' % (tokens[0], record.name)
			#endif
		#endif
	#endfor
	specFile.close()
#enddef



def PrintRecord(record):
	argList = MakeCArgList(record.params)
	if record.category == "Chromium":
		prefix = "cr"
	else:
		prefix = "gl"
	print '%s %s%s(%s);' % (record.returnType, prefix, record.name, argList )
	if len(record.props) > 0:
		print '   /* %s */' % string.join(record.props, ' ')

#ProcessSpecFile("APIspec.txt", PrintRecord)


# Dictionary [name] of APIFunction:
__FunctionDict = {}

# Dictionary [name] of name
__VectorVersion = {}

# Reverse mapping of function name aliases
__ReverseAliases = {}


def AddFunction(record):
	assert not __FunctionDict.has_key(record.name)
	if not "omit" in record.chromium:
		__FunctionDict[record.name] = record



def GetFunctionDict(specFile = ""):
	if not specFile:
		specFile = "../glapi_parser/APIspec.txt"
	if len(__FunctionDict) == 0:
		ProcessSpecFile(specFile, AddFunction)
		# Look for vector aliased functions
		for func in __FunctionDict.keys():
			va = __FunctionDict[func].vectoralias
			if va != '':
				__VectorVersion[va] = func
			#endif

			# and look for regular aliases (for glloader)
			a = __FunctionDict[func].alias
			if a:
				__ReverseAliases[a] = func
			#endif
		#endfor
	#endif
	return __FunctionDict


def GetAllFunctions(specFile = ""):
	"""Return sorted list of all functions known to Chromium."""
	d = GetFunctionDict(specFile)
	funcs = []
	for func in d.keys():
		rec = d[func]
		if not "omit" in rec.chromium:
			funcs.append(func)
	funcs.sort()
	return funcs
	

def GetDispatchedFunctions(specFile = ""):
	"""Return sorted list of all functions handled by SPU dispatch table."""
	d = GetFunctionDict(specFile)
	funcs = []
	for func in d.keys():
		rec = d[func]
		if (not "omit" in rec.chromium and
			not "stub" in rec.chromium and
			rec.alias == ''):
			funcs.append(func)
	funcs.sort()
	return funcs


#======================================================================

def ReturnType(funcName):
	"""Return the C return type of named function."""
	d = GetFunctionDict()
	return d[funcName].returnType


def Parameters(funcName):
	"""Return list of tuples (name, type, vecSize) of function parameters."""
	d = GetFunctionDict()
	return d[funcName].params


def Properties(funcName):
	"""Return list of properties of the named GL function."""
	d = GetFunctionDict()
	return d[funcName].props


def Category(funcName):
	"""Return the category of the named GL function."""
	d = GetFunctionDict()
	return d[funcName].category


def ChromiumProps(funcName):
	"""Return list of Chromium-specific properties of the named GL function."""
	d = GetFunctionDict()
	return d[funcName].chromium


def Alias(funcName):
	"""Return the function that the named function is an alias of.
	Ex: Alias('DrawArraysEXT') = 'DrawArrays'.
	"""
	d = GetFunctionDict()
	return d[funcName].alias


def ReverseAlias(funcName):
	"""Like Alias(), but the inverse."""
	d = GetFunctionDict()
	if funcName in __ReverseAliases.keys():
		return __ReverseAliases[funcName]
	else:
		return ''


def VectorAlias(funcName):
	"""Return the non-vector version of the given function, or ''.
	For example: VectorAlias("Color3fv") = "Color3f"."""
	d = GetFunctionDict()
	return d[funcName].vectoralias


def NonVectorFunction(funcName):
	"""Return the non-vector version of the given function, or ''.
	For example: VectorAlias("Color3fv") = "Color3f"."""
	return VectorAlias(funcName)


def VectorFunction(funcName):
	"""Return the vector version of the given non-vector-valued function,
	or ''.
	For example: VectorVersion("Color3f") = "Color3fv"."""
	d = GetFunctionDict()
	if funcName in __VectorVersion.keys():
		return __VectorVersion[funcName]
	else:
		return ''


def GetCategoryWrapper(func_name):
	"""Return a C preprocessor token to test in order to wrap code.
	This handles extensions.
	Example: GetTestWrapper("glActiveTextureARB") = "CR_multitexture"
	Example: GetTestWrapper("glBegin") = ""
	"""
	cat = Category(func_name)
	if (cat == "1.0" or
		cat == "1.1" or
		cat == "1.2" or
		cat == "Chromium" or
		cat == "GL_chromium"):
		return ''
	elif cat[0] =='1':
		# i.e. OpenGL 1.3 or 1.4 or 1.5
		return "OPENGL_VERSION_" + string.replace(cat, ".", "_")
	else:
		assert cat != ''
		return string.replace(cat, "GL_", "")


def CanCompile(funcName):
	"""Return 1 if the function can be compiled into display lists, else 0."""
	d = GetFunctionDict()
	props = Properties(funcName)
	if ("nolist" in props or
		"get" in props or
		"setclient" in props or
		"useclient" in props):
		return 0
	else:
		return 1


def CanPack(funcName):
	"""Return 1 if the function can be packed, else 0."""
	d = GetFunctionDict()
	props = ChromiumProps(funcName)
	if ("pack" in props) or ("extpack" in props):
		return 1
	# check for packable alias
	alias = NonVectorFunction(funcName)
	if alias:
		props = ChromiumProps(alias)
		if ("pack" in props) or ("extpack" in props):
			return 1
	# check for vector version of packing function
	alias = VectorFunction(funcName)
	if alias:
		props = ChromiumProps(alias)
		if ("pack" in props) or ("extpack" in props):
			return 1
	return 0


def IsPointer(dataType):
	"""Determine if the datatype is a pointer.  Return 1 or 0."""
	if string.find(dataType, "*") == -1:
		return 0
	else:
		return 1


def PointerType(pointerType):
	"""Return the type of a pointer.
	Ex: PointerType('const GLubyte *') = 'GLubyte'
	"""
	t = string.split(pointerType, ' ')
	if t[0] == "const":
		t[0] = t[1]
	return t[0]




def OpcodeName(funcName):
	"""Return the C token for the opcode for the given function."""
	return "CR_" + string.upper(funcName) + "_OPCODE"


def ExtendedOpcodeName(funcName):
	"""Return the C token for the extended opcode for the given function."""
	return "CR_" + string.upper(funcName) + "_EXTEND_OPCODE"




#======================================================================

def MakeCallString(params):
	"""Given a list of (name, type, vectorSize) parameters, make a C-style
	formal parameter string.
	Ex return: 'index, x, y, z'.
	"""
	result = ''
	i = 1
	n = len(params)
	for (name, type, vecSize) in params:
		result += name
		if i < n:
			result = result + ', '
		i += 1
	#endfor
	return result
#enddef


def MakeDeclarationString(params):
	"""Given a list of (name, type, vectorSize) parameters, make a C-style
	parameter declaration string.
	Ex return: 'GLuint index, GLfloat x, GLfloat y, GLfloat z'.
	"""
	n = len(params)
	if n == 0:
		return 'void'
	else:
		result = ''
		i = 1
		for (name, type, vecSize) in params:
			result = result + type + ' ' + name
			if i < n:
				result = result + ', '
			i += 1
		#endfor
		return result
	#endif
#enddef


def MakePrototypeString(params):
	"""Given a list of (name, type, vectorSize) parameters, make a C-style
	parameter prototype string (types only).
	Ex return: 'GLuint, GLfloat, GLfloat, GLfloat'.
	"""
	n = len(params)
	if n == 0:
		return 'void'
	else:
		result = ''
		i = 1
		for (name, type, vecSize) in params:
			result = result + type
			# see if we need a comma separator
			if i < n:
				result = result + ', '
			i += 1
		#endfor
		return result
	#endif
#enddef


#======================================================================
	
__lengths = {
	'GLbyte': 1,
	'GLubyte': 1,
	'GLshort': 2,
	'GLushort': 2,
	'GLint': 4,
	'GLuint': 4,
	'GLfloat': 4,
	'GLclampf': 4,
	'GLdouble': 8,
	'GLclampd': 8,
	'GLenum': 4,
	'GLboolean': 1,
	'GLsizei': 4,
	'GLbitfield': 4,
	'void': 0,  # XXX why?
	'int': 4,
	'GLintptrARB': 4,   # XXX or 8 bytes?
	'GLsizeiptrARB': 4  # XXX or 8 bytes?
}

def sizeof(type):
	"""Return size of C datatype, in bytes."""
	if not type in __lengths.keys():
		print >>sys.stderr, "%s not in lengths!" % type
	return __lengths[type]


#======================================================================
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

def PacketLength( params ):
	len = 0
	for (name, type, vecSize) in params:
		if IsPointer(type):
			size = PointerSize()
		else:
			assert string.find(type, "const") == -1
			size = sizeof(type)
		len = FixAlignment( len, size ) + size
	len = WordAlign( len )
	return len

#======================================================================

__specials = {}

def LoadSpecials( filename ):
	table = {}
	try:
		f = open( filename, "r" )
	except:
		__specials[filename] = {}
		print "%s not present" % filename
		return {}
	
	for line in f.readlines():
		line = string.strip(line)
		if line == "" or line[0] == '#':
			continue
		table[line] = 1
	
	__specials[filename] = table
	return table


def FindSpecial( table_file, glName ):
	table = {}
	filename = table_file + "_special"
	try:
		table = __specials[filename]
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
		table = __specials[filename]
	except KeyError:
		table = LoadSpecials( filename )
	
	keys = table.keys()
	keys.sort()
	return keys


def AllSpecials( table_file ):
	filename = table_file + "_special"
	table = {}
	try:
		table = __specials[filename]
	except KeyError:
		table = LoadSpecials(filename)
	
	ret = table.keys()
	ret.sort()
	return ret
	

def NumSpecials( table_file ):
	filename = table_file + "_special"
	table = {}
	try:
		table = __specials[filename]
	except KeyError:
		table = LoadSpecials(filename)
	return len(table.keys())