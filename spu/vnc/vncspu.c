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


/*
 * Move SRC region to DST region, leaving SRC empty.
 */
#define MOVE_REGION(DST, SRC) \
  do { \
    REGION_EMPTY((DST)); \
    *(DST) = *(SRC); \
    (SRC)->data = NULL; \
    REGION_EMPTY((SRC)); \
	} while (0)


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


/**
 * Compute a hash value for the given region.
 */
static GLint
RegionHash(const RegionPtr region)
{
	const BoxPtr rects = REGION_RECTS(region);
	const GLint n = REGION_NUM_RECTS(region);
	int hash = n, i;

	/* XXX improve this hash computation! */
	for (i = 0; i < n; i++) {
		hash += rects[i].x1 + rects[i].y1 + rects[i].x2 + rects[i].y2;
	}
	return hash;
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

	vnc_spu.timer = crTimerNewTimer();

	crDebug("VNC SPU: double buffering: %d", vnc_spu.double_buffer);
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


/**
 * Encoders call this function to get pointer to framebufer data.
 */
CARD32 *
GetFrameBuffer(CARD16 *w, CARD16 *h)
{
	CRASSERT(vnc_spu.screen_buffer_locked);
	*w = vnc_spu.screen_width;
	*h = vnc_spu.screen_height;
	return (CARD32 *) vnc_spu.screen_buffer[0]->buffer;
}


/**
 * Called by the RFB encoder to lock access to 0th screen_buffer.
 */
void
vncspuLockFrameBuffer(void)
{
	crLockMutex(&vnc_spu.fblock);
	vnc_spu.screen_buffer_locked = GL_TRUE;
	if (vnc_spu.double_buffer) {
		crUnlockMutex(&vnc_spu.fblock);
	}
	else {
		/* keep/hold lock when single-buffered */
	}
}


/**
 * Called by the RFB encoder thread to unlock access to 0th screen_buffer.
 */
void
vncspuUnlockFrameBuffer(void)
{
	ScreenBuffer *sb;

	if (vnc_spu.double_buffer) {
		crLockMutex(&vnc_spu.fblock);
	}
	else {
		/* lock is already held when single-buffered */
	}

	CRASSERT(vnc_spu.screen_buffer_locked);

	sb = vnc_spu.screen_buffer[0];

	sb->regionSent = GL_TRUE;

	/*
	if (miRegionArea(&sb->dirtyRegion) > 0)
		crDebug("In %s non-empty dirty region", __FUNCTION__);
	*/
	REGION_EMPTY(&sb->dirtyRegion);

	vnc_spu.screen_buffer_locked = GL_FALSE;
	crUnlockMutex(&vnc_spu.fblock);
}


/**
 * Vertically invert a region.
 */
static void
InvertRegion(RegionPtr reg, int height)
{
	const BoxPtr b = REGION_RECTS(reg);
	const int n = REGION_NUM_RECTS(reg);
	int i;
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
	}

	CRASSERT(miValidRegion(&newRegion));

	REGION_COPY(reg, &newRegion);
	REGION_UNINIT(&newRegion);
}


/**
 * Update the per-window accumulated dirty region.
 * This is used when our virtual framebuffer is double-bufferd.
 * Basically, we have an accumulated dirty region for both buffers.
 * When we swap, we discard the previous-accum-dirty buffer, replace it
 * with the current-accum-dirty buffer, then reset the current-accum-dirty
 * buffer to empty.
 *
 * Called by crHashtableWalk().
 */
static void
WindowUpdateAccumCB(unsigned long key, void *windowData, void *regionData)
{
	WindowInfo *window = (WindowInfo *) windowData;

	CRASSERT(miValidRegion(&window->prevAccumDirtyRegion));
	CRASSERT(miValidRegion(&window->accumDirtyRegion));

	MOVE_REGION(&window->prevAccumDirtyRegion,
							&window->accumDirtyRegion);

	CRASSERT(miValidRegion(&window->prevAccumDirtyRegion));
}


/**
 * Union the window's accum-dirty and prev-accum-dirty regions with the
 * incoming region.  Called during SwapBuffers to determine the current
 * dirty rendering region.
 *
 * Called by crHashtableWalk().
 */
