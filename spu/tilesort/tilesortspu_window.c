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
#include "tilesortspu_proto.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_packfunctions.h"
#include "cr_string.h"

#ifdef USE_DMX
#include <X11/Xlib.h>
#include <X11/extensions/dmxext.h>
#ifdef DmxBadXinerama
#define DMX_API_VERSION 2
#else
#define DMX_API_VERSION 1
#endif
#endif


#ifdef USE_DMX
/*
 * XXX taken from renderspu_glx.c, share somehow?
 *
 * Find suitable GLX visual for a back-end child window.
 */
static XVisualInfo *
chooseVisual( Display *dpy, int screen, GLint visAttribs )
{
	XVisualInfo *vis;
	int attribList[100];
	int i = 0;

	CRASSERT(visAttribs & CR_RGB_BIT);  /* anybody need color index */

	attribList[i++] = GLX_RGBA;
	attribList[i++] = GLX_RED_SIZE;
	attribList[i++] = 1;
	attribList[i++] = GLX_GREEN_SIZE;
	attribList[i++] = 1;
	attribList[i++] = GLX_BLUE_SIZE;
	attribList[i++] = 1;

	if (visAttribs & CR_ALPHA_BIT)
	{
		attribList[i++] = GLX_ALPHA_SIZE;
		attribList[i++] = 1;
	}

	if (visAttribs & CR_DOUBLE_BIT)
		attribList[i++] = GLX_DOUBLEBUFFER;

	if (visAttribs & CR_STEREO_BIT)
		attribList[i++] = GLX_STEREO;

	if (visAttribs & CR_DEPTH_BIT)
	{
		attribList[i++] = GLX_DEPTH_SIZE;
		attribList[i++] = 1;
	}

	if (visAttribs & CR_STENCIL_BIT)
	{
		attribList[i++] = GLX_STENCIL_SIZE;
		attribList[i++] = 1;
	}

	if (visAttribs & CR_ACCUM_BIT)
	{
		attribList[i++] = GLX_ACCUM_RED_SIZE;
		attribList[i++] = 1;
		attribList[i++] = GLX_ACCUM_GREEN_SIZE;
		attribList[i++] = 1;
		attribList[i++] = GLX_ACCUM_BLUE_SIZE;
		attribList[i++] = 1;
		if (visAttribs & CR_ALPHA_BIT)
		{
			attribList[i++] = GLX_ACCUM_ALPHA_SIZE;
			attribList[i++] = 1;
		}
	}

	if (visAttribs & CR_MULTISAMPLE_BIT)
	{
		attribList[i++] = GLX_SAMPLE_BUFFERS_SGIS;
		attribList[i++] = 1;
		attribList[i++] = GLX_SAMPLES_SGIS;
		attribList[i++] = 4;
	}

	/* End the list */
	attribList[i++] = None;

	CRASSERT(tilesort_spu.ws.glXChooseVisual);
	vis = tilesort_spu.ws.glXChooseVisual( dpy, screen, attribList );
	return vis;
}

static XVisualInfo *
chooseVisualRetry( Display *dpy, int screen, GLbitfield visAttribs )
{
  while (1) {
	XVisualInfo *vis = chooseVisual(dpy, screen, visAttribs);
	if (vis)
	  return vis;

	if (visAttribs & CR_MULTISAMPLE_BIT)
	  visAttribs &= ~CR_MULTISAMPLE_BIT;
	else if (visAttribs & CR_STEREO_BIT)
	  visAttribs &= ~CR_STEREO_BIT;
	else if (visAttribs & CR_ACCUM_BIT)
	  visAttribs &= ~CR_ACCUM_BIT;
	else if (visAttribs & CR_ALPHA_BIT)
	  visAttribs &= ~CR_ALPHA_BIT;
	else
	  return NULL;
  }
}

#endif /* not WINDOWS */


#ifdef USE_DMX
/*
 * DMX only: for the given WindowInfo, query DMX to get the X window
 * IDs for the corresponding back-end windows.  Create back-end sub
 * windows if needed.  Compute new tiling.
 */
