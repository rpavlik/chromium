import sys,os;
import cPickle;
import string;
import re;

sys.path.append( "../opengl_stub" )
parsed_file = open( "../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

import stub_common;

keys = gl_mapping.keys()
keys.sort();

# This regular expression is used to extract pointer sizes from function names
extractNumber = re.compile('[0-9]+')

# A routine that can create call strings from instance names
def InstanceCallString( arg_names ):
	output = ''
	for index in range(0,len(arg_names)):
		if index > 0:
			output += ", "
		if arg_names[index] != '':
			output += 'instance->' + arg_names[index]
	return output

# This function looks for a class of annotations (e.g. "pointer=special")
# and returns the desired section (in this case, "special").
def GetAnnotationClass(functionName, prefix):
	annotations = stub_common.GetAnnotations("dlm_functions", functionName)
	for annotation in annotations:
		if annotation[:len(prefix) + 1] == prefix + "=":
			return annotation[len(prefix) + 1:]
	return None

def GetPointerType(basetype):
	words = basetype.split()
	if words[0] == 'const':
		words = words[1:]
	if words[-1].endswith('*'):
		words[-1] = words[-1][:-1].strip()
		if words[-1] == '':
			words = words[:-1]
	if words[0] == 'void' or words[0] == 'GLvoid':
		words[0] = 'int'
	return ' '.join(words)

# These subroutines do the real work of creating the wrapper
# functions.  The passthrough wrapper just passes the same parameters
# down to the next layer.
def wrap_passthrough(functionName):
	(return_type, args, types) = gl_mapping[func_name]
	callstring = stub_common.InternalCallString(args)
	print 'static %s DLM_APIENTRY pass%s%s' % (return_type, func_name, stub_common.ArgumentString(args, types))
	print '{'
	print '	CRDLMContextState *state = CURRENT_STATE();'
	# If this function affects client-side state that will affect the
	# display list (including pixel storage modes), we need to know about
	# it first.  Its wrapper should call our own function first.
	if stub_common.FindAnnotation("dlm_functions", func_name, "setclient"):
		if callstring:
			print '	crdlm%s(%s, state->clientState);' % (func_name, callstring);
		else:
			print '	crdlm%s(state->clientState);' % (func_name);
	if return_type != "void":
		print '	return state->savedDispatchTable.%s(%s);' % (func_name, callstring)
	else:
		print '	state->savedDispatchTable.%s(%s);' % (func_name, callstring)
	print '}'
	return

def GetPointerInfo(functionName, args, types):
	# We'll keep track of all the parameters that require pointers.
	# They'll require special handling later.
	pointers = []
	pointername=''
	pointerarg=''
	pointertype=''
	pointersize=0
	pointercomment=''
	for index in range(len(args)):
		# Watch out for the word "const" (which should be ignored)
		# and for types that end in "*" (which are pointers and need
		# special treatment)
		words = types[index].split()
		if words[-1].endswith('*'):
			pointers.append(index)
	# If any argument was a pointer, we need a special pointer data
	# array.  The pointer data will be stored into this array, and
	# references to the array will be generated as parameters.
	if len(pointers) == 1:
		pointername = args[pointers[0]]
		pointerarg = pointername + 'Data'
		pointertype = GetPointerType(types[pointers[0]])
		pointersize = GetAnnotationClass(functionName, "pointer")
		if pointersize == None:
			# Try to intuit a pointer size.  If an integer exists in the
			# function name, that is a good guess for the pointer size.
			guess = extractNumber.search(functionName)
			if guess:
				pointersize=guess.group()
				pointercomment = ' /* Guessed pointer size %s from name */' % pointersize
	elif len(pointers) > 1:
		pointerarg = 'data';
		pointertype = GetPointerType(types[pointers[0]])
		for index in range(1,len(pointers)):
			if GetPointerType(types[pointers[index]]) != pointertype:
				pointertype = 'GLvoid *'

	return (pointers,pointername,pointerarg,pointertype,pointersize,pointercomment)

def wrap_header(functionName):
	(return_type, args, types) = gl_mapping[functionName]
	argstring = stub_common.InternalArgumentString(args, types)

	# We'll keep track of all the parameters that require pointers.
	# They'll require special handling later.
	(pointers, pointername, pointerarg, pointertype, pointersize, pointercomment) = GetPointerInfo(functionName, args, types)

	# Start writing the header
	print 'struct instance%s {' % (functionName)
	print '	DLMInstanceList *next;'
	print '	void (*execute)(DLMInstanceList *instance, SPUDispatchTable *dispatchTable);'
	for index in range(len(args)):
		# Watch out for the word "const" (which should be ignored)
		# and for types that end in "*" (which are pointers and need
		# special treatment)
		words = types[index].split()
		if words[0] == 'const':
			words = words[1:]
		if words[0] != "void":
			print '	%s %s;' % (' '.join(words), args[index])

	# If any argument was a pointer, we need a special pointer data
	# array.  The pointer data will be stored into this array, and
	# references to the array will be generated as parameters.
	if len(pointers) == 1:
		if pointersize == None:
			print "	/* Oh no - pointer parameter %s found, but no pointer class specified and can't guess */" % pointername
		else:
			if pointersize == 'special':
				print '	%s %s[0];%s' % (pointertype, pointerarg, pointercomment)
			else:
				print '	%s %s[%s];%s' % (pointertype, pointerarg, pointersize,pointercomment)
	elif len(pointers) > 1:
		print '	%s %s[0];%s' % (pointertype, pointerarg,pointercomment)

	print '};'

	# Pointers only happen with instances
	if len(pointers) > 1 or (len(pointers) == 1 and pointersize == 'special'):
		print 'int crdlm_pointers_%s(struct instance%s *instance, %s);' % (functionName, functionName, argstring)
		
	# See if the GL function must sometimes allow passthrough even
	# if the display list is open
	if stub_common.FindAnnotation("dlm_functions", func_name, "checkpass"):
		print 'int crdlm_checkpass_%s(%s);' % (functionName, argstring)

	return

def wrap_compile_header(functionName):
	(return_type, args, types) = gl_mapping[functionName]
	argstring = stub_common.InternalArgumentString(args, types)
	print 'void crdlm_compile_%s(%s);' % (functionName, argstring)


def generate_bbox_code(functionName):
	assert functionName[0:6] == "Vertex"
	(return_type, args, types) = gl_mapping[functionName]
	pattern = "(VertexAttribs|VertexAttrib|Vertex)(1|2|3|4)(N?)(f|d|i|s|b|ub|us|ui)(v?)"
	m = re.match(pattern, functionName)
	if m:
		name = m.group(1)
		size = int(m.group(2))
		normalize = m.group(3)
		type = m.group(4)
		vector = m.group(5)

		# only update bbox for vertex attribs if index == 0
		if name == "VertexAttrib":
			test = "if (index == 0) {"
		elif name == "VertexAttribs":
			test = "if (index == 0) {"
		else:
			assert name == "Vertex"
			test = "{"

		# find names of the X, Y, Z, W values
		xName = ""
		yName = ""
		zName = "0.0"
		wName = ""
		if vector == "v":
			xName = "v[0]"
			if size > 1:
				yName = "v[1]"
			if size > 2:
				zName = "v[2]"
			if size > 3:
				wName = "v[3]"
		else:
			xName = "x"
			if size > 1:
				yName = "y"
			if size > 2:
				zName = "z"
			if size > 3:
				wName = "w"

		# start emitting code
		print '\t%s' % test

		if normalize == "N":
			if type == "b":
				denom = "128.0"
			elif type == "s":
				denom = "32768.0"
			elif type == "i":
				denom = "2147483647.0"
			elif type == "ub":
				denom = "255.0"
			elif type == "us":
				denom = "65535.0"
			elif type == "ui":
				denom = "4294967295.0"
			
			print '\t\tGLfloat nx = (GLfloat) %s / %s;' % (xName, denom)
			xName = "nx"
			if yName:
				print '\t\tGLfloat ny = (GLfloat) %s / %s;' % (yName, denom)
				yName = "ny"
			if zName:
				print '\t\tGLfloat nz = (GLfloat) %s / %s;' % (zName, denom)
				zName = "nz"
			if 0 and wName:
				print '\t\tGLfloat nw = (GLfloat) %s / %s;' % (wName, denom)
				wName = "nw"

		if xName:
			print '\t\tif (%s < state->currentListInfo->bbox.xmin)' % xName
			print '\t\t\tstate->currentListInfo->bbox.xmin = %s;' % xName
			print '\t\tif (%s > state->currentListInfo->bbox.xmax)' % xName
			print '\t\t\tstate->currentListInfo->bbox.xmax = %s;' % xName
		if yName:
			print '\t\tif (%s < state->currentListInfo->bbox.ymin)' % yName
			print '\t\t\tstate->currentListInfo->bbox.ymin = %s;' % yName
			print '\t\tif (%s > state->currentListInfo->bbox.ymax)' % yName
			print '\t\t\tstate->currentListInfo->bbox.ymax = %s;' % yName
		if zName:
			print '\t\tif (%s < state->currentListInfo->bbox.zmin)' % zName
			print '\t\t\tstate->currentListInfo->bbox.zmin = %s;' % zName
			print '\t\tif (%s > state->currentListInfo->bbox.zmax)' % zName
			print '\t\t\tstate->currentListInfo->bbox.zmax = %s;' % zName
		# XXX what about divide by W if we have 4 components?
		print '\t}'
			
	else:
		print ' /* bbox error for %s !!!!! */' % functionName


# The compile wrapper collects the parameters into a DLMInstanceList
# element, and adds that element to the end of the display list currently
# being compiled.
def wrap_compile(functionName):
	(return_type, args, types) = gl_mapping[functionName]
	callstring = stub_common.InternalCallString(args)
	# Make sure the return type is void.  It's nonsensical to compile
	# an element with any other return type.
	if return_type != 'void':
		print '/* Nonsense: DL function %s has a %s return type?!? */' % (functionName, return_type)
		return
	# Define a structure to hold all the parameters.  Note that the
	# top parameters must exactly match the DLMInstanceList structure
	# in include/cr_dlm.h, or everything will break horribly.
	# Start off by getting all the pointer info we could ever use
	# from the parameters
	(pointers, pointername, pointerarg, pointertype, pointersize, pointercomment) = GetPointerInfo(functionName, args, types)

	# Next must come the execute wrapper, as it is is referenced by the
	# compile wrapper (to install the self-execute function). 
	executefunc = 'execute%s' % (functionName)
	print 'static void DLM_APIENTRY execute%s(DLMInstanceList *x, SPUDispatchTable *dispatchTable)' % (functionName)
	print '{'
	# Don't need the instance pointer if there's no parameters
	if args[0] != '':
		print '	struct instance%s *instance = (struct instance%s *)x;' % (functionName, functionName)
	print '	dispatchTable->%s(%s);' % (functionName, InstanceCallString(args))
	print '}'

	# Finally, the compile wrapper.  This one will diverge strongly
	# depending on whether or not there are pointer parameters. 
	# It might also generate a passthrough on occasion.
	print 'void DLM_APIENTRY crdlm_compile_%s%s' % (functionName, stub_common.ArgumentString(args, types))
	print '{'
	print '	CRDLMContextState *state = CURRENT_STATE();'
	print '	struct instance%s *instance;' % (functionName)

	# If the function requires the opportunity to force a passthrough call
	# (based on its parameters), allow it
	if stub_common.FindAnnotation("dlm_functions", functionName, "checkpass"):
		print '	if (crdlm_checkpass_%s(%s)) {' % (functionName, callstring)
		print '		pass%s(%s);' % (functionName, callstring)
		print '		return;'
		print '	}'

	if len(pointers) > 1 or pointersize == 'special':
		# Pass NULL, to just allocate space
		print '	instance = crAlloc(sizeof(struct instance%s) + crdlm_pointers_%s(NULL, %s));' % (functionName, functionName, callstring)
	else:
		print '	instance = crAlloc(sizeof(struct instance%s));' % (functionName)
	print '	if (!instance) {'
	print '		crdlm_error(__LINE__, __FILE__, GL_OUT_OF_MEMORY,'
	print '			"out of memory adding %s to display list");' % (functionName)
	print '		return;'
	print '	}'
	# Put in the fields that must always exist
	print '	instance->next = NULL;'
	print '	instance->execute = %s;' % (executefunc)

	# Apply all the simple (i.e. non-pointer) parameters
	for index in range(len(args)):
		if index not in pointers and args[index] != '':
			print '	instance->%s = %s;' % (args[index], args[index])

	# If there's a pointer parameter, apply it.
	if len(pointers) == 1:
		print '	if (%s == NULL) {' % (args[pointers[0]])
		print '		instance->%s = NULL;' % (args[pointers[0]])
		print '	}'
		print '	else {'
		print '		instance->%s = instance->%s;' % (args[pointers[0]], pointerarg)
		print '	}'
		if pointersize == 'special':
			print '	(void) crdlm_pointers_%s(instance, %s);' % (functionName, callstring)
		else:
			print '	crMemcpy((void *)instance->%s, (void *) %s, %s*sizeof(%s));' % (args[pointers[0]], args[pointers[0]], pointersize, pointertype)
	elif len(pointers) == 2:
		# this seems to work
		print '	(void) crdlm_pointers_%s(instance, %s);' % (functionName, callstring)
	elif len(pointers) > 2:
		print ' /*** WARNING broken code here ***/'

	# Add the element to the current display list
	print '	if (!state->currentListInfo->first) {'
	print '		state->currentListInfo->first = (DLMInstanceList *)instance;'
	print '	}'
	print '	else {'
	print '		state->currentListInfo->last->next = (DLMInstanceList *)instance;'
	print '	}'
	print '	state->currentListInfo->last = (DLMInstanceList *)instance;'

	# XXX might need a better test here
	if functionName[0:6] == "Vertex":
		generate_bbox_code(functionName)

	print '}'

def wrap_compileAndExecute(functionName):
	return
	(return_type, args, types) = gl_mapping[functionName]
	callstring = stub_common.InternalCallString(args)
	# Add a compile-and-execute function
	print 'static %s DLM_APIENTRY compileAndExecute%s%s' % (return_type, functionName, stub_common.ArgumentString(args, types))
	print '{'
	print '	CRDLMContextState *state = CURRENT_STATE();'
	print '	crdlm_compile_%s(%s);' % (functionName, callstring)
	print '	state->savedDispatchTable.%s(%s);' % (functionName, callstring)
	print '}'
	return



# Choose our output based on which file we're trying to generate.
whichfile=sys.argv[1]
if whichfile == 'headers':
	print """#ifndef _DLM_DISPATCH_H

/* DO NOT EDIT.  This file is auto-generated by dlm_dispatch.py. */
"""

	for func_name in keys:
		print "\n/*** %s ***/" % func_name
		annotations = stub_common.GetAnnotations("dlm_functions", func_name)
		if "dl" in annotations:
			# Auto-generate an appropriate DL function.  First, functions
			# that go into the display list but that rely on state will
			# have to have their argument strings expanded, to take pointers
			# to that appropriate state.
			wrap_header(func_name)
			wrap_compile_header(func_name)
		elif "dlcompile" in annotations:
			wrap_compile_header(func_name)
		elif ("setclient" in annotations or
			  "get" in annotations or
			  "nodl" in annotations):
			# emit nothing
			pass
		else:
			print '/* %s - what do I do with this?  Edit dlm_functions. */'
			assert 0


	print """#endif"""
else:
	print """#include <stdio.h>
#include "cr_spu.h"
#include "cr_dlm.h"
#include "cr_mem.h"
#include "cr_error.h"
#include "dlm.h"
#include "dlm_dispatch.h"
#include "dlm_client.h"

/* DO NOT EDIT.  This file is auto-generated by dlm_dispatch.py. */
"""
	# If it's listed as "local": the function enters the dispatch table, but
	# is not auto-generated.  An implementation of the function with the
	# proper parameters and the name "crdlm_<functionName>" must exist elsewhere;
	# this function will be used for all purposes in the dispatch table.
	#
	# If it's listed as "dl" (but not "local"), we create several distinct
	# wrappers for it, one each to compile the function call (into a crAlloc'ed
	# memory glob that we can later interpret), to compile and execute the
	# call, to pass the call to the next layer, and to replay a previously-
	# created memory glob.
	#
	# Any functions not listed, or not listed with "dl", are not
	# display-list functions; only a wrapper containing their passthrough
	# function will be created and used.

	print """/* Following are all the auto-generated functions. */
	"""
	for func_name in keys:
		print "\n/*** %s ***/" % func_name
		annotations = stub_common.GetAnnotations("dlm_functions", func_name)
		if "local" in annotations:
  			# Not auto-generated
  			pass
		elif "dl" in annotations:
  			# Auto-generate an appropriate DL function.  First, functions
  			# that go into the display list but that rely on state will
  			# have to have their argument strings expanded, to take pointers
  			# to that appropriate state.
  			wrap_passthrough(func_name)
  			wrap_compile(func_name)
  			wrap_compileAndExecute(func_name)
  		elif "dlcompile" in annotations:
  			# Generate and use a passthrough, but we'll need a crdlm_compile_*
  			# function for all other uses.
  			wrap_passthrough(func_name)
  			wrap_compileAndExecute(func_name)
  		else:
  			# All others just pass through
  			wrap_passthrough(func_name)

	# All done with all the wrappers (at least the ones we're going to make).
	# Time now for the dispatch tables.
#  	print ''
#  	print '/********** Dispatch tables following ****************/'
#  	print ''
#  	for table in ['pass', 'compile', 'compileAndExecute']:
#  		print ''
#  		print 'SPUDispatchTable dlm_dispatch_%s = {' % (table)
#  		for func_name in keys:
#  			if stub_common.FindAnnotation("dlm_functions", func_name, "local"):
#  				# local reference
#  				print '	crdlm_%s,' % (func_name)
#  			elif stub_common.FindAnnotation("dlm_functions", func_name, "dl"):
#  				# autogenerated display list function
#  				if table == 'compile':
#  					print '	crdlm_compile_%s,' % (func_name)
#  				else:
#  					print '	%s%s,' % (table, func_name)
#  			elif stub_common.FindAnnotation("dlm_functions", func_name, "dlcompile"):
#  				if table == 'pass':
#  					print '	pass%s,' % (func_name)
#  				elif table == 'compile':
#  					print '	crdlm_compile_%s,' % (func_name)
#  				else:
#  					print '	%s%s,' % (table, func_name)
#  			else:
#  				# passthrough
#  				print '	pass%s,' % (func_name)

#  		print "};"



	print '/********** Dispatch tables following ****************/'
	print ''
	print ''
	print 'void crdlm_setup_compile_dispatcher(SPUDispatchTable *t)'
	print "{"
	for func_name in keys:
		annotations = stub_common.GetAnnotations("dlm_functions", func_name)
		if "local" in annotations:
			# local reference
			print '\tcrSPUChangeInterface(t, (void *) t->%s, crdlm_%s);' % (func_name, func_name)
		elif "dl" in annotations:
			# autogenerated display list function
			print '\tcrSPUChangeInterface(t, (void *) t->%s, crdlm_compile_%s);' % (func_name, func_name)
		elif "dlcompile" in annotations:
			print '\tcrSPUChangeInterface(t, (void *) t->%s, crdlm_compile_%s);' % (func_name, func_name)
		elif ("get" in annotations or
			  "nodl" in annotations or
			  "setclient" in annotations):
			# passthrough
			print '/*	pass%s,*/' % (func_name)
		else:
			print '/* %s - what do I do with this?  Edit dlm_functions. */'
			assert 0

	print "}"

	print ''
	print ''
	print 'void crdlm_restore_dispatcher(SPUDispatchTable *t, const SPUDispatchTable *original)'
	print "{"
	for func_name in keys:
		print '\tif (t->%s != original->%s)' % (func_name, func_name)
		print '\t\tcrSPUChangeInterface(t, t->%s, original->%s);' % (func_name, func_name)
	print "}"


	# to prevent warnings:

	print ''
	print ''
	print 'void *crdlm_silence_warnings[] = {'
	for func_name in keys:
		print '\t(void *) pass%s,' % func_name
	print '\tNULL'
	print '};'
	
