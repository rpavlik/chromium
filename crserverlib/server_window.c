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
	GLint windowID = -1;
	GLint dims[2];

	if (cr_server.sharedWindows) {
		int pos;
		/* find empty position in my (curclient) windowList */
		for (pos = 0; pos < CR_MAX_WINDOWS; pos++) {
			if (cr_server.curClient->windowList[pos] == 0) {
				break;
			}
		}
		if (pos == CR_MAX_WINDOWS) {
			crWarning("Too many windows in crserver!");
			return -1;
		}

		/* Look if any other client has a window for this slot */
		unsigned int j;
		for (j = 0; j < cr_server.numClients; j++) {
			if (cr_server.clients[j].windowList[pos] != 0) {
				/* use that client's window */
				windowID = cr_server.clients[j].windowList[pos];
				cr_server.curClient->windowList[pos] = windowID;
				crServerReturnValue( &windowID, sizeof(windowID) ); /* real return value */
				crDebug("CRServer: client %d sharing window %d", cr_server.curClient->number, windowID);
				return windowID;
			}
		}
	}

	/*
	 * Have first SPU make a new window, get initial size.
	 */
	windowID = cr_server.head_spu->dispatch_table.WindowCreate( dpyName, visBits );
	crDebug("CRServer: client %d created new window %d", cr_server.curClient->number, windowID);
	if (windowID < 0)
		return windowID; /* error case */
	crServerReturnValue( &windowID, sizeof(windowID) ); /* real return value */
	cr_server.head_spu->dispatch_table.GetChromiumParametervCR(GL_WINDOW_SIZE_CR, windowID, GL_INT, 2, dims);

	/*
	 * Create a new mural for the new window.
	 */
	mural = (CRMuralInfo *) crCalloc(sizeof(CRMuralInfo));
	if (mural) {
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
		/* hash tables are indexed by window ID */
		crHashtableAdd(cr_server.muralTable, windowID, mural);
	}
	else {
		/* out of memory */
		return -1;
	}

	if (cr_server.sharedWindows) {
		int pos;
		for (pos = 0; pos < CR_MAX_WINDOWS; pos++) {
			if (cr_server.curClient->windowList[pos] == 0) {
				cr_server.curClient->windowList[pos] = windowID;
				break;
			}
		}
	}

	return windowID; /* WILL PROBABLY BE IGNORED */
}


void SERVER_DISPATCH_APIENTRY crServerDispatchWindowDestroy( GLint window )
{
	cr_server.head_spu->dispatch_table.WindowDestroy( window );
}


void SERVER_DISPATCH_APIENTRY crServerDispatchWindowSize( GLint window, GLint width, GLint height )
{
  CRMuralInfo *mural;

	mural = (CRMuralInfo *) crHashtableSearch(cr_server.muralTable, window);
	CRASSERT(mural);
	mural->underlyingDisplay[2] = width;
	mural->underlyingDisplay[3] = height;
	crServerInitializeTiling(mural);

	cr_server.head_spu->dispatch_table.WindowSize(window, width, height);
}
