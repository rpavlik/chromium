/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "tilesortspu.h"
#include "tilesortspu_gen.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_mothership.h"
#include "cr_packfunctions.h"
#include "cr_string.h"


#ifdef USE_DMX
/**
 * DMX only: for the given WindowInfo, query DMX to get the X window
 * IDs for the corresponding back-end windows.  Create back-end sub
 * windows if needed.  Compute new tiling.
 */
static GLboolean
tilesortspuGetBackendWindowInfo(WindowInfo *winInfo)
{
	/* This is a bit clunky, but we're using the crOpenGLInterface parameter
	 * as a flag to indicate whether or not child windows (of the back-end
	 * X windows) should be created if not already present.
	 * We need to be careful about that when a parallel application is
	 * using tilesort to drive a DMX display.
	 */
	const crOpenGLInterface *openglInterface
		= (tilesort_spu.rank == 0) ? &tilesort_spu.ws : NULL;

	GLboolean b;

	b = crDMXGetBackendWindowInfo(winInfo->dpy, winInfo->xwin,
																tilesort_spu.num_servers,
																winInfo->backendWindows,
																openglInterface,
																winInfo->visBits);

	if (b) {
		/* Update the tally of the number of back-end windows which have
		 * actually been realized.
		 */
		int i;
		winInfo->numBackendsRealized = 0;
		for (i = 0; i < tilesort_spu.num_servers; i++) {
			if (winInfo->backendWindows[i].xsubwin) {
				winInfo->numBackendsRealized++;
			}
		}
	}

	return b;
}
#endif /* USE_DMX */



/**
 * Update winInfo->lastWidth/Height/X/Y by looking at the native Win32
 * or X11 window.
 * Use the fake_window_dims if there is no native window.
 * Set lastWidth/Height to zero if all else fails.
 * If the window is on a DMX display, use the DMX API to query the
 * geometry of the back-end windows.
 * \return GL_TRUE if anything changes, GL_FALSE otherwise.
 */
GLboolean
tilesortspuUpdateWindowInfo(WindowInfo *winInfo)
{
#ifdef WINDOWS
	/** XXX \todo this used to be in __getWindowSize() - hope it still works */
	RECT r;

	if (!winInfo->client_hwnd) {
		if (tilesort_spu.fakeWindowWidth != 0) {
			winInfo->lastWidth = tilesort_spu.fakeWindowWidth;
			winInfo->lastHeight = tilesort_spu.fakeWindowHeight;
		}
		else {
			/* This will trigger an error message later */
			winInfo->lastWidth = 0;
			winInfo->lastHeight = 0;
		}
		return GL_TRUE;
	}

	GetClientRect( winInfo->client_hwnd, &r );
	winInfo->lastWidth = r.right - r.left;
	winInfo->lastHeight = r.bottom - r.top;
	winInfo->lastX = r.left;  /** XXX \todo are these screen coords? */
	winInfo->lastY = r.top;

	/* These lines provided by Ricky Uy (19 July 2004) */
	if (winInfo->lastWidth == 0 || winInfo->lastHeight == 0) {
		if (tilesort_spu.fakeWindowWidth != 0) {
			winInfo->lastWidth = tilesort_spu.fakeWindowWidth;
			winInfo->lastHeight = tilesort_spu.fakeWindowHeight;
		}
	}
	return GL_TRUE;

#elif defined(Darwin)

	GrafPtr save;
	Rect rect;

	if( !winInfo->window ) {
		if( tilesort_spu.fakeWindowWidth != 0 ) {
			winInfo->lastWidth = tilesort_spu.fakeWindowWidth;
			winInfo->lastHeight = tilesort_spu.fakeWindowHeight;
		} else {
			winInfo->lastWidth = 0;
			winInfo->lastHeight = 0;
		}
		return GL_TRUE;
	}

#if 0
	GetWindowBounds( winInfo->window, kWindowContentRgn, &rect );
#else
	GetPort( &save );
	SetPortWindowPort( winInfo->window );
	GetWindowPortBounds( winInfo->window, &rect );
	SetPort( save );
#endif

	winInfo->lastWidth = rect.right - rect.left;
	winInfo->lastHeight = rect.bottom - rect.top;
	winInfo->lastX = rect.left;
	winInfo->lastY = rect.top;

	if( winInfo->lastWidth == 0 || winInfo->lastHeight == 0 ) {
		if( tilesort_spu.fakeWindowWidth != 0 ) {
			winInfo->lastWidth = tilesort_spu.fakeWindowWidth;
			winInfo->lastHeight = tilesort_spu.fakeWindowHeight;
		}
	}
	return GL_TRUE;

#else /* GLX */

	int x, y;
	unsigned int width, height, borderWidth, depth;
	Window root, child;
	GLboolean change;

	CRASSERT(winInfo);

	if (!winInfo->dpy ||
			!winInfo->xwin ||
			!XGetGeometry(winInfo->dpy, winInfo->xwin,
										&root, &x, &y, &width, &height,
										&borderWidth, &depth)) {
		/* we were unable to get the window geometry */
		if (tilesort_spu.fakeWindowWidth != 0) {
			width = tilesort_spu.fakeWindowWidth;
			height = tilesort_spu.fakeWindowHeight;
		}
		else {
			/* This will trigger an error message later */
			width = 0;
			height = 0;
		}
		x = winInfo->lastX;
		y = winInfo->lastY;
	}
	else {
		/* we got the window geometry, now translate x/y to screen coordinates */
		CRASSERT(root);
		if (!XTranslateCoordinates(winInfo->dpy, winInfo->xwin,
															 root, x, y, &x, &y, &child)) {
			crDebug("XTranslateCoordinates(%d) failed in tilesortspuUpdateWindowInfo",
							(int) winInfo->xwin);
			return GL_FALSE;
		}

#ifdef USE_DMX
		if (winInfo->isDMXWindow &&
				(winInfo->numBackendsRealized == 0 ||
				 winInfo->lastX != x ||
				 winInfo->lastY != y ||
				 winInfo->lastWidth != (int)width ||
				 winInfo->lastHeight != (int)height))
		{
			/* Get/update the backend window information for this window. */
			winInfo->newBackendWindows |= tilesortspuGetBackendWindowInfo(winInfo);
		}
#endif
	}

	change = (winInfo->lastX != x ||
						winInfo->lastY != y ||
						winInfo->lastWidth != (int) width ||
						winInfo->lastHeight != (int) height);

	winInfo->lastX = x;
	winInfo->lastY = y;
	winInfo->lastWidth = width;
	winInfo->lastHeight = height;

	return change;

#endif /*GLX*/
}


