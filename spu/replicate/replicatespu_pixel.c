/* Copyright (c) 2001, Stanford University
   All rights reserved.

   See the file LICENSE.txt for information on redistributing this software. */
	
#include "cr_packfunctions.h"
#include "cr_glstate.h"
#include "cr_pixeldata.h"
#include "cr_version.h"
#include "replicatespu.h"
#include "replicatespu_proto.h"

void REPLICATESPU_APIENTRY replicatespu_GetTexImage (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels)
{
	crStateGetTexImage( target, level, format, type, pixels );
}

void REPLICATESPU_APIENTRY replicatespu_DrawPixels( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels )
{
	GET_THREAD(thread);
	ContextInfo *ctx = thread->currentContext;
	CRClientState *clientState = &(ctx->State->client);

	if (replicate_spu.swap)
	{
		crPackDrawPixelsSWAP( width, height, format, type, pixels, &(clientState->unpack) );
	}
	else
	{
		crPackDrawPixels( width, height, format, type, pixels, &(clientState->unpack) );
	}
}

void REPLICATESPU_APIENTRY replicatespu_ReadPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels )
{
	GET_THREAD(thread);
	unsigned int i;
	ContextInfo *ctx = thread->currentContext;
	CRClientState *clientState = &(ctx->State->client);

	/* flush any pending data, before broadcast unset */
	replicatespuFlush( (void *) thread );

	replicate_spu.ReadPixels++;

	thread->broadcast = 0;

	for (i = 1; i < CR_MAX_REPLICANTS; i++) {
		/* hijack the current packer context */
		if (replicate_spu.rserver[i].conn && replicate_spu.rserver[i].conn->type != CR_NO_CONNECTION) {
			thread->server.conn = replicate_spu.rserver[i].conn;

			if (replicate_spu.swap)
			{
				crPackReadPixelsSWAP( x, y, width, height, format, type, pixels,
							&(clientState->pack) );
			}
			else
			{
				crPackReadPixels( x, y, width, height, format, type, pixels,
							&(clientState->pack) );
			}
			replicatespuFlush( (void *) thread );

			while (replicate_spu.ReadPixels) 
				crNetRecv();

			thread->server.conn = replicate_spu.rserver[0].conn;
		}
	}

	thread->broadcast = 1;
}

void REPLICATESPU_APIENTRY replicatespu_CopyPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum type )
{
	GET_THREAD(thread);
	if (replicate_spu.swap)
	{
		crPackCopyPixelsSWAP( x, y, width, height, type );
	}
	else
	{
		crPackCopyPixels( x, y, width, height, type );
	}
	replicatespuFlush( (void *) thread );
}

void REPLICATESPU_APIENTRY replicatespu_Bitmap( GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap )
{
	GET_CONTEXT(ctx);
	CRClientState *clientState = &(ctx->State->client);
	if (replicate_spu.swap)
	{
		crPackBitmapSWAP( width, height, xorig, yorig, xmove, ymove, bitmap, &(clientState->unpack) );
	}
	else
	{
		crPackBitmap( width, height, xorig, yorig, xmove, ymove, bitmap, &(clientState->unpack) );
	}
	crStateBitmap( width, height, xorig, yorig, xmove, ymove, bitmap );
}

void REPLICATESPU_APIENTRY replicatespu_TexImage1D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels )
{
	GET_CONTEXT(ctx);
	CRClientState *clientState = &(ctx->State->client);
	
	crStateTexImage1D( target, level, internalformat, width, border, format, type, pixels );
	
	if (replicate_spu.swap)
	{
		crPackTexImage1DSWAP( target, level, internalformat, width, border, format, type, pixels, &(clientState->unpack) );
	}
	else
	{
		crPackTexImage1D( target, level, internalformat, width, border, format, type, pixels, &(clientState->unpack) );
	}
}
void REPLICATESPU_APIENTRY replicatespu_TexImage2D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels )
{
	GET_CONTEXT(ctx);
	CRClientState *clientState = &(ctx->State->client);
	
	crStateTexImage2D( target, level, internalformat, width, height, border, format, type, pixels );
	
	if (replicate_spu.swap)
	{
		crPackTexImage2DSWAP( target, level, internalformat, width, height, border, format, type, pixels, &(clientState->unpack) );
	}
	else
	{
		crPackTexImage2D( target, level, internalformat, width, height, border, format, type, pixels, &(clientState->unpack) );
	}
}

