#!/usr/bin/python

# Copyright (c) 2004, Red Hat.
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

# This script generates the packertest*.c files from the APIspec.txt file.


import sys, string, re
import copy

sys.path.append( "../../glapi_parser" )
import apiutil

"""
Various ways of permuting a list of lists (a.k.a. Cartesian product).
The permutation is returned as a list of tuples.

For example:

    permute( [ [1,2], [3,4,5] ] ) gives
       [(1,3), (2,3), (1,4), (2,4), (1,5), (2,5)]
"""
#
# global flags
#

omit = 0
nopack = 0
allfuncs = 0
stub = 0
verbose = 0
debug = 0

def permute(Lists):
  import operator
  if Lists:
    result = map(lambda I: (I,), Lists[0])

    for list in Lists[1:]:
      curr = []
      for item in list:
        new = map(operator.add, result, [(item,)]*len(result))
        curr[len(curr):] = new
      result = curr
  else:
    result = []

  return result

# This version was written by Tim Peters and is somewhat faster,
# especially for large numbers of small lists.
def permute2(seqs):
    n = len(seqs)
    if n == 0:
        return []
    if n == 1:
        return map(lambda i: (i,), seqs[0])
    # find good splitting point
    prods = []
    prod = 1
    for x in seqs:
        prod = prod * len(x)
        prods.append(prod)
    for i in range(n):
        if prods[i] ** 2 >= prod:
            break
    n = min(i + 1, n - 1)
    a = permute2(seqs[:n])
    b = permute2(seqs[n:])
    sprayb = []
    lena = len(a)
    for x in b:
        sprayb[len(sprayb):] = [x] * lena
    import operator
    return map(operator.add, a * len(b), sprayb)

#
# Debug funcs
#
def PrintRecord(record):
	argList = apiutil.MakeDeclarationString(record.params)
	if record.category == "Chromium":
		prefix = "cr"
	else:
		prefix = "gl"
	print '%s %s%s(%s);' % (record.returnType, prefix, record.name, argList )
	if len(record.props) > 0:
		print '   /* %s */' % string.join(record.props, ' ')

def PrintGet(record):
	argList = apiutil.MakeDeclarationString(record.params)
	if record.category == "Chromium":
		prefix = "cr"
	else:
		prefix = "gl"
	if 'get' in record.props:
		print '%s %s%s(%s);' % (record.returnType, prefix, record.name, argList )

def PrintSetClient(record):
	argList = apiutil.MakeDeclarationString(record.params)
	if record.category == "Chromium":
		prefix = "cr"
	else:
		prefix = "gl"
	if 'setclient' in record.props:
		print '%s %s%s(%s);' % (record.returnType, prefix, record.name, argList )

def PrintEnum(record):
	paramList = apiutil.MakeDeclarationString(record.params)
	if record.category == "Chromium":
		prefix = "cr"
	else:
		prefix = "gl"
	for (name, type, vecSize) in record.params:
		if type == "GLenum" :
			for i in range(len(record.paramprop)):
				(name,enums) = record.paramprop[i]
				print 'name = %s' % name
				print 'enums = %s' % enums
				#evec = string.split(enums,' ')
				#for j in range(len(evec)):
					#print 'evec%d = %s' % (j, evec[j])


def DumpTest():
	apiutil.ProcessSpecFile("../../glapi_parser/APIspec.txt", PrintRecord)
	apiutil.ProcessSpecFile("../../glapi_parser/APIspec.txt", PrintGet)
	apiutil.ProcessSpecFile("../../glapi_parser/APIspec.txt", PrintSetClient)
	apiutil.ProcessSpecFile("../../glapi_parser/APIspec.txt", PrintEnum)

#======================================================================

def CopyrightC(f ):
	f.write( """/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */
	""")

def CopyrightDef(f):
	f.write( """; Copyright (c) 2001, Stanford University
	; All rights reserved.
	;
	; See the file LICENSE.txt for information on redistributing this software.
	""")

#======================================================================

printf_mapping = {
        'GLint':      ('%d','int'),
        'GLshort':    ('%hd','short'),
        'GLbyte':     ('%d','int'),
        'GLubyte':    ('%u','unsigned'),
        'GLuint':     ('%u','unsigned'),
        'GLushort':   ('%hu','unsigned short'),
        'GLenum':     ('%s','' ),
        'GLfloat':    ('%f','float'),
        'GLclampf':   ('%f','float'),
        'GLdouble':   ('%f','float'),
        'GLclampd':   ('%f','float'),
        'GLbitfield': ('0x%x','int'),
        'GLboolean':  ('%s',''),
        'GLsizei':    ('%u','unsigned'),
        'GLsizeiptrARB':    ('%u','unsigned'),
        'GLintptrARB':    ('%u','unsigned')
}

# currently not used
printf_pointer_mapping = {
        'GLint *':      ('%d','int'),
        'GLshort *':    ('%hd','short'),
        'GLbyte *':     ('%d','int'),
        'GLubyte *':    ('%u','unsigned'),
        'GLuint *':     ('%u','unsigned'),
        'GLushort *':   ('%hu','unsigned short'),
        'GLenum *':     ('%s','' ),
        'GLfloat *':    ('%f','float'),
        'GLclampf *':   ('%f','float'),
        'GLdouble *':   ('%f','float'),
        'GLclampd *':   ('%f','float'),
        'GLbitfield *': ('0x%x','int'),
        'GLboolean *':  ('%s',''),
        'GLsizeiptrARB':    ('%u','unsigned'),
        'GLintptrARB':    ('%u','unsigned'),
        'GLsizei *':    ('%u','unsigned')
}

#
# Not used but will work if renamed range_mapping
#
limit_mapping = {
	'GLuint': (['0', '4294967294u']),
	'GLsizei': ([0,  65535]),
	'GLfloat': ([-3.40282347e+38, 3.40282347e+38]),
	'GLbyte': ([-128, 127]),
	'GLvoid': ([0]),
	'GLubyte': ([0, 255]),
	'GLdouble': ([-1.7976931348623157e+308, 1.7976931348623157e+308]),
	'GLshort': ([-32768, 32767]),
	'GLint': ([-2147483647, 2147483646]),
	'GLbitfield': ([0, 0xffffffff]),
	'GLushort': ([0, 65535]),
	'GLclampf': ([-3.40282347e+38, 3.40282347e+38]),
	'GLclampd': ([-1.7976931348623157e+308, 1.7976931348623157e+308]),
        'GLsizeiptrARB': ([0]),
        'GLintptrARB': ([0]),
	'GLboolean': ['GL_FALSE', 'GL_TRUE']
}

