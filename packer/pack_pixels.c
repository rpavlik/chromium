#include "packer.h"
#include "cr_glwrapper.h"
#include "cr_pixeldata.h"
#include "cr_error.h"

#include "state/cr_pixel.h"

void PACK_APIENTRY crPackDrawPixels( GLsizei width, GLsizei height, 
		GLenum format, GLenum type, const GLvoid *pixels, CRPackState *packstate )
{
	crError( "Unimplemented crPackDrawPixels" );

	(void) width;
	(void) height;
	(void) format;
	(void) type;
	(void) pixels;
	(void) packstate;
}

void PACK_APIENTRY crPackReadPixels( GLint x, GLint y, GLsizei width, 
		GLsizei height, GLenum format, GLenum type, GLvoid *pixels )
{
	crError( "Unimplemented crPackReadPixels" );

	(void) x;
	(void) y;
	(void) width;
	(void) height;
	(void) format;
	(void) type;
	(void) pixels;
}

void PACK_APIENTRY crPackBitmap( GLsizei width, GLsizei height, 
		GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove,
		const GLubyte *bitmap )
{
	crError( "Unimplemented crPackDrawPixels" );

	(void) width;
	(void) height;
	(void) xorig;
	(void) yorig;
	(void) xmove;
	(void) ymove;
	(void) bitmap;
}