#ifdef GL_EXT_texture3D
void REPLICATESPU_APIENTRY replicatespu_TexImage3DEXT( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels )
{
        GET_CONTEXT(ctx);
        CRClientState *clientState = &(ctx->State->client);

	crStateTexImage3D( target, level, internalformat, width, height, depth, border, format, type, pixels );

        if (replicate_spu.swap)
        {
                crPackTexImage3DEXTSWAP( target, level, internalformat, width, height, depth, border, format, type, pixels, &(clientState->unpack) );
        }
        else
        {
                crPackTexImage3DEXT( target, level, internalformat, width, height, depth, border, format, type, pixels, &(clientState->unpack) );
        }
}
#endif

#ifdef CR_OPENGL_VERSION_1_2
void REPLICATESPU_APIENTRY replicatespu_TexImage3D(GLenum target, GLint level,
#if defined(IRIX) || defined(IRIX64) || defined(AIX) || defined (SunOS)
                                    GLenum internalformat,
#else
                                    GLint internalformat,
#endif
                                    GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels )
{
        GET_CONTEXT(ctx);
        CRClientState *clientState = &(ctx->State->client);

	crStateTexImage3D( target, level, internalformat, width, height, depth, border, format, type, pixels );

        if (replicate_spu.swap)
        {
                crPackTexImage3DSWAP( target, level, internalformat, width, height, depth, border, format, type, pixels, &(clientState->unpack) );
        }
        else
        {
                crPackTexImage3D( target, level, internalformat, width, height, depth, border, format, type, pixels, &(clientState->unpack) );
        }
}
#endif /* CR_OPENGL_VERSION_1_2 */

void REPLICATESPU_APIENTRY replicatespu_TexSubImage1D( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels )
{
	GET_CONTEXT(ctx);
	CRClientState *clientState = &(ctx->State->client);
	
	crStateTexSubImage1D( target, level, xoffset, width, format, type, pixels );
	
	if (replicate_spu.swap)
	{
		crPackTexSubImage1DSWAP( target, level, xoffset, width, format, type, pixels, &(clientState->unpack) );
	}
	else
	{
		crPackTexSubImage1D( target, level, xoffset, width, format, type, pixels, &(clientState->unpack) );
	}
}

void REPLICATESPU_APIENTRY replicatespu_TexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels )
{
	GET_CONTEXT(ctx);
	CRClientState *clientState = &(ctx->State->client);
	
	crStateTexSubImage2D( target, level, xoffset, yoffset, width, height, format, type, pixels );
	
	if (replicate_spu.swap)
	{
		crPackTexSubImage2DSWAP( target, level, xoffset, yoffset, width, height, format, type, pixels, &(clientState->unpack) );
	}
	else
	{
		crPackTexSubImage2D( target, level, xoffset, yoffset, width, height, format, type, pixels, &(clientState->unpack) );
	}
}

#ifdef CR_OPENGL_VERSION_1_2
void REPLICATESPU_APIENTRY replicatespu_TexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels )
{
        GET_CONTEXT(ctx);
        CRClientState *clientState = &(ctx->State->client);

        crStateTexSubImage3D( target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels );

        if (replicate_spu.swap)
        {
                crPackTexSubImage3DSWAP( target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels, &(clientState->unpack) );
        }
        else
        {
                crPackTexSubImage3D( target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels, &(clientState->unpack) );
        }
}
#endif /* CR_OPENGL_VERSION_1_2 */