#
# Not used but will work if renamed range_mapping
#
range_mapping1 = {
	'GLuint': (['0', '1024']),
	'GLsizei': ([0,  1024]),
	'GLfloat': ([-3.40282347e+2, 3.40282347e+2]),
	'GLbyte': ([-20, 100]),
	'GLvoid': ([0]),
	'GLubyte': ([0, 64]),
	'GLdouble': ([-1.7976931348623157e+3, 1.7976931348623157e+3]),
	'GLshort': ([-250, 230]),
	'GLint': ([-2147, 500]),
	'GLbitfield': ([0, 0xffffffff]),
	'GLushort': ([0, 400]),
	'GLclampf': ([-3.40282347e+3, 3.40282347e+8]),
	'GLclampd': ([-1.7976931348623157e+3, 1.7976931348623157e+3]),
        'GLsizeiptrARB': ([0]),
        'GLintptrARB': ([0]),
	'GLboolean': ['GL_FALSE', 'GL_TRUE']
}

range_mapping = {
	'GLuint': ([3]),
	'GLsizei': ([10]),
	'GLfloat': ([3.40]),
	'GLbyte': ([2]),
	'GLvoid': ([0]),
	'GLubyte': ([14]),
	'GLdouble': ([10.79]),
	'GLshort': ([2]),
	'GLint': ([1]),
	'GLbitfield': ([0xffffff]),
	'GLushort': ([5]),
	'GLclampf': ([ 245.66]),
	'GLclampd': ([1234.33]),
        'GLsizeiptrARB': ([0]),
        'GLintptrARB': ([0]),
	'GLboolean': ['GL_FALSE', 'GL_TRUE']
}

#
# Special names that trigger a separate file since the
# output for these GL calls are huge
# CombinerOutputNV generates a massive file
# TexImage3DEXT isn't used much anymore
#
special_keys = [
	'BlendFuncSeparateEXT',
	'ColorTable',
	'ColorTableEXT',
	'CombinerInputNV',
	#'CombinerOutputNV',
	'TexSubImage1D',
	'TexSubImage2D',
	'TexSubImage3D',
	'TexImage1D',
	'TexImage2D',
	#'TexImage3DEXT',
	'TexImage3D'
]


#
# special casing
#
#	'CreateContext',
#	'DestroyContext',
#	'MakeCurrent',
#	'WindowDestroy',
#	'WindowPosition',
#	'WindowShow',
#	'WindowSize',
#	'Writeback',
#	'WindowCreate',
#	'SwapBuffers',
special_funcs = [
	'Begin',
	'End',
	'BoundsInfoCR',
	'BarrierCreateCR',
	'BarrierDestroyCR',
	'BarrierExecCR',
	'SemaphoreCreateCR',
	'SemaphoreDestroyCR',
	'SemaphorePCR',
	'SemaphoreVCR',
	'AreTexturesResident',
	'CallLists',
	'EndList',
	'DeleteTextures',
	'PointParameterfvARB',
	'PointParameteriv',
	'PrioritizeTextures',
	'PushAttrib',
	'PopAttrib',
	'AreProgramsResidentNV',
	'DeleteProgramsARB',
	'DeleteProgramsNV',
	'ExecuteProgramNV',
	'GenProgramsARB',
	'GenProgramsNV',
	'GetProgramEnvParameterdvARB',
	'GetProgramEnvParameterfvARB',
	'GetProgramivARB',
	'GetProgramivNV',
	'GetProgramLocalParameterdvARB',
	'GetProgramLocalParameterfvARB',
	'GetProgramNamedParameterdvNV',
	'GetProgramNamedParameterfvNV',
	'GetProgramParameterdvNV',
	'GetProgramParameterfvNV',
	'GetProgramStringARB',
	'GetProgramStringNV',
	'LoadProgramNV',
	'ProgramEnvParameter4dARB',
	'ProgramEnvParameter4dvARB',
	'ProgramEnvParameter4fARB',
	'ProgramEnvParameter4fvARB',
	'ProgramLocalParameter4dARB',
	'ProgramLocalParameter4dvARB',
	'ProgramLocalParameter4fARB',
	'ProgramLocalParameter4fvARB',
	'ProgramNamedParameter4dNV',
	'ProgramNamedParameter4dvNV',
	'ProgramNamedParameter4fNV',
	'ProgramNamedParameter4fvNV',
	'ProgramParameter4dNV',
	'ProgramParameter4dvNV',
	'ProgramParameter4fNV',
	'ProgramParameter4fvNV',
	'ProgramParameters4dvNV',
	'ProgramParameters4fvNV',
	'ProgramStringARB',
	'RequestResidentProgramsNV',
	'DeleteQueriesARB',
	'GenQueriesARB',
	'BufferSubDataARB',
	'GetBufferSubDataARB',
	'BufferDataARB',
	'GenBuffersARB',
	'DeleteBuffersARB',
	'GenFencesNV',
	'IsFenceNV',
	'TestFenceNV',
	'GetFenceivNV',
	'DeleteFencesNV',
	'GetVertexAttribPointervNV',
	'CompressedTexImage1DARB',
	'CompressedTexImage2DARB',
	'CompressedTexImage3DARB',
	'CompressedTexSubImage1DARB',
	'CompressedTexSubImage2DARB',
	'CompressedTexSubImage3DARB',
	'GetCompressedTexImageARB',
	#'TexParameterfv',
	#'TexParameteriv',
	'GetVertexAttribPointervARB',
	'ReadPixels',
	'ChromiumParametervCR',
	'GetChromiumParametervCR',
	#'GetTexImage'
]
#

def enableTex(f):
	f.write( """

void enableTex(void)
{
    glEnable(GL_TEXTURE_1D);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_3D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MAP1_COLOR_4);
    glEnable(GL_MAP1_INDEX);
    glEnable(GL_MAP1_NORMAL);
    glEnable(GL_MAP1_TEXTURE_COORD_1);
    glEnable(GL_MAP1_TEXTURE_COORD_2);
    glEnable(GL_MAP1_TEXTURE_COORD_3);
    glEnable(GL_MAP1_TEXTURE_COORD_4);
    glEnable(GL_MAP1_VERTEX_3);
    glEnable(GL_MAP1_VERTEX_4);
    glEnable(GL_MAP2_COLOR_4);
    glEnable(GL_MAP2_INDEX);
    glEnable(GL_MAP2_NORMAL);
    glEnable(GL_MAP2_TEXTURE_COORD_1);
    glEnable(GL_MAP2_TEXTURE_COORD_2);
    glEnable(GL_MAP2_TEXTURE_COORD_3);
    glEnable(GL_MAP2_TEXTURE_COORD_4);
    glEnable(GL_MAP2_VERTEX_3);
    glEnable(GL_MAP2_VERTEX_4);
    glFrontFace(GL_CCW);
    glActiveTextureARB(GL_TEXTURE0_ARB);
}

	""")