static void tilesortspuGetBackendWindowInfo(WindowInfo *winInfo)
{
	int numScreens, count, i;
#if DMX_API_VERSION == 2
	DMXScreenAttributes *dmxScreenInfo;
	DMXWindowAttributes *dmxWinInfo;
#else
	DMXScreenInformation *dmxScreenInfo;
	DMXWindowInformation *dmxWinInfo;
#endif

	CRASSERT(winInfo->dpy);
	DMXGetScreenCount(winInfo->dpy, &numScreens);
	CRASSERT(numScreens == tilesort_spu.num_servers);

#if DMX_API_VERSION == 2
	dmxScreenInfo = (DMXScreenAttributes *) crAlloc(tilesort_spu.num_servers
																									 * sizeof(*dmxScreenInfo));
#else
	dmxScreenInfo = (DMXScreenInformation *) crAlloc(tilesort_spu.num_servers
																									 * sizeof(*dmxScreenInfo));
#endif
	if (!dmxScreenInfo)
		return;

#if DMX_API_VERSION == 2
	dmxWinInfo = (DMXWindowAttributes *) crAlloc(tilesort_spu.num_servers
																						* sizeof(*dmxWinInfo));
#else
	dmxWinInfo = (DMXWindowInformation *) crAlloc(tilesort_spu.num_servers
																						* sizeof(*dmxWinInfo));
#endif
	if (!dmxWinInfo) {
		crFree(dmxScreenInfo);
		return;
	}

	for (i = 0; i < numScreens; i++) {
#if DMX_API_VERSION == 2
		if (!DMXGetScreenAttributes(winInfo->dpy, i, dmxScreenInfo + i)) {
#else
		if (!DMXGetScreenInformation(winInfo->dpy, i, dmxScreenInfo + i)) {
#endif
		  crDebug("Could not get screen information for screen %d\n", i);
		  crFree(dmxScreenInfo);
		  crFree(dmxWinInfo);
		  return;
		}
	}
		
#if DMX_API_VERSION == 2
	if (!DMXGetWindowAttributes(winInfo->dpy, winInfo->xwin, &count,
															 tilesort_spu.num_servers, dmxWinInfo)) {
#else
	if (!DMXGetWindowInformation(winInfo->dpy, winInfo->xwin, &count,
															 tilesort_spu.num_servers, dmxWinInfo)) {
#endif
		crDebug("Could not get window information for 0x%x\n", (int) winInfo->xwin);
		crFree(dmxScreenInfo);
		crFree(dmxWinInfo);
		return;
	}

	/* From the DMX info, compute tiling info.
	 * Also setup child X windows for back-end rendering.
	 */
	for (i = 0; i < count; i++) {
		int server = dmxWinInfo[i].screen;
		BackendWindowInfo *backend = winInfo->backendWindows + server;
		int subwinX, subwinY, subwinW, subwinH;

		if (!backend->dpy) {
			/* Open display connection to backend if we don't have one. */
			backend->dpy = XOpenDisplay(dmxScreenInfo[server].displayName);
			CRASSERT(backend->dpy);
		}

		backend->xwin = dmxWinInfo[i].window;

		/* save tiling information */
		backend->visrect.x1 = dmxWinInfo[i].vis.x;
		backend->visrect.y1 = dmxWinInfo[i].vis.y;
		backend->visrect.x2 = dmxWinInfo[i].vis.x + dmxWinInfo[i].vis.width;
		backend->visrect.y2 = dmxWinInfo[i].vis.y + dmxWinInfo[i].vis.height;

		/* subwindow pos and size (at least 1x1) */
		subwinX = backend->visrect.x1;
		subwinY = backend->visrect.y1;
		subwinW = backend->visrect.x2 - backend->visrect.x1;
		subwinH = backend->visrect.y2 - backend->visrect.y1;
		if (subwinW <= 0)
			subwinW = 1;
		if (subwinH <= 0)
			subwinH = 1;

		if (backend->xwin != 0 && backend->xsubwin == 0) {
			/* Create a child of the back-end X window.  We do this to work
			 * around a memory allocation problem found with NVIDIA drivers.
			 * See discussion from Feb 2002 on the DMX-devel mailing list.
			 */
			XSetWindowAttributes attribs;
			Window root;
			unsigned long attribMask;
			int scr;
			XVisualInfo *visInfo;

			scr = DefaultScreen(backend->dpy);
			root = RootWindow(backend->dpy, scr);

			visInfo = chooseVisualRetry(backend->dpy, scr, winInfo->visBits);
			CRASSERT(visInfo);

			attribs.background_pixel = 0;
			attribs.border_pixel = 0;
			attribs.colormap = XCreateColormap(backend->dpy, root,
																				 visInfo->visual, AllocNone);
			attribMask = /*CWBackPixel |*/ CWBorderPixel | CWColormap;

			backend->xsubwin =
				XCreateWindow(backend->dpy,
							  backend->xwin, /* parent */
							  subwinX, subwinY,
							  subwinW, subwinH,
							  0, /* border width */
							  visInfo->depth, /* depth */
							  InputOutput, /* class */
							  visInfo->visual,
							  attribMask, &attribs);

			/*
			crDebug("Created child 0x%x of 0x%x on server %d with visual 0x%x\n",
						 (int)backend->xsubwin, (int)backend->xwin, i,
						 (int) visInfo->visualid);
			*/
			CRASSERT(backend->xsubwin);
			XMapWindow(backend->dpy, backend->xsubwin);
			XSync(backend->dpy, 0);
			winInfo->newBackendWindows = GL_TRUE;
		}
		else if (backend->xsubwin) {
			 /* Move/resize the existing child window.  We want the child to
				* basically have the same pos/size as the parent, but clipped to
				* the screen.
				*/
			XMoveResizeWindow(backend->dpy, backend->xsubwin, subwinX, subwinY,
												(unsigned int) subwinW, (unsigned int) subwinH);
			XSync(backend->dpy, 0);
		}

#if 0
		printf("Backend Window %d:  scrn %d  backwin 0x%x  childwin 0x%x:\n",
					 i, server, (int) backend->xwin, (int) backend->xsubwin);
		printf("  screen offset: %d, %d\n", dmxScreenInfo[server].xorigin,
			   dmxScreenInfo[server].yorigin);
		printf("  visrect = %d, %d .. %d, %d\n", 
			   backend->visrect.x1, backend->visrect.y1,
			   backend->visrect.x2, backend->visrect.y2);
#endif
	}

	crFree(dmxWinInfo);
	crFree(dmxScreenInfo);
}
#endif /* USE_DMX */


/*
 * Update winInfo->lastWidth/Height/X/Y by looking at the native Win32
 * or X11 window.
 * Use the fake_window_dims if there is no native window.
 * Set lastWidth/Height to zero if all else fails.
 */
void tilesortspuUpdateWindowInfo(WindowInfo *winInfo)
{
#ifdef WINDOWS
	/* XXX this used to be in __getWindowSize() - hope it still works */
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
		return;
	}

	GetClientRect( winInfo->client_hwnd, &r );
	winInfo->lastWidth = r.right - r.left;
	winInfo->lastHeight = r.bottom - r.top;
	winInfo->lastX = r.left;  /* XXX are these screen coords? */
	winInfo->lastY = r.top;
#else
	int x, y;
	unsigned int width, height, borderWidth, depth;
	Window root, child;

	CRASSERT(winInfo);

	if (!winInfo->dpy ||
			!winInfo->xwin ||
			!XGetGeometry(winInfo->dpy, winInfo->xwin,
										&root, &x, &y, &width, &height,
										&borderWidth, &depth)) {
		if (tilesort_spu.fakeWindowWidth != 0) {
			winInfo->lastWidth = tilesort_spu.fakeWindowWidth;
			winInfo->lastHeight = tilesort_spu.fakeWindowHeight;
		}
		else {
			/* This will trigger an error message later */
			winInfo->lastWidth = 0;
			winInfo->lastHeight = 0;
		}
		return;
	}

	if (!XTranslateCoordinates(winInfo->dpy, winInfo->xwin,
														 root, x, y, &x, &y, &child)) {
			crDebug("XTranslateCoordinates(%d) failed in tilesortspuUpdateWindowInfo",
							(int) winInfo->xwin);
			return;
	}

#ifdef USE_DMX
	if (winInfo->isDMXWindow &&
			(winInfo->lastX != x || winInfo->lastY != y ||
			 winInfo->lastWidth != (int)width || winInfo->lastHeight != (int)height)) {
		tilesortspuGetBackendWindowInfo(winInfo);
	}
#endif

	winInfo->lastX = x;
	winInfo->lastY = y;
	winInfo->lastWidth = width;
	winInfo->lastHeight = height;
#endif
}


/*
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

	winInfo->backendWindows = (BackendWindowInfo *) crCalloc(tilesort_spu.num_servers * sizeof(BackendWindowInfo));
	if (!winInfo->backendWindows) {
		crFree(winInfo);
		return NULL;
	}

	winInfo->server = (ServerWindowInfo *) crCalloc(tilesort_spu.num_servers * sizeof(ServerWindowInfo));
	if (!winInfo->server) {
		crFree(winInfo->backendWindows);
		crFree(winInfo);
		return NULL;
	}

	winInfo->id = window;
	winInfo->visBits = visBits;
	winInfo->bucketMode = tilesort_spu.defaultBucketMode;
	winInfo->lastX = winInfo->lastY = -1;
	winInfo->lastWidth = 0;
	winInfo->lastHeight = 0;
	/* XXX maybe examine window title and match strings to set this flag */
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
			winInfo->server[i].viewMatrix = winInfo0->server[i].viewMatrix;
			winInfo->server[i].projectionMatrix = winInfo0->server[i].projectionMatrix;
		}
	}

	crHashtableAdd(tilesort_spu.windowTable, window, winInfo);

	return winInfo;
}


