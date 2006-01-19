/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */
	

#include <stdio.h>
#include "printspu.h"
#include "cr_mem.h"

const char *printspuListToStr(GLsizei n, GLenum type, const GLvoid *list)
{
	static unsigned int bufSize = 0;
	static char *buf = NULL;
	char *dest;
	int numBytesPrinted;
	register GLsizei i;

	/* Make sure we have enough buffer space to go around, based on 
	 * the largest type we'll see (GL_UNSIGNED_INT).  Each byte of
	 * the type could require up to 4 characters to represent the
	 * byte itself (e.g. "-127"), plus a comma and a space, plus
	 * brackets at the beginning and end, and a terminating nul.
	 */

	if (bufSize < n*(sizeof(GL_UNSIGNED_INT))*6 + 3) {
	    bufSize = n*(sizeof(GL_UNSIGNED_INT))*6 + 3;
	    if (buf != NULL) {
		crFree(buf);
	    }
	    buf = crAlloc(bufSize);
	    if (buf == NULL) {
		bufSize = 0;
		return "[buffer allocation error]";
	    }
	}

	/* Start at the top, and create a string. */
	dest = buf;
	*dest++ = '[';

	/* We'll have to take separate action based on the type we have. */
	switch(type) {
	    case GL_BYTE: {
		GLbyte *src = (GLbyte *)list;
		for (i = 0; i < n; i++) {
		    if (i > 0) {
			*dest++ = ',';
			*dest++ = ' ';
		    }
		    numBytesPrinted = sprintf(dest, "%d", *src++);
		    dest += numBytesPrinted;
		}
	    }
	    break;

	    case GL_UNSIGNED_BYTE: {
		GLubyte *src = (GLubyte *)list;
		for (i = 0; i < n; i++) {
		    if (i > 0) {
			*dest++ = ',';
			*dest++ = ' ';
		    }
		    numBytesPrinted = sprintf(dest, "%u", *src++);
		    dest += numBytesPrinted;
		}
	    }
	    break;

	    case GL_SHORT: {
		GLshort *src = (GLshort *)list;
		for (i = 0; i < n; i++) {
		    if (i > 0) {
			*dest++ = ',';
			*dest++ = ' ';
		    }
		    numBytesPrinted = sprintf(dest, "%d", *src++);
		    dest += numBytesPrinted;
		}
	    }
	    break;

	    case GL_UNSIGNED_SHORT: {
		GLushort *src = (GLushort *)list;
		for (i = 0; i < n; i++) {
		    if (i > 0) {
			*dest++ = ',';
			*dest++ = ' ';
		    }
		    numBytesPrinted = sprintf(dest, "%u", *src++);
		    dest += numBytesPrinted;
		}
	    }
	    break;

	    case GL_INT: {
		GLint *src = (GLint *)list;
		for (i = 0; i < n; i++) {
		    if (i > 0) {
			*dest++ = ',';
			*dest++ = ' ';
		    }
		    numBytesPrinted = sprintf(dest, "%d", *src++);
		    dest += numBytesPrinted;
		}
	    }
	    break;

	    case GL_UNSIGNED_INT: {
		GLuint *src = (GLuint *)list;
		for (i = 0; i < n; i++) {
		    if (i > 0) {
			*dest++ = ',';
			*dest++ = ' ';
		    }
		    numBytesPrinted = sprintf(dest, "%u", *src++);
		    dest += numBytesPrinted;
		}
	    }
	    break;

	    case GL_FLOAT: {
		GLfloat *src = (GLfloat *)list;
		for (i = 0; i < n; i++) {
		    if (i > 0) {
			*dest++ = ',';
			*dest++ = ' ';
		    }
		    numBytesPrinted = sprintf(dest, "%f", *src++);
		    dest += numBytesPrinted;
		}
	    }
	    break;

	    case GL_2_BYTES: {
		unsigned char *src = (unsigned char *)list;
		for (i = 0; i < n; i++) {
		    if (i > 0) {
			*dest++ = ',';
			*dest++ = ' ';
		    }
		    numBytesPrinted = sprintf(dest, "%u", src[0]*256 + src[1]);
		    src += 2;
		    dest += numBytesPrinted;
		}
	    }
	    break;

	    case GL_3_BYTES: {
		unsigned char *src = (unsigned char *)list;
		for (i = 0; i < n; i++) {
		    if (i > 0) {
			*dest++ = ',';
			*dest++ = ' ';
		    }
		    numBytesPrinted = sprintf(dest, "%u", src[0]*256*256 + src[1]*256 + src[2]);
		    src += 3;
		    dest += numBytesPrinted;
		}
	    }
	    break;

	    case GL_4_BYTES: {
		unsigned char *src = (unsigned char *)list;
		for (i = 0; i < n; i++) {
		    if (i > 0) {
			*dest++ = ',';
			*dest++ = ' ';
		    }
		    numBytesPrinted = sprintf(dest, "%u", src[0]*256*256*256 + src[1]*256*256 + src[2]*256 + src[3]);
		    src += 4;
		    dest += numBytesPrinted;
		}
	    }
	    break;

	    default:
		return "[unknown type]";
		break;
	}

	/* Add the terminating bracket and nil, and we're done. */
	*dest++ = ']';
	*dest++ = '\0';

	return buf;
}

void PRINT_APIENTRY printCallLists( GLsizei n, GLenum type, const GLvoid * lists )
{
	fprintf( print_spu.fp, "CallLists( %u, %s, %s )\n", (unsigned) n, printspuEnumToStr( type ), printspuListToStr(n, type, lists));
	fflush( print_spu.fp );
	print_spu.passthrough.CallLists( n, type, lists );
}
