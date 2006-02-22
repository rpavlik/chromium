/* Copyright (c) 2004, Tungsten Graphics, Inc.
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_spu.h"
#include "cr_mem.h"
#include "cr_threads.h"
#include "vncspu.h"

#if defined(HAVE_XCLIPLIST_EXT)
#include <X11/extensions/Xcliplist.h>
#endif

#if defined(HAVE_VNC_EXT)
#include <X11/extensions/vnc.h>
#endif

#include "async_io.h"
#include "rfblib.h"
#include "reflector.h"
#include "region.h"
#include "host_connect.h"
#include "client_io.h"


#if defined(HAVE_VNC_EXT)
/**
 * \param port  my VNC server port for clients to connect to.
 */
void
vncspuSendVncStartUpMsg(int serverPort)
{
	if (vnc_spu.haveVncExt) {
		VncConnectionList *vnclist;
		int num_conn, i;

		vnclist = XVncListConnections(vnc_spu.dpy, &num_conn);
		crDebug("VNC SPU: Found %d open VNC connection(s)", num_conn);

		for (i = 0; i < num_conn; i++) {
			int mothershipPort = 2;
			CRASSERT(vnclist->ipaddress);
			crDebug("VNC SPU: Sending ChromiumStart message to VNC server 0x%x:"
							" use port %d", vnclist[i].ipaddress, serverPort);
			XVncChromiumStart(vnc_spu.dpy, vnclist[i].ipaddress,
												serverPort, mothershipPort);
		}
	}
}
#endif


void
vncspuInitialize(void)
{
#if defined(HAVE_XCLIPLIST_EXT) || defined(HAVE_VNC_EXT)
	char *dpyStr = NULL;

	vnc_spu.dpy = XOpenDisplay(vnc_spu.display_string);
	if (!vnc_spu.dpy)
		vnc_spu.dpy = XOpenDisplay(":0");

	CRASSERT(vnc_spu.dpy);
	dpyStr = DisplayString(vnc_spu.dpy);
#endif

	/*
	 * XClipList extension - get clip rects for an X window.
	 */
#if defined(HAVE_XCLIPLIST_EXT)
	{
		 int eventBase, errorBase;
		 vnc_spu.haveXClipListExt = XClipListQueryExtension(vnc_spu.dpy,
																												&eventBase,
																												&errorBase);
		 if (vnc_spu.haveXClipListExt) {
				crDebug("VNC SPU: XClipList extension present on %s", dpyStr);
		 }
		 else {
				crWarning("VNC SPU: The display %s doesn't support the XClipList extension", dpyStr);
		 }
		 /* Note: not checking XClipList extension version at this time */
	}
#else
	crWarning("VNC SPU: Not compiled with HAVE_XCLIPLIST_EXT=1");
#endif /* HAVE_XCLIPLIST_EXT */

	/*
	 * XClipList extension - get clip rects for an X window.
	 */
#if defined(HAVE_VNC_EXT)
	{
		int major, minor;
		vnc_spu.haveVncExt = XVncQueryExtension(vnc_spu.dpy, &major, &minor);
		if (vnc_spu.haveVncExt) {
			crDebug("VNC SPU: XVnc extension present on %s", dpyStr);
		}
		else {
			crWarning("VNC SPU: The display %s doesn't support the VNC extension", dpyStr);
		}
	}
#else
	crWarning("VNC SPU: Not compiled with HAVE_VNC_EXT=1");
#endif /* HAVE_VNC_EXT */
}


void
vncspuStartServerThread(void)
{
	crInitMutex(&vnc_spu.lock);
	crInitCondition(&vnc_spu.cond);

	{
#ifdef WINDOWS
		crError("VNC SPU not supported on Windows yet");
#else
		extern void * vnc_main(void *);
		pthread_t th;
		int id = pthread_create(&th, NULL, vnc_main, NULL);
		(void) id;
#endif

		if (vnc_spu.server_port == -1) {
			/* Wait until the child thread has successfully allocated a port
			 * for clients to connect to.
			 */
			crLockMutex(&vnc_spu.lock);
			while (vnc_spu.server_port == -1) {
				crWaitCondition(&vnc_spu.cond, &vnc_spu.lock);
			}
			crUnlockMutex(&vnc_spu.lock);
		}

		crDebug("VNC SPU: VNC Server port: %d", vnc_spu.server_port);

		/*
		 * OK, we know our VNC server's port now.  Use the libVncExt library
		 * call to tell the VNC server to send the ChromiumStart message to
		 * all attached viewers.  Upon getting that message the viewers will
		 * connect the VNC server thread which we just started.
		 */
		CRASSERT(vnc_spu.server_port != -1);
#if defined(HAVE_VNC_EXT)
		vncspuSendVncStartUpMsg(vnc_spu.server_port);
#endif
	}
}