def makeStripeImage(f):
	f.write( """

void makeStripeImage( GLubyte *stripeImage)
{  
   int j;
 
   for (j = 0; j < 32; j++) {
      stripeImage[4*j] = (GLubyte) ((j<=4) ? 255 : 0);
      stripeImage[4*j+1] = (GLubyte) ((j>4) ? 255 : 0);
      stripeImage[4*j+2] = (GLubyte) 0;
      stripeImage[4*j+3] = (GLubyte) 255;
   }
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glPixelStorei(GL_PACK_ALIGNMENT, 1);
   glEnable(GL_TEXTURE_1D);
   glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 8, 0, GL_RGB, GL_UNSIGNED_BYTE, (const GLvoid *)stripeImage);
   glDisable(GL_TEXTURE_1D);
}  
	""")

def makeGenTexture(f):
	f.write( """

void genTexture( GLubyte *stripeImage)
{  
   int j;
 
   for (j = 0; j < 32; j++) {
      stripeImage[4*j] = (GLubyte) ((j<=4) ? 255 : 0);
      stripeImage[4*j+1] = (GLubyte) ((j>4) ? 255 : 0);
      stripeImage[4*j+2] = (GLubyte) 0;
      stripeImage[4*j+3] = (GLubyte) 255;
   }
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glPixelStorei(GL_PACK_ALIGNMENT, 1);
   glEnable(GL_TEXTURE_2D);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 8, 8, 0, GL_RGB, GL_UNSIGNED_BYTE, (const GLvoid *)stripeImage);
   glDisable(GL_TEXTURE_2D);
}  
	""")

def genIdentf(f):
	f.write( """

	GLfloat m[16] = {
    		1, 0, 0, 0,
    		0, 1, 0, 0,
    		0, 0, 1, 0,
    		0, 0, 0, 1,
	};

	""")

def genIdentd(f):
	f.write( """

	GLdouble m[16] = {
    		1, 0, 0, 0,
    		0, 1, 0, 0,
    		0, 0, 1, 0,
    		0, 0, 0, 1,
	};

	""")

#
# Generate the permuted parameter lists for a GL function
# Return a list of lists containing the arguments
# A definition in APUutil.txt looks like this
#	name		ColorMaterial
#	return		void
#	param		face		GLenum
#	paramprop	face		GL_FRONT GL_BACK GL_FRONT_AND_BACK
#	param		mode		GLenum
#	paramprop	mode		GL_EMISSION GL_AMBIENT GL_DIFFUSE GL_SPECULAR GL_AMBIENT_AND_DIFFUSE
#	category	1.0
#	chromium	pack
# The input to this function is read in like this: 
#
# FUNC_NAME ColorMaterial PARAMS [('face', 'GLenum', 0), ('mode', 'GLenum', 0)]
# ['GL_FRONT', 'GL_BACK', 'GL_FRONT_AND_BACK']
# ['GL_EMISSION', 'GL_AMBIENT', 'GL_DIFFUSE', 'GL_SPECULAR', 'GL_AMBIENT_AND_DIFFUSE']
#
# Other non enumerated args can be specified by using the paramlist keyword
#	paramlist	size	2 4 5 6
#
# After permuting all of the arguments the function we get:
#
# ('GL_FRONT', 'GL_EMISSION')
# ('GL_BACK', 'GL_EMISSION')
# ('GL_FRONT_AND_BACK', 'GL_EMISSION')
# ('GL_FRONT', 'GL_AMBIENT')
# ('GL_BACK', 'GL_AMBIENT')
# ('GL_FRONT_AND_BACK', 'GL_AMBIENT')
# ('GL_FRONT', 'GL_DIFFUSE')
# ('GL_BACK', 'GL_DIFFUSE')
# ('GL_FRONT_AND_BACK', 'GL_DIFFUSE')
# ('GL_FRONT', 'GL_SPECULAR')
# ('GL_BACK', 'GL_SPECULAR')
# ('GL_FRONT_AND_BACK', 'GL_SPECULAR')
# ('GL_FRONT', 'GL_AMBIENT_AND_DIFFUSE')
# ('GL_BACK', 'GL_AMBIENT_AND_DIFFUSE')
# ('GL_FRONT_AND_BACK', 'GL_AMBIENT_AND_DIFFUSE')
#
# paramset format
#
# paramset	[format type] [GL_RED GL_GREEN GL_BLUE GL_ALPHA GL_BGR GL_LUMINANCE GL_LUMINANCE_ALPHA] [GL_UNSIGNED_BYTE GL_BYTE GL_UNSIGNED_SHORT GL_SHORT GL_UNSIGNED_INT GL_INT GL_FLOAT]
# paramset	[format type] [GL_RGB] [GL_UNSIGNED_BYTE GL_BYTE GL_UNSIGNED_SHORT GL_SHORT GL_UNSIGNED_INT GL_INT GL_FLOAT GL_UNSIGNED_BYTE_3_3_2 GL_UNSIGNED_BYTE_2_3_3_REV GL_UNSIGNED_SHORT_5_6_5 GL_UNSIGNED_SHORT_5_6_5_REV]
# paramset	[format type]  [GL_RGBA GL_BGRA] [GL_UNSIGNED_BYTE GL_BYTE GL_UNSIGNED_SHORT GL_SHORT GL_UNSIGNED_INT GL_INT GL_FLOAT GL_UNSIGNED_SHORT_4_4_4_4 GL_UNSIGNED_SHORT_4_4_4_4_REV GL_UNSIGNED_SHORT_5_5_5_1 GL_UNSIGNED_SHORT_1_5_5_5_REV GL_UNSIGNED_INT_8_8_8_8 GL_UNSIGNED_INT_8_8_8_8_REV GL_UNSIGNED_INT_10_10_10_2 GL_UNSIGNED_INT_2_10_10_10_REV]
# 

def GenParmSetLists(func_name):
        pset = apiutil.ParamSet(func_name)
	params = apiutil.Parameters(func_name)
	for index in range(len(params)):
		(name, type, vecSize) = params[index]
	
        returnlist = []
	rlist = []
        if pset != []:
		# number of disjoint sets
			
                for i in range(len(pset)):
			parset = pset[i]
			namelist = parset[0]
			setlist = parset[1:]
			for j in range(len(namelist)):
				returnlist.append((namelist[j],setlist[j]))
			rlist.append(returnlist)
        return rlist


