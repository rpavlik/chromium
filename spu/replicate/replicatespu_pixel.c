/* Copyright (c) 2001, Stanford University
   All rights reserved.

   See the file LICENSE.txt for information on redistributing this software. */
	
#include "cr_packfunctions.h"
#include "cr_glstate.h"
#include "cr_pixeldata.h"
#include "cr_version.h"
#include "replicatespu.h"
#include "replicatespu_proto.h"


void REPLICATESPU_APIENTRY
replicatespu_ReadPixels( GLint x, GLint y, GLsizei width, GLsizei height,
												 GLenum format, GLenum type, GLvoid *pixels )
{
	GET_THREAD(thread);
	unsigned int i;
	ContextInfo *ctx = thread->currentContext;
	CRClientState *clientState = &(ctx->State->client);

	replicatespuFlush( (void *) thread );

	replicate_spu.ReadPixels++;

	/*
	 * Only send ReadPixels to one server, the first active one we find.
	 */

	for (i = 1; i < CR_MAX_REPLICANTS; i++) {
		/* hijack the current packer context */
		if (replicate_spu.rserver[i].conn &&
				replicate_spu.rserver[i].conn->type != CR_NO_CONNECTION) {
			int writeback;

			if (replicate_spu.swap)
			{
				crPackReadPixelsSWAP( x, y, width, height, format, type, pixels,
							&(clientState->pack), &writeback );
			}
			else
			{
				crPackReadPixels( x, y, width, height, format, type, pixels,
							&(clientState->pack), &writeback );
			}
			replicatespuFlushOne(thread, i);

			while (replicate_spu.ReadPixels) 
				crNetRecv();

			/* Only need pixels from one server - all done now */
			return;
		}
	}
}
