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

stack_sizes = {
  'Accum': 8,
  'AlphaFunc': 8,
  'AreTexturesResident': 12,
  'ArrayElement': 4,
  'Begin': 4,
  'BindTexture': 8,
  'Bitmap': 28,
  'BlendFunc': 8,
  'CallList': 4,
  'CallLists': 12,
  'Clear': 4,
  'ClearAccum': 16,
  'ClearColor': 16,
  'ClearDepth': 8,
  'ClearIndex': 4,
  'ClearStencil': 4,
  'ClipPlane': 8,
  'Color3b': 12,
  'Color3bv': 4,
  'Color3d': 24,
  'Color3dv': 4,
  'Color3f': 12,
  'Color3fv': 4,
  'Color3i': 12,
  'Color3iv': 4,
  'Color3s': 12,
  'Color3sv': 4,
  'Color3ub': 12,
  'Color3ubv': 4,
  'Color3ui': 12,
  'Color3uiv': 4,
  'Color3us': 12,
  'Color3usv': 4,
  'Color4b': 16,
  'Color4bv': 4,
  'Color4d': 32,
  'Color4dv': 4,
  'Color4f': 16,
  'Color4fv': 4,
  'Color4i': 16,
  'Color4iv': 4,
  'Color4s': 16,
  'Color4sv': 4,
  'Color4ub': 16,
  'Color4ubv': 4,
  'Color4ui': 16,
  'Color4uiv': 4,
  'Color4us': 16,
  'Color4usv': 4,
  'ColorMask': 16,
  'ColorMaterial': 8,
  'ColorPointer': 16,
  'CopyPixels': 20,
  'CopyTexImage1D': 28,
  'CopyTexImage2D': 32,
  'CopyTexSubImage1D': 24,
  'CopyTexSubImage2D': 32,
  'CullFace': 4,
  'DebugEntry': 8,
  'DeleteLists': 8,
  'DeleteTextures': 8,
  'DepthFunc': 4,
  'DepthMask': 4,
  'DepthRange': 16,
  'Disable': 4,
  'DisableClientState': 4,
  'DrawArrays': 12,
  'DrawBuffer': 4,
  'DrawElements': 16,
  'DrawPixels': 20,
  'EdgeFlag': 4,
  'EdgeFlagPointer': 8,
  'EdgeFlagv': 4,
  'Enable': 4,
  'EnableClientState': 4,
  'End': 0,
  'EndList': 0,
  'EvalCoord1d': 8,
  'EvalCoord1dv': 4,
  'EvalCoord1f': 4,
  'EvalCoord1fv': 4,
  'EvalCoord2d': 16,
  'EvalCoord2dv': 4,
  'EvalCoord2f': 8,
  'EvalCoord2fv': 4,
  'EvalMesh1': 12,
  'EvalMesh2': 20,
  'EvalPoint1': 4,
  'EvalPoint2': 8,
  'FeedbackBuffer': 12,
  'Finish': 0,
  'Flush': 0,
  'Fogf': 8,
  'Fogfv': 8,
  'Fogi': 8,
  'Fogiv': 8,
  'FrontFace': 4,
  'Frustum': 48,
  'GenLists': 4,
  'GenTextures': 8,
  'GetBooleanv': 8,
  'GetClipPlane': 8,
  'GetDoublev': 8,
  'GetError': 0,
  'GetFloatv': 8,
  'GetIntegerv': 8,
  'GetLightfv': 12,
  'GetLightiv': 12,
  'GetMapdv': 12,
  'GetMapfv': 12,
  'GetMapiv': 12,
  'GetMaterialfv': 12,
  'GetMaterialiv': 12,
  'GetPixelMapfv': 8,
  'GetPixelMapuiv': 8,
  'GetPixelMapusv': 8,
  'GetPointerv': 8,
  'GetPolygonStipple': 4,
  'GetString': 4,
  'GetTexEnvfv': 12,
  'GetTexEnviv': 12,
  'GetTexGendv': 12,
  'GetTexGenfv': 12,
  'GetTexGeniv': 12,
  'GetTexImage': 20,
  'GetTexLevelParameterfv': 16,
  'GetTexLevelParameteriv': 16,
  'GetTexParameterfv': 12,
  'GetTexParameteriv': 12,
  'Hint': 8,
  'IndexMask': 4,
  'IndexPointer': 12,
  'Indexd': 8,
  'Indexdv': 4,
  'Indexf': 4,
  'Indexfv': 4,
  'Indexi': 4,
  'Indexiv': 4,
  'Indexs': 4,
  'Indexsv': 4,
  'Indexub': 4,
  'Indexubv': 4,
  'InitNames': 0,
  'InterleavedArrays': 12,
  'IsEnabled': 4,
  'IsList': 4,
  'IsTexture': 4,
  'LightModelf': 8,
  'LightModelfv': 8,
  'LightModeli': 8,
  'LightModeliv': 8,
  'Lightf': 12,
  'Lightfv': 12,
  'Lighti': 12,
  'Lightiv': 12,
  'LineStipple': 8,
  'LineWidth': 4,
  'ListBase': 4,
  'LoadIdentity': 0,
  'LoadMatrixd': 4,
  'LoadMatrixf': 4,
  'LoadName': 4,
  'LogicOp': 4,
  'Map1d': 32,
  'Map1f': 24,
  'Map2d': 56,
  'Map2f': 40,
  'MapGrid1d': 20,
  'MapGrid1f': 12,
  'MapGrid2d': 40,
  'MapGrid2f': 24,
  'Materialf': 12,
  'Materialfv': 12,
  'Materiali': 12,
  'Materialiv': 12,
  'MatrixMode': 4,
  'MultMatrixd': 4,
  'MultMatrixf': 4,
  'NewList': 8,
  'Normal3b': 12,
  'Normal3bv': 4,
  'Normal3d': 24,
  'Normal3dv': 4,
  'Normal3f': 12,
  'Normal3fv': 4,
  'Normal3i': 12,
  'Normal3iv': 4,
  'Normal3s': 12,
  'Normal3sv': 4,
  'NormalPointer': 12,
  'Ortho': 48,
  'PassThrough': 4,
  'PixelMapfv': 12,
  'PixelMapuiv': 12,
  'PixelMapusv': 12,
  'PixelStoref': 8,
  'PixelStorei': 8,
  'PixelTransferf': 8,
  'PixelTransferi': 8,
  'PixelZoom': 8,
  'PointSize': 4,
  'PolygonMode': 8,
  'PolygonOffset': 8,
  'PolygonStipple': 4,
  'PopAttrib': 0,
  'PopClientAttrib': 0,
  'PopMatrix': 0,
  'PopName': 0,
  'PrioritizeTextures': 12,
  'PushAttrib': 4,
  'PushClientAttrib': 4,
  'PushMatrix': 0,
  'PushName': 4,
  'RasterPos2d': 16,
  'RasterPos2dv': 4,
  'RasterPos2f': 8,
  'RasterPos2fv': 4,
  'RasterPos2i': 8,
  'RasterPos2iv': 4,
  'RasterPos2s': 8,
  'RasterPos2sv': 4,
  'RasterPos3d': 24,
  'RasterPos3dv': 4,
  'RasterPos3f': 12,
  'RasterPos3fv': 4,
  'RasterPos3i': 12,
  'RasterPos3iv': 4,
  'RasterPos3s': 12,
  'RasterPos3sv': 4,
  'RasterPos4d': 32,
  'RasterPos4dv': 4,
  'RasterPos4f': 16,
  'RasterPos4fv': 4,
  'RasterPos4i': 16,
  'RasterPos4iv': 4,
  'RasterPos4s': 16,
  'RasterPos4sv': 4,
  'ReadBuffer': 4,
  'ReadPixels': 28,
  'Rectd': 32,
  'Rectdv': 8,
  'Rectf': 16,
  'Rectfv': 8,
  'Recti': 16,
  'Rectiv': 8,
  'Rects': 16,
  'Rectsv': 8,
  'RenderMode': 4,
  'Rotated': 32,
  'Rotatef': 16,
  'Scaled': 24,
  'Scalef': 12,
  'Scissor': 16,
  'SelectBuffer': 8,
  'ShadeModel': 4,
  'StencilFunc': 12,
  'StencilMask': 4,
  'StencilOp': 12,
  'TexCoord1d': 8,
  'TexCoord1dv': 4,
  'TexCoord1f': 4,
  'TexCoord1fv': 4,
  'TexCoord1i': 4,
  'TexCoord1iv': 4,
  'TexCoord1s': 4,
  'TexCoord1sv': 4,
  'TexCoord2d': 16,
  'TexCoord2dv': 4,
  'TexCoord2f': 8,
  'TexCoord2fv': 4,
  'TexCoord2i': 8,
  'TexCoord2iv': 4,
  'TexCoord2s': 8,
  'TexCoord2sv': 4,
  'TexCoord3d': 24,
  'TexCoord3dv': 4,
  'TexCoord3f': 12,
  'TexCoord3fv': 4,
  'TexCoord3i': 12,
  'TexCoord3iv': 4,
  'TexCoord3s': 12,
  'TexCoord3sv': 4,
  'TexCoord4d': 32,
  'TexCoord4dv': 4,
  'TexCoord4f': 16,
  'TexCoord4fv': 4,
  'TexCoord4i': 16,
  'TexCoord4iv': 4,
  'TexCoord4s': 16,
  'TexCoord4sv': 4,
  'TexCoordPointer': 16,
  'TexEnvf': 12,
  'TexEnvfv': 12,
  'TexEnvi': 12,
  'TexEnviv': 12,
  'TexGend': 16,
  'TexGendv': 12,
  'TexGenf': 12,
  'TexGenfv': 12,
  'TexGeni': 12,
  'TexGeniv': 12,
  'TexImage1D': 32,
  'TexImage2D': 36,
  'TexParameterf': 12,
  'TexParameterfv': 12,
  'TexParameteri': 12,
  'TexParameteriv': 12,
  'TexSubImage1D': 28,
  'TexSubImage2D': 36,
  'Translated': 24,
  'Translatef': 12,
  'Vertex2d': 16,
  'Vertex2dv': 4,
  'Vertex2f': 8,
  'Vertex2fv': 4,
  'Vertex2i': 8,
  'Vertex2iv': 4,
  'Vertex2s': 8,
  'Vertex2sv': 4,
  'Vertex3d': 24,
  'Vertex3dv': 4,
  'Vertex3f': 12,
  'Vertex3fv': 4,
  'Vertex3i': 12,
  'Vertex3iv': 4,
  'Vertex3s': 12,
  'Vertex3sv': 4,
  'Vertex4d': 32,
  'Vertex4dv': 4,
  'Vertex4f': 16,
  'Vertex4fv': 4,
  'Vertex4i': 16,
  'Vertex4iv': 4,
  'Vertex4s': 16,
  'Vertex4sv': 4,
  'VertexPointer': 16,
  'Viewport': 16,
  'wglChoosePixelFormat': 8,
  'wglCopyContext': 12,
  'wglCreateContext': 4,
  'wglCreateLayerContext': 8,
  'wglDeleteContext': 4,
  'wglDescribeLayerPlane': 20,
  'wglDescribePixelFormat': 16,
  'wglGetCurrentContext': 0,
  'wglGetCurrentDC': 0,
  'wglGetDefaultProcAddress': 4,
  'wglGetLayerPaletteEntries': 20,
  'wglGetPixelFormat': 4,
  'wglGetProcAddress': 4,
  'wglMakeCurrent': 8,
  'wglRealizeLayerPalette': 12,
  'wglSetLayerPaletteEntries': 20,
  'wglSetPixelFormat': 12,
  'wglShareLists': 8,
  'wglSwapBuffers': 4,
  'wglSwapLayerBuffers': 8,
  'wglSwapMultipleBuffers': 8,
  'wglUseFontBitmapsA': 16,
  'wglUseFontBitmapsW': 16,
  'wglUseFontOutlinesA': 32,
  'wglUseFontOutlinesW': 32
}

keys = gl_mapping.keys()
keys.sort();
for func_name in keys:
	if stub_common.FindSpecial( 'noexport', func_name ):
		continue
	try:
		print "gl%s@%d = cr_gl%s" % (func_name,stack_sizes[func_name],func_name)
	except KeyError:
		pass

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
    print "%s@%d = %s_prox" % (func_name,stack_sizes[func_name],func_name)

print "crCreateContext"
print "crMakeCurrent"
print "crSwapBuffers"
print "crGetProcAddress"
#print "DllMain"