def GenParmLists(func_name,f):

	# Fetch the parameter properties

        params = apiutil.Parameters(func_name)
        return_type = apiutil.ReturnType(func_name)
        if return_type != 'void':
                # Yet another gross hack for glGetString
                if string.find( return_type, '*' ) == -1:
                        return_type = return_type + " *"


	#print ""
	#print "FUNC_NAME %s PARAMS %s" %  (func_name,params)
	
	pvec = []
	tvec = []
	rlist = GenParmSetLists(func_name)
	#print "RETURNLIST"
	#print rlist
	#
	# At this point rlist is constructed as
	# 
	#
	#

	fvec = copy.deepcopy(apiutil.ParamProps(func_name))
	vallist = copy.deepcopy(apiutil.ParamList(func_name))
	valinit = copy.deepcopy(apiutil.ParamVec(func_name))
	multiList = []
	if rlist != []:
		for kk in range(len(rlist)):
			#print "RLIST-KK"
			pvec = []
			#print rlist[kk]
			rvec = rlist[kk]
			params = apiutil.Parameters(func_name)
			for index in range(len(params)):
				(name, type, vecSize) = params[index]
				action = apiutil.ParamAction(func_name)

				#print "name = %s type = %s" % (name,type)

				if vecSize >= 1:
					#
					#  Vector arg
					#
					#print "/* VECTOR */"
					f.write( "\t%s %s[%d]; /* VECTOR1 */\n" % (type[0:type.index('*')],name, vecSize))
					# TODO: Add dummy vars to vector/matrix
					tvec = name

				if type == "GLenum":
					#print "name = %s type = %s" % (name,type)
					if rvec != []:
						fvec = rvec
					else:
						fvec = copy.deepcopy(apiutil.ParamProps(func_name))
					#print "fvec = %s" % fvec
					for k in range(len(fvec)):
						d = fvec.pop(0)
						if d[0] == name:
							tvec = d[1]
							break
				elif vallist != []:
					#print "name = %s type = %s" % (name,type)
					vallist = copy.deepcopy(apiutil.ParamList(func_name))
					#print "vallist = %s" % vallist
					for k in range(len(vallist)):
						d = vallist[k]
						(dname,vec) = d
						#print d[0]
						if d[0] == name :
							#print "name = %s (dname,vec) = (%s) %s" % (name,dname,vec)
							tvec = vec
							break;
				elif valinit != []:
					#print "name = %s type = %s" % (name,type)
					valinit = copy.deepcopy(apiutil.ParamVec(func_name))
					#print "valinit = %s" % valinit
					cnt = 0
					d = valinit[k]
					(dname,vec) = d
					#print d[0]
					if d[0] == name :
						#print "name = %s (dname,vec) = (%s) %s" % (name,dname,vec)
						for nint in range(len(vec)):
							f.write("\t%s[%d] = %s;/* VA1 */\n" % (name,nint,vec[nint]))
							#print "\t%s[%d] = %s;\n" % (name,nint,vec[nint])
						break;
				elif range_mapping.has_key( type ):
					#print "name = %s type = %s" % (name,type)
					if rvec != []:
						#print "rvec = %s" % rvec
						fvec = rvec
						#print "fvec = %s" % fvec
						for k in range(len(fvec)):
							d = fvec.pop(0)
							if d[0] == name:
								tvec = d[1]
								break
						else:
							fvec = copy.deepcopy(apiutil.ParamProps(func_name))
					else:
						tvec = range_mapping[type]
				else:
					tvec = [0]
				pvec.append(tvec)
				#print "PVEC.APPEND(%s)" % tvec

			#print "PVEC %s" % pvec
			#print "PVECLEN = %d" % len(pvec)

			#for i in range(len(pvec)):
				#print  pvec[i]

			argLists = permute2(pvec)

			#print argLists
			#print "ARGLIST = %d" % len(argLists)

			#for i in range(len(argLists)):
				#print argLists[i]
			multiList.append(argLists)

	else:
		for index in range(len(params)):
			(name, type, vecSize) = params[index]
			action = apiutil.ParamAction(func_name)

			#print "name = %s type = %s" % (name,type)
			#print valinit

			if vecSize >= 1:
				#
				#  Vector arg
				#
				#print "/* VECTOR */"
				if type[0:5] == "const" :
					f.write( "\t%s %s[%d]; /* VECTOR2a */\n" % (type[6:type.index('*')],name, vecSize))
				else:
					f.write( "\t%s %s[%d]; /* VECTOR2 */\n" % (type[0:type.index('*')],name, vecSize))
				# TODO: Add dummy vars to vector/matrix
				tvec = name

		for index in range(len(params)):
			(name, type, vecSize) = params[index]
			action = apiutil.ParamAction(func_name)

			if type == "GLenum":
				fvec = copy.deepcopy(apiutil.ParamProps(func_name))
				for k in range(len(fvec)):
					d = fvec.pop(0)
					if d[0] == name:
						tvec = d[1]
						break
			elif vallist != []:
				vallist = copy.deepcopy(apiutil.ParamList(func_name))
				for k in range(len(vallist)):
					d = vallist.pop(0)
					if d[0] == name:
						tvec = d[1]
						break;
			elif valinit != []:
				#print "name = %s type = %s" % (name,type)
				valinit = copy.deepcopy(apiutil.ParamVec(func_name))
				#print "valinit = %s" % valinit
				cnt = 0
				for k in range(len(valinit)):
					d = valinit[k]
					(dname,vec) = d
					#print d[0]
					if d[0] == name :
						#print "name = %s (dname,vec) = (%s) %s" % (name,dname,vec)
						for nint in range(len(vec)):
							f.write("\t%s[%d] = (%s)%s;/* VA2 */\n" % (name,nint,type[0:type.index('*')],vec[nint]))
							#print "\t%s[%d] = %s;\n" % (name,nint,vec[nint])
						break;
			elif range_mapping.has_key( type ):
				tvec = range_mapping[type]
			else:
				tvec = [0]
			pvec.append(tvec)
			#print tvec
			#print "PVEC %s" % pvec

		#print "PVEC %s" % pvec
		#print "PVECLEN = %d" % len(pvec)

		#for i in range(len(pvec)):
			#print  pvec[i]

		argLists = permute2(pvec)

		#print argLists
		#print "ARGLIST = %d" % len(argLists)

		#for i in range(len(argLists)):
			#print argLists[i]
		multiList.append(argLists)
	return multiList

def PerformAction(func_name,f):
	action = apiutil.ParamAction(func_name)
	if action != []:
		#
		# Actions are performed here such as limiting the number
		# of textures that are being enumerated
		#
		(name,doit) = action[0]
		#print "func_name = " + func_name + " action = %s"  % action
		if doit == ['makeStripeImage']:
			f.write("\tmakeStripeImage((GLubyte *)%s);\n" % name)
		elif doit == ['enableTex']:
			f.write("\tenableTex();\n")
		elif doit == ['maxTex']:
			print "Limit textures"
		elif doit == ['genTex']:
			print "Gen textures"
			f.write("\tgenTexture((GLubyte *)%s);\n" % name)
		elif doit == ['pixelStore']:
			f.write("glPixelStorei( GL_PACK_LSB_FIRST, 0);\n")
			f.write("glPixelStorei( GL_UNPACK_LSB_FIRST, 0);\n")
			f.write("glPixelStorei( GL_PACK_SWAP_BYTES, 0);\n")
			f.write("glPixelStorei( GL_UNPACK_SWAP_BYTES, 0);\n")
		elif doit == ['identf']:
			genIdentf(f)
		elif doit == ['identd']:
			genIdentd(f)
