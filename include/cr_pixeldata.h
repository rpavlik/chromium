/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_PIXELDATA_H
#define CR_PIXELDATA_H

#include "chromium.h"
#include "state/cr_client.h"

#ifdef __cplusplus
extern "C" {
#endif

int crPixelSize( GLenum format, GLenum type );

unsigned int crImageSize( GLenum format, GLenum type,
													GLsizei width, GLsizei height );

unsigned int crTextureSize( GLenum format, GLenum type, GLsizei width, GLsizei height, GLsizei depth );

void crPixelCopy1D( GLvoid *dstPtr, GLenum dstFormat, GLenum dstType,
										const GLvoid *srcPtr, GLenum srcFormat, GLenum srcType,
										GLsizei width, const CRPixelPackState *srcPacking );

void crPixelCopy2D( GLsizei width, GLsizei height,
										GLvoid *dstPtr, GLenum dstFormat, GLenum dstType,
										const CRPixelPackState *dstPacking,
										const GLvoid *srcPtr, GLenum srcFormat, GLenum srcType,
										const CRPixelPackState *srcPacking );

void crPixelCopy3D( GLsizei width, GLsizei height, GLsizei depth,
										GLvoid *dstPtr, GLenum dstFormat, GLenum dstType,
										const CRPixelPackState *dstPacking, const GLvoid *srcPtr,
										GLenum srcFormat, GLenum srcType,
										const CRPixelPackState *srcPacking );

void crBitmapCopy( GLsizei width, GLsizei height, GLubyte *dstPtr,
									 const GLubyte *srcPtr, const CRPixelPackState *srcPacking );


#ifdef __cplusplus
}
#endif

#endif /* CR_PIXELDATA_H */
