#include "cr_pixeldata.h"
#include "cr_error.h"
#include "cr_opengl_types.h"
#include "cr_opengl_enums.h"

#include <memory.h>

int crPixelSize (GLenum format, GLenum type, GLsizei width, GLsizei height )
{
	int pixelbytes = width*height;

	switch (format) {
		case GL_COLOR_INDEX :
		case GL_STENCIL_INDEX :
		case GL_DEPTH_COMPONENT :
		case GL_RED :
		case GL_GREEN :
		case GL_BLUE :
		case GL_ALPHA :
		case GL_LUMINANCE :
			break;
		case GL_LUMINANCE_ALPHA :
			pixelbytes *= 2;
			break;
		case GL_RGB :
		case GL_BGR_EXT:
			pixelbytes *= 3;
			break;
		case GL_RGBA :
		case GL_BGRA_EXT :
			pixelbytes *= 4;
			break;
		default:
			crError( "Unknown pixel format in crPixelSize: %d", format );
	}

	switch (type) {
		case GL_UNSIGNED_BYTE:
		case GL_BYTE:
			break;
		case GL_BITMAP:
			pixelbytes = width*height/8;
			if((width*height)%8)
				pixelbytes ++;
			break;
		case GL_UNSIGNED_SHORT:
		case GL_SHORT:
			pixelbytes *= 2;
			break;
		case GL_UNSIGNED_INT:
		case GL_INT:
		case GL_FLOAT:
			pixelbytes *= 4;
			break;
		default: 
			crError( "Unknown pixel type in crPixelSize: %d", format );
	}

	return pixelbytes;
}

void crPixelCopy1D( GLvoid *dstptr, const GLvoid *srcptr, GLenum format,
		GLenum type, GLsizei width, CRPackState *packstate )
{
	crPixelCopy2D( dstptr, srcptr, format, type, width, 1, packstate );
}

void crPixelCopy2D( GLvoid *dstptr, const GLvoid *srcptr, GLenum format,
		GLenum type, GLsizei width, GLsizei height, CRPackState *packstate )
{
	const char *src = (const char *) srcptr;
	char *dst = (char *) dstptr;
	int pixelbytes = 1;
	int rowwidth = width;
	int rowbytes;
	int subrowbytes;
	int i;
	int is_bitmap = 0;

	switch (format) {
		case GL_COLOR_INDEX :
		case GL_STENCIL_INDEX :
		case GL_DEPTH_COMPONENT :
		case GL_RED :
		case GL_GREEN :
		case GL_BLUE :
		case GL_ALPHA :
		case GL_LUMINANCE :
			break;
		case GL_LUMINANCE_ALPHA :
			pixelbytes *= 2;
			break;
		case GL_RGB :
		case GL_BGR_EXT:
			pixelbytes *= 3;
			break;
		case GL_RGBA :
		case GL_BGRA_EXT :
			pixelbytes *= 4;
			break;
		default:
			crError( "Unknown format in crPixelCopy2D: %d", format );
	}

	switch (type) {
		case GL_UNSIGNED_BYTE:
		case GL_BYTE:
			break;
		case GL_BITMAP:
			is_bitmap = 1;
			break;
		case GL_UNSIGNED_SHORT:
		case GL_SHORT:
			pixelbytes *= 2;
			break;
		case GL_UNSIGNED_INT:
		case GL_INT:
		case GL_FLOAT:
			pixelbytes *= 4;
			break;
		default: 
			crError( "Unknown type in crPixelCopy2D: %d", type );
	}

	/* width and height */
	if (packstate->rowLength > 0)
		rowwidth = packstate->rowLength;

	rowbytes = rowwidth*pixelbytes;
	subrowbytes = width*pixelbytes;

	/* handle the  alignment */
	i = ((int)src)%packstate->alignment;
	if (i) 
		src += packstate->alignment - i;

	/* handle skip rows */
	src += packstate->skipRows*rowbytes;

	/* handle skip pixels */
	src += packstate->skipPixels*pixelbytes;

	if (packstate->LSBFirst)
		crError( "Sorry, no lsbfirst for you" );
	if (packstate->swapBytes)
		crError( "Sorry, no swapbytes for you" );

	if (is_bitmap) {
		src = ((const char *)srcptr) + ((int)(((const char *)srcptr) - src))/8;
		for (i=0; i<height; i++) {
			memcpy ((void *) dst, (const void *) src, subrowbytes);
			dst+=subrowbytes/8;
			src+=rowbytes/8;
		} 
	}
	else
	{
		for (i=0; i<height; i++) {
			memcpy ((void *) dst, (const void *) src, subrowbytes);
			dst+=subrowbytes;
			src+=rowbytes;
		}
	}
}
