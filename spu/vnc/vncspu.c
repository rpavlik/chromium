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


#define DB 0


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

	InitScreenBufferQueue(&vnc_spu.emptyQueue, "empty");
	InitScreenBufferQueue(&vnc_spu.filledQueue, "filled");
	{
		int i;
		/* create screen buffers and put them into the 'empty buffer' queue */
		crDebug("VNC SPU: Creating %d screen buffers", vnc_spu.double_buffer + 1);
		for (i = 0; i <= vnc_spu.double_buffer; i++) {
			ScreenBuffer *b = AllocScreenBuffer();
			EnqueueBuffer(b, &vnc_spu.emptyQueue);
		}
	}
}


/**
 * Start the thread which handles all the RFB serving.
 */
void
vncspuStartServerThread(void)
{
	CRthread thread;

	/* init locks and condition vars */
	crInitMutex(&vnc_spu.lock);
	crInitCondition(&vnc_spu.cond);

	/* spawn the thread */
	(void) crCreateThread(&thread, 0, vnc_main, NULL);

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


/**
 * Encoders call this function to get pointer to framebufer data.
 */
CARD32 *
GetFrameBuffer(CARD16 *w, CARD16 *h)
{
	CRASSERT(vnc_spu.serverBuffer);
	*w = vnc_spu.screen_width;
	*h = vnc_spu.screen_height;
	return (CARD32 *) vnc_spu.serverBuffer->buffer;
}


/**
 * Called by the RFB encoder thread to unlock access to 0th screen_buffer.
 */
void
vncspuUnlockFrameBuffer(void)
{
	CRASSERT(vnc_spu.serverBuffer);
	/* put server buffer into empty queue */
	vnc_spu.serverBuffer->regionSent = GL_TRUE;
	REGION_EMPTY(&vnc_spu.serverBuffer->dirtyRegion);

#if DB
	crDebug("Putting sent buffer %p into empty queue",
					(void *) vnc_spu.serverBuffer);
#endif
	EnqueueBuffer(vnc_spu.serverBuffer, &vnc_spu.emptyQueue);
	vnc_spu.serverBuffer = NULL;
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
 * incoming region.
 * Called by crHashtableWalk() via vncspuGetScreenRects().
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
 * Get current clip rects intersected with dirty regions for whole screen.
 */
void
vncspuGetScreenRects(RegionPtr reg)
{
	REGION_EMPTY(reg);
	crHashtableWalk(vnc_spu.windowTable, WindowDirtyUnionCB, reg);
}


/**
 * Get list of dirty rects in current virtual frame buffer.
 * This is the union of all Cr/GL windows.
 * NOTE: Called from vnc server thread.
 * \return  GL_TRUE if any dirty rects, GL_FALSE if no dirty rects
 * If GL_TRUE is returned, the serverBuffer will be set and will be in a
 * locked state.
 */
GLboolean
vncspuGetDirtyRects(RegionPtr region)
{
	vnc_spu.serverBuffer = DequeueBufferNoBlock(&vnc_spu.filledQueue);
	if (vnc_spu.serverBuffer) {
		ScreenBuffer *b = vnc_spu.serverBuffer;
		if (REGION_NUM_RECTS(&b->dirtyRegion) > 0) {
			REGION_COPY(region, &b->dirtyRegion);
			REGION_EMPTY(&b->dirtyRegion)
			return GL_TRUE;
		}
	}
	return GL_FALSE;
}


/**
 * Wait for new pixel data to send to the VNC viewer/client.
 * Return the region in 'region' and return GL_TRUE.
 * When we return, the serverBuffer will be non-null and will be in a
 * locked state.
 * NOTE: Called from vnc server thread.
 */
GLboolean
vncspuWaitDirtyRects(RegionPtr region, const BoxRec *roi, int serial_no)
{
	CRASSERT(!vnc_spu.serverBuffer);
#if DB
	crDebug("Getting filled buffer");
#endif
#ifdef NETLOGGER
	if (vnc_spu.netlogger_url) {
		NL_info("vncspu", "spu.wait.filledbuffer", "NODE=s NUMBER=i", vnc_spu.hostname, serial_no);
	}
#endif
	vnc_spu.serverBuffer = DequeueBuffer(&vnc_spu.filledQueue);
#ifdef NETLOGGER
	if (vnc_spu.netlogger_url) {
		NL_info("vncspu", "spu.got.filledbuffer", "NODE=s NUMBER=i", vnc_spu.hostname, serial_no);
	}
#endif
#if DB
	crDebug("Got filled buffer %p", (void*) vnc_spu.serverBuffer);
#endif
	CRASSERT(vnc_spu.serverBuffer);

	/* Get the new region and see if it intersects the region of interest */
	/* XXX intersect with roi */
	REGION_COPY(region, &vnc_spu.serverBuffer->dirtyRegion);
	REGION_EMPTY(&vnc_spu.serverBuffer->dirtyRegion);
	return GL_TRUE;
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
	ScreenBuffer *sb = vnc_spu.readpixBuffer;

	/* clip region against screen buffer size */
	if (scrx + width > vnc_spu.screen_width)
		width = vnc_spu.screen_width - scrx;
	if (scry + height > vnc_spu.screen_height)
		height = vnc_spu.screen_height - scry;
	if (width < 1 || height < 1)
		return;

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

#if 0
		if (0*vnc_spu.half_rez)	{
			GLubyte *src = sb->buffer
				+ (scryFlipped * vnc_spu.screen_width + scrx) * 4;
			vncspuHalfImage(width, height, vnc_spu.screen_width,	src,
											vnc_spu.screen_width, src, GL_BGRA);
		}
#endif

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
	ScreenBuffer *sb = vnc_spu.readpixBuffer;
	CRASSERT(sb);

	if (sb->regionSent) {
		/* for all windows, update the accumulated region info */
		crHashtableWalk(vnc_spu.windowTable, WindowUpdateAccumCB, NULL);
		sb->regionSent = GL_FALSE;
	}

	/* get new screen-space dirty rectangle list by walking over all windows */
	vncspuGetScreenRects(&sb->dirtyRegion);

	/* If we can't drop frames, we always have to send _something_ */
	if (REGION_NIL(&sb->dirtyRegion)) {
		/* make dummy 1x1 region */
		BoxRec b;
		b.x1 = b.y1 = 0;
		b.x2 = b.y2 = 1;
		REGION_INIT(&sb->dirtyRegion, &b, 1);
		CRASSERT(miRegionArea(&sb->dirtyRegion) > 0);
	}

	CRASSERT(miRegionArea(&sb->dirtyRegion) > 0);
	return GL_TRUE;
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
 * Update window's clipRegion field (which is in window coords).
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
		BoxRec bounds;
		/* clipping has changed */
		window->clippingHash = hash;
		window->newSize = 3;
		crDebug("VNC SPU: New clipping rects (%ld rects)",
						REGION_NUM_RECTS(&window->clipRegion));
		bounds = *miRegionExtents(&window->clipRegion);
		bounds.x1 += window->xPos;
		bounds.y1 += window->yPos;
		bounds.x2 += window->xPos;
		bounds.y2 += window->yPos;
		signal_new_clipping(&bounds);
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
		crDebug("new accum dirty area: %d  curr: %d  prev %d",
						miRegionArea(&window->accumDirtyRegion),
						miRegionArea(&window->currDirtyRegion),
						miRegionArea(&window->prevDirtyRegion));
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

	swap = SwapScreenBuffers();
	if (swap && vnc_spu.use_bounding_boxes) {
		/* If we really did swap, shift curr dirty region to prev dirty region.
		 * Afterward, currDirtyRegion will be empty.
		 */
		MOVE_REGION(&window->prevDirtyRegion,
								&window->currDirtyRegion);
	}
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


/**
 * Called by SwapBuffers, glFinish, glFlush after something's been rendered.
 * Determine which window regions have changed, call ReadPixels to store
 * those regions into the framebuffer.
 */
static void
vncspuUpdateVirtualFramebuffer(WindowInfo *window)
{
	GLint readBuf;
	GLint alignment, skipPixels, skipRows, rowLength;

	CRASSERT(window);
	window->frameCounter++;

#if DB
	crDebug("Getting empty buffer");
#endif
#ifdef NETLOGGER
	if (vnc_spu.netlogger_url) {
		NL_info("vncspu", "spu.wait.emptybuffer", "NUMBER=i", window->frameCounter);
	}
#endif
	CRASSERT(!vnc_spu.readpixBuffer);
	if (vnc_spu.frame_drop && vnc_spu.double_buffer) {
		vnc_spu.readpixBuffer = DequeueBuffer2(&vnc_spu.emptyQueue,
																					 &vnc_spu.filledQueue);
	}
	else {
		vnc_spu.readpixBuffer = DequeueBuffer(&vnc_spu.emptyQueue);
	}
#ifdef NETLOGGER
	if (vnc_spu.netlogger_url) {
		NL_info("vncspu", "spu.got.emptybuffer", "NUMBER=i", window->frameCounter);
	}
#endif
#if DB
	crDebug("Got empty buffer %p", (void*) vnc_spu.readpixBuffer);
#endif

	/* Save GL state */
	vnc_spu.super.GetIntegerv(GL_READ_BUFFER, &readBuf);
	vnc_spu.super.ReadBuffer(GL_FRONT);
	vnc_spu.super.GetIntegerv(GL_PACK_ALIGNMENT, &alignment);
	vnc_spu.super.GetIntegerv(GL_PACK_SKIP_PIXELS, &skipPixels);
	vnc_spu.super.GetIntegerv(GL_PACK_SKIP_ROWS, &skipRows);
	vnc_spu.super.GetIntegerv(GL_PACK_ROW_LENGTH, &rowLength);

	DoReadback(window);

	CRASSERT(vnc_spu.readpixBuffer);
#if DB
	crDebug("Putting filled buffer %p into filled queue",
					(void *) vnc_spu.readpixBuffer);
#endif
	EnqueueBuffer(vnc_spu.readpixBuffer, &vnc_spu.filledQueue);
	vnc_spu.readpixBuffer = NULL;

	/* Restore GL state */
	vnc_spu.super.ReadBuffer(readBuf);
	vnc_spu.super.PixelStorei(GL_PACK_ALIGNMENT, alignment);
	vnc_spu.super.PixelStorei(GL_PACK_SKIP_PIXELS, skipPixels);
	vnc_spu.super.PixelStorei(GL_PACK_SKIP_ROWS, skipRows);
	vnc_spu.super.PixelStorei(GL_PACK_ROW_LENGTH, rowLength);
}


static void VNCSPU_APIENTRY
vncspuSwapBuffers(GLint win, GLint flags)
{
	WindowInfo *window = LookupWindow(win, 0);

	vnc_spu.super.SwapBuffers(win, flags);

	if (window) {
		vncspuUpdateVirtualFramebuffer(window);
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
 * Update VNC data if single-buffered or drawing to non-back buffer.
 */
static void VNCSPU_APIENTRY
vncspuFinish(void)
{
	GLboolean db;
	GLint drawBuf;
	vnc_spu.self.GetBooleanv(GL_DOUBLEBUFFER, &db);
	vnc_spu.self.GetIntegerv(GL_DRAW_BUFFER, &drawBuf);
	if (!db || drawBuf != GL_BACK) {
		WindowInfo *window = vnc_spu.currentWindow;
		if (window) {
			vncspuUpdateVirtualFramebuffer(window);
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