static void
WindowDirtyUnionCB(unsigned long key, void *windowData, void *regionData)
{
	WindowInfo *window = (WindowInfo *) windowData;
	RegionPtr regionUnion = (RegionPtr) regionData;
	RegionRec accumScrn; /* accumulated region, in screen coords */
	Bool overlap;

	miRegionInit(&accumScrn, NULL, 0); /* init local var */

	CRASSERT(miValidRegion(&window->accumDirtyRegion));
	CRASSERT(miValidRegion(&window->prevAccumDirtyRegion));

	/*
	crDebug("accum area: %d   prev accum: %d",
					miRegionArea(&window->accumDirtyRegion),
					miRegionArea(&window->prevAccumDirtyRegion));
	*/

	/* at first, accumScrn region is in window coords */
	REGION_UNION(&accumScrn,
							 &window->accumDirtyRegion,
							 &window->prevAccumDirtyRegion);

	/* intersect with window bounds */
	REGION_INTERSECT(&accumScrn,
									 &accumScrn,
									 &window->clipRegion);

	REGION_VALIDATE(&accumScrn, &overlap);

	/* change y=0=bottom to y=0=top */
	InvertRegion(&accumScrn, window->height ? window->height : 1);

	if (REGION_NUM_RECTS(&accumScrn) == 1 &&
			accumScrn.extents.x1 == 0 &&
			accumScrn.extents.y1 == 0 &&
			accumScrn.extents.x2 == 1 &&
			accumScrn.extents.y2 == 1) {
		/* empty / sentinal region */
	}
	else {
		/* add window offset */
		miTranslateRegion(&accumScrn, window->xPos, window->yPos);
	}

	/* now, accumScrn region is in screen coords */

	REGION_UNION(regionUnion, regionUnion, &accumScrn);

	REGION_UNINIT(&accumScrn); /* done with local var */
}


/**
 * Get list of dirty rects in current virtual frame buffer.
 * This is the union of all Cr/GL windows.
 * NOTE: Called from vnc server thread.
 * \return  GL_TRUE if any dirty rects, GL_FALSE if no dirty rects
 */
GLboolean
vncspuGetDirtyRects(RegionPtr region)
{
	GLboolean retVal;

	crLockMutex(&vnc_spu.fblock);

	if (REGION_NUM_RECTS(&vnc_spu.screen_buffer[0]->dirtyRegion) > 0) {
		REGION_COPY(region, &vnc_spu.screen_buffer[0]->dirtyRegion);
		REGION_EMPTY(&vnc_spu.screen_buffer[0]->dirtyRegion);
		retVal = GL_TRUE;
	}
	else {
		retVal = GL_FALSE;
	}

	crUnlockMutex(&vnc_spu.fblock);

	return retVal;
}


/**
 * Wait until there's some new data inside the given region of interest (roi).
 * Return the region in 'region' and return GL_TRUE.
 * NOTE: Called from vnc server thread.
 */
GLboolean
vncspuWaitDirtyRects(RegionPtr region, const BoxRec *roi, int serial_no)
{
	RegionRec roiRegion;

  REGION_INIT(&roiRegion, roi, 1);

	while (1) {
		/* wait until something is rendered/changed */
		crWaitSemaphore(&vnc_spu.dirtyRectsReady);

		crLockMutex(&vnc_spu.fblock);

		/* Get the new region and see if it intersects the region of interest */
		REGION_INTERSECT(region,
										 &vnc_spu.screen_buffer[0]->dirtyRegion,
										 &roiRegion);

		if (REGION_NUM_RECTS(region) > 0) {
			vnc_spu.screen_buffer_locked = GL_TRUE;
			REGION_EMPTY(&vnc_spu.screen_buffer[0]->dirtyRegion);

			/*
			crDebug("Wait: region area %d", miRegionArea(region));
			*/

			crUnlockMutex(&vnc_spu.fblock);
			return GL_TRUE;
		}

		crUnlockMutex(&vnc_spu.fblock);
	}
}