#
# write the test function
#
def PrintTableFunc( func_name, params,  can_have_pointers,f):
	"""Emit a packer test function."""
	#print "PrintTableFunc (%s,%s,%d,f)" % (func_name,str(params),can_have_pointers)

	# Save original function name
	orig_func_name = func_name

	# Convert to a non-vector version of the function if possible
	#func_name = apiutil.NonVectorFunction( func_name )
	if not func_name:
		func_name = orig_func_name

	# Check if there are any pointer parameters.
	# That's usually a problem so we'll emit an error function.
	nonVecParams = apiutil.Parameters(func_name)
	return_type = apiutil.ReturnType(func_name)

	f.write("\nstruct %s_params {\n" % func_name)

	for (name, type, vecSize) in nonVecParams:
		if not apiutil.IsPointer(type) :
			f.write("\t%s %s_%s;\n" % (type,func_name,name))

	if verbose:
		f.write( '\tchar *prgstr;\n')
	f.write( '} %s_tab[] = {\n' % func_name)


	bail_out = 0
	for (name, type, vecSize) in nonVecParams:
		if apiutil.IsPointer(type) and vecSize == 0 and not can_have_pointers:
			bail_out = 1


	counter = 0

	# generate the calls

	if len(params) == 0:
		argstr = ""
	else:
		argstr = ", "

	printfstr = ""
	argstr = ""
	
	if return_type != 'void':
		# Yet another gross hack for glGetString
		if string.find( return_type, '*' ) == 1:
			return_type = return_type + " *"
		f.write( '\t%s return_val;\n' % return_type)


	# At this point all of the declarations have been generated
	# The argument lists with the various parameter values have to
	# be permuted

	# Generate the lists to be permuted
	argLists  = GenParmLists(func_name,f)

	#print argLists
	#print len(argLists)

	# 
	# Now iterate over the permuted argument lists generating gl calls with
	# the permuted arguments
	#

	for ki in range(len(argLists)):
		argList = argLists[ki]
		ncount = 0
		if len(argList) > 0:
			allargstr = ""
			for i in range(len(argList)):
				#print argList[i]
				q = argList[i]

				ll = 0
				f.write("{ ")
				ncount = ncount + 1
				for k in range(len(q)):
					(name, type, vecSize) = params[k]
					#
					# ordinary typed parameter, or vector of unknown size
					#
					# TODO Vector arg
					#
					#if vecSize >= 1:
						#f.write('%s /* VEC1 */' %  name)
						#f.write( "\t%s %s[%d]; /* VECTOR1 */\n" % (type[0:type.index('*')],name, vecSize))
					#if apiutil.IsPointer ( type ):
						# POINTER
						#f.write( "\t(%s)%s /* VEC3 */" % (type,name))
					if printf_mapping.has_key( type ):
						(format_str, cast) = printf_mapping[type]
						printfstr += format_str
						cast_str = ''
						if cast != '':
							cast_str = '(%s)' % cast
						if type == 'GLenum':
							argstr += "%s" % q[k]
						elif type == 'GLboolean':
							argstr += "%s" % q[k]
						else:
							argstr += '%s %s' % (cast_str,q[k])
					#elif type.find( "*" ):
						#printfstr += "%p"
						#argstr += "(void *)"
						#argstr += '%s' % q[k]
					else:
						argstr = ""
						printfstr = "???"
						break
				

					if ll != len(params):
						printfstr += ", "
						argstr += ", "

					ll += 1

					f.write( '\t%s' % argstr)
					allargstr = allargstr + argstr
					argstr = ""
					printfstr = ""

				if verbose:
					f.write( '\n\t\"%s_tab.%s\"' % (func_name,allargstr))
				allargstr = ""
				f.write( '},\n')
		
	# finish up
	f.write( '};\n\n' )
	f.write( '/* COUNT = %d */\n' % ncount)
	f.write( 'void crPackTest%s(void)\n' % (func_name))
	f.write( '{\n')
	f.write( '\tint i;\n')
	for (name, type, vecSize) in nonVecParams:
		if apiutil.IsPointer(type) and vecSize == 0 and not can_have_pointers:
			if vecSize == 0:
				if type == "GLvoid *" or type == "GLvoid **" or type == "GLubyte *" or type == "const GLvoid *"  or type == "const GLubyte *":
					f.write( "\t%s %s[100000];/* VECP2b (%s,%s)*/\n" % (type,name,type,name))
				elif  type == "const GLfloat *"  or type == "GLfloat *" or type == "const GLint *" or type == "GLint *" or type == "GLdouble *" or type == "const GLdouble *" or type == "const GLuint *" or type == "GLuint *" or type == "const GLushort *" or type == "GLushort *":
					f.write( "\t%s %s[100000];/* VECP2a (%s,%s)*/\n" % (type[0:type.index('*')],name,type,name))
					#f.write( "\t%s %s[100000];/* VECP2a */\n" % (type,name))
			bail_out = 1
		elif apiutil.IsPointer(type) :
			if vecSize == 0:
				if type == "GLvoid *" or type == "GLvoid **" or type == "GLubyte *" or type == "const GLvoid *" or type == "const GLubyte *" or type == "GLint *" or type == "const GLint *" or type == "const GLfloat *" or type == "GLfloat *" or type == "GLdouble *" or type == "const GLdouble *" or type == "const GLuint *" or type == "GLuint *"  or type == "const GLushort *" or type == "GLushort *":
					f.write( "\t%s %s[100000];/* VECP9 (%s,%s)*/\n" % (type[0:type.index('*')],name,type,name))
				else:
					f.write( "\tGLubyte %s[100000];/* VECP7 */\n" % name)

	PerformAction(func_name,f)

	f.write('\tfor ( i = 0; i < %d; i++) {\n' % ncount)

	if verbose:
		f.write('\tif (verbose)\n\t\tcrDebug(\"gl%s( %%s )\",%s_tab[i].prgstr);\n'  % (func_name,func_name))
	if return_type != 'void':
		f.write( '\t\treturn_val = gl%s(' % func_name)
	else:
		f.write( '\t\tgl%s(' % func_name)

	ll = 0
	for (name, type, vecSize) in nonVecParams:
		if apiutil.IsPointer(type) and vecSize == 0 and not can_have_pointers:
			if vecSize == 0:
				if type == "GLvoid *" or type == "GLvoid **" or type == "GLubyte *" or type == "const GLvoid *" or type == "const GLubyte *":
					f.write( "%s /* VECS2 */\n" % name)
			bail_out = 1
		elif apiutil.IsPointer(type) :
			if vecSize == 0:
				if type == "GLvoid *" or type == "GLvoid **" or type == "GLubyte *" or type == "const GLvoid *" or type == "const GLubyte *" or type == "GLint *" or type == "const GLint *" or type == "GLfloat *" or type == "GLdouble *" or type == "const GLdouble *" or type == "const GLuint *" or type == "GLuint *":
					f.write( "\t%s/* VECS4 */\n" % name)
				else:
					f.write( "\t%s/* VECS5 */\n" % name)
		else:
			f.write("\t%s_tab[i].%s_%s" % (func_name,func_name,name))
		if ll != len(nonVecParams) -1:
			f.write(', ')
		ll = ll + 1
		