CARD32 *
GetFrameBuffer(CARD16 *w, CARD16 *h)
{
	*w = vnc_spu.screen_width;
	*h = vnc_spu.screen_height;
	return (CARD32 *) vnc_spu.screen_buffer;
}


CARD32
GetSerialNumber(void)
{
	if (vnc_spu.currentWindow)
		return vnc_spu.currentWindow->frameCounter;
	else
		return 0;
}


/*
 * Data used in callback called by aio_walk_slots().
 * Only valid while doing readback for a single window.
 */
static BoxPtr CurrentClipRects = NULL; /* Note: y=0=top */
static int CurrentClipRectsCount = 0;

/* Callback called from aio_walk_slots() below */
static void
fn_host_add_client_rect(AIO_SLOT *slot)
{
	int i;
	for (i = 0; i < CurrentClipRectsCount; i++) {
		FB_RECT r;
		r.x = CurrentClipRects[i].x1;
		r.y = CurrentClipRects[i].y1;
		r.w = CurrentClipRects[i].x2 - CurrentClipRects[i].x1;
		r.h = CurrentClipRects[i].y2 - CurrentClipRects[i].y1;
		r.enc = 0; /* not really used */
		fn_client_add_rect(slot, &r);
	}
}


/**
 * Read back a window region and place in the screen buffer.
 * \param scrx, scry - destination position in screen coords (y=0=top)
 * \param winx, winy - source position in window coords (y=0=bottom)
 * \param width, height - size of region to copy
 * Note:  y = 0 = top of screen or window
 */
static void
ReadbackRegion(int scrx, int scry, int winx, int winy, int width, int height)
{
#if RASTER_BOTTOM_TO_TOP
	{
		/* yFlipped = 0 = bottom of screen */
		const int scryFlipped = vnc_spu.screen_height - (scry + height);

		/* pack to dest coordinate */
		vnc_spu.super.PixelStorei(GL_PACK_ALIGNMENT, 1);
		vnc_spu.super.PixelStorei(GL_PACK_SKIP_PIXELS, scrx);
		vnc_spu.super.PixelStorei(GL_PACK_SKIP_ROWS, scryFlipped);
		vnc_spu.super.PixelStorei(GL_PACK_ROW_LENGTH, vnc_spu.screen_width);
		if (vnc_spu.pixel_size == 24) {
			vnc_spu.super.ReadPixels(winx, winy, width, height,
															 GL_BGR, GL_UNSIGNED_BYTE,
															 vnc_spu.screen_buffer);
		}
		else {
			vnc_spu.super.ReadPixels(winx, winy, width, height,
															 GL_BGRA, GL_UNSIGNED_BYTE,
															 vnc_spu.screen_buffer);
		}
	}
#else
	{
		GLubyte *buffer;
		GLubyte *src, *dst;
		int i;
		int pixelBytes;

		buffer = (GLubyte *) crAlloc(width * height * 4);

		if (vnc_spu.pixel_size == 24) {
			pixelBytes = 3;
			vnc_spu.super.ReadPixels(winx, winy, width, height,
															 GL_BGR, GL_UNSIGNED_BYTE, buffer);
		}
		else {
			pixelBytes = 4;
			vnc_spu.super.ReadPixels(winx, winy, width, height,
															 GL_BGRA, GL_UNSIGNED_BYTE, buffer);
		}

		/* copy/flip */
		src = buffer + (height - 1) * width * pixelBytes; /* top row */
		dst = vnc_spu.screen_buffer + (vnc_spu.screen_width * scry + scrx) * pixelBytes;
		for (i = 0; i < height; i++) {
			crMemcpy(dst, src, width * pixelBytes);
			src -= width * pixelBytes;
			dst += vnc_spu.screen_width * pixelBytes;
		}
		crFree(buffer);
	}
#endif
}


