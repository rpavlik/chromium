/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_pixeldata.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_version.h"

#include <memory.h>
#include <stdio.h>

/*
 * Return bytes per pixel for the given format/type combination.
 */
int crPixelSize( GLenum format, GLenum type )
{
	int bytes = 1; /* picky Windows compiler, we override later */

	switch (type) {
#ifdef CR_OPENGL_VERSION_1_2
		case GL_UNSIGNED_BYTE_3_3_2:
		case GL_UNSIGNED_BYTE_2_3_3_REV:
#endif
		case GL_UNSIGNED_BYTE:
		case GL_BYTE:
			bytes = 1;
			break;
		case GL_BITMAP:
			return 0;  /* special case */
#ifdef CR_OPENGL_VERSION_1_2
		case GL_UNSIGNED_SHORT_5_6_5:
		case GL_UNSIGNED_SHORT_5_6_5_REV:
		case GL_UNSIGNED_SHORT_5_5_5_1:
		case GL_UNSIGNED_SHORT_1_5_5_5_REV:
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_4_4_4_4_REV:
#endif
		case GL_UNSIGNED_SHORT:
		case GL_SHORT:
			bytes = 2;
			break;
#ifdef CR_OPENGL_VERSION_1_2
		case GL_UNSIGNED_INT_8_8_8_8:
		case GL_UNSIGNED_INT_8_8_8_8_REV:
		case GL_UNSIGNED_INT_10_10_10_2:
		case GL_UNSIGNED_INT_2_10_10_10_REV:
#endif
		case GL_UNSIGNED_INT:
		case GL_INT:
		case GL_FLOAT:
			bytes = 4;
			break;
		default: 
			crError( "Unknown pixel type in crPixelSize: 0x%x", type );
	}

	switch (format) {
		case GL_COLOR_INDEX:
		case GL_STENCIL_INDEX:
		case GL_DEPTH_COMPONENT:
		case GL_RED:
		case GL_GREEN:
		case GL_BLUE:
		case GL_ALPHA:
		case GL_LUMINANCE:
		case GL_INTENSITY:
			break;
		case GL_LUMINANCE_ALPHA:
			bytes *= 2;
			break;
		case GL_RGB:
#ifdef CR_OPENGL_VERSION_1_2
		case GL_BGR:
#endif
			bytes *= 3;
			break;
		case GL_RGBA:
#ifdef GL_ABGR_EXT
		case GL_ABGR_EXT:
#endif
#ifdef CR_OPENGL_VERSION_1_2
		case GL_BGRA:
#endif
			bytes *= 4;
			break;
		default:
			crError( "Unknown pixel format in crPixelSize: 0x%x", format );
	}

	return bytes;
}


#define BYTE_TO_FLOAT(b)  ((b) * (1.0/127.0))
#define FLOAT_TO_BYTE(f)  ((GLbyte) ((f) * 127.0))

#define UBYTE_TO_FLOAT(b)  ((b) * (1.0/255.0))
#define FLOAT_TO_UBYTE(f)  ((GLbyte) ((f) * 255.0))

#define SHORT_TO_FLOAT(s)  ((s) * (1.0/32768.0))
#define FLOAT_TO_SHORT(f)  ((GLshort) ((f) * 32768.0))

#define USHORT_TO_FLOAT(s)  ((s) * (1.0/65535.0))
#define FLOAT_TO_USHORT(f)  ((GLushort) ((f) * 65535.0))

#define INT_TO_FLOAT(i)  ((i) * (1.0F/2147483647.0))
#define FLOAT_TO_INT(f)  ((GLint) ((f) * 2147483647.0))

#define UINT_TO_FLOAT(i)  ((i) * (1.0F / 4294967295.0F))
#define FLOAT_TO_UINT(f)  ((GLuint) ((f) * 4294967295.0))



/*
 * Pack src pixel data into tmpRow array as either GLfloat[][1] or
 * GLfloat[][4] depending on whether the format is for colors.
 */