/**
 * Read back a window rectangle, placing pixels in the screen buffer.
 * \param scrx, scry - destination position in screen coords (y=0=top)
 * \param winx, winy - source position in window coords (y=0=bottom)
 * \param width, height - size of region to copy
 * Note:  y = 0 = top of screen or window
 */
static void
ReadbackRect(int scrx, int scry, int winx, int winy, int width, int height)
{
	ScreenBuffer *sb = vnc_spu.double_buffer
		? vnc_spu.screen_buffer[1] : vnc_spu.screen_buffer[0];

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
															 GL_BGR, GL_UNSIGNED_BYTE, sb->buffer);
		}
		else {
			vnc_spu.super.ReadPixels(winx, winy, width, height,
															 GL_BGRA, GL_UNSIGNED_BYTE, sb->buffer);
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
															 GL_BGR, GL_UNSIGNED_BYTE, sb->buffer);
		}
		else {
			pixelBytes = 4;
			vnc_spu.super.ReadPixels(winx, winy, width, height,
															 GL_BGRA, GL_UNSIGNED_BYTE, sb->buffer);
		}

		/* copy/flip */
		src = sb->buffer + (height - 1) * width * pixelBytes; /* top row */
		dst = sb->buffer + (vnc_spu.screen_width * scry + scrx) * pixelBytes;
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
 * Called at end-of-frame to swap the ScreenBuffers, if double-buffering
 * is enabled.
 * Note that this has nothing to do with double-buffered OpenGL windows.
 * Our virtual framebuffer / screenbuffer can be double buffered so that
 * the application can write to one buffer while the VNC encoder/server
 * thread uses the other ScreenBuffer.
 *
 * We can only swap buffers while the encoder thread does not have a
 * ScreenBuffer locked.
 *
 * The screen buffer that the main thread puts pixels into and the RFB
 * encoders grab pixels from is double-buffered to avoid a big lock around
 * a single buffer.
 * screen_buffer[0] is read by the encoders while screen_buffer[1] is written
 * by the main thread/glReadPixels.
 *
 * \return GL_TRUE if we actually swapped, GL_FALSE otherwise.
 */
static GLboolean
SwapScreenBuffers(void)
{
	GLboolean retVal = GL_FALSE;
	ScreenBuffer *sb;

	crLockMutex(&vnc_spu.fblock);

	if (vnc_spu.double_buffer) {
		if (!vnc_spu.screen_buffer_locked) {
			/* swap [0] [1] ScreenBuffer pointers */
			ScreenBuffer *tmp;
			tmp = vnc_spu.screen_buffer[0];
			vnc_spu.screen_buffer[0] = vnc_spu.screen_buffer[1];
			vnc_spu.screen_buffer[1] = tmp;
			retVal = GL_TRUE;
		}
		else {
			/*crDebug("No swap - locked");*/
			retVal = GL_FALSE;
		}
	}
	else {
		/* Not double buffering, but return TRUE so that dirty regions get
		 * handled properly.
		 */
		retVal = GL_TRUE;
	}

	/* check if the screenbuffer's dirty region was sent to client */
	sb = vnc_spu.double_buffer
		? vnc_spu.screen_buffer[1] : vnc_spu.screen_buffer[0];
	if (sb->regionSent) {
		/* for all windows, update the accumulated region info */
		crHashtableWalk(vnc_spu.windowTable, WindowUpdateAccumCB, NULL);
		sb->regionSent = GL_FALSE;
	}

	/* get new screen-space dirty rectangle list by walking over all windows */
	sb = vnc_spu.screen_buffer[0];
	REGION_EMPTY(&sb->dirtyRegion);
	crHashtableWalk(vnc_spu.windowTable, WindowDirtyUnionCB,
									&sb->dirtyRegion);

	/* If we can't drop frames, we always have to send _something_ */
	if (vnc_spu.frame_drop == 0 && REGION_NIL(&sb->dirtyRegion)) {
		/* make dummy 1x1 region */
		BoxRec b;
		b.x1 = b.y1 = 0;
		b.x2 = b.y2 = 1;
		REGION_INIT(&sb->dirtyRegion, &b, 1);
		CRASSERT(miRegionArea(&sb->dirtyRegion) > 0);
	}

	if (miRegionArea(&sb->dirtyRegion) > 0) {
		/* Tell vnc server thread that new pixel data is available */
#ifdef NETLOGGER_foo
		if (vnc_spu.netlogger_url) {
			NL_info("vncspu", "spu.signal.rects", "NUMBER=i",
							window->frameCounter);
		}
#endif
		crSignalSemaphore(&vnc_spu.dirtyRectsReady);
	}

	crUnlockMutex(&vnc_spu.fblock);

	return retVal;
}