#if 0
static void
printRegions(RegionPtr rgn)
{
	int i, num;
	BoxPtr rects;
	int totalArea = 0;
	int extentArea;
	Bool overlap;

	num = REGION_NUM_RECTS(rgn);
	/*size = REGION_SIZE(rgn);*/
	rects = REGION_RECTS(rgn);
	/*
    ErrorF("extents: %d %d %d %d\n",
		rgn->extents.x1, rgn->extents.y1, rgn->extents.x2, rgn->extents.y2);
	*/
	for (i = 0; i < num; i++) {
		crDebug(" %d: %d, %d .. %d, %d", i,
	     rects[i].x1, rects[i].y1, rects[i].x2, rects[i].y2);

		totalArea += (rects[i].x2 - rects[i].x1) * (rects[i].y2 - rects[i].y1);
	}

	REGION_VALIDATE(rgn, &overlap);
	extentArea = (rgn->extents.x2 - rgn->extents.x1) * (rgn->extents.y2 - rgn->extents.y1);
	if (extentArea)
		crDebug("region area: %d  extent area: %d  percent: %f",
						totalArea, extentArea, (float) totalArea / extentArea);
}
#endif


static GLint
ComputeRectListHash(int numRects, const BoxPtr rects)
{
	int hash = numRects, i;

	for (i = 0; i < numRects; i++) {
		hash += rects[i].x1 + rects[i].y1 + rects[i].x2 + rects[i].y2;
	}
	return hash;
}


static GLint
RegionArea(const RegionPtr region)
{
	const BoxPtr rects = REGION_RECTS(region);
	GLint area = 0, n = REGION_NUM_RECTS(region), i;

	for (i = 0; i < n; i++) {
		CRASSERT(rects[i].x1 < rects[i].x2);
		CRASSERT(rects[i].y1 < rects[i].y2);
		area += (rects[i].x2 - rects[i].x1) * (rects[i].y2 - rects[i].y1);
	}
	return area;
}


#if 1
static void
PrintRegion(const char *s, const RegionPtr r)
{
	const BoxPtr rects = REGION_RECTS(r);
	GLint n = REGION_NUM_RECTS(r), i;
	crDebug("Region %s", s);
	for (i = 0; i < n; i++) {
		crDebug("  Rect %d: %d, %d .. %d, %d", i, rects[i].x1, rects[i].y1,
						rects[i].x2, rects[i].y2);
	}
	(void) PrintRegion;
}
#endif


/**
 * Read back the image from the OpenGL window and store in the screen buffer
 * (i.e. vnc_spu.screen_buffer) at the given x/y screen position.
 * Then, update the dirty rectangle info.
 */