static void get_row(const char *src, GLenum srcFormat, GLenum srcType,
										GLsizei width, GLfloat *tmpRow)
{
	const GLbyte *bSrc = (GLbyte *) src;
	const GLubyte *ubSrc = (GLubyte *) src;
	const GLshort *sSrc = (GLshort *) src;
	const GLushort *usSrc = (GLushort *) src;
	const GLint *iSrc = (GLint *) src;
	const GLuint *uiSrc = (GLuint *) src;
	const GLfloat *fSrc = (GLfloat *) src;
	const GLdouble *dSrc = (GLdouble *) src;
	int i;

	switch (srcFormat) {
		case GL_COLOR_INDEX:
		case GL_STENCIL_INDEX:
			switch (srcType) {
				case GL_BYTE:
					for (i = 0; i < width; i++)
						tmpRow[i] = (GLfloat) bSrc[i];
					break;
				case GL_UNSIGNED_BYTE:
					for (i = 0; i < width; i++)
						tmpRow[i] = (GLfloat) ubSrc[i];
					break;
				case GL_SHORT:
					for (i = 0; i < width; i++)
						tmpRow[i] = (GLfloat) sSrc[i];
					break;
				case GL_UNSIGNED_SHORT:
					for (i = 0; i < width; i++)
						tmpRow[i] = (GLfloat) usSrc[i];
					break;
				case GL_INT:
					for (i = 0; i < width; i++)
						tmpRow[i] = (GLfloat) iSrc[i];
					break;
				case GL_UNSIGNED_INT:
					for (i = 0; i < width; i++)
						tmpRow[i] = (GLfloat) uiSrc[i];
					break;
				case GL_FLOAT:
					for (i = 0; i < width; i++)
						tmpRow[i] = fSrc[i];
					break;
				case GL_DOUBLE:
					for (i = 0; i < width; i++)
						tmpRow[i] = (GLfloat) dSrc[i];
					break;
				default:
					crError("unexpected type in get_row in pixel.c");
			}
			break;
		case GL_DEPTH_COMPONENT:
			switch (srcType) {
				case GL_BYTE:
					for (i = 0; i < width; i++)
						tmpRow[i] = (GLfloat) BYTE_TO_FLOAT(bSrc[i]);
					break;
				case GL_UNSIGNED_BYTE:
					for (i = 0; i < width; i++)
						tmpRow[i] = (GLfloat) UBYTE_TO_FLOAT(ubSrc[i]);
					break;
				case GL_SHORT:
					for (i = 0; i < width; i++)
						tmpRow[i] = (GLfloat) SHORT_TO_FLOAT(sSrc[i]);
					break;
				case GL_UNSIGNED_SHORT:
					for (i = 0; i < width; i++)
						tmpRow[i] = (GLfloat) USHORT_TO_FLOAT(usSrc[i]);
					break;
				case GL_INT:
					for (i = 0; i < width; i++)
						tmpRow[i] = (GLfloat) INT_TO_FLOAT(bSrc[i]);
					break;
				case GL_UNSIGNED_INT:
					for (i = 0; i < width; i++)
						tmpRow[i] = (GLfloat) UINT_TO_FLOAT(bSrc[i]);
					break;
				case GL_FLOAT:
					for (i = 0; i < width; i++)
						tmpRow[i] = fSrc[i];
					break;
				case GL_DOUBLE:
					for (i = 0; i < width; i++)
						tmpRow[i] = (GLfloat) dSrc[i];
					break;
				default:
					crError("unexpected type in get_row in pixel.c");
			}
			break;
		case GL_RED:
		case GL_GREEN:
		case GL_BLUE:
		case GL_ALPHA:
			{
				int dst;
				if (srcFormat == GL_RED)
					dst = 0;
				else if (srcFormat == GL_GREEN)
					dst = 1;
				else if (srcFormat == GL_BLUE)
					dst = 2;
				else
					dst = 3;
				for (i = 0; i < width; i++) {
					tmpRow[i*4+0] = 0.0;
					tmpRow[i*4+1] = 0.0;
					tmpRow[i*4+2] = 0.0;
					tmpRow[i*4+3] = 1.0;
				}
				switch (srcType) {
					case GL_BYTE:
						for (i = 0; i < width; i++, dst += 4)
							tmpRow[dst] = (GLfloat) BYTE_TO_FLOAT(bSrc[i]);
						break;
					case GL_UNSIGNED_BYTE:
						for (i = 0; i < width; i++, dst += 4)
							tmpRow[dst] = (GLfloat) UBYTE_TO_FLOAT(ubSrc[i]);
						break;
					case GL_SHORT:
						for (i = 0; i < width; i++, dst += 4)
							tmpRow[dst] = (GLfloat) SHORT_TO_FLOAT(sSrc[i]);
						break;
					case GL_UNSIGNED_SHORT:
						for (i = 0; i < width; i++, dst += 4)
							tmpRow[dst] = (GLfloat) USHORT_TO_FLOAT(usSrc[i]);
						break;
					case GL_INT:
						for (i = 0; i < width; i++, dst += 4)
							tmpRow[dst] = (GLfloat) INT_TO_FLOAT(iSrc[i]);
						break;
					case GL_UNSIGNED_INT:
						for (i = 0; i < width; i++, dst += 4)
							tmpRow[dst] = (GLfloat) UINT_TO_FLOAT(uiSrc[i]);
						break;
					case GL_FLOAT:
						for (i = 0; i < width; i++, dst += 4)
							tmpRow[dst] = fSrc[i];
						break;
					case GL_DOUBLE:
						for (i = 0; i < width; i++, dst += 4)
							tmpRow[dst] = (GLfloat) fSrc[i];
						break;
					default:
						crError("unexpected type in get_row in pixel.c");
				}
			}
			break;
		case GL_LUMINANCE:
			switch (srcType) {
				case GL_BYTE:
					for (i = 0; i < width; i++) {
						tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
							= (GLfloat) BYTE_TO_FLOAT(bSrc[i]);
						tmpRow[i*4+3] = 1.0;
					}
					break;
				case GL_UNSIGNED_BYTE:
					for (i = 0; i < width; i++) {
						tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
							= (GLfloat) UBYTE_TO_FLOAT(ubSrc[i]);
						tmpRow[i*4+3] = 1.0;
					}
					break;
				case GL_SHORT:
					for (i = 0; i < width; i++) {
						tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
							= (GLfloat) SHORT_TO_FLOAT(sSrc[i]);
						tmpRow[i*4+3] = 1.0;
					}
					break;
				case GL_UNSIGNED_SHORT:
					for (i = 0; i < width; i++) {
						tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
							= (GLfloat) USHORT_TO_FLOAT(usSrc[i]);
						tmpRow[i*4+3] = 1.0;
					}
					break;
				case GL_INT:
					for (i = 0; i < width; i++) {
						tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
							= (GLfloat) INT_TO_FLOAT(iSrc[i]);
						tmpRow[i*4+3] = 1.0;
					}
					break;
				case GL_UNSIGNED_INT:
					for (i = 0; i < width; i++) {
						tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
							= (GLfloat) UINT_TO_FLOAT(uiSrc[i]);
						tmpRow[i*4+3] = 1.0;
					}
					break;
				case GL_FLOAT:
					for (i = 0; i < width; i++) {
						tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]	= fSrc[i];
						tmpRow[i*4+3] = 1.0;
					}
					break;
				case GL_DOUBLE:
					for (i = 0; i < width; i++) {
						tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2] = (GLfloat) dSrc[i];
						tmpRow[i*4+3] = 1.0;
					}
					break;
				default:
					crError("unexpected type in get_row in pixel.c");
			}
			break;
		case GL_INTENSITY:
			switch (srcType) {
				case GL_BYTE:
					for (i = 0; i < width; i++)
						tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2] = tmpRow[i*4+3]
							= (GLfloat) BYTE_TO_FLOAT(bSrc[i]);
					break;
				case GL_UNSIGNED_BYTE:
					for (i = 0; i < width; i++)
						tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2] = tmpRow[i*4+3]
							= (GLfloat) UBYTE_TO_FLOAT(ubSrc[i]);
					break;
				case GL_SHORT:
					for (i = 0; i < width; i++)
						tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2] = tmpRow[i*4+3]
							= (GLfloat) SHORT_TO_FLOAT(sSrc[i]);
					break;
				case GL_UNSIGNED_SHORT:
					for (i = 0; i < width; i++)
						tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2] = tmpRow[i*4+3]
							= (GLfloat) USHORT_TO_FLOAT(usSrc[i]);
					break;
				case GL_INT:
					for (i = 0; i < width; i++)
						tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2] = tmpRow[i*4+3]
							= (GLfloat) INT_TO_FLOAT(iSrc[i]);
					break;
				case GL_UNSIGNED_INT:
					for (i = 0; i < width; i++)
						tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2] = tmpRow[i*4+3]
							= UINT_TO_FLOAT(uiSrc[i]);
					break;
				case GL_FLOAT:
					for (i = 0; i < width; i++)
						tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2] = tmpRow[i*4+3]
							= fSrc[i];
					break;
				case GL_DOUBLE:
					for (i = 0; i < width; i++)
						tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2] = tmpRow[i*4+3]
							= (GLfloat) dSrc[i];
					break;
				default:
					crError("unexpected type in get_row in pixel.c");
			}
			break;
		case GL_LUMINANCE_ALPHA:
			switch (srcType) {
				case GL_BYTE:
					for (i = 0; i < width; i++) {
						tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
							= (GLfloat) BYTE_TO_FLOAT(bSrc[i*2+0]);
						tmpRow[i*4+3] = (GLfloat) BYTE_TO_FLOAT(bSrc[i*2+1]);
					}
					break;
				case GL_UNSIGNED_BYTE:
					for (i = 0; i < width; i++) {
						tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
							= (GLfloat) UBYTE_TO_FLOAT(ubSrc[i*2+0]);
						tmpRow[i*4+3] = (GLfloat) UBYTE_TO_FLOAT(ubSrc[i*2+1]);
					}
					break;
				case GL_SHORT:
					for (i = 0; i < width; i++) {
						tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
							= (GLfloat) SHORT_TO_FLOAT(sSrc[i*2+0]);
						tmpRow[i*4+3] = (GLfloat) SHORT_TO_FLOAT(sSrc[i*2+1]);
					}
					break;
				case GL_UNSIGNED_SHORT:
					for (i = 0; i < width; i++) {
						tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
							= (GLfloat) USHORT_TO_FLOAT(usSrc[i*2+0]);
						tmpRow[i*4+3] = (GLfloat) USHORT_TO_FLOAT(usSrc[i*2+1]);
					}
					break;
				case GL_INT:
					for (i = 0; i < width; i++) {
						tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
							= (GLfloat) INT_TO_FLOAT(iSrc[i*2+0]);
						tmpRow[i*4+3] = (GLfloat) INT_TO_FLOAT(iSrc[i*2+1]);
					}
					break;
				case GL_UNSIGNED_INT:
					for (i = 0; i < width; i++) {
						tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
							= (GLfloat) UINT_TO_FLOAT(uiSrc[i*2+0]);
						tmpRow[i*4+3] = (GLfloat) UINT_TO_FLOAT(uiSrc[i*2+1]);
					}
					break;
				case GL_FLOAT:
					for (i = 0; i < width; i++) {
						tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]	= fSrc[i*2+0];
						tmpRow[i*4+3] = fSrc[i*2+1];
					}
					break;
				case GL_DOUBLE:
					for (i = 0; i < width; i++) {
						tmpRow[i*4+0] = tmpRow[i*4+1] = tmpRow[i*4+2]
							= (GLfloat) dSrc[i*2+0];
						tmpRow[i*4+3] = (GLfloat) dSrc[i*2+1];
					}
					break;
				default:
					crError("unexpected type in get_row in pixel.c");
			}
			break;
		case GL_RGB:
#ifdef CR_OPENGL_VERSION_1_2
		case GL_BGR:
#endif
			{
				int r, b;
				if (srcFormat == GL_RGB) {
					r = 0; b = 2;
				}
				else {
					r = 2; b = 0;
				}
				switch (srcType) {
					case GL_BYTE:
						for (i = 0; i < width; i++) {
							tmpRow[i*4+0] = (GLfloat) BYTE_TO_FLOAT(bSrc[i*3+r]);
							tmpRow[i*4+1] = (GLfloat) BYTE_TO_FLOAT(bSrc[i*3+1]);
							tmpRow[i*4+2] = (GLfloat) BYTE_TO_FLOAT(bSrc[i*3+b]);
							tmpRow[i*4+3] = 1.0;
						}
						break;
					case GL_UNSIGNED_BYTE:
						for (i = 0; i < width; i++) {
							tmpRow[i*4+0] = (GLfloat) UBYTE_TO_FLOAT(ubSrc[i*3+r]);
							tmpRow[i*4+1] = (GLfloat) UBYTE_TO_FLOAT(ubSrc[i*3+1]);
							tmpRow[i*4+2] = (GLfloat) UBYTE_TO_FLOAT(ubSrc[i*3+b]);
							tmpRow[i*4+3] = 1.0;
						}
						break;
					case GL_SHORT:
						for (i = 0; i < width; i++) {
							tmpRow[i*4+0] = (GLfloat) SHORT_TO_FLOAT(sSrc[i*3+r]);
							tmpRow[i*4+1] = (GLfloat) SHORT_TO_FLOAT(sSrc[i*3+1]);
							tmpRow[i*4+2] = (GLfloat) SHORT_TO_FLOAT(sSrc[i*3+b]);
							tmpRow[i*4+3] = 1.0;
						}
						break;
					case GL_UNSIGNED_SHORT:
						for (i = 0; i < width; i++) {
							tmpRow[i*4+0] = (GLfloat) USHORT_TO_FLOAT(usSrc[i*3+r]);
							tmpRow[i*4+1] = (GLfloat) USHORT_TO_FLOAT(usSrc[i*3+1]);
							tmpRow[i*4+2] = (GLfloat) USHORT_TO_FLOAT(usSrc[i*3+b]);
							tmpRow[i*4+3] = 1.0;
						}
						break;
					case GL_INT:
						for (i = 0; i < width; i++) {
							tmpRow[i*4+0] = (GLfloat) INT_TO_FLOAT(iSrc[i*3+r]);
							tmpRow[i*4+1] = (GLfloat) INT_TO_FLOAT(iSrc[i*3+1]);
							tmpRow[i*4+2] = (GLfloat) INT_TO_FLOAT(iSrc[i*3+b]);
							tmpRow[i*4+3] = 1.0;
						}
						break;
					case GL_UNSIGNED_INT:
						for (i = 0; i < width; i++) {
							tmpRow[i*4+0] = (GLfloat) UINT_TO_FLOAT(uiSrc[i*3+r]);
							tmpRow[i*4+1] = (GLfloat) UINT_TO_FLOAT(uiSrc[i*3+1]);
							tmpRow[i*4+2] = (GLfloat) UINT_TO_FLOAT(uiSrc[i*3+b]);
							tmpRow[i*4+3] = 1.0;
						}
						break;
					case GL_FLOAT:
						for (i = 0; i < width; i++) {
							tmpRow[i*4+0] = fSrc[i*3+r];
							tmpRow[i*4+1] = fSrc[i*3+1];
							tmpRow[i*4+2] = fSrc[i*3+b];
							tmpRow[i*4+3] = 1.0;
						}
						break;
					case GL_DOUBLE:
						for (i = 0; i < width; i++) {
							tmpRow[i*4+0] = (GLfloat) dSrc[i*3+r];
							tmpRow[i*4+1] = (GLfloat) dSrc[i*3+1];
							tmpRow[i*4+2] = (GLfloat) dSrc[i*3+b];
							tmpRow[i*4+3] = 1.0;
						}
						break;
#ifdef CR_OPENGL_VERSION_1_2
					case GL_UNSIGNED_BYTE_3_3_2:
					case GL_UNSIGNED_BYTE_2_3_3_REV:
					case GL_UNSIGNED_SHORT_5_6_5:
					case GL_UNSIGNED_SHORT_5_6_5_REV:
						/* XXX to do */
#endif

					default:
						crError("unexpected type in get_row in pixel.c");
				}
			}
			break;
		case GL_RGBA:
