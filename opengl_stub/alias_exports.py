# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

aliases = [ 		('ActiveTexture','ActiveTextureARB'),
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
			('MultiTexCoord4sv','MultiTexCoord4svARB') ]

def AliasMap( func_name ):
    for aliased_index in range(len(aliases)):
	(aliased_func_name, real_func_name) = aliases[aliased_index]
	if real_func_name == func_name:
		return aliased_func_name;