static void
DoReadback(WindowInfo *window)
{
	int size[2], pos[2];
	int winX, winY, winWidth, winHeight;
	GLboolean newWindowSize = GL_FALSE;
	BoxRec wholeWindowRect;
	int i;
	BoxPtr clipRects;
	int numClipRects;
	RegionRec dirtyRegion;
	GLint hash;
	float ratio;
	const float ratioThreshold = 0.8;

	CRASSERT(window);

	if (!window->nativeWindow) {
		return;
	}

#ifdef NETLOGGER
	if (vnc_spu.netlogger_url) {
		NL_info("vncspu", "spu.readback.begin", "NUMBER=i", window->frameCounter);
	}
#endif

	/* get window size and position (in screen coords) */
	vnc_spu.super.GetChromiumParametervCR(GL_WINDOW_SIZE_CR,
																				window->id, GL_INT, 2, size);
	vnc_spu.super.GetChromiumParametervCR(GL_WINDOW_POSITION_CR,
																				window->id, GL_INT, 2, pos);
	winX = pos[0];
	winY = pos[1];
	winWidth = size[0];
	winHeight = size[1];

	/* check for window size change */
	if (window->prevWidth != winWidth || window->prevHeight != winHeight) {
		newWindowSize = GL_TRUE;
		window->prevWidth = winWidth;
		window->prevHeight = winHeight;
	}

	/* check/alloc the screen buffer now */
	if (!vnc_spu.screen_buffer) {
		vnc_spu.screen_buffer = (GLubyte *)
			crAlloc(vnc_spu.screen_width * vnc_spu.screen_height * 4);
		if (!vnc_spu.screen_buffer) {
			crWarning("VNC SPU: Out of memory!");
			return;
		}
	}

	CRASSERT(winWidth >= 1);
	CRASSERT(winHeight >= 1);

	/** Get window clip rects **/
#if defined(HAVE_XCLIPLIST_EXT)
	if (vnc_spu.haveXClipListExt) {
		clipRects = XGetClipList(vnc_spu.dpy, window->nativeWindow,
														 &numClipRects);
		if (numClipRects == 0) {
			/* whole window is obscured */
			CRASSERT(!clipRects);
			return;
		}
		if (vnc_spu.use_bounding_boxes) {
			/* convert to y=0=bottom */
			for (i = 0; i < numClipRects; i++) {
				const int y1 = clipRects[i].y1;
				const int y2 = clipRects[i].y2;
				clipRects[i].y1 = winHeight - y2;
				clipRects[i].y2 = winHeight - y1;
			}
		}
	}
	else
#endif /* HAVE_XCLIPLIST_EXT */
	{
		/* whole window */
		wholeWindowRect.x1 = 0;
		wholeWindowRect.y1 = 0;
		wholeWindowRect.x2 = winWidth;
		wholeWindowRect.y2 = winHeight;
		clipRects = &wholeWindowRect;
		numClipRects = 1;
	}

	hash = ComputeRectListHash(numClipRects, clipRects);

	if (hash != window->clippingHash) {
		/* clipping changed */
		crDebug("Clipping change");
		window->clippingHash = hash;
		newWindowSize = GL_TRUE;
	}


	CRASSERT(numClipRects >= 1);
	CRASSERT(clipRects);

	if (vnc_spu.use_bounding_boxes) {
		/* use dirty rects / regions */
		BoxPtr rects;
		int regionArea, extentArea;

		if (newWindowSize) {
			/* dirty region is whole window */
			BoxRec initRec;
			initRec.x1 = 0;
			initRec.y1 = 0;
			initRec.x2 = winWidth;
			initRec.y2 = winHeight;
			/* curr region will replace prev region at end of frame */
			REGION_UNINIT(&window->currDirtyRegion);
			REGION_UNINIT(&window->prevDirtyRegion);
			REGION_INIT(&window->prevDirtyRegion, &initRec, 1);
			REGION_INIT(&dirtyRegion, &initRec, 1);
		}
		else {
			/* The dirty region is the union of the previous frame's bounding
			 * boxes and this frame's bounding boxes.
			 */
			miRegionInit(&dirtyRegion, NULL, 0);
			REGION_UNION(&dirtyRegion,
									 &window->currDirtyRegion, &window->prevDirtyRegion);
			/*
				PrintRegion("dirty", &dirtyRegion);
			*/
		}

		regionArea = RegionArea(&dirtyRegion);
		extentArea = (dirtyRegion.extents.x2 - dirtyRegion.extents.x1)
			* (dirtyRegion.extents.y2 - dirtyRegion.extents.y1);
		ratio = (float) regionArea / extentArea;
		/*
		crDebug("Region ratio filled: %.2f (%ld boxes) (ext area %d)",
						ratio,
						REGION_NUM_RECTS(&dirtyRegion),
						extentArea);
		*/
		if (ratio >= ratioThreshold) {
			/* OPTIMIZATION:  Rather than send a bunch of sub-rects, just
			 * send the whole region contained by the bounding box.
			 */
			BoxRec extents = *REGION_EXTENTS(&dirtyRegion);
			REGION_UNINIT(&dirtyRegion);
			miRegionInit(&dirtyRegion, &extents, 1);
			/*
			crDebug("Optimized region %d,%d .. %d, %d",
							extents.x1, extents.y1, extents.x2, extents.y2);
			*/
		}

		/* window clipping */
		if (clipRects != &wholeWindowRect) {
			/* intersect dirty region with window clipping region */
			RegionRec clipRegion;
			miRegionInit(&clipRegion, NULL, 0);
			miBoxesToRegion(&clipRegion, numClipRects, clipRects);
			REGION_INTERSECT(&dirtyRegion, &dirtyRegion, &clipRegion);
			REGION_UNINIT(&clipRegion);
		}

		/* convert rect list to y=0=top orientation */
		rects = REGION_RECTS(&dirtyRegion);
		CurrentClipRectsCount = REGION_NUM_RECTS(&dirtyRegion);
		CurrentClipRects = (BoxPtr) crAlloc(sizeof(BoxRec) * CurrentClipRectsCount);
		for (i = 0; i < CurrentClipRectsCount; i++) {
			/* convert y=0=bottom to y=0=top */
			CurrentClipRects[i].x1 = rects[i].x1;
			CurrentClipRects[i].y1 = winHeight - rects[i].y2;
			CurrentClipRects[i].x2 = rects[i].x2;
			CurrentClipRects[i].y2 = winHeight - rects[i].y1;
			CRASSERT(CurrentClipRects[i].x1 < CurrentClipRects[i].x2);
			CRASSERT(CurrentClipRects[i].y1 < CurrentClipRects[i].y2);
		}

		REGION_UNINIT(&dirtyRegion);
	}
	else {
		/* no object bounding boxes, just use window cliprect regions */
		CurrentClipRects = clipRects;
		CurrentClipRectsCount = numClipRects;
	}


	CRASSERT(CurrentClipRects);
#if 0
	for (i = 0; i < CurrentClipRectsCount; i++) {
		crDebug("ClipRect %d: %d, %d .. %d, %d", i,
						CurrentClipRects[i].x1, CurrentClipRects[i].y1,
						CurrentClipRects[i].x2, CurrentClipRects[i].y2);
	}
#endif

	/*
	 * Now do actual pixel readback to update the screen buffer.
	 */
	for (i = 0; i < CurrentClipRectsCount; i++) {
		BoxPtr r = CurrentClipRects + i;     /* in window coords, y=0=top */
		int width = r->x2 - r->x1;
		int height = r->y2 - r->y1;
		int srcx = r->x1;                    /* in window coords */
		int srcy = winHeight - r->y2;        /* in window coords, y=0=bottom */
		int destx = winX + r->x1;            /* in screen coords */
		int desty = winY + r->y1;            /* in screen coords, y=0=top */
		CRASSERT(width > 0);
		CRASSERT(height > 0);
		ReadbackRegion(destx, desty, srcx, srcy, width, height);
	}

	/* translate cliprects to screen coords for the VNC server */
	for (i = 0; i < CurrentClipRectsCount; i++) {
		BoxPtr r = CurrentClipRects + i;
		r->x1 += winX;
		r->y1 += winY;
		r->x2 += winX;
		r->y2 += winY;
	}

	/* append the dirty rectlist to all clients' pending lists */
	crLockMutex(&vnc_spu.lock);
	aio_walk_slots(fn_host_add_client_rect, TYPE_CL_SLOT);
	crUnlockMutex(&vnc_spu.lock);

	/* free dynamic allocations */
	if (clipRects != &wholeWindowRect) {
		XFree(clipRects);
	}
	if (vnc_spu.use_bounding_boxes) {
		crFree(CurrentClipRects);
	}
	CurrentClipRects = NULL;
	CurrentClipRectsCount = 0;

	/* Send the new dirty rects to clients */
	/* XXX at this point we should just "nudge" the other thread to tell
	 * it to send the new updates to the clients.
	 */
	aio_walk_slots(fn_client_send_rects, TYPE_CL_SLOT);


#ifdef NETLOGGER
	if (vnc_spu.netlogger_url) {
		NL_info("vncspu", "spu.readback.end", "NUMBER=i", window->frameCounter);
	}
#endif
}