#
	f.write(');\n')
	if return_type != 'void':
		f.write( '\n\t\tif(errChk)\n\t\tprintError(\"gl(%s)\");\n' % func_name)
	else:
		f.write( '\t\tif(errChk)\n\t\t\tprintError(\"gl(%s)\");\n' % func_name)
	f.write( '\t}\n' )
	f.write( '}\n' )

def PrintFunc( func_name, params,  can_have_pointers,f):
	f.write( 'void crPackTest%s(void)\n' % (func_name))
	f.write( '{\n')

	# Save original function name
	orig_func_name = func_name

	# Convert to a non-vector version of the function if possible
	#func_name = apiutil.NonVectorFunction( func_name )
	if not func_name:
		func_name = orig_func_name

	# Check if there are any pointer parameters.
	# That's usually a problem so we'll emit an error function.
	nonVecParams = apiutil.Parameters(func_name)
	return_type = apiutil.ReturnType(func_name)

	bail_out = 0
	for (name, type, vecSize) in nonVecParams:
		if apiutil.IsPointer(type) and vecSize == 0 and not can_have_pointers:
			if vecSize == 0:
				if type == "GLvoid *" or type == "GLvoid **" or type == "GLubyte *" or type == "const GLvoid *"  or type == "const GLubyte *":
					f.write( "\t%s %s[100000];/* VECP7b (%s,%s)*/\n" % (type,name,type,name))
				elif   type == "GLfloat *" or type == "GLint *"  or type == "GLdouble *" or type == "GLuint *"  or  type == "GLushort *":
					f.write( "\t%s %s[100000];/* VECP7a (%s,%s)*/\n" % (type[0:type.index('*')],name,type,name))
				elif  type == "const GLfloat *" or type == "const GLint *"  or type == "const GLdouble *" or type == "const GLuint *" or type == "const GLushort *":
					f.write( "\t%s %s[100000];/* VECP7b (%s,%s)*/\n" % (type[6:type.index('*')],name,type,name))
					#f.write( "\t%s %s[100000];/* VECP7a */\n" % (type,name))
			bail_out = 1
		elif apiutil.IsPointer(type) :
			if vecSize == 0:
				if  type == "GLsizei *" or  type == "GLubyte *" or type == "const GLvoid *" or type == "const GLubyte *" or type == "GLint *" or type == "const GLint *" or type == "const GLfloat *" or type == "GLfloat *" or type == "GLdouble *" or type == "const GLdouble *" or type == "const GLuint *" or type == "GLuint *":
					f.write( "\t%s %s[100000];/* VECP5a (%s,%s)*/\n" % (type[0:type.index('*')],name,type,name))
				else:
					f.write( "\t%s %s[100000];/* VECP5 */\n" % (type,name))
	PerformAction(func_name,f)

#	if bail_out:
#		for (name, type, vecSize) in nonVecParams:
#			print '\t(void)%s;' % (name)
		#
		# Special casing indicates that some arbitrary data appropriate to
		# the call must be supplied
#		f.write( '\tcrDebug ( "%s needs to be special cased %d %d!");\n' % (func_name, vecSize, can_have_pointers))

	if "extpack" in apiutil.ChromiumProps(func_name):
		is_extended = 1
	else:
		is_extended = 0

	counter = 0

	# generate the calls

	if len(params) == 0:
		argstr = ""
	else:
		argstr = ", "

	printfstr = ""
	argstr = ""
	
	if return_type != 'void':
		# Yet another gross hack for glGetString
		if string.find( return_type, '*' ) == 1:
			return_type = return_type + " *"
		f.write( '\t%s return_val;\n' % return_type)


	# At this point all of the declarations have been generated
	# The argument lists with the various parameter values have to
	# be permuted

	# Generate the lists to be permuted
	argLists  = GenParmLists(func_name,f)

	#print argLists
	#print len(argLists)

	# 
	# Now iterate over the permuted argument lists generating gl calls with
	# the permuted arguments
	#

	for ki in range(len(argLists)):
		argList = argLists[ki]
		if len(argList) > 0:
			allargstr = ""
			for i in range(len(argList)):
				#print argList[i]
				q = argList[i]

				if return_type != 'void':
					f.write( '\treturn_val = gl%s(' % func_name)
				else:
					f.write( '\tgl%s(' % func_name)
				ll = 0
				nparms = len(params)
				if len(q) > nparms:
					nparms = len(q)
				#for k in range(len(q)):
				for k in range(nparms):
					(name, type, vecSize) = params[k]

					# ordinary typed parameter, or vector of unknown size
					#
					# TODO Vector arg
					#
					if vecSize >= 1:
						f.write("%s /* VEC2a */" %  name)
					elif apiutil.IsPointer ( type ):
						# POINTER
						f.write( "\t(%s)%s /* VEC3a */\n" % (type,name))
#						(format_str, cast) = printf_mapping[type]
#						printfstr += format_str
#						cast_str = ''
#						if cast != '':
#							cast_str = '(%s)' % cast
#						if type == 'GLenum':
#							argstr += "%s" % q[k]
#						elif type == 'GLboolean':
#							argstr += "%s" % q[k]
#						else:
#							argstr += '%s %s' % (cast_str,q[k])
					elif printf_mapping.has_key( type ):
						(format_str, cast) = printf_mapping[type]
						printfstr += format_str
						cast_str = ''
						if cast != '':
							cast_str = '(%s)' % cast
						if type == 'GLenum':
							argstr += "%s" % q[k]
						elif type == 'GLboolean':
							argstr += "%s" % q[k]
						else:
							argstr += '%s %s' % (cast_str,q[k])
					elif type.find( "*" ):
						printfstr += "%p"
						argstr += "(void *)"
						argstr += '%s' % q[k]
					else:
						argstr = ""
						printfstr = "???"
						break;
				

					if ll != len(params) - 1:
						printfstr += ", "
						argstr += ", "

					ll += 1

					f.write( '%s' % argstr)
					allargstr = allargstr + argstr
					argstr = ""
					printfstr = ""

				f.write( ');\n\tif(errChk)\n\t\tprintError(\"gl%s(%s)\");\n' % (func_name,allargstr))
				if verbose:
					f.write('\t\tif (verbose)\n\t\t\tcrDebug(\"gl%s( %s )\");\n'  % (func_name,allargstr))
				allargstr = ""
				#f.write( ');\n')
		else:
			if return_type != 'void':
				f.write( '\treturn_val = gl%s();\n\tif(errChk)\n\t\tprintError(\"gl(%s)\");\n' % (func_name,func_name))
			else:
				f.write( '\tgl%s();\n\tif(errChk)\n\t\tprintError(\"gl(%s)\");\n' % (func_name,func_name))
				if verbose:
					f.write('\t\tif (verbose)\n\t\t\tcrDebug(\"gl%s( )\");\n'  % (func_name))
		
	# finish up
	f.write( '}\n' )


