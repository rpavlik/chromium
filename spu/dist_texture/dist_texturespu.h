/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef DIST_TEXTURE_SPU_H
#define DIST_TEXTURE_SPU_H

#ifdef WINDOWS
#define DIST_TEXTURESPU_APIENTRY __stdcall
#else
#define DIST_TEXTURESPU_APIENTRY
#endif

#include "cr_spu.h"

typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super;
} Dist_textureSPU;

extern Dist_textureSPU dist_texture_spu;

extern SPUNamedFunctionTable _cr_dist_texture_table[];

extern SPUOptions dist_textureSPUOptions[];

extern void dist_texturespuGatherConfiguration( void );

extern void DIST_TEXTURESPU_APIENTRY dist_textureTexImage2D( 
	GLenum target, GLint level, GLint internalformat,
	GLsizei width, GLsizei height, GLint border,
	GLenum format, GLenum type, const GLvoid *pixels ) ;

#endif /* DIST_TEXTURE_SPU_H */