/**
 * Given an integer window ID, return the WindowInfo.  Create a new
 * WindowInfo for new ID.
 */
static WindowInfo *
LookupWindow(GLint win, GLint nativeWindow)
{
	WindowInfo *window;
	window = (WindowInfo *) crHashtableSearch(vnc_spu.windowTable, win);
	if (!window) {
		/* create new */
		window = (WindowInfo *) crCalloc(sizeof(WindowInfo));
		crHashtableAdd(vnc_spu.windowTable, win, window);
		window->id = win;
	}

	if (window->nativeWindow != nativeWindow && nativeWindow != 0) {
		/* got the handle to a particular X window */
		window->nativeWindow = nativeWindow;
	}

	return window;
}


static void VNCSPU_APIENTRY
vncspuSwapBuffers(GLint win, GLint flags)
{
	WindowInfo *window = LookupWindow(win, 0);
	if (window) {
		window->frameCounter++;

		DoReadback(window);

		if (vnc_spu.use_bounding_boxes) {
			/* free prevDirtyRegion */
			REGION_UNINIT(&window->prevDirtyRegion);
			/* move currDirtyRegion to prevDirtyRegion */
			window->prevDirtyRegion = window->currDirtyRegion;
			/* reset curr dirty region */
			window->currDirtyRegion.data = NULL;
		}
	}
	else {
		crWarning("VNC SPU: SwapBuffers called for invalid window id");
	}
	vnc_spu.super.SwapBuffers(win, flags);
}