#
# Print the header portion of the test program files.
#
def PrintHeaders (f):
	CopyrightC(f)

	f.write( "\n")
	f.write( "/* DO NOT EDIT - THIS FILE GENERATED BY THE opcodes.py SCRIPT */\n")
	f.write( "\n")
	f.write( "\n")

	f.write( """
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chromium.h"
#include "cr_error.h"
#include "packertest.h"

int errChk;
int verbose;
void printError(char *name);
""")

def PrintDynProto(func_name,f):
        if apiutil.FindSpecial( "packertest", func_name ):
                return

	if not allfuncs:
        	if not apiutil.CanPack(func_name):
                	return

        pointers_ok = 0


        params = apiutil.Parameters(func_name)

	if "Chromium" ==  apiutil.Category(func_name):
		is_extended = 1
	else:
		is_extended = 0

	if is_extended:
		f.write( "typedef %s (APIENTRY *gl%s_t) (%s);\n" % (return_type,func_name,apiutil.MakeDeclarationString(params)))
		f.write( "static gl%s_t %s_func;\n" % (func_name,func_name))

#
# Generate function prototypes
#
def PrintProto(func_name,f,no_special):
	if no_special == 1:
        	if apiutil.FindSpecial( "packertest", func_name ):
                	return

	if not allfuncs:
        	if not apiutil.CanPack(func_name):
                	return

        pointers_ok = 0

        return_type = apiutil.ReturnType(func_name)
        params = apiutil.Parameters(func_name)

	if "Chromium" ==  apiutil.Category(func_name):
		is_extended = 1
	else:
		is_extended = 0

	if is_extended:
		f.write( "gl%s gl%s_func = (gl%s_t)crGetProcAddress(\"gl%s\");\n" % (func_name,func_name,func_name,func_name))

        if return_type != 'void':
                # Yet another gross hack for glGetString
                if string.find( return_type, '*' ) == -1:
                        return_type = return_type + " *"

        #if "get" in apiutil.Properties(func_name):
                #pointers_ok = 1

        if func_name == 'Writeback':
                pointers_ok = 1

        f.write( 'void crPackTest%s (void);\n' % func_name)

#
# Generate the body of the test functions
#
def PrintBodies(func_name,f,no_special, gentables):
	#print "func_name = %s  no_special = %d" % (func_name, no_special)
	if no_special == 1:
        	if apiutil.FindSpecial( "packertest", func_name ):
                	return

	if not allfuncs:
        	if not apiutil.CanPack(func_name):
                	return

        pointers_ok = 0

        return_type = apiutil.ReturnType(func_name)
        params = apiutil.Parameters(func_name)

        if "get" in apiutil.Properties(func_name):
                pointers_ok = 1

        if func_name == 'Writeback':
                pointers_ok = 1

	#print "func_name = %s pointers_ok = %d no_special = %d" % (func_name, pointers_ok,no_special)
	#print params
	if gentables == 1:
        	PrintTableFunc( func_name, params,  pointers_ok ,f)
	else:
        	PrintFunc( func_name, params,  pointers_ok ,f)



def PrintMiddle(f):
	f.write( """

#define BUFSIZE 2048
static GLuint sbuffer[BUFSIZE];
static void Init( void )
{
	glSelectBuffer (BUFSIZE, sbuffer);
	glDepthMask(GL_TRUE);
	glStencilMask(0xffffffff);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_FLAT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}



static void Draw(void)
{

	"""
	)

def printTail(f):
	f.write ("""

void printError(char *name)
{
	GLenum ret = GL_NO_ERROR;

	while((ret = glGetError()) != GL_NO_ERROR) 
		switch (ret) {
		case GL_NO_ERROR:
			break;
		case GL_INVALID_ENUM:
			crWarning("%s: GL_INVALID_ENUM",name);
			break;
		case GL_INVALID_VALUE:
			crWarning("%s: GL_INVALID_VALUE",name);
			break;
		case GL_INVALID_OPERATION:
			crWarning("%s: GL_INVALID_OPERATION",name);
			break;
		case GL_STACK_OVERFLOW:
			crWarning("%s: GL_STACK_OVERFLOW",name);
			break;
		case GL_STACK_UNDERFLOW:
			crWarning("%s: GL_STACK_UNDERFLOW",name);
			break;
		case GL_OUT_OF_MEMORY:
			crWarning("%s: GL_OUT_OF_MEMORY",name);
			break;
		case GL_TABLE_TOO_LARGE:
			crWarning("%s: GL_TABLE_TOO_LARGE",name);
			break;
		default:
			crWarning("%s: Unknown GL Error",name);
			break;
		}
}

static void Key( unsigned char key, int x, int y )
{
	(void) x;
	(void) y;
	switch (key) {
	case 'q':
	case 27:
		exit(0);
		break;
	}
	glutPostRedisplay();
}

static void
PrintHelp(void)
{
	printf(\"Usage: packertest [options]\\n\");
	printf(\"Options:\\n\");
	printf(\"  -a        enable accum buffer mode (default)\\n\");
	printf(\"  -A        enable all information\\n\");
	printf(\"  -d        double buffer mode (default)\\n\");
	printf(\"  -error    do error check after each call (slow)\\n\");
	printf(\"  -h        print this information\\n\");
	printf(\"  -i        index mode\\n\");
	printf(\"  -m        multisample mode\\n\");
	printf(\"  -r        rgba mode (default)\\n\");
	printf(\"  -S        enable stencil buffer mode (default)\\n\");
	printf(\"  -s        stereo mode\\n\");
	printf(\"  -v        verbose output\\n\");
}


int main(int argc, char *argv[])
{
   int i;
   int mode;

   setbuf(stdout,NULL);
   setbuf(stderr,NULL);
   mode = GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ACCUM | GLUT_STENCIL | GLUT_MULTISAMPLE;
   for (i = 1; i < argc; i++) {
	if (!strcmp( argv[i], \"-error\")) {
	    errChk = 1;
	}
	else if (!strcmp( argv[i], \"-A\")) {
   	    mode = GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ACCUM | GLUT_STENCIL | GLUT_MULTISAMPLE;
	}
	else if (strcmp(argv[i], \"-s\") == 0) {
	    mode |= GLUT_STEREO;
	}
	else if (strcmp(argv[i], \"-i\") == 0) {
	    mode |= GLUT_INDEX;
	    mode &= ~GLUT_RGBA;
	}
	else if (strcmp(argv[i], \"-r\") == 0) {
	    mode |= GLUT_RGBA;
	    mode &= ~GLUT_RGBA;
	}
	else if (strcmp(argv[i], \"-d\") == 0) {
	    mode |= GLUT_DOUBLE;
	}
	else if (strcmp(argv[i], \"-D\") == 0) {
	    mode |= GLUT_DEPTH;
	}
	else if (strcmp(argv[i], \"-a\") == 0) {
	    mode |= GLUT_ACCUM;
	}
	else if (strcmp(argv[i], \"-S\") == 0) {
	    mode |= GLUT_STENCIL;
	}
	else if (strcmp(argv[i], \"-m\") == 0) {
	    mode |= GLUT_MULTISAMPLE;
	}
	else if (strcmp(argv[i], \"-v\") == 0) {
	    verbose = 1;
	}
	else if (!strcmp( argv[i], \"-h\" ) || !strcmp(argv[i], \"--help\"))
	{
	    PrintHelp();
	    exit(0);
	}

   }
   glutInit( &argc, argv );
   glutInitWindowPosition(0, 0);
   glutInitWindowSize(400, 300);
   glutInitDisplayMode(mode);
   glutCreateWindow(argv[0]);
   glutDisplayFunc( Draw );
   glutKeyboardFunc( Key );
   Init();
	""")

	f.write( 'printf("Press q or <ESC> to exit\\n");')

	f.write( """
   glutMainLoop();
   return 0;
}
	"""
	)
	f.write("\n\n")


