/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_PIXELDATA_H
#define CR_PIXELDATA_H

#include "cr_glwrapper.h"
#include "state/cr_pixel.h"

int crPixelSize( GLenum format, GLenum type, GLsizei width, GLsizei height );
void crPixelCopy1D( GLvoid *dst, const GLvoid *src, GLenum format, 
		                GLenum type, GLsizei width, CRPackState *packstate );
void crPixelCopy2D( GLvoid *dst, const GLvoid *src, GLenum format, 
		                GLenum type, GLsizei width, GLsizei height,
										CRPackState *packstate );

#endif /* CR_PIXELDATA_H */