#ifdef CR_OPENGL_VERSION_1_2
		case GL_BGRA:
#endif
			{
				int r, b;
				if (srcFormat == GL_RGB) {
					r = 0; b = 2;
				}
				else {
					r = 2; b = 0;
				}
				switch (srcType) {
					case GL_BYTE:
						for (i = 0; i < width; i++) {
							tmpRow[i*4+0] = (GLfloat) BYTE_TO_FLOAT(bSrc[i*4+r]);
							tmpRow[i*4+1] = (GLfloat) BYTE_TO_FLOAT(bSrc[i*4+1]);
							tmpRow[i*4+2] = (GLfloat) BYTE_TO_FLOAT(bSrc[i*4+b]);
							tmpRow[i*4+3] = (GLfloat) BYTE_TO_FLOAT(bSrc[i*4+3]);
						}
						break;
					case GL_UNSIGNED_BYTE:
						for (i = 0; i < width; i++) {
							tmpRow[i*4+0] = (GLfloat) UBYTE_TO_FLOAT(ubSrc[i*4+r]);
							tmpRow[i*4+1] = (GLfloat) UBYTE_TO_FLOAT(ubSrc[i*4+1]);
							tmpRow[i*4+2] = (GLfloat) UBYTE_TO_FLOAT(ubSrc[i*4+b]);
							tmpRow[i*4+3] = (GLfloat) UBYTE_TO_FLOAT(ubSrc[i*4+3]);
						}
						break;
					case GL_SHORT:
						for (i = 0; i < width; i++) {
							tmpRow[i*4+0] = (GLfloat) SHORT_TO_FLOAT(sSrc[i*4+r]);
							tmpRow[i*4+1] = (GLfloat) SHORT_TO_FLOAT(sSrc[i*4+1]);
							tmpRow[i*4+2] = (GLfloat) SHORT_TO_FLOAT(sSrc[i*4+b]);
							tmpRow[i*4+3] = (GLfloat) SHORT_TO_FLOAT(sSrc[i*4+3]);
						}
						break;
					case GL_UNSIGNED_SHORT:
						for (i = 0; i < width; i++) {
							tmpRow[i*4+0] = (GLfloat) USHORT_TO_FLOAT(usSrc[i*4+r]);
							tmpRow[i*4+1] = (GLfloat) USHORT_TO_FLOAT(usSrc[i*4+1]);
							tmpRow[i*4+2] = (GLfloat) USHORT_TO_FLOAT(usSrc[i*4+b]);
							tmpRow[i*4+3] = (GLfloat) USHORT_TO_FLOAT(usSrc[i*4+3]);
						}
						break;
					case GL_INT:
						for (i = 0; i < width; i++) {
							tmpRow[i*4+0] = (GLfloat) INT_TO_FLOAT(iSrc[i*4+r]);
							tmpRow[i*4+1] = (GLfloat) INT_TO_FLOAT(iSrc[i*4+1]);
							tmpRow[i*4+2] = (GLfloat) INT_TO_FLOAT(iSrc[i*4+b]);
							tmpRow[i*4+3] = (GLfloat) INT_TO_FLOAT(iSrc[i*4+3]);
						}
						break;
					case GL_UNSIGNED_INT:
						for (i = 0; i < width; i++) {
							tmpRow[i*4+0] = (GLfloat) UINT_TO_FLOAT(uiSrc[i*4+r]);
							tmpRow[i*4+1] = (GLfloat) UINT_TO_FLOAT(uiSrc[i*4+1]);
							tmpRow[i*4+2] = (GLfloat) UINT_TO_FLOAT(uiSrc[i*4+b]);
							tmpRow[i*4+3] = (GLfloat) UINT_TO_FLOAT(uiSrc[i*4+3]);
						}
						break;
					case GL_FLOAT:
						for (i = 0; i < width; i++) {
							tmpRow[i*4+0] = fSrc[i*4+r];
							tmpRow[i*4+1] = fSrc[i*4+1];
							tmpRow[i*4+2] = fSrc[i*4+b];
							tmpRow[i*4+3] = fSrc[i*4+3];
						}
						break;
					case GL_DOUBLE:
						for (i = 0; i < width; i++) {
							tmpRow[i*4+0] = (GLfloat) dSrc[i*4+r];
							tmpRow[i*4+1] = (GLfloat) dSrc[i*4+1];
							tmpRow[i*4+2] = (GLfloat) dSrc[i*4+b];
							tmpRow[i*4+3] = (GLfloat) dSrc[i*4+3];
						}
						break;
#ifdef CR_OPENGL_VERSION_1_2
					case GL_UNSIGNED_SHORT_5_5_5_1:
					case GL_UNSIGNED_SHORT_1_5_5_5_REV:
					case GL_UNSIGNED_SHORT_4_4_4_4:
					case GL_UNSIGNED_SHORT_4_4_4_4_REV:
					case GL_UNSIGNED_INT_8_8_8_8:
					case GL_UNSIGNED_INT_8_8_8_8_REV:
					case GL_UNSIGNED_INT_10_10_10_2:
					case GL_UNSIGNED_INT_2_10_10_10_REV:
						/* XXX to do */
#endif

					default:
						crError("unexpected type in get_row in pixel.c");
				}
			}
			break;
	}
}


