/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "server.h"
#include "server_dispatch.h"
#include "cr_mem.h"

void SERVER_DISPATCH_APIENTRY crServerDispatchWindowDestroy( GLint window );


GLint SERVER_DISPATCH_APIENTRY crServerDispatchWindowCreate( const char *dpyName, GLint visBits )
{
	CRMuralInfo *mural;
	GLint windowID;
	GLint dims[2];

	/*
	 * Create a new mural when a new window is created.
	 */
	mural = (CRMuralInfo *) crCalloc(sizeof(CRMuralInfo));
	if (!mural) {
		return -1;
	}

	windowID = cr_server.head_spu->dispatch_table.WindowCreate( dpyName, visBits );
	crServerReturnValue( &windowID, sizeof(windowID) ); /* real return value */

	/* Get window's initial size */
	cr_server.head_spu->dispatch_table.GetChromiumParametervCR(GL_WINDOW_SIZE_CR, windowID, GL_INT, 2, dims);

	/* initialize the window/mural using the default mural */
	{
		CRMuralInfo *defaultMural = (CRMuralInfo *) crHashtableSearch(cr_server.muralTable, 0);
		CRASSERT(defaultMural);
		mural->width = defaultMural->width;
		mural->height = defaultMural->height;
		mural->optimizeBucket = 0; /* might get enabled later */
		mural->numExtents = defaultMural->numExtents;
		mural->curExtent = 0;
		crMemcpy(mural->extents, defaultMural->extents,
				 defaultMural->numExtents * sizeof(CRExtent));
		mural->underlyingDisplay[0] = 0;
		mural->underlyingDisplay[1] = 0;
		mural->underlyingDisplay[2] = dims[0];
		mural->underlyingDisplay[3] = dims[1];

		crServerInitializeTiling(mural);
	}

	/*
	 * We use the window ID returned from the SPU chain to also identify
	 * murals.
	 */
	crHashtableAdd(cr_server.muralTable, windowID, mural);

	return windowID; /* WILL PROBABLY BE IGNORED */
}


void SERVER_DISPATCH_APIENTRY crServerDispatchWindowDestroy( GLint window )
{
	cr_server.head_spu->dispatch_table.WindowDestroy( window );
}
