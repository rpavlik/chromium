#include "cr_pack.h"
#include "cr_opengl_types.h"
#include "cr_opengl_enums.h"
#include "cr_pixeldata.h"
#include "cr_error.h"

void PACK_APIENTRY crPackDrawPixels( GLsizei width, GLsizei height, 
		GLenum format, GLenum type, const GLvoid *pixels )
{
	crError( "Unimplemented crPackDrawPixels" );

	(void) width;
	(void) height;
	(void) format;
	(void) type;
	(void) pixels;
}

void PACK_APIENTRY crPackReadPixels( GLint x, GLint y, GLsizei width, 
		GLsizei height, GLenum format, GLenum type, const GLvoid *pixels )
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