/**
 * Allocate a new WindowInfo object.
 * The initial tiling info is copied from the default window, which was
 * initialized according to the configuration script.
 * Input:  window - the integer window ID for the window
 *         visBits - describes the desired visual.
 * Return: pointer to new WindowInfo
 */
WindowInfo *tilesortspuCreateWindowInfo(GLint window, GLint visBits)
{
	WindowInfo *winInfo = (WindowInfo *) crCalloc(sizeof(WindowInfo));
	if (!winInfo)
		return NULL;

	CRASSERT(tilesort_spu.num_servers > 0);

#ifdef USE_DMX
	winInfo->backendWindows = crDMXAllocBackendWindowInfo(tilesort_spu.num_servers);
	if (!winInfo->backendWindows) {
		crFree(winInfo);
		return NULL;
	}
#endif

	winInfo->server = (ServerWindowInfo *) crCalloc(tilesort_spu.num_servers * sizeof(ServerWindowInfo));
	if (!winInfo->server) {
#ifdef USE_DMX
		crFree(winInfo->backendWindows);
#endif
		crFree(winInfo);
		return NULL;
	}

	winInfo->id = window;
	winInfo->visBits = visBits;
	winInfo->bucketMode = tilesort_spu.defaultBucketMode;
	winInfo->lastX = winInfo->lastY = -1;
	winInfo->lastWidth = 0;
	winInfo->lastHeight = 0;
	/** XXX \todo maybe examine window title and match strings to set this flag */
	winInfo->forceQuadBuffering = tilesort_spu.forceQuadBuffering;
	/* Check if we have local stereo matrix info */
	if (!crMatrixIsIdentity(&tilesort_spu.stereoViewMatrices[0]) ||
			!crMatrixIsIdentity(&tilesort_spu.stereoViewMatrices[1]) ||
			!crMatrixIsIdentity(&tilesort_spu.stereoProjMatrices[0]) ||
			!crMatrixIsIdentity(&tilesort_spu.stereoProjMatrices[1])) {
		winInfo->matrixSource = MATRIX_SOURCE_CONFIG;
	}
	else {
		/* this may get set to MATRIX_SOURCE_SERVERS when we get the tiling info */
		winInfo->matrixSource = MATRIX_SOURCE_APP;
	}

	if (window > 0) {
		/* copy the default window's tiling info */
		WindowInfo *winInfo0 = (WindowInfo *) crHashtableSearch(tilesort_spu.windowTable, 0);
		int i, j, k;
		CRASSERT(winInfo0);
		winInfo->muralWidth = winInfo0->muralWidth;
		winInfo->muralHeight = winInfo0->muralHeight;
		winInfo->passiveStereo = winInfo0->passiveStereo;
		winInfo->matrixSource = winInfo0->matrixSource;
		for (i = 0; i < tilesort_spu.num_servers; i++) {
			winInfo->server[i].eyeFlags = winInfo0->server[i].eyeFlags;
			winInfo->server[i].num_extents = winInfo0->server[i].num_extents;
			for (j = 0; j < winInfo0->server[i].num_extents; j++) {
				winInfo->server[i].extents[j] = winInfo0->server[i].extents[j];
				for (k=0; k<8; k++) {
					winInfo->server[i].world_extents[j][k] = winInfo0->server[i].world_extents[j][k];
				}
			}
			/* copy view /projection matrices too */
			winInfo->server[i].viewMatrix[0] = winInfo0->server[i].viewMatrix[0];
			winInfo->server[i].viewMatrix[1] = winInfo0->server[i].viewMatrix[1];
			winInfo->server[i].projectionMatrix[0] = winInfo0->server[i].projectionMatrix[0];
			winInfo->server[i].projectionMatrix[1] = winInfo0->server[i].projectionMatrix[1];
		}
	}

	crHashtableAdd(tilesort_spu.windowTable, window, winInfo);

	return winInfo;
}