/**
 * Update window's width, height fields.
 */
static void
GetWindowSize(WindowInfo *window)
{
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
}


/**
 * Update window's clipRegion field.
 */
static void
GetWindowBounds(WindowInfo *window)
{
	REGION_UNINIT(&window->clipRegion);

#if defined(HAVE_XCLIPLIST_EXT)
	if (vnc_spu.haveXClipListExt && window->nativeWindow) {
		int numClipRects, i;
		BoxPtr clipRects = XGetClipList(vnc_spu.dpy, window->nativeWindow,
																		&numClipRects);
		/* convert to y=0=bottom (OpenGL style) */
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
		miBoxesToRegion(&window->clipRegion, numClipRects, clipRects);
		XFree(clipRects);
	}
	else
#endif /* HAVE_XCLIPLIST_EXT */
	{
		/* use whole window */
		BoxRec windowRect;
		windowRect.x1 = 0;
		windowRect.y1 = 0;
		windowRect.x2 = window->width;
		windowRect.y2 = window->height;
		miRegionInit(&window->clipRegion, &windowRect, 1);
	}
}



/**
 * Read back the image from the OpenGL window and store in the screen buffer
 * (i.e. vnc_spu.screen_buffer) at the given x/y screen position.
 * Then, update the dirty rectangle info.
 */
static void
DoReadback(WindowInfo *window)
{
	int i;
	RegionRec dirtyRegion;
	GLint hash;
	float ratio;
	const float ratioThreshold = 0.8;
	GLboolean swap;

	CRASSERT(window);

	miRegionInit(&dirtyRegion, NULL, 0); /* init this local var */

#ifdef NETLOGGER
	if (vnc_spu.netlogger_url) {
		NL_info("vncspu", "spu.readback.begin", "NUMBER=i", window->frameCounter);
	}
#endif

	GetWindowSize(window);
	GetWindowBounds(window);
	hash = RegionHash(&window->clipRegion);
	if (hash != window->clippingHash) {
		/* clipping has changed */
		window->clippingHash = hash;
		window->newSize = 3;
	}


	/**
	 ** Compute dirtyRegion - the pixel areas to read back.
	 ** (for region rects: y=0=bottom)
	 **/
	if (vnc_spu.use_bounding_boxes) {
		/* use the window's curr/prev dirty regions */

		if ((window->newSize || window->isClear)
				&& window->width && window->height) {
			/* dirty region is whole window */
			BoxRec windowBox;
			windowBox.x1 = 0;
			windowBox.y1 = 0;
			windowBox.x2 = window->width;
			windowBox.y2 = window->height;
			/* discard accum dirty region */
			REGION_EMPTY(&window->accumDirtyRegion);
			/* discard prev dirty region */
			REGION_EMPTY(&window->prevDirtyRegion);
			/* set curr dirty region to whole window */
			REGION_EMPTY(&window->currDirtyRegion);
			REGION_INIT(&window->currDirtyRegion, &windowBox, 1);
			/* decrement counter */
			if (window->newSize > 0)
				window->newSize--;
		}

		/* The dirty region is the union of the previous frame's bounding
		 * boxes and this frame's bounding boxes.
		 */
		REGION_UNION(&dirtyRegion,
								 &window->currDirtyRegion, &window->prevDirtyRegion);

		/* Add the dirty region to the accumulated dirty region */
		REGION_UNION(&window->accumDirtyRegion,
								 &window->accumDirtyRegion,
								 &dirtyRegion);
		/*
		crDebug("new accum dirty area: %d", miRegionArea(&window->accumDirtyRegion));
		*/

		/* add previous ScreenBuffer's dirty region */
		REGION_UNION(&dirtyRegion,
								 &dirtyRegion,
								 &window->prevAccumDirtyRegion);

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
				REGION_EMPTY(&dirtyRegion);
				miRegionInit(&dirtyRegion, &extents, 1);
				/*
				crDebug("Optimized region %d,%d .. %d, %d (%.2f ratio)",
								extents.x1, extents.y1, extents.x2, extents.y2, ratio);
				*/
			}
		}

		/* window clipping */
		REGION_INTERSECT(&dirtyRegion, &dirtyRegion, &window->clipRegion);
	}
	else {
		/* no object bounding boxes, so dirty region = window cliprect region */
		REGION_COPY(&dirtyRegion, &window->clipRegion);
		/* the window's accumulated dirty region is the clip region */
		REGION_COPY(&window->accumDirtyRegion, &window->clipRegion);
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
				ReadbackRect(destx, desty, srcx, srcy, width, height);
			}
		}

