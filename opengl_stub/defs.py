# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.


import sys,os;
import cPickle;
import string;
import stub_common;

parsed_file = open( "../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

stub_common.CopyrightDef()

print "DESCRIPTION \"\""
print "EXPORTS"

keys = gl_mapping.keys()
keys.sort();
for func_name in keys:
	if stub_common.FindSpecial( 'noexport', func_name ):
		continue
	print "gl%s = cr_gl%s" % (func_name,func_name)

for func_name in ( "wglChoosePixelFormat", 
		   "wglCopyContext",
		   "wglCreateContext",
		   "wglCreateLayerContext",
		   "wglDeleteContext",
		   "wglDescribeLayerPlane",
		   "wglDescribePixelFormat",
		   "wglGetCurrentContext",
		   "wglGetCurrentDC",
		   "wglGetLayerPaletteEntries",
		   "wglGetPixelFormat",
		   "wglGetProcAddress",
		   "wglMakeCurrent",
		   "wglRealizeLayerPalette",
		   "wglSetLayerPaletteEntries",
		   "wglSetPixelFormat",
		   "wglShareLists",
		   "wglSwapBuffers",
		   "wglSwapLayerBuffers",
		   "wglSwapMultipleBuffers",
		   "wglUseFontBitmapsA",
		   "wglUseFontBitmapsW",
		   "wglUseFontOutlinesA",
		   "wglUseFontOutlinesW", 
		   "wglChoosePixelFormat" ):
    print func_name + " = " + func_name + "_prox";

print "crCreateContext"
print "crMakeCurrent"
print "crSwapBuffers"
#print "DllMain"
