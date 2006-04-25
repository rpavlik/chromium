/* Copyright (c) 2004-2006, Tungsten Graphics, Inc.
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_hash.h"
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


void
PrintRegion(const char *s, const RegionPtr r)
{
	const BoxPtr rects = REGION_RECTS(r);
	GLint n = REGION_NUM_RECTS(r), i;
	crDebug("Region %s (data at %p rects at %p)",
					s, (void*) r->data, (void*) rects);
	if (n == 0)
		crDebug("  EMPTY");
	for (i = 0; i < n; i++) {
		crDebug("  Rect %d: %d, %d .. %d, %d", i, rects[i].x1, rects[i].y1,
						rects[i].x2, rects[i].y2);
	}
}

static void
VerifyRegion(const char *s, const RegionPtr r)
{
	const BoxPtr rects = REGION_RECTS(r);
	GLint n = REGION_NUM_RECTS(r), i;
	(void) rects;
	crDebug("Verify %s", s);
	for (i = 0; i < n; i++) {
		 CRASSERT(rects[i].x1 >= 0);
		 CRASSERT(rects[i].x1 < 2000);
		 CRASSERT(rects[i].y1 >= 0);
		 CRASSERT(rects[i].y1 < 2000);
		 CRASSERT(rects[i].x2 >= 0);
		 CRASSERT(rects[i].x2 < 2000);
		 CRASSERT(rects[i].y2 >= 0);
		 CRASSERT(rects[i].y2 < 2000);
		 CRASSERT(rects[i].x1 <= rects[i].x2);
		 CRASSERT(rects[i].y1 <= rects[i].y2);
	}
	(void) VerifyRegion;
}


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


/**
 * Start the thread which handles all the RFB serving.
 */