static void put_row(char *dst, GLenum dstFormat, GLenum dstType,
										GLsizei width, const GLfloat *tmpRow)
{
	GLbyte *bDst = (GLbyte *) dst;
	GLubyte *ubDst = (GLubyte *) dst;
	GLshort *sDst = (GLshort *) dst;
	GLushort *usDst = (GLushort *) dst;
	GLint *iDst = (GLint *) dst;
	GLuint *uiDst = (GLuint *) dst;
	GLfloat *fDst = (GLfloat *) dst;
	GLdouble *dDst = (GLdouble *) dst;
	int i;

	switch (dstFormat) {
		case GL_COLOR_INDEX:
		case GL_STENCIL_INDEX:
			switch (dstType) {
				case GL_BYTE:
					for (i = 0; i < width; i++)
						bDst[i] = (GLbyte) tmpRow[i];
					break;
				case GL_UNSIGNED_BYTE:
					for (i = 0; i < width; i++)
						ubDst[i] = (GLubyte) tmpRow[i];
					break;
				case GL_SHORT:
					for (i = 0; i < width; i++)
						sDst[i] = (GLshort) tmpRow[i];
					break;
				case GL_UNSIGNED_SHORT:
					for (i = 0; i < width; i++)
						usDst[i] = (GLushort) tmpRow[i];
					break;
				case GL_INT:
					for (i = 0; i < width; i++)
						iDst[i] = (GLint) tmpRow[i];
					break;
				case GL_UNSIGNED_INT:
					for (i = 0; i < width; i++)
						uiDst[i] = (GLuint) tmpRow[i];
					break;
				case GL_FLOAT:
					for (i = 0; i < width; i++)
						fDst[i] = tmpRow[i];
					break;
				case GL_DOUBLE:
					for (i = 0; i < width; i++)
						dDst[i] = tmpRow[i];
					break;
				default:
					crError("unexpected type in put_row in pixel.c");
			}
			break;
		case GL_DEPTH_COMPONENT:
			switch (dstType) {
				case GL_BYTE:
					for (i = 0; i < width; i++)
						bDst[i] = FLOAT_TO_BYTE(tmpRow[i]);
					break;
				case GL_UNSIGNED_BYTE:
					for (i = 0; i < width; i++)
						ubDst[i] = FLOAT_TO_UBYTE(tmpRow[i]);
					break;
				case GL_SHORT:
					for (i = 0; i < width; i++)
						sDst[i] = FLOAT_TO_SHORT(tmpRow[i]);
					break;
				case GL_UNSIGNED_SHORT:
					for (i = 0; i < width; i++)
						sDst[i] = FLOAT_TO_SHORT(tmpRow[i]);
					break;
				case GL_INT:
					for (i = 0; i < width; i++)
						bDst[i] = (GLbyte) FLOAT_TO_INT(tmpRow[i]);
					break;
				case GL_UNSIGNED_INT:
					for (i = 0; i < width; i++)
						bDst[i] = (GLbyte) FLOAT_TO_UINT(tmpRow[i]);
					break;
				case GL_FLOAT:
					for (i = 0; i < width; i++)
						fDst[i] = tmpRow[i];
					break;
				case GL_DOUBLE:
					for (i = 0; i < width; i++)
						dDst[i] = tmpRow[i];
					break;
				default:
					crError("unexpected type in put_row in pixel.c");
			}
			break;
		case GL_RED:
		case GL_GREEN:
		case GL_BLUE:
		case GL_ALPHA:
		case GL_LUMINANCE:
		case GL_INTENSITY:
			{
				int index;
				if (dstFormat == GL_RED)
					index = 0;
				else if (dstFormat == GL_LUMINANCE)
					index = 0;
				else if (dstFormat == GL_INTENSITY)
					index = 0;
				else if (dstFormat == GL_GREEN)
					index = 1;
				else if (dstFormat == GL_BLUE)
					index = 2;
				else
					index = 3;
				switch (dstType) {
					case GL_BYTE:
						for (i = 0; i < width; i++)
							bDst[i] = FLOAT_TO_BYTE(tmpRow[i*4+index]);
						break;
					case GL_UNSIGNED_BYTE:
						for (i = 0; i < width; i++)
							ubDst[i] = FLOAT_TO_UBYTE(tmpRow[i*4+index]);
						break;
					case GL_SHORT:
						for (i = 0; i < width; i++)
							sDst[i] = FLOAT_TO_SHORT(tmpRow[i*4+index]);
						break;
					case GL_UNSIGNED_SHORT:
						for (i = 0; i < width; i++)
							usDst[i] = FLOAT_TO_USHORT(tmpRow[i*4+index]);
						break;
					case GL_INT:
						for (i = 0; i < width; i++)
							iDst[i] = FLOAT_TO_INT(tmpRow[i*4+index]);
						break;
					case GL_UNSIGNED_INT:
						for (i = 0; i < width; i++)
							uiDst[i] = FLOAT_TO_UINT(tmpRow[i*4+index]);
						break;
					case GL_FLOAT:
						for (i = 0; i < width; i++)
							fDst[i] = tmpRow[i*4+index];
						break;
					case GL_DOUBLE:
						for (i = 0; i < width; i++)
							dDst[i] = tmpRow[i*4+index];
						break;
					default:
						crError("unexpected type in put_row in pixel.c");
				}
			}
			break;
		case GL_LUMINANCE_ALPHA:
			switch (dstType) {
				case GL_BYTE:
					for (i = 0; i < width; i++) {
						bDst[i*2+0] = FLOAT_TO_BYTE(tmpRow[i*4+0]);
						bDst[i*2+1] = FLOAT_TO_BYTE(tmpRow[i*4+3]);
					}
					break;
				case GL_UNSIGNED_BYTE:
					for (i = 0; i < width; i++) {
						ubDst[i*2+0] = FLOAT_TO_UBYTE(tmpRow[i*4+0]);
						ubDst[i*2+1] = FLOAT_TO_UBYTE(tmpRow[i*4+3]);
					}
					break;
				case GL_SHORT:
					for (i = 0; i < width; i++) {
						sDst[i*2+0] = FLOAT_TO_SHORT(tmpRow[i*4+0]);
						sDst[i*2+1] = FLOAT_TO_SHORT(tmpRow[i*4+3]);
					}
					break;
				case GL_UNSIGNED_SHORT:
					for (i = 0; i < width; i++) {
						usDst[i*2+0] = FLOAT_TO_USHORT(tmpRow[i*4+0]);
						usDst[i*2+1] = FLOAT_TO_USHORT(tmpRow[i*4+3]);
					}
					break;
				case GL_INT:
					for (i = 0; i < width; i++) {
						iDst[i*2+0] = FLOAT_TO_INT(tmpRow[i*4+0]);
						iDst[i*2+1] = FLOAT_TO_INT(tmpRow[i*4+3]);
					}
					break;
				case GL_UNSIGNED_INT:
					for (i = 0; i < width; i++) {
						uiDst[i*2+0] = FLOAT_TO_UINT(tmpRow[i*4+0]);
						uiDst[i*2+1] = FLOAT_TO_UINT(tmpRow[i*4+3]);
					}
					break;
				case GL_FLOAT:
					for (i = 0; i < width; i++) {
						fDst[i*2+0] = tmpRow[i*4+0];
						fDst[i*2+1] = tmpRow[i*4+3];
					}
					break;
				case GL_DOUBLE:
					for (i = 0; i < width; i++) {
						dDst[i*2+0] = tmpRow[i*4+0];
						dDst[i*2+1] = tmpRow[i*4+3];
					}
					break;
				default:
					crError("unexpected type in put_row in pixel.c");
			}
			break;
		case GL_RGB:
#ifdef CR_OPENGL_VERSION_1_2
		case GL_BGR:
#endif
			{
				int r, b;
				if (dstFormat == GL_RGB) {
					r = 0; b = 2;
				}
				else {
					r = 2; b = 0;
				}
				switch (dstType) {
					case GL_BYTE:
						for (i = 0; i < width; i++) {
							bDst[i*3+r] = FLOAT_TO_BYTE(tmpRow[i*4+0]);
							bDst[i*3+1] = FLOAT_TO_BYTE(tmpRow[i*4+1]);
							bDst[i*3+b] = FLOAT_TO_BYTE(tmpRow[i*4+2]);
						}
						break;
					case GL_UNSIGNED_BYTE:
						for (i = 0; i < width; i++) {
							ubDst[i*3+r] = FLOAT_TO_UBYTE(tmpRow[i*4+0]);
							ubDst[i*3+1] = FLOAT_TO_UBYTE(tmpRow[i*4+1]);
							ubDst[i*3+b] = FLOAT_TO_UBYTE(tmpRow[i*4+2]);
						}
						break;
					case GL_SHORT:
						for (i = 0; i < width; i++) {
							sDst[i*3+r] = FLOAT_TO_SHORT(tmpRow[i*4+0]);
							sDst[i*3+1] = FLOAT_TO_SHORT(tmpRow[i*4+1]);
							sDst[i*3+b] = FLOAT_TO_SHORT(tmpRow[i*4+2]);
						}
						break;
					case GL_UNSIGNED_SHORT:
						for (i = 0; i < width; i++) {
							usDst[i*3+r] = FLOAT_TO_USHORT(tmpRow[i*4+0]);
							usDst[i*3+1] = FLOAT_TO_USHORT(tmpRow[i*4+1]);
							usDst[i*3+b] = FLOAT_TO_USHORT(tmpRow[i*4+2]);
						}
						break;
					case GL_INT:
						for (i = 0; i < width; i++) {
							iDst[i*3+r] = FLOAT_TO_INT(tmpRow[i*4+0]);
							iDst[i*3+1] = FLOAT_TO_INT(tmpRow[i*4+1]);
							iDst[i*3+b] = FLOAT_TO_INT(tmpRow[i*4+2]);
						}
						break;
					case GL_UNSIGNED_INT:
						for (i = 0; i < width; i++) {
							uiDst[i*3+r] = FLOAT_TO_UINT(tmpRow[i*4+0]);
							uiDst[i*3+1] = FLOAT_TO_UINT(tmpRow[i*4+1]);
							uiDst[i*3+b] = FLOAT_TO_UINT(tmpRow[i*4+2]);
						}
						break;
					case GL_FLOAT:
						for (i = 0; i < width; i++) {
							fDst[i*3+r] = tmpRow[i*4+0];
							fDst[i*3+1] = tmpRow[i*4+1];
							fDst[i*3+b] = tmpRow[i*4+2];
						}
						break;
					case GL_DOUBLE:
						for (i = 0; i < width; i++) {
							dDst[i*3+r] = tmpRow[i*4+0];
							dDst[i*3+1] = tmpRow[i*4+1];
							dDst[i*3+b] = tmpRow[i*4+2];
						}
						break;
#ifdef CR_OPENGL_VERSION_1_2
					case GL_UNSIGNED_BYTE_3_3_2:
					case GL_UNSIGNED_BYTE_2_3_3_REV:
					case GL_UNSIGNED_SHORT_5_6_5:
					case GL_UNSIGNED_SHORT_5_6_5_REV:
						/* XXX to do */
#endif
					default:
						crError("unexpected type in put_row in pixel.c");
				}
			}
			break;
		case GL_RGBA:
#ifdef CR_OPENGL_VERSION_1_2
		case GL_BGRA:
#endif
			{
				int r, b;
				if (dstFormat == GL_RGB) {
					r = 0; b = 2;
				}
				else {
					r = 2; b = 0;
				}
				switch (dstType) {
					case GL_BYTE:
						for (i = 0; i < width; i++) {
							bDst[i*4+r] = FLOAT_TO_BYTE(tmpRow[i*4+0]);
							bDst[i*4+1] = FLOAT_TO_BYTE(tmpRow[i*4+1]);
							bDst[i*4+b] = FLOAT_TO_BYTE(tmpRow[i*4+2]);
							bDst[i*4+3] = FLOAT_TO_BYTE(tmpRow[i*4+3]);
						}
						break;
					case GL_UNSIGNED_BYTE:
						for (i = 0; i < width; i++) {
							ubDst[i*4+r] = FLOAT_TO_UBYTE(tmpRow[i*4+0]);
							ubDst[i*4+1] = FLOAT_TO_UBYTE(tmpRow[i*4+1]);
							ubDst[i*4+b] = FLOAT_TO_UBYTE(tmpRow[i*4+2]);
							ubDst[i*4+3] = FLOAT_TO_UBYTE(tmpRow[i*4+3]);
						}
						break;
					case GL_SHORT:
						for (i = 0; i < width; i++) {
							sDst[i*4+r] = FLOAT_TO_SHORT(tmpRow[i*4+0]);
							sDst[i*4+1] = FLOAT_TO_SHORT(tmpRow[i*4+1]);
							sDst[i*4+b] = FLOAT_TO_SHORT(tmpRow[i*4+2]);
							sDst[i*4+3] = FLOAT_TO_SHORT(tmpRow[i*4+3]);
						}
						break;
					case GL_UNSIGNED_SHORT:
						for (i = 0; i < width; i++) {
							usDst[i*4+r] = FLOAT_TO_USHORT(tmpRow[i*4+0]);
							usDst[i*4+1] = FLOAT_TO_USHORT(tmpRow[i*4+1]);
							usDst[i*4+b] = FLOAT_TO_USHORT(tmpRow[i*4+2]);
							usDst[i*4+3] = FLOAT_TO_USHORT(tmpRow[i*4+3]);
						}
						break;
					case GL_INT:
						for (i = 0; i < width; i++) {
							iDst[i*4+r] = FLOAT_TO_INT(tmpRow[i*4+0]);
							iDst[i*4+1] = FLOAT_TO_INT(tmpRow[i*4+1]);
							iDst[i*4+b] = FLOAT_TO_INT(tmpRow[i*4+2]);
							iDst[i*4+3] = FLOAT_TO_INT(tmpRow[i*4+3]);
						}
						break;
					case GL_UNSIGNED_INT:
						for (i = 0; i < width; i++) {
							uiDst[i*4+r] = FLOAT_TO_UINT(tmpRow[i*4+0]);
							uiDst[i*4+1] = FLOAT_TO_UINT(tmpRow[i*4+1]);
							uiDst[i*4+b] = FLOAT_TO_UINT(tmpRow[i*4+2]);
							uiDst[i*4+3] = FLOAT_TO_UINT(tmpRow[i*4+3]);
						}
						break;
					case GL_FLOAT:
						for (i = 0; i < width; i++) {
							fDst[i*4+r] = tmpRow[i*4+0];
							fDst[i*4+1] = tmpRow[i*4+1];
							fDst[i*4+b] = tmpRow[i*4+2];
							fDst[i*4+3] = tmpRow[i*4+3];
						}
						break;
					case GL_DOUBLE:
						for (i = 0; i < width; i++) {
							dDst[i*4+r] = tmpRow[i*4+0];
							dDst[i*4+1] = tmpRow[i*4+1];
							dDst[i*4+b] = tmpRow[i*4+2];
							dDst[i*4+3] = tmpRow[i*4+3];
						}
						break;
#ifdef CR_OPENGL_VERSION_1_2
					case GL_UNSIGNED_SHORT_5_5_5_1:
					case GL_UNSIGNED_SHORT_1_5_5_5_REV:
					case GL_UNSIGNED_SHORT_4_4_4_4:
					case GL_UNSIGNED_SHORT_4_4_4_4_REV:
					case GL_UNSIGNED_INT_8_8_8_8:
					case GL_UNSIGNED_INT_8_8_8_8_REV:
					case GL_UNSIGNED_INT_10_10_10_2:
					case GL_UNSIGNED_INT_2_10_10_10_REV:
						/* XXX to do */
#endif
					default:
						crError("unexpected type in put_row in pixel.c");
				}
			}
			break;
		default:
			crError("unexpected type in put_row in pixel.c");
	}
}


