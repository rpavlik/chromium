/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "server.h"
#include "server_dispatch.h"
#include "cr_mem.h"
#include "cr_rand.h"


/**
 * Generate a new window ID.
 */
static GLint
generateID(void)
{
	if (cr_server.uniqueWindows) {
		static GLboolean firstCall = GL_TRUE;
		if (firstCall) {
			crRandAutoSeed();
			firstCall = GL_FALSE;
		}
		return crRandInt(20000, 50000); /* XXX FIX */
	}
	else {
		static GLint freeID = 1;
		return freeID++;
	}
}



GLint SERVER_DISPATCH_APIENTRY
crServerDispatchWindowCreate( const char *dpyName, GLint visBits )
{
	CRMuralInfo *mural;
	GLint windowID = -1;
	GLint spuWindow;
	GLint dims[2];

	if (cr_server.sharedWindows) {
		int pos, j;

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
		for (j = 0; j < cr_server.numClients; j++) {
			if (cr_server.clients[j]->windowList[pos] != 0) {
				/* use that client's window */
				windowID = cr_server.clients[j]->windowList[pos];
				cr_server.curClient->windowList[pos] = windowID;
				crServerReturnValue( &windowID, sizeof(windowID) ); /* real return value */
				crDebug("CRServer: client %p sharing window %d",
								cr_server.curClient, windowID);
				return windowID;
			}
		}
	}

	/*
	 * Have first SPU make a new window.
	 */
	spuWindow = cr_server.head_spu->dispatch_table.WindowCreate( dpyName, visBits );
	if (spuWindow < 0) {
		crServerReturnValue( &spuWindow, sizeof(spuWindow) );
		return spuWindow;
	}

	/* get initial window size */
	cr_server.head_spu->dispatch_table.GetChromiumParametervCR(GL_WINDOW_SIZE_CR, spuWindow, GL_INT, 2, dims);

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

		mural->spuWindow = spuWindow;
		crServerInitializeTiling(mural);

		/* generate ID for this new window/mural (special-case for file conns) */
		if (cr_server.curClient->conn->type == CR_FILE)
			windowID = spuWindow;
		else
			windowID = generateID();
		crHashtableAdd(cr_server.muralTable, windowID, mural);
	}

	crDebug("CRServer: client %p created new window %d (SPU window %d)",
					cr_server.curClient, windowID, spuWindow);

	if (windowID != -1 && cr_server.sharedWindows) {
		int pos;
		for (pos = 0; pos < CR_MAX_WINDOWS; pos++) {
			if (cr_server.curClient->windowList[pos] == 0) {
				cr_server.curClient->windowList[pos] = windowID;
				break;
			}
		}
	}

	crServerReturnValue( &windowID, sizeof(windowID) );
	return windowID;
}


#define EXTRA_WARN 0

void SERVER_DISPATCH_APIENTRY
crServerDispatchWindowDestroy( GLint window )
{
  CRMuralInfo *mural;

	mural = (CRMuralInfo *) crHashtableSearch(cr_server.muralTable, window);
	if (!mural) {
#if EXTRA_WARN
		 crWarning("CRServer: invalid window %d passed to WindowDestroy()", window);
#endif
		 return;
	}

	crDebug("CRServer: Destroying window %d (spu window %d)", window, mural->spuWindow);
	cr_server.head_spu->dispatch_table.WindowDestroy( mural->spuWindow );
}


void SERVER_DISPATCH_APIENTRY
crServerDispatchWindowSize( GLint window, GLint width, GLint height )
{
  CRMuralInfo *mural;

	/*	crDebug("CRServer: Window %d size %d x %d", window, width, height);*/
	mural = (CRMuralInfo *) crHashtableSearch(cr_server.muralTable, window);
	if (!mural) {
#if EXTRA_WARN
		 crWarning("CRServer: invalid window %d passed to WindowSize()", window);
#endif
		 return;
	}
	mural->underlyingDisplay[2] = width;
	mural->underlyingDisplay[3] = height;
	crServerInitializeTiling(mural);

	cr_server.head_spu->dispatch_table.WindowSize(mural->spuWindow, width, height);
}


void SERVER_DISPATCH_APIENTRY
crServerDispatchWindowPosition( GLint window, GLint x, GLint y )
{
  CRMuralInfo *mural = (CRMuralInfo *) crHashtableSearch(cr_server.muralTable, window);
	/*	crDebug("CRServer: Window %d pos %d, %d", window, x, y);*/
	if (!mural) {
#if EXTRA_WARN
		 crWarning("CRServer: invalid window %d passed to WindowPosition()", window);
#endif
		 return;
	}
#if EXTRA_WARN /* don't believe this is needed */
	mural->underlyingDisplay[0] = x;
	mural->underlyingDisplay[1] = y;
	crServerInitializeTiling(mural);
#endif
	cr_server.head_spu->dispatch_table.WindowPosition(mural->spuWindow, x, y);
}



void SERVER_DISPATCH_APIENTRY
crServerDispatchWindowShow( GLint window, GLint state )
{
  CRMuralInfo *mural = (CRMuralInfo *) crHashtableSearch(cr_server.muralTable, window);
	if (!mural) {
#if EXTRA_WARN
		 crWarning("CRServer: invalid window %d passed to WindowShow()", window);
#endif
		 return;
	}
	cr_server.head_spu->dispatch_table.WindowShow(mural->spuWindow, state);
}


GLint
crServerSPUWindowID(GLint serverWindow)
{
  CRMuralInfo *mural = (CRMuralInfo *) crHashtableSearch(cr_server.muralTable, serverWindow);
	if (!mural) {
#if EXTRA_WARN
		 crWarning("CRServer: invalid window %d passed to crServerSPUWindowID()",
							 serverWindow);
#endif
		 return -1;
	}
	return mural->spuWindow;
}
