/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_VERSION_H
#define CR_VERSION_H


/* These define the Chromium release number.
 * Not sure what to do for alpha/beta releases.
 */
#define CR_MAJOR_VERSION 0
#define CR_MINOR_VERSION 1
#define CR_PATCH_VERSION 0


/* These define the OpenGL version that Chromium supports.
 * This lets users easily recompile Chromium with/without OpenGL 1.x support.
 * We use OpenGL's GL_VERSION_1_x convention.
 */
#define CR_OPENGL_VERSION_1_0 1
#define CR_OPENGL_VERSION_1_1 1
/*#define CR_OPENGL_VERSION_1_2 1   not yet 1.2 */


/* These define the OpenGL extensions that Chromium supports.
 * Users can enable/disable support for particular OpenGL extensions here.
 * Again, use OpenGL's convention.
 * WARNING: if you add new extensions here, also update spu/loader/limits.c
 * and spu/state_tracker/extensions.c
 */
#define CR_ARB_multitexture 1
#define CR_ARB_texture_border_clamp 1
#define CR_ARB_texture_cube_map 1
#define CR_EXT_blend_color 1
#define CR_EXT_blend_minmax 1
#define CR_EXT_blend_subtract 1
#define CR_EXT_secondary_color 1
#define CR_EXT_separate_specular_color 1
#define CR_EXT_texture_cube_map 1
#define CR_EXT_texture_edge_clamp 1
#define CR_EXT_texture_filter_anisotropic 1
#define CR_EXT_texture_object 1
#define CR_NV_fog_distance 1
#define CR_NV_register_combiners 1
#define CR_NV_register_combiners2 1
#define CR_NV_texgen_reflection 1
#define CR_SGIS_texture_border_clamp 1
#define CR_SGIS_texture_edge_clamp 1
/*#define CR_ARB_imaging 1    not yet */
/*#define CR_NV_vertex_program 1    not yet */


#endif /* CR_VERSION_H */