/*
 * Return number of bytes of storage needed to accomodate an
 * image with the given format, type, and size.
 */
unsigned int crImageSize( GLenum format, GLenum type, GLsizei width, GLsizei height )
{
	unsigned int bytes = width * height;

	if (type == GL_BITMAP)
	{
		/* This was wrong in the old code! */
		bytes = ((width + 7) / 8) * height;
	}
	else
	{
		bytes = width * height * crPixelSize( format, type );
	}

	return bytes;
}

/*
 * Return number of bytes of storage needed to accomodate a
 * 3D texture with the give format, type, and size.
 */
unsigned int crTextureSize( GLenum format, GLenum type, GLsizei width, GLsizei height, GLsizei depth )
{
        unsigned int bytes = width * height;

        if (type == GL_BITMAP)
        {
		/*
		 * Not sure about this one, so just multiply
		 * by the depth?
		 */
                bytes = ((width + 7) / 8) * height * depth;
        }
        else
        {
                bytes = width * height * depth * crPixelSize( format, type );
        }

        return bytes;
}

static const CRPixelPackState defaultPacking = {
	0, 		/* rowLength */
	0, 		/* skipRows */
	0, 		/* skipPixels */
	1, 		/* alignment */
	0, 		/* imageHeight */
	0, 		/* skipImages */
	GL_FALSE, 	/* swapBytes */
	GL_FALSE  	/* psLSBFirst */
};