#ifdef NETLOGGER
		if (vnc_spu.netlogger_url) {
			NL_info("vncspu", "spu.readpix.end", "NUMBER=i", window->frameCounter);
		}
#endif
	}

	vnc_spu.frameCounter++;

	swap = SwapScreenBuffers();
	if (swap && vnc_spu.use_bounding_boxes) {
		/* If we really did swap, shift curr dirty region to prev dirty region.
		 * Afterward, currDirtyRegion will be empty.
		 */
		MOVE_REGION(&window->prevDirtyRegion,
								&window->currDirtyRegion);
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
		window->prevSwapTime = -1.0;
	}

	if (window->nativeWindow != nativeWindow && nativeWindow != 0) {
		/* got the handle to a particular X window */
		window->nativeWindow = nativeWindow;
	}

	return window;
}


static ScreenBuffer *
AllocScreenbuffer(void)
{
	ScreenBuffer *s = crCalloc(sizeof(ScreenBuffer));
	if (s) {
		s->buffer = (GLubyte *)
			crAlloc(vnc_spu.screen_width * vnc_spu.screen_height * 4);
		if (!s->buffer) {
			crFree(s);
			s = NULL;
		}
	}
	if (!s) {
		crError("VNC SPU: Out of memory allocating %d x %d screen buffer",
						vnc_spu.screen_width, vnc_spu.screen_height);
	}
	return s;
}


/**
 * Called by SwapBuffers, glFinish, glFlush after something's been rendered.
 * Determine which window regions have changed, call ReadPixels to store
 * those regions into the framebuffer.
 */
static void
vncspuUpdateFramebuffer(WindowInfo *window)
{
	GLint readBuf;

	CRASSERT(window);

	/* check/alloc the screen buffer(s) now */
	if (!vnc_spu.screen_buffer[0]) {
		vnc_spu.screen_buffer[0] = AllocScreenbuffer();
		if (!vnc_spu.screen_buffer[0])
			return;
	}
	if (!vnc_spu.screen_buffer[1]) {
		vnc_spu.screen_buffer[1] = AllocScreenbuffer();
		if (!vnc_spu.screen_buffer[1])
			return;
	}

	window->frameCounter++;

	vnc_spu.super.GetIntegerv(GL_READ_BUFFER, &readBuf);
	vnc_spu.super.ReadBuffer(GL_FRONT);

	DoReadback(window);

	vnc_spu.super.ReadBuffer(readBuf);
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

	/* compute FPS (update rate) */
	{
		const double minPeriod = 10.0; /* seconds */
		double t = crTimerTime(vnc_spu.timer);
		if (window->prevSwapTime < 0.0) {
			/* first-time init */
			window->prevSwapTime = t;
			window->swapCounter = 0;
		}
		else {
			window->swapCounter++;
			if (t - window->prevSwapTime >= minPeriod) {
				/* report rate */
				double rate = window->swapCounter / (t - window->prevSwapTime);
				window->prevSwapTime = t;
				window->swapCounter = 0;
				/*
				crDebug("VNC SPU: SwapBuffers frame rate: %.2f", rate);
				*/
				(void) rate;
#ifdef NETLOGGER
				if (vnc_spu.netlogger_url) {
					NL_info("vncspu", "spu.updates_per_sec", "RATE=d", rate);
				}
#endif
			}
		}
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