/*
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
#else
	if (!winInfo->xwin)
		winInfo->xwin = xwindowID;
#endif

	return winInfo;
}


/*
 * Free a WindowInfo object.  The caller is responsible for removing
 * the object from the hash table if needed.
 */
void tilesortspuFreeWindowInfo(WindowInfo *winInfo)
{
#ifdef USE_DMX
	int i;

	for (i = 0; i < tilesort_spu.num_servers; i++) {
#if 0
		/* Don't destroy the window - its parent was probably
		 * already destroyed!
		 */
		if (winInfo->backendWindows[i].xsubwin) {
			XDestroyWindow(winInfo->backendWindows[i].dpy,
										 winInfo->backendWindows[i].xsubwin);
		}
#endif
		if (winInfo->backendWindows[i].dpy) {
			XCloseDisplay(winInfo->backendWindows[i].dpy);
		}
	}
#endif

	crFree(winInfo->backendWindows);
	crFree(winInfo->server);
	crFree(winInfo);
}


/*
 * API function: set the size of the named window.
 * We'll try to find a new tiling for the new window size either by
 * contacting the mothership, DMX, etc.
 */
void TILESORTSPU_APIENTRY tilesortspu_WindowSize(GLint window, GLint newWidth, GLint newHeight)
{
	WindowInfo *winInfo = tilesortspuGetWindowInfo(window, 0);

	CRASSERT(winInfo);

	/* Update window size */
	winInfo->lastWidth = newWidth;
	winInfo->lastHeight = newHeight;

#ifdef USE_DMX
	if (winInfo->isDMXWindow && winInfo->xwin) {
		tilesortspuGetBackendWindowInfo(winInfo);
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


/*
 * API function: set the position of the named window.
 * If running DMX, we need to get a new tiling too.
 */
void TILESORTSPU_APIENTRY tilesortspu_WindowPosition(GLint window, GLint x, GLint y)
{
	/* we only care about the window position when we're running on DMX */
#ifdef USE_DMX
	WindowInfo *winInfo = tilesortspuGetWindowInfo(window, 0);
	if (winInfo->isDMXWindow) {
		if (winInfo->xwin) /* if window's realized */
			 tilesortspuGetBackendWindowInfo(winInfo);
		tilesortspuGetNewTiling(winInfo);
	}
#endif
	(void) x;
	(void) y;
}


/*
 * API function: create a new window.
 * Input:  dpyName - display name for the window (X only)
 *         visBits - desired visual
 * Return: window ID handle or -1 if error.
 */
GLint TILESORTSPU_APIENTRY
tilesortspu_WindowCreate( const char *dpyName, GLint visBits)
{
   GET_THREAD(thread);
	ThreadInfo *thread0 = &(tilesort_spu.thread[0]);
	static GLint freeWinID = 1 /*400*/;
	WindowInfo *winInfo;
	int i;

	if (tilesort_spu.forceQuadBuffering)
		visBits |=  CR_STEREO_BIT;
	/* release geometry buffer */
	crPackReleaseBuffer(thread->packer );

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
		tilesortspuSendServerBuffer( i );

		if (!thread0->net[0].conn->actual_network)
		{
			/* XXX Revisit for DMX!!! */

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


/*
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