void crPixelCopy1D( GLvoid *dstPtr, GLenum dstFormat, GLenum dstType,
										const GLvoid *srcPtr, GLenum srcFormat, GLenum srcType,
										GLsizei width, const CRPixelPackState *srcPacking )
{
	crPixelCopy2D( width, 1,
								 dstPtr, dstFormat, dstType, NULL,  /* dst */
								 srcPtr, srcFormat, srcType, srcPacking );  /* src */
}

void crPixelCopy2D( GLsizei width, GLsizei height,
										GLvoid *dstPtr, GLenum dstFormat, GLenum dstType,
										const CRPixelPackState *dstPacking,
										const GLvoid *srcPtr, GLenum srcFormat, GLenum srcType,
										const CRPixelPackState *srcPacking )
										
{
	const char *src = (const char *) srcPtr;
	char *dst = (char *) dstPtr;
	int srcBytesPerPixel;
	int dstBytesPerPixel;
	int srcBytesPerRow;
	int dstBytesPerRow;
 	int srcRowStrideBytes;
 	int dstRowStrideBytes;
	int bytesPerRow;
	int i;

	if (!dstPacking)
		 dstPacking = &defaultPacking;

	if (!srcPacking)
		 srcPacking = &defaultPacking;

	if (srcType == GL_BITMAP)
	{
		CRASSERT(dstType == GL_BITMAP);
		bytesPerRow = (width + 7) / 8;
		if (srcPacking->rowLength > 0)
			srcRowStrideBytes = (srcPacking->rowLength + 7) / 8;
		else
			srcRowStrideBytes = bytesPerRow;
		dstRowStrideBytes = bytesPerRow;

		for (i=0; i<height; i++) {
			crMemcpy( (void *) dst, (const void *) src, bytesPerRow );
			dst += dstRowStrideBytes;
			src += srcRowStrideBytes;
		} 
	}
	else
	{
		CRASSERT(dstType != GL_BITMAP);
		srcBytesPerPixel = crPixelSize( srcFormat, srcType );
		dstBytesPerPixel = crPixelSize( dstFormat, dstType );

		/* Stride between rows (in bytes) */
		if (srcPacking->rowLength > 0)
			srcRowStrideBytes = srcPacking->rowLength * srcBytesPerPixel;
		else
			srcRowStrideBytes = width * srcBytesPerPixel;

		if (dstPacking->rowLength > 0)
			 dstRowStrideBytes = dstPacking->rowLength * dstBytesPerPixel;
		else
			 dstRowStrideBytes = width * dstBytesPerPixel;

		/* bytes per row */
		srcBytesPerRow = width * srcBytesPerPixel;
		dstBytesPerRow = width * dstBytesPerPixel;

		/* handle the alignment */
		if (srcPacking->alignment != 1) {
			i = ((long) src) % srcPacking->alignment;
			if (i)
				src += srcPacking->alignment - i;
			i = (long) srcRowStrideBytes % srcPacking->alignment;
			if (i)
				srcRowStrideBytes += srcPacking->alignment - i;
		}

		if (dstPacking->alignment != 1) {
			i = ((long) dst) % dstPacking->alignment;
			if (i)
				dst += dstPacking->alignment - i;
			i = (long) dstRowStrideBytes % dstPacking->alignment;
			if (i)
				dstRowStrideBytes += dstPacking->alignment - i;
		}

		/* handle skip rows */
		src += srcPacking->skipRows * srcRowStrideBytes;
		dst += dstPacking->skipRows * dstRowStrideBytes;

		/* handle skip pixels */
		src += srcPacking->skipPixels * srcBytesPerPixel;
		dst += dstPacking->skipPixels * dstBytesPerPixel;

		/* we don't do LSBFirst or byteswapping, yet */
		if (srcPacking->psLSBFirst)
			crError( "Sorry, no lsbfirst for you" );
		if (srcPacking->swapBytes)
			crError( "Sorry, no swapbytes for you" );
		if (dstPacking->psLSBFirst)
			crError( "Sorry, no lsbfirst for you" );
		if (dstPacking->swapBytes)
			crError( "Sorry, no swapbytes for you" );

		if (srcFormat == dstFormat && srcType == dstType)
		{
			CRASSERT(srcBytesPerRow == dstBytesPerRow);
			for (i = 0; i < height; i++)
			{
				crMemcpy( (void *) dst, (const void *) src, srcBytesPerRow );
				dst += dstRowStrideBytes;
				src += srcRowStrideBytes;
			}
		}
		else
		{
			/* need to do format and/or type conversion */
			GLfloat *tmpRow = crAlloc( 4 * width * sizeof(GLfloat) );
			if (!tmpRow)
				crError("Out of memory in crPixelCopy2D");

			for (i = 0; i < height; i++)
			{
				get_row(src, srcFormat, srcType, width, tmpRow);
				put_row(dst, dstFormat, dstType, width, tmpRow);
				dst += dstRowStrideBytes;
				src += srcRowStrideBytes;
			}

			crFree(tmpRow);
		}
	}
}

void crPixelCopy3D( GLsizei width, GLsizei height, GLsizei depth, 
                    GLvoid *dstPtr, GLenum dstFormat, GLenum dstType,
                    const CRPixelPackState *dstPacking,
                    const GLvoid *srcPtr, GLenum srcFormat, GLenum srcType,
                    const CRPixelPackState *srcPacking )

{
	int tex_size = 0;
	
	(void)srcPacking;
	(void)srcType;
	(void)srcFormat;
	(void)dstPacking;

	crWarning( "crPixelCopy3D:  simply crMemcpy'ing from srcPtr to dstPtr" );

	tex_size = crTextureSize( dstFormat, dstType, width, height, depth );
	crMemcpy( (void *) dstPtr, (void *) srcPtr, tex_size ); 
}
