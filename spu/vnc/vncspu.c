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
#include "async_io.h"
#include "rfblib.h"
#include "reflector.h"
#include "region.h"



void
vncspuInitialize(void)
{
#if defined(HAVE_XCLIPLIST_EXT)
	int eventBase, errorBase;

	vnc_spu.dpy = XOpenDisplay(NULL);
	CRASSERT(vnc_spu.dpy);

	vnc_spu.haveXClipListExt = XClipListQueryExtension(vnc_spu.dpy, &eventBase,
																										 &errorBase);
	if (vnc_spu.haveXClipListExt) {
		crDebug("VNC SPU: XClipList extension present on %s", DisplayString(vnc_spu.dpy));
	}
	else {
		crWarning("VNC SPU: The display %s doesn't support the XClipList extension", DisplayString(vnc_spu.dpy));

	}
	/* Note: not checking XClipList extension version at this time */
#endif
}


void
vncspuStartServerThread(void)
{
	crInitMutex(&vnc_spu.lock);

	{
#ifdef WINDOWS
		crError("VNC SPU not supported on Windows yet");
#else
		extern void * vnc_main(void *);
		pthread_t th;
		int id = pthread_create(&th, NULL, vnc_main, NULL);
		(void) id;
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


/* data used in callback called by aio_walk_slots(). */
static BoxPtr CurrentClipRects = NULL;
static int CurrentClipRectsCount = 0;

/* Callback */
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
		r.enc = 0; /* XXXX TEMPORARY */
		fn_client_add_rect(slot, &r);
	}
}


/*
 * Read back the image from the OpenGL window and store in the screen buffer
 * (i.e. vnc_spu.screen_buffer) at the given x/y position.
 * Then, update the dirty rectangle info.
 */
static void
DoReadback(int x, int y, int width, int height)
{
	/* check/alloc the screen buffer now */
	if (!vnc_spu.screen_buffer) {
		vnc_spu.screen_buffer = (GLubyte *)
			crAlloc(vnc_spu.screen_width * vnc_spu.screen_height * 4);
		if (!vnc_spu.screen_buffer) {
			crWarning("VNC SPU: Out of memory!");
			return;
		}
	}

#if RASTER_BOTTOM_TO_TOP
	{
		int yFlipped;
		yFlipped = vnc_spu.screen_height - (y + height);

		vnc_spu.super.PixelStorei(GL_PACK_SKIP_PIXELS, x);
		vnc_spu.super.PixelStorei(GL_PACK_SKIP_ROWS, yFlipped);
		vnc_spu.super.PixelStorei(GL_PACK_ROW_LENGTH, vnc_spu.screen_width);
		vnc_spu.super.ReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE,
														 vnc_spu.screen_buffer);
	}
#else
	{
		GLubyte *buffer;
		GLubyte *src, *dst;
		int i;

		buffer = (GLubyte *) crAlloc(width * height * 4);
		vnc_spu.super.ReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE,
														 buffer);

		/* copy/flip */
		src = buffer + (height - 1) * width * 4;
		dst = vnc_spu.screen_buffer + (vnc_spu.screen_width * y + x) * 4;
		for (i = 0; i < height; i++) {
			crMemcpy(dst, src, width * 4);
			src -= width * 4;
			dst += vnc_spu.screen_width * 4;
		}
		crFree(buffer);
	}
#endif

	CRASSERT(width >= 1);
	CRASSERT(height >= 1);
	if (vnc_spu.currentWindow && vnc_spu.currentWindow->nativeWindow) {
		BoxRec rect;
		
#if defined(HAVE_XCLIPLIST_EXT)
		if (vnc_spu.haveXClipListExt) {
			CurrentClipRects = XGetClipList(vnc_spu.dpy,
																			vnc_spu.currentWindow->nativeWindow,
																			&CurrentClipRectsCount);
		}
		else
#endif
		{
			rect.x1 = x;
			rect.y1 = y;
			rect.x2 = x + width;
			rect.y2 = y + height;
			CurrentClipRects = &rect;
			CurrentClipRectsCount = 1;
		}

    crLockMutex(&vnc_spu.lock);

		/* append this dirty rect to all clients' pending lists */
		aio_walk_slots(fn_host_add_client_rect, TYPE_CL_SLOT);

    crUnlockMutex(&vnc_spu.lock);

		/* reset/clear to be safe */
		CurrentClipRects = NULL;
		CurrentClipRectsCount = 0;

		/* Send the new dirty rects to clients */
		aio_walk_slots(fn_client_send_rects, TYPE_CL_SLOT);
	}
}


static void VNCSPU_APIENTRY
vncspuSwapBuffers(GLint win, GLint flags)
{
	int size[2], pos[2];
	vnc_spu.super.GetChromiumParametervCR(GL_WINDOW_SIZE_CR,
																				win, GL_INT, 2, size);
	vnc_spu.super.GetChromiumParametervCR(GL_WINDOW_POSITION_CR,
																				win, GL_INT, 2, pos);
	/*
	crDebug("%s %d x %d at %d, %d", __FUNCTION__, size[0], size[1], pos[0], pos[1]);
	*/
	/*
	 * XXX if window pos/size changes, compute difference and tell
	 * vnc server that that region is dirty. - Jul 21, 2005
	 */
	/* XXX swap, then readback from front???  - July 21, 2005 */
	DoReadback(pos[0], pos[1], size[0], size[1]);
	vnc_spu.super.SwapBuffers(win, flags);
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
	}

	if (window->nativeWindow != nativeWindow && nativeWindow != 0) {
		/* got the handle to a particular X window */
		window->nativeWindow = nativeWindow;
	}

	return window;
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


/**
 * SPU function table
 */
SPUNamedFunctionTable _cr_vnc_table[] = {
	{"SwapBuffers", (SPUGenericFunction) vncspuSwapBuffers},
	{"MakeCurrent", (SPUGenericFunction) vncspuMakeCurrent},
	{ NULL, NULL }
};