/**
 * Lookup the WindowInfo object for the given integer window ID.
 * Also, set the window's native window handle to xwindowID if it's
 * non-zero.
 * Return: pointer to WindowInfo or NULL if window ID is invalid.
 */
WindowInfo *tilesortspuGetWindowInfo(GLint window, GLint xwindowID)
{
	WindowInfo *winInfo;

	winInfo = crHashtableSearch(tilesort_spu.windowTable, window);
	if (!winInfo)
		return NULL;

#ifdef WINDOWS
	if (!winInfo->client_hwnd)
		winInfo->client_hwnd = (HWND) xwindowID;
#elif defined(Darwin)
	if( !winInfo->window )
		winInfo->window = (WindowRef) xwindowID;
#else
	/* GLX / DMX */
	if (!xwindowID && tilesort_spu.renderToCrutWindow && !winInfo->xwin) {
		/* now's a good time to try to get the CRUT window ID */
		CRConnection *conn = crMothershipConnect();
		if (conn) {
			char response[100];
			crMothershipGetParam( conn, "crut_drawable", response );
			winInfo->xwin = crStrToInt(response);
			crDebug("Tilesort SPU: Got CRUT window 0x%x", (int) winInfo->xwin);
			crMothershipDisconnect(conn);
		}
	}
	else if (!winInfo->xwin) {
		winInfo->xwin = xwindowID;
	}
#endif

	return winInfo;
}


/**
 * Free a WindowInfo object.  The caller is responsible for removing
 * the object from the hash table if needed.
 */
void
tilesortspuFreeWindowInfo(WindowInfo *winInfo)
{
#ifdef USE_DMX
	crDMXFreeBackendWindowInfo(tilesort_spu.num_servers, winInfo->backendWindows);
#endif

	crFree(winInfo->server);
	crFree(winInfo);
}


/**
 * API function: set the size of the named window.
 * We'll try to find a new tiling for the new window size either by
 * contacting the mothership, DMX, etc.
 */
void TILESORTSPU_APIENTRY
tilesortspu_WindowSize(GLint window, GLint newWidth, GLint newHeight)
{
	WindowInfo *winInfo = tilesortspuGetWindowInfo(window, 0);

	CRASSERT(winInfo);

	/* Update window size */
	winInfo->lastWidth = newWidth;
	winInfo->lastHeight = newHeight;

#ifdef USE_DMX
	if (winInfo->isDMXWindow && winInfo->xwin) {
		winInfo->newBackendWindows |= tilesortspuGetBackendWindowInfo(winInfo);
	}
#endif

	if (tilesort_spu.retileOnResize) {
		winInfo->muralWidth = newWidth;
		winInfo->muralHeight = newHeight;
		tilesortspuGetNewTiling(winInfo);
	}

	/*
	 * If WindowSize() is called and there's only one server for this
	 * tilesorter, propogate the WindowSize() call to the server (and
	 * likely the render SPU).
	 * This generally only happens if the app node's track_window_size option
	 * is set.  Normally, track_window_size is zero in tilesort configurations
	 * so this window resize business isn't of any concern.
	 *
	 * This feature was requested by Wes Bethel.
	 */
	if (tilesort_spu.num_servers == 1) {
		GET_THREAD(thread);
		int i;

		/* release geometry buffer */
		crPackReleaseBuffer( thread->packer );

		/* keep this loop, in case we change the behaviour someday */
		for (i = 0; i < tilesort_spu.num_servers; i++)
		{
			const int serverWin = winInfo->server[i].window;
			crPackSetBuffer( thread->packer, &(thread->buffer[i]) );

			if (tilesort_spu.swap)
				crPackWindowSizeSWAP( serverWin, newWidth, newHeight );
			else
				crPackWindowSize( serverWin, newWidth, newHeight );

			/* release server buffer */
			crPackReleaseBuffer( thread->packer );
		}

		/* The default buffer */
		crPackSetBuffer( thread->packer, &(thread->geometry_buffer) );
	}
}


