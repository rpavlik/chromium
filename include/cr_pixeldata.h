#ifndef CR_PIXELDATA_H
#define CR_PIXELDATA_H

#include "cr_opengl_types.h"
#include "state/cr_pixel.h"

int crPixelSize( GLenum format, GLenum type, GLsizei width, GLsizei height );
void crPixelCopy1D( GLvoid *dst, const GLvoid *src, GLenum format, 
		                GLenum type, GLsizei width, CRPackState *packstate );
void crPixelCopy2D( GLvoid *dst, const GLvoid *src, GLenum format, 
		                GLenum type, GLsizei width, GLsizei height,
										CRPackState *packstate );

#endif /* CR_PIXELDATA_H */