#
# Generate calls to the test functions
#
def GenCalls(func_name,f,no_special):
	if no_special == 1:
        	if apiutil.FindSpecial( "packertest", func_name ):
                	return

	if not allfuncs:
        	if not apiutil.CanPack(func_name):
                	return

        pointers_ok = 0

        return_type = apiutil.ReturnType(func_name)
        params = apiutil.Parameters(func_name)

	if "Chromium" ==  apiutil.Category(func_name):
		is_extended = 1
	else:
		is_extended = 0

	if is_extended:
		f.write( "gl%s gl%s_func = (gl%s_t)crGetProcAddress(\"gl%s\");\n" % (func_name,func_name,func_name,func_name))

        if return_type != 'void':
                # Yet another gross hack for glGetString
                if string.find( return_type, '*' ) == -1:
                        return_type = return_type + " *"

        #if "get" in apiutil.Properties(func_name):
                #pointers_ok = 1

        if func_name == 'Writeback':
                pointers_ok = 1

	#print "func_name = %s pointers_ok = %d" % (func_name, pointers_ok)
	#print params
        f.write( '\tcrPackTest%s ();\n' % func_name)
        #f.write( '\tglutSwapBuffers ();\n')

def EndGenCalls(f):
	f.write("\tglFinish();\n\tif(errChk)\n\t\tprintError(\"glFinish()\");\n")
	f.write("\tcrDebug(\"DONE\\n\");\n")
	f.write("\texit(0);\n")
	f.write("}\n")
	f.write("\n")

def PrintPart1(file):
	PrintHeaders(file)

def PrintPart2(file, gentables):
	#
	# Generate function bodies
	#
	for func_name in keys:
		PrintBodies(func_name,file,1, gentables)

def PrintPart3(file):
	enableTex(file)
	makeStripeImage(file)
	makeGenTexture(file)
	PrintMiddle(file)

	#
	# Generate calls
	#


	i = 0
	for fname in special_funcs:
		GenCalls(fname,file,0)
		i = i + 1
		if i % 20 == 0:
        		file.write( '\tglutSwapBuffers ();\n')

	for func_name in keys:
		GenCalls(func_name,file,1)
		i = i + 1
		if i % 20 == 0:
        		file.write( '\tglutSwapBuffers ();\n')

	for fname in special_keys:
		GenCalls(fname,file,0)
		i = i + 1
		if i % 20 == 0:
        		file.write( '\tglutSwapBuffers ();\n')


  	file.write( '\tglutSwapBuffers ();\n')
	EndGenCalls(file)
	printTail(file)

def PrintAll(file, gentables):
	PrintPart1(file)
	PrintPart2(file, gentables)
	PrintPart3(file)

def GenSpecial(func_name,file, gentables):
	file = open(file,"w")
	PrintHeaders(file)

	print "Writing %s" % func_name
	PrintBodies(func_name,file,0, gentables)
	file.close()

if __name__ == "__main__":
	# Parse command line
	import getopt
	try:
		opts, args = getopt.getopt(sys.argv[1:], "adtnosv")
	except getopt.error, msg:
		print msg
		exit

	# Process options
	gentables = 0
	addpath = []
	exclude = []
	for o, a in opts:
		if o == '-a':
			allfuncs = 1
		if o == '-d':
			debug = debug + 1
		if o == '-t':
			gentables = 1
		if o == '-o':
			omit = 1
		if o == '-n':
			nopack = 1
		if o == '-s':
			stub = 1
		if o == '-v':
			verbose = 1
	# Provide default arguments
	if not args:
		script = "hello.py"
	else:
		script = args[0]

	
	d = apiutil.GetFunctionDict("../../glapi_parser/APIspec.txt")
	for func in d.keys():
		rec = d[func]
		if "stub" in rec.chromium:
			print "%s is a stub in Chromium, no test function generated." % func
	print ""
	for func in d.keys():
		rec = d[func]
		if "omit" in rec.chromium:
			print "%s is not handled by Chromium, no test function generated." % func

	print ""
	d = []
	if allfuncs == 1:
		funcs = apiutil.GetFunctionDict("../../glapi_parser/APIspec.txt")
        	keys = []
		for key in funcs.keys():
			keys.append(key)
        	keys.sort()

	else:
		keys = apiutil.GetDispatchedFunctions("../../glapi_parser/APIspec.txt")
	nkeys = len(keys)
	file = open("packertest.h","w")
	#
	# Generate prototypes for dynanimically loaded funcs
	#
	for fname1 in keys:
		PrintDynProto(fname1,file)

	#
	# Generate prototypes
	#
	for func_name in keys:
		PrintProto(func_name,file,1)
	for fname in special_funcs:
		PrintProto(fname,file,0)
	for fname in special_keys:
		PrintProto(fname,file,0)

	file.write("void makeStripeImage( GLubyte *stripeImage);\n")
	file.write("void enableTex( void );\n")
	file.write("void genTexture( GLubyte *stripeImage);\n")

	file.close()

	file = open("packertest100.c","w")
	i = 0
	for func_name in keys:
		if debug:
			print "Generating for %s" % func_name
		if i  % 25 == 0:
			file.close()
			fname = "packertest" + str(i) + ".c"
			file = open(fname,"w")
			print "Writing %s" % fname
			PrintPart1(file)
			PrintBodies(func_name,file,1, gentables)
		else:
			PrintBodies(func_name,file,1, gentables)
		i = i + 1

	file = open("packertest.c","w")
	PrintPart1(file)
	PrintPart3(file)
	file.close()

	for fname in special_keys:
		filename = "packertest" + fname + ".c"
		GenSpecial(fname,filename, 1)

