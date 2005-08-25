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



#if defined(HAVE_VNC_EXT)
static void
SendVncStartUpMsg(void)
{
	if (vnc_spu.haveVncExt) {
		VncConnectionList *vnclist;
		int num_conn, i;

		vnclist = XVncListConnections(vnc_spu.dpy, &num_conn);
		crDebug("VNC SPU: Found %d open VNC connection(s)", num_conn);
		for (i = 0; i < num_conn; i++) {
			int serverPort = vnc_spu.server_port;
			int mothershipPort = 2;
			CRASSERT(vnclist->ipaddress);
			crDebug("VNC SPU: Sending ChromiumStart message to VNC server 0x%x", vnclist[i].ipaddress);
			XVncChromiumStart(vnc_spu.dpy, vnclist[i].ipaddress,
												serverPort, mothershipPort);
		}
	}
}
#endif


void
vncspuInitialize(void)
{
#if defined(HAVE_XCLIPLIST_EXT)
	int eventBase, errorBase;
	char *dpyStr = NULL;

	vnc_spu.dpy = XOpenDisplay(NULL);
	if (!vnc_spu.dpy)
		vnc_spu.dpy = XOpenDisplay(":0");

	CRASSERT(vnc_spu.dpy);
	dpyStr = DisplayString(vnc_spu.dpy);

	vnc_spu.haveXClipListExt = XClipListQueryExtension(vnc_spu.dpy, &eventBase,
																										 &errorBase);
	if (vnc_spu.haveXClipListExt) {
		crDebug("VNC SPU: XClipList extension present on %s", dpyStr);
	}
	else {
		crWarning("VNC SPU: The display %s doesn't support the XClipList extension", dpyStr);
	}
	/* Note: not checking XClipList extension version at this time */
#endif /* HAVE_XCLIPLIST_EXT */

#if defined(HAVE_VNC_EXT)
	{
		int major, minor;
		vnc_spu.haveVncExt = XVncQueryExtension(vnc_spu.dpy, &major, &minor);
		if (vnc_spu.haveVncExt) {
			crDebug("VNC SPU: XVnc extension present on %s", dpyStr);
			SendVncStartUpMsg();
		}
		else {
			crWarning("VNC SPU: The display %s doesn't support the VNC extension", dpyStr);
		}
			
	}
#endif /* HAVE_VNC_EXT */
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
		vnc_spu.super.PixelStorei(GL_PACK_SKIP_PIXELS, scrx);
		vnc_spu.super.PixelStorei(GL_PACK_SKIP_ROWS, scryFlipped);
		vnc_spu.super.PixelStorei(GL_PACK_ROW_LENGTH, vnc_spu.screen_width);
		vnc_spu.super.ReadPixels(winx, winy, width, height,
														 GL_BGRA, GL_UNSIGNED_BYTE,
														 vnc_spu.screen_buffer);
	}
#else
	{
		GLubyte *buffer;
		GLubyte *src, *dst;
		int i;

		buffer = (GLubyte *) crAlloc(width * height * 4);
		vnc_spu.super.ReadPixels(winx, winy, width, height,
														 GL_BGRA, GL_UNSIGNED_BYTE, buffer);

		/* copy/flip */
		src = buffer + (height - 1) * width * 4; /* top row */
		dst = vnc_spu.screen_buffer + (vnc_spu.screen_width * scry + scrx) * 4;
		for (i = 0; i < height; i++) {
			crMemcpy(dst, src, width * 4);
			src -= width * 4;
			dst += vnc_spu.screen_width * 4;
		}
		crFree(buffer);
	}
#endif
}


/**
 * Read back the image from the OpenGL window and store in the screen buffer
 * (i.e. vnc_spu.screen_buffer) at the given x/y screen position.
 * Then, update the dirty rectangle info.
 */
static void
DoReadback(int scrx, int scry, int winWidth, int winHeight)
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

	CRASSERT(winWidth >= 1);
	CRASSERT(winHeight >= 1);
	if (vnc_spu.currentWindow && vnc_spu.currentWindow->nativeWindow) {
		BoxRec rect;
		int i;

#if defined(HAVE_XCLIPLIST_EXT)
		if (vnc_spu.haveXClipListExt) {
			CurrentClipRects = XGetClipList(vnc_spu.dpy,
																			vnc_spu.currentWindow->nativeWindow,
																			&CurrentClipRectsCount);
		}
		else
#endif
		{
			/* whole window, in window coords */
			rect.x1 = 0;
			rect.y1 = 0;
			rect.x2 = winWidth;
			rect.y2 = winHeight;
			CurrentClipRects = &rect;
			CurrentClipRectsCount = 1;
		}

		/* read back just the visible regions */
		for (i = 0; i < CurrentClipRectsCount; i++) {
			BoxPtr r = CurrentClipRects + i;     /* in window coords! */
			int width = r->x2 - r->x1;
			int height = r->y2 - r->y1;
			int srcx = r->x1;                    /* in window coords */
			int srcy = winHeight - r->y2;        /* in window coords, y=0=bottom */
			int destx = scrx + r->x1;            /* in screen coords */
			int desty = scry + r->y1;            /* in screen coords, y=0=top */
			ReadbackRegion(destx, desty, srcx, srcy, width, height);
		}

		/* translate cliprects to screen coords for the VNC server */
		for (i = 0; i < CurrentClipRectsCount; i++) {
			BoxPtr r = CurrentClipRects + i;
			r->x1 += scrx;
			r->y1 += scry;
			r->x2 += scrx;
			r->y2 += scry;
		}

		/* append this dirty rect to all clients' pending lists */
    crLockMutex(&vnc_spu.lock);
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
