
import sys,os;
import cPickle;
import string;
import stub_common;

parsed_file = open( "../glapi_parser/gl_header.parsed", "rb" )
gl_mapping = cPickle.load( parsed_file )

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

#print "DllMain"
