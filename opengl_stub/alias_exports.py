# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

aliases = [
	# GL_ARB_multitexture / OpenGL 1.2.1
	('ActiveTexture','ActiveTextureARB'),
	('ClientActiveTexture','ClientActiveTextureARB'),
	('MultiTexCoord1d', 'MultiTexCoord1dARB'),
	('MultiTexCoord1dv','MultiTexCoord1dvARB'),
	('MultiTexCoord1f', 'MultiTexCoord1fARB'),
	('MultiTexCoord1fv','MultiTexCoord1fvARB'),
	('MultiTexCoord1i', 'MultiTexCoord1iARB'),
	('MultiTexCoord1iv','MultiTexCoord1ivARB'),
	('MultiTexCoord1s', 'MultiTexCoord1sARB'),
	('MultiTexCoord1sv','MultiTexCoord1svARB'),
	('MultiTexCoord2d', 'MultiTexCoord2dARB'),
	('MultiTexCoord2dv','MultiTexCoord2dvARB'),
	('MultiTexCoord2f', 'MultiTexCoord2fARB'),
	('MultiTexCoord2fv','MultiTexCoord2fvARB'),
	('MultiTexCoord2i', 'MultiTexCoord2iARB'),
	('MultiTexCoord2iv','MultiTexCoord2ivARB'),
	('MultiTexCoord2s', 'MultiTexCoord2sARB'),
	('MultiTexCoord2sv','MultiTexCoord2svARB'),
	('MultiTexCoord3d', 'MultiTexCoord3dARB'),
	('MultiTexCoord3dv','MultiTexCoord3dvARB'),
	('MultiTexCoord3f', 'MultiTexCoord3fARB'),
	('MultiTexCoord3fv','MultiTexCoord3fvARB'),
	('MultiTexCoord3i', 'MultiTexCoord3iARB'),
	('MultiTexCoord3iv','MultiTexCoord3ivARB'),
	('MultiTexCoord3s', 'MultiTexCoord3sARB'),
	('MultiTexCoord3sv','MultiTexCoord3svARB'),
	('MultiTexCoord4d', 'MultiTexCoord4dARB'),
	('MultiTexCoord4dv','MultiTexCoord4dvARB'),
	('MultiTexCoord4f', 'MultiTexCoord4fARB'),
	('MultiTexCoord4fv','MultiTexCoord4fvARB'),
	('MultiTexCoord4i', 'MultiTexCoord4iARB'),
	('MultiTexCoord4iv','MultiTexCoord4ivARB'),
	('MultiTexCoord4s', 'MultiTexCoord4sARB'),
	('MultiTexCoord4sv','MultiTexCoord4svARB'),
	# GL_ARB_transpose_matrix / OpenGL 1.3
	('LoadTransposeMatrixf','LoadTransposeMatrixfARB'),
	('LoadTransposeMatrixd','LoadTransposeMatrixdARB'),
	('MultTransposeMatrixf','MultTransposeMatrixfARB'),
	('MultTransposeMatrixd','MultTransposeMatrixdARB'),
	# GL_ARB_texture_compression / OpenGL 1.3
	('CompressedTexImage3D', 'CompressedTexImage3DARB'),
	('CompressedTexImage2D', 'CompressedTexImage2DARB'),
	('CompressedTexImage1D', 'CompressedTexImage1DARB'),
	('CompressedTexSubImage3D', 'CompressedTexSubImage3DARB'),
	('CompressedTexSubImage2D', 'CompressedTexSubImage2DARB'),
	('CompressedTexSubImage1D', 'CompressedTexSubImage1DARB'),
	('GetCompressedTexImage', 'GetCompressedTexImageARB'),
	# GL_ARB_multisample / OpenGL 1.3
	('SampleCoverage', 'SampleCoverageARB'),
	# GL_ARB_window_pos / OpenGL 1.4
	('WindowPos2d', 'WindowPos2dARB'),
	('WindowPos2dv', 'WindowPos2dvARB'),
	('WindowPos2f', 'WindowPos2fARB'),
	('WindowPos2fv', 'WindowPos2fvARB'),
	('WindowPos2i', 'WindowPos2iARB'),
	('WindowPos2iv', 'WindowPos2ivARB'),
	('WindowPos2s', 'WindowPos2sARB'),
	('WindowPos2sv', 'WindowPos2svARB'),
	('WindowPos3d', 'WindowPos3dARB'),
	('WindowPos3dv', 'WindowPos3dvARB'),
	('WindowPos3f', 'WindowPos3fARB'),
	('WindowPos3fv', 'WindowPos3fvARB'),
	('WindowPos3i', 'WindowPos3iARB'),
	('WindowPos3iv', 'WindowPos3ivARB'),
	('WindowPos3s', 'WindowPos3sARB'),
	('WindowPos3sv', 'WindowPos3svARB'),
	# GL_ARB_point_parameter / OpenGL 1.4
	('PointParameterf', 'PointParameterfARB'),
	('PointParameterfv', 'PointParameterfvARB'),
	# GL_EXT_blend_color / OpenGL 1.4
	('BlendColor', 'BlendColorEXT'),
	# GL_EXT_blend_function_separate / OpenGL 1.4
	('BlendFuncSeparate', 'BlendFuncSeparateEXT'),
	# GL_EXT_blend_equation / OpenGL 1.4
	('glBlendEquationEXT', 'glBlendEquation'),
	# GL_EXT_multidraw_arrays / OpenGL 1.4
	('MultiDrawArraysEXT', 'MultiDrawArrays'),
	('MultiDrawElementsEXT', 'MultiDrawElements'),
	# GL_EXT_secondary_color / OpenGL 1.4
	('SecondaryColor3bEXT', 'SecondaryColor3b'),
	('SecondaryColor3bvEXT', 'SecondaryColor3bv'),
	('SecondaryColor3dEXT', 'SecondaryColor3d'),
	('SecondaryColor3dvEXT', 'SecondaryColor3dv'),
	('SecondaryColor3fEXT', 'SecondaryColor3f'),
	('SecondaryColor3fvEXT', 'SecondaryColor3fv'),
	('SecondaryColor3iEXT', 'SecondaryColor3i'),
	('SecondaryColor3ivEXT', 'SecondaryColor3iv'),
	('SecondaryColor3sEXT', 'SecondaryColor3s'),
	('SecondaryColor3svEXT', 'SecondaryColor3sv'),
	('SecondaryColor3ubEXT', 'SecondaryColor3ub'),
	('SecondaryColor3ubvEXT', 'SecondaryColor3ubv'),
	('SecondaryColor3uiEXT', 'SecondaryColor3ui'),
	('SecondaryColor3uivEXT', 'SecondaryColor3uiv'),
	('SecondaryColor3usEXT', 'SecondaryColor3us'),
	('SecondaryColor3usvEXT', 'SecondaryColor3usv'),
	('SecondaryColorPointerEXT', 'SecondaryColorPointer'),
	# GL_EXT_fog_coord / OpenGL 1.4
	('FogCoordfEXT', 'FogCoordf'),
	('FogCoordfvEXT', 'FogCoordfv'),
	('FogCoorddEXT', 'FogCoordd'),
	('FogCoorddvEXT', 'FogCoorddv'),
	('FogCoordPointerEXT', 'FogCoordPointer'),
]

def AliasMap( func_name ):
	for aliased_index in range(len(aliases)):
		(aliased_func_name, real_func_name) = aliases[aliased_index]
		if real_func_name == func_name:
			return aliased_func_name;
	return None