/**
 * API function: set the position of the named window.
 * If running DMX, we need to get a new tiling too.
 */
void TILESORTSPU_APIENTRY
tilesortspu_WindowPosition(GLint window, GLint x, GLint y)
{
	/* we only care about the window position when we're running on DMX */
#ifdef USE_DMX
	WindowInfo *winInfo = tilesortspuGetWindowInfo(window, 0);
	if (winInfo->isDMXWindow) {
		if (winInfo->xwin){
			/* if window's realized */
			winInfo->newBackendWindows |= tilesortspuGetBackendWindowInfo(winInfo);
		}
		tilesortspuGetNewTiling(winInfo);
	}
#endif
	(void) x;
	(void) y;
}


/**
 * API function: create a new window.
 * Input:  dpyName - display name for the window (X only)
 *         visBits - desired visual
 * Return: window ID handle or -1 if error.
 */
GLint TILESORTSPU_APIENTRY
tilesortspu_WindowCreate( const char *dpyName, GLint visBits)
{
	ThreadInfo *thread0 = &(tilesort_spu.thread[0]);
	static GLint freeWinID = 1 /*400*/;
	WindowInfo *winInfo;
	int i;

	if (tilesort_spu.forceQuadBuffering && tilesort_spu.stereoMode == CRYSTAL)
		visBits |= CR_STEREO_BIT;

	/* release geometry buffer */
	crPackReleaseBuffer(thread0->packer );

	winInfo = tilesortspuCreateWindowInfo( freeWinID, visBits );
	if (!winInfo)
		return -1;

	/*
	 * Send a WindowCreate message to each server.
	 * We send it via the zero-th thread's server connections.
	 */
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		int writeback = 1;
		GLint return_val = 0;

		crPackSetBuffer( thread0->packer, &(thread0->buffer[i]) );

		if (tilesort_spu.swap)
			crPackWindowCreateSWAP( dpyName, visBits, &return_val, &writeback);
		else
			crPackWindowCreate( dpyName, visBits, &return_val, &writeback );

		/* release server buffer */
		crPackReleaseBuffer( thread0->packer );

		/* Flush buffer (send to server) */
		tilesortspuSendServerBufferThread( i, thread0 );

		if (!thread0->netServer[0].conn->actual_network)
		{
			/** XXX \todo Revisit for DMX!!! */

			/* HUMUNGOUS HACK TO MATCH SERVER NUMBERING (from packspu_context.c)
			 *
			 * The hack exists solely to make file networking work for now.  This
			 * is totally gross, but since the server expects the numbers to start
			 * from 5000, we need to write them out this way.  This would be
			 * marginally less gross if the numbers (500 and 5000) were maybe
			 * some sort of #define'd constants somewhere so the client and the
			 * server could be aware of how each other were numbering things in
			 * cases like file networking where they actually
			 * care. 
			 *
			 * 	-Humper 
			 *
			 */
			return_val = 5000;
		}
		else
		{
			/* Get return value */
			while (writeback) {
				crNetRecv();
			}

			if (tilesort_spu.swap)
				return_val = (GLint) SWAP32(return_val);

			if (!return_val)
				return -1;  /* something went wrong on the server */
		}

		winInfo->server[i].window = return_val;
	}

	/* The default geometry pack buffer */
	crPackSetBuffer( thread0->packer, &(thread0->geometry_buffer) );

	tilesortspuBucketingInit(winInfo);

	return freeWinID++;
}


/**
 * API function: destroy the named window
 * Input: window - id of window to delete
 */
void TILESORTSPU_APIENTRY
tilesortspu_WindowDestroy( GLint window )
{
	GET_THREAD(thread);
	WindowInfo *winInfo = tilesortspuGetWindowInfo(window, 0);

	if (thread->currentContext->currentWindow == winInfo) {
		thread->currentContext->currentWindow = NULL;
	}

	/* flush any pending rendering */
	tilesortspuFlush( thread );

	/* Tell servers to destroy corresponding windows */
	if (tilesort_spu.swap)
		crPackWindowDestroySWAP( winInfo->server[0].window );
	else
		crPackWindowDestroy( winInfo->server[0].window );

	tilesortspuFlush( thread );

	tilesortspuFreeWindowInfo( winInfo );
	crHashtableDelete(tilesort_spu.windowTable, winInfo->id, GL_FALSE);
}

