/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <assert.h>
#include "packspu.h"
#include "cr_packfunctions.h"


GLint PACKSPU_APIENTRY packspu_CreateContext( void *display, GLint visual )
{
	int writeback = pack_spu.server.conn->type == CR_DROP_PACKETS ? 0 : 1;
	GLint return_val = (GLint) -1;

	(void) display;
	(void) visual;

	/* Pack the command */
	if (pack_spu.swap)
		crPackCreateContextSWAP( display, visual, &return_val, &writeback );
	else
		crPackCreateContext( display, visual, &return_val, &writeback );

	/* Flush buffer and get return value */
	packspuFlush(NULL);
	while (writeback)
		crNetRecv();

	if (pack_spu.swap) {
		return_val = (GLint) SWAP32(return_val);
	}

	/* XXX we should really maintain a client-side state structure for
	 * each context we create (ala the GLX/libGL).  Otherwise, we're
	 * effectively sharing one instance of client-side state among all
	 * rendering contexts.
	 * This results in a Glean failure because vertex array state is
	 * erroneously carried across contexts.  The following temporary
	 * hack solves that problem (reset client-side state whenever we
	 * create a new context).
	 */
	crStateClientInit( &(pack_spu.currentCtx->client) );

	if (return_val > 0)
		return return_val;
	else
		return 0;
}


void PACKSPU_APIENTRY packspu_DestroyContext( void * dpy, GLint ctx )
{
	if (pack_spu.swap)
		crPackDestroyContextSWAP( dpy, ctx );
	else
		crPackDestroyContext( dpy, ctx );
}


void PACKSPU_APIENTRY packspu_MakeCurrent( void *dpy, GLint draw, GLint ctx )
{
	if (pack_spu.swap)
		crPackMakeCurrentSWAP( dpy, draw, ctx );
	else
		crPackMakeCurrent( dpy, draw, ctx );
}


extern void PACKSPU_APIENTRY packspu_GetIntegerv( GLenum pname, GLint *params );

void PACKSPU_APIENTRY packspu_Finish( void )
{
#if 0
	if (pack_spu.swap)
	{
		crPackFinishSWAP(  );
	}
	else
	{
		crPackFinish(  );
	}
	packspuFlush( NULL );
#else
	/* Finish should not return until the pipeline has been flushed and
	 * rendering is completed.  Accomplish this with a round-trip command
	 * such as glGetIntegerv.
	 */
	GLint k;
	packspu_GetIntegerv(GL_BLEND, &k);
#endif
}