/**
 * For both glFlush and glFinish.
 * Update VNC data if single-buffered.
 */
static void VNCSPU_APIENTRY
vncspuFinish(void)
{
	GLboolean db;
	vnc_spu.self.GetBooleanv(GL_DOUBLEBUFFER, &db);
	if (!db) {
		/* single buffered: update VNC info */
		WindowInfo *window = vnc_spu.currentWindow;
		if (window) {
			DoReadback(window);
		}
	}
}


static void VNCSPU_APIENTRY
vncspuMakeCurrent(GLint win, GLint nativeWindow, GLint ctx)
{
	if (win >= 0) {
		WindowInfo *window = LookupWindow(win, nativeWindow);
		CRASSERT(window);
		vnc_spu.currentWindow = window;
	}
	else {
		vnc_spu.currentWindow = NULL;
	}
	vnc_spu.super.MakeCurrent(win, nativeWindow, ctx);
}


static void VNCSPU_APIENTRY
vncspuClear(GLbitfield mask)
{
	WindowInfo *window = vnc_spu.currentWindow;

	/*crDebug("VNC SPU Clear");*/
	vnc_spu.super.Clear(mask);
	REGION_UNINIT(&window->currDirtyRegion);
	REGION_EMPTY(&window->currDirtyRegion);
}


static void VNCSPU_APIENTRY
vncspuBoundsInfoCR(const CRrecti *bounds, const GLbyte *payload,
									 GLint len, GLint num_opcodes)
{
	vnc_spu.super.BoundsInfoCR(bounds, payload, len, num_opcodes);

	if (vnc_spu.use_bounding_boxes) {
		WindowInfo *window = vnc_spu.currentWindow;
		RegionRec newRegion;
		BoxRec newRect;

		if (bounds->x1 == -CR_MAXINT) {
			/* infinite bounding box, use whole window */
			newRect.x1 = 0;
			newRect.y1 = 0;
			newRect.x2 = window->prevWidth;
			newRect.y2 = window->prevHeight;
		}
		else {
			newRect.x1 = bounds->x1;
			newRect.y1 = bounds->y1;
			newRect.x2 = bounds->x2;
			newRect.y2 = bounds->y2;
		}

		/*
		crDebug("VNC SPU BoundsInfo %i, %i .. %i, %i",
						newRect.x1, newRect.y1, newRect.x2, newRect.y2);
		*/
		CRASSERT(newRect.x1 <= newRect.x2);
		CRASSERT(newRect.y1 <= newRect.y2);

		REGION_INIT(&newRegion, &newRect, 1);
		REGION_UNION(&window->currDirtyRegion, &window->currDirtyRegion, &newRegion);
	}
}


/**
 * SPU function table
 */
SPUNamedFunctionTable _cr_vnc_table[] = {
	{"SwapBuffers", (SPUGenericFunction) vncspuSwapBuffers},
	{"MakeCurrent", (SPUGenericFunction) vncspuMakeCurrent},
	{"Finish", (SPUGenericFunction) vncspuFinish},
	{"Flush", (SPUGenericFunction) vncspuFinish},
	{"Clear", (SPUGenericFunction) vncspuClear},
	{"BoundsInfoCR", (SPUGenericFunction) vncspuBoundsInfoCR},
	{ NULL, NULL }
};