void
vncspuStartServerThread(void)
{
	/* init locks and condition vars */
	crInitMutex(&vnc_spu.fblock);
	crInitMutex(&vnc_spu.lock);
	crInitCondition(&vnc_spu.cond);
	crInitSemaphore(&vnc_spu.updateRequested, 0);
	crInitSemaphore(&vnc_spu.dirtyRectsReady, 0);

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
		 * connect to the VNC server thread which we just started.
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


/* XXX obsolete */
CARD32
GetSerialNumber(void)
{
	if (vnc_spu.currentWindow)
		return vnc_spu.currentWindow->frameCounter;
	else
		return 0;
}


static void
InvertRegion(RegionPtr reg, int height)
{
	const BoxPtr b = REGION_RECTS(reg);
	const int n = REGION_NUM_RECTS(reg);
	int i;
#if 0
	Bool overlap;
#endif
	RegionRec newRegion;

	miRegionInit(&newRegion, NULL, 0);

	CRASSERT(miValidRegion(reg));

	for (i = 0; i < n; i++) {
		BoxRec invBox;
		RegionRec invReg;
		invBox.x1 = b[i].x1;
		invBox.y1 = height - b[i].y2;
		invBox.x2 = b[i].x2;
		invBox.y2 = height - b[i].y1;

		CRASSERT(invBox.y1 <= invBox.y2);

		miRegionInit(&invReg, &invBox, 1);
		REGION_UNION(&newRegion, &newRegion, &invReg);
		REGION_UNINIT(&invReg);
#if 0
		int y1 = b[i].y1;
		int y2 = b[i].y2;
		b[i].y1 = height - y2;
		b[i].y2 = height - y1;
#endif
	}

	CRASSERT(miValidRegion(&newRegion));

#if 0
	miRegionValidate(reg, &overlap);
	CRASSERT(miValidRegion(reg));
#else
	REGION_COPY(reg, &newRegion);
	REGION_UNINIT(&newRegion);
#endif
}


/**
 * Called by crHashtableWalk() below to get union of all windows' dirty
 * regions.
 */
static void
windowUnionCB(unsigned long key, void *data1, void *data2)
{
	WindowInfo *window = (WindowInfo *) data1;
	RegionPtr regionUnion = (RegionPtr) data2;

	/* change y=0=bottom to y=0=top */
	InvertRegion(&window->accumDirtyRegion, window->height ? window->height : 1);

	if (REGION_NUM_RECTS(&window->accumDirtyRegion) == 1 &&
			window->accumDirtyRegion.extents.x1 == 0 &&
			window->accumDirtyRegion.extents.y1 == 0 &&
			window->accumDirtyRegion.extents.x2 == 1 &&
			window->accumDirtyRegion.extents.y2 == 1) {
		/* empty / sentinal region */
	}
	else {
		/* add window offset */
		miTranslateRegion(&window->accumDirtyRegion, window->xPos, window->yPos);
	}

	/* XXX intersect w/ window bounds here? */
	REGION_UNION(regionUnion, regionUnion, &window->accumDirtyRegion);
	REGION_EMPTY(&window->accumDirtyRegion);
}


/**
 * Check/return list of dirty rects in our frame buffer.
 * This is the union of all Cr/GL windows.
 */
GLboolean
vncspuGetDirtyRects(RegionPtr region, int *frame_num)
{
	GLboolean retval;
	RegionRec dirtyUnion;

	crLockMutex(&vnc_spu.lock);

	miRegionInit(&dirtyUnion, NULL, 0);
	crHashtableWalk(vnc_spu.windowTable, windowUnionCB, &dirtyUnion);

	if (REGION_NOTEMPTY(&dirtyUnion)) {
#if 0
		/* XXX this _should_ work, and be faster */
		REGION_EMPTY(region);
		*region = dirtyUnion;
#else
		REGION_COPY(region, &dirtyUnion);
		REGION_EMPTY(&dirtyUnion);
#endif

		*frame_num = vnc_spu.frameCounter;
		retval = GL_TRUE;
	}
	else {
		retval = GL_FALSE;
	}

	crUnlockMutex(&vnc_spu.lock);
	return retval;
}


/**
 * Wait until there's new "dirty" rectangles (changed window/screen
 * regions) that lie inside the given region of interest.
 */
GLboolean
vncspuWaitDirtyRects(RegionPtr region, const BoxRec *roi,
										 int *frame_num, int serial_no)
{
	GLboolean ready;
	RegionRec clipRegion;

  REGION_INIT(&clipRegion, roi, 1);

	/* XXX this is a bit of a hack - using a condition variable instead of
	 * a semaphore would probably be best.
	 */

#ifdef NETLOGGER
	if (vnc_spu.netlogger_url) {
		NL_info("vncspu", "spu.wait.rects", "NODE=s NUMBER=i",
						vnc_spu.hostname, serial_no);
	}
#endif

	do {
		/* wait until something is rendered/changed */
		crWaitSemaphore(&vnc_spu.dirtyRectsReady);
		/* Get the new region and see if it intersects the region of interest */
		ready = vncspuGetDirtyRects(region, frame_num);
		if (ready) {
			REGION_INTERSECT(region, region, &clipRegion);
			if (REGION_NUM_RECTS(region) == 0) {
				/* the dirty region was outside the specified region of interest */
				ready = 0;
			}
		}
	} while (!ready);

#ifdef NETLOGGER
	if (vnc_spu.netlogger_url) {
		NL_info("vncspu", "spu.obtain.rects", "NODE=s NUMBER=i",
						vnc_spu.hostname, serial_no);
	}
#endif

	return ready;
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


/**
 * Compute a hash value for the given rectangle list.
 */
static GLint
ComputeRectListHash(int numRects, const BoxPtr rects)
{
	int hash = numRects, i;

	/* XXX improve this hash computation! */
	for (i = 0; i < numRects; i++) {
		hash += rects[i].x1 + rects[i].y1 + rects[i].x2 + rects[i].y2;
	}
	return hash;
}


#if 0
static void
WindowCleanUp(WindowInfo *window)
{
	 vnc_spu.dpy, window->nativeWindow,
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
	BoxRec wholeWindowRect;
	int i;
	BoxPtr clipRects;
	int numClipRects;
	RegionRec dirtyRegion;
	GLint hash;
	float ratio;
	const float ratioThreshold = 0.8;

	CRASSERT(window);

#ifdef NETLOGGER
	if (vnc_spu.netlogger_url) {
		NL_info("vncspu", "spu.readback.begin", "NUMBER=i", window->frameCounter);
	}
#endif

	/* get window size and position (in screen coords) */
	if (window->nativeWindow) {
		int size[2], pos[2];
		vnc_spu.super.GetChromiumParametervCR(GL_WINDOW_SIZE_CR,
																					window->id, GL_INT, 2, size);
		vnc_spu.super.GetChromiumParametervCR(GL_WINDOW_POSITION_CR,
																					window->id, GL_INT, 2, pos);
		if (window->width != size[0] || window->height != size[1]) {
			window->newSize = 3;
			window->width = size[0];
			window->height = size[1];
		}
		window->xPos = pos[0];
		window->yPos = pos[1];
	}
	else {
		window->width = window->height = window->xPos = window->yPos = 0;
	}

	/** Get window clip rects **/
#if defined(HAVE_XCLIPLIST_EXT)
	if (vnc_spu.haveXClipListExt) {
		if (!window->nativeWindow) {
			clipRects = NULL;
			numClipRects = 0;
		}
		else {
			clipRects = XGetClipList(vnc_spu.dpy, window->nativeWindow,
															 &numClipRects);
		}
		if (numClipRects == 0) {
			/* whole window is obscured */
			CRASSERT(!clipRects);
		}
		/* convert to y=0=bottom */
		for (i = 0; i < numClipRects; i++) {
			const int y1 = clipRects[i].y1;
			const int y2 = clipRects[i].y2;
			clipRects[i].y1 = window->height - y2;
			clipRects[i].y2 = window->height - y1;
			/*
				crDebug("ClipRect %d: %d, %d .. %d, %d", i,
				clipRects[i].x1, clipRects[i].y1,
				clipRects[i].x2, clipRects[i].y2);
			*/
		}
	}
	else
#endif /* HAVE_XCLIPLIST_EXT */
	{
		/* use whole window */
		wholeWindowRect.x1 = 0;
		wholeWindowRect.y1 = 0;
		wholeWindowRect.x2 = window->width;
		wholeWindowRect.y2 = window->height;
		clipRects = &wholeWindowRect;
		numClipRects = 1;
	}

	/* check for clipping change */
	hash = ComputeRectListHash(numClipRects, clipRects);
	if (hash != window->clippingHash) {
		/* clipping has changed */
		crDebug("Clipping change");
		window->clippingHash = hash;
		window->newSize = 3;
	}


	/**
	 ** Compute dirtyRegion - the pixel areas to read back.
	 ** (for region rects: y=0=bottom)
	 **/
	if (vnc_spu.use_bounding_boxes) {
		/* use dirty rects / regions */

		if ((window->newSize || window->isClear) && window->width && window->height) {
			/* dirty region is whole window */
			BoxRec initRec;
			initRec.x1 = 0;
			initRec.y1 = 0;
			initRec.x2 = window->width;
			initRec.y2 = window->height;
			/* curr region will replace prev region at end of frame */
			REGION_UNINIT(&window->currDirtyRegion);
			REGION_UNINIT(&window->prevDirtyRegion);
			REGION_INIT(&window->prevDirtyRegion, &initRec, 1);
			REGION_INIT(&dirtyRegion, &initRec, 1);
			if (window->newSize > 0)
				window->newSize--;
		}
		else {
			/* The dirty region is the union of the previous frame's bounding
			 * boxes and this frame's bounding boxes.
			 */
			miRegionInit(&dirtyRegion, NULL, 0);
			REGION_UNION(&dirtyRegion,
									 &window->currDirtyRegion, &window->prevDirtyRegion);
		}


		/* OPTIMIZATION:  Rather than send a bunch of little rects, see if we
		 * can just send one big rect containing the little rects.
		 */
		if (REGION_NUM_RECTS(&dirtyRegion) > 1) {
			int regionArea, extentArea;

			regionArea = miRegionArea(&dirtyRegion);
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
				BoxRec extents = *REGION_EXTENTS(&dirtyRegion);
				REGION_UNINIT(&dirtyRegion);
				miRegionInit(&dirtyRegion, &extents, 1);
				/*
				crDebug("Optimized region %d,%d .. %d, %d (%.2f ratio)",
								extents.x1, extents.y1, extents.x2, extents.y2, ratio);
				*/
			}
		}

		/* window clipping */
		if (clipRects != &wholeWindowRect) {
			/* intersect dirty region with window clipping region */
			if (numClipRects > 0) {
				RegionRec clipRegion;
				miRegionInit(&clipRegion, NULL, 0);
				miBoxesToRegion(&clipRegion, numClipRects, clipRects);
				REGION_INTERSECT(&dirtyRegion, &dirtyRegion, &clipRegion);
				REGION_UNINIT(&clipRegion);
			}
			else {
				/* dirty region is empty */
				REGION_EMPTY(&dirtyRegion);
			}
		}
	}
	else {
		/* no object bounding boxes, just use window cliprect regions */
		miRegionInit(&dirtyRegion, NULL, 0);
		if (numClipRects > 0)
			miBoxesToRegion(&dirtyRegion, numClipRects, clipRects);
	}

	/**
	 ** OK, dirtyRegion is all set now.
	 **/

	/*
	 * Now do actual pixel readback to update the screen buffer.
	 */
	{
		const BoxPtr rects = REGION_RECTS(&dirtyRegion);
		const int n = REGION_NUM_RECTS(&dirtyRegion);
#ifdef NETLOGGER
		if (vnc_spu.netlogger_url) {
			NL_info("vncspu", "spu.wait.fbmutex1", "NUMBER=i",
							window->frameCounter);
		}
#endif
		crLockMutex(&vnc_spu.fblock);
#ifdef NETLOGGER
		if (vnc_spu.netlogger_url) {
			NL_info("vncspu", "spu.readpix.begin", "NUMBER=i", window->frameCounter);
		}
#endif
		for (i = 0; i < n; i++) {
			int width = rects[i].x2 - rects[i].x1;
			int height = rects[i].y2 - rects[i].y1;
			if (width > 0 && height > 0) {
				/* source in window coords, y=0=bottom */
				int srcx = rects[i].x1;
				int srcy = rects[i].y1;
				/* dest in scrn coords, y=0=top */
				int destx = window->xPos + rects[i].x1;
				int desty = window->yPos + (window->height - rects[i].y2);
				ReadbackRegion(destx, desty, srcx, srcy, width, height);
			}
		}
		crUnlockMutex(&vnc_spu.fblock);
#ifdef NETLOGGER
		if (vnc_spu.netlogger_url) {
			NL_info("vncspu", "spu.readpix.end", "NUMBER=i", window->frameCounter);
		}
#endif
	}

	/* Having a nil dirty region is a special case */
	if (miRegionArea(&dirtyRegion) == 0) {
		if (window->isClear && window->width > 0 && window->height > 0) {
			/* send whole-window (which is blank) */
			BoxRec box;
			box.x1 = 0;
			box.y1 = 0;
			box.x2 = window->width;
			box.y2 = window->height;
			REGION_INIT(&dirtyRegion, &box, 1);
			CRASSERT(window->width > 0);
			CRASSERT(window->height > 0);
		}
		else {
			/* Add empty/dummy rect so we have _something_ to send.
			 * This is needed for framesync.  We always need to send something
			 * in response to an RFB update request in that case.
			 */
			BoxRec mtBox = {0, 0, 1, 1};  /* XXX not really empty! */
			REGION_INIT(&dirtyRegion, &mtBox, 1);
		}
	}


	/*
	 * Update accumulated window dirty region.  This keeps track of all dirty
	 * regions found between RFB update requests from the client.
	 *
	 * window->accumDirtyRegion is accessed by both threads, so we need locking.
	 */
	crLockMutex(&vnc_spu.lock);
	CRASSERT(miValidRegion(&window->accumDirtyRegion));
	REGION_UNION(&window->accumDirtyRegion,
							 &window->accumDirtyRegion, &dirtyRegion);
	CRASSERT(miValidRegion(&window->accumDirtyRegion));
	vnc_spu.frameCounter++;
	crUnlockMutex(&vnc_spu.lock);
#ifdef NETLOGGER
	if (vnc_spu.netlogger_url) {
		NL_info("vncspu", "spu.signal.rects", "NUMBER=i", window->frameCounter);
	}
#endif
	/* Tell vnc server thread that an update is ready */
	crSignalSemaphore(&vnc_spu.dirtyRectsReady);

	/* free dynamic allocations */
	if (clipRects && clipRects != &wholeWindowRect) {
		XFree(clipRects);
	}


	/*
	 * frame sync
	 */
	if (!vnc_spu.frame_drop && num_clients() > 0) {
#ifdef NETLOGGER
		if (vnc_spu.netlogger_url) {
			NL_info("vncspu", "spu.wait.signal", "NUMBER=i", window->frameCounter);
		}
#endif
		/*crDebug("Waiting for updateRequested signal");*/
		crWaitSemaphore(&vnc_spu.updateRequested);
		/*crDebug("Got updateRequested signal");*/
#ifdef NETLOGGER
		if (vnc_spu.netlogger_url) {
			NL_info("vncspu", "spu.get.signal", "NUMBER=i", window->frameCounter);
		}
#endif
	}

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


static void
vncspuUpdateFramebuffer(WindowInfo *window)
{
	GLint readBuf;

	CRASSERT(window);

	/* check/alloc the screen buffer now */
	if (!vnc_spu.screen_buffer) {
		vnc_spu.screen_buffer = (GLubyte *)
			crAlloc(vnc_spu.screen_width * vnc_spu.screen_height * 4);
		if (!vnc_spu.screen_buffer) {
			crWarning("VNC SPU: Out of memory!");
			return;
		}
	}

	window->frameCounter++;

	vnc_spu.super.GetIntegerv(GL_READ_BUFFER, &readBuf);
	vnc_spu.super.ReadBuffer(GL_FRONT);

	DoReadback(window);

	vnc_spu.super.ReadBuffer(readBuf);

	if (vnc_spu.use_bounding_boxes) {
		/* free prevDirtyRegion */
		REGION_EMPTY(&window->prevDirtyRegion);
		/* move currDirtyRegion to prevDirtyRegion */
		window->prevDirtyRegion = window->currDirtyRegion;
		/* reset curr dirty region */
		window->currDirtyRegion.data = NULL;
	}
}


static void VNCSPU_APIENTRY
vncspuSwapBuffers(GLint win, GLint flags)
{
	WindowInfo *window = LookupWindow(win, 0);

	vnc_spu.super.SwapBuffers(win, flags);

	if (window) {
		vncspuUpdateFramebuffer(window);
	}
	else {
		crWarning("VNC SPU: SwapBuffers called for invalid window id");
	}
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
			vncspuUpdateFramebuffer(window);
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

	vnc_spu.super.Clear(mask);
	REGION_EMPTY(&window->currDirtyRegion);

	window->isClear = GL_TRUE; /* window cleared, but no bounding boxes (yet) */
}


static void VNCSPU_APIENTRY
vncspuClearColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a)
{
	/* if window clear color changes, need to mark whole window dirty */
	WindowInfo *window = vnc_spu.currentWindow;
	BoxRec box;
	RegionRec whole;

	box.x1 = 0;
	box.y1 = 0;
	box.x2 = window->width;
	box.y2 = window->height;

	REGION_INIT(&whole, &box, 1);
	REGION_UNION(&window->currDirtyRegion, &window->currDirtyRegion, &whole);

	vnc_spu.super.ClearColor(r, g, b, a);
}


static void VNCSPU_APIENTRY
vncspuBoundsInfoCR(const CRrecti *bounds, const GLbyte *payload,
									 GLint len, GLint num_opcodes)
{
	const int margin = 2; /* two-pixel margin */

	vnc_spu.super.BoundsInfoCR(bounds, payload, len, num_opcodes);

	if (vnc_spu.use_bounding_boxes) {
		WindowInfo *window = vnc_spu.currentWindow;
		RegionRec newRegion;
		BoxRec newRect;

		window->isClear = GL_FALSE;

		if (bounds->x1 == -CR_MAXINT) {
			/* infinite bounding box, use whole window */
			newRect.x1 = 0;
			newRect.y1 = 0;
			newRect.x2 = window->width;
			newRect.y2 = window->height;
		}
		else {
			newRect.x1 = bounds->x1 - margin;
			newRect.y1 = bounds->y1 - margin;
			newRect.x2 = bounds->x2 + margin;
			newRect.y2 = bounds->y2 + margin;
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
 * Need to special-case DrawPixels just to compute bounding box.
 * The tilesort SPU doesn't always pass that info to the crservers.
 */
static void
vncspuDrawPixels(GLsizei width, GLsizei height, GLenum format,
								 GLenum type, const GLvoid *pixels)
{
	if (vnc_spu.use_bounding_boxes) {
		/* compute window bounds of drawpixels (y=0=bottom) */
		WindowInfo *window = vnc_spu.currentWindow;
		GLfloat pos[4], xzoom, yzoom;
		BoxRec boundsBox;
		RegionRec boundsReg;

		vnc_spu.super.GetFloatv(GL_CURRENT_RASTER_POSITION, pos);
		vnc_spu.super.GetFloatv(GL_ZOOM_X, &xzoom);
		vnc_spu.super.GetFloatv(GL_ZOOM_Y, &yzoom);

		boundsBox.x1 = (int) pos[0];
		boundsBox.y1 = (int) pos[1];
		boundsBox.x2 = boundsBox.x1 + (int) (width * xzoom + 0.5);
		boundsBox.y2 = boundsBox.y1 + (int) (height * yzoom + 0.5);

		REGION_INIT(&boundsReg, &boundsBox, 1);
		REGION_UNION(&window->currDirtyRegion,
								 &window->currDirtyRegion, &boundsReg);
	}

	vnc_spu.super.DrawPixels(width, height, format, type, pixels);
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
	{"ClearColor", (SPUGenericFunction) vncspuClearColor},
	{"BoundsInfoCR", (SPUGenericFunction) vncspuBoundsInfoCR},
	{"DrawPixels", (SPUGenericFunction) vncspuDrawPixels},
	{ NULL, NULL }
};
