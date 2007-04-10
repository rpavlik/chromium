/* Copyright (c) 2007, Tungsten Graphics, Inc.
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_spu.h"
#include "cr_mem.h"
#include "cr_string.h"
#include "windowtrackerspu.h"


#if defined(GLX)
static Window
findWindowByTitle(Display *dpy, int scr, Window start, const char *pattern)
{
	unsigned int i, num;
	Window root, parent, *children;
	char *title;

	/*
	XSynchronize(dpy, 1);
	crDebug("%s %d 0x%x %s", __FUNCTION__, scr, (int) start, pattern);
	*/

	if (XFetchName(dpy, start, &title)) {
		char *match = crStrPatternMatch(title, pattern);
		XFree(title);
		if (match)
			return start; /* found it */
	}
 
	/* breadth-first search */
	if (XQueryTree(dpy, start, &root, &parent, &children, &num)) {
		/* test each child window for a match */
		for (i = 0; i < num; i++) {
			/*
			crDebug("%s i=%d/%d win=0x%x", __FUNCTION__, i, num, (int) children[i]);
			*/
			Window w = children[i];
			if (XFetchName(dpy, w, &title)) {
				char *match = crStrPatternMatch(title, pattern);
				XFree(title);
				if (match) {
					XFree(children);
					return w; /* found it */
				}
			}
		}

		/* search the descendents of each child for a match: */
		for (i = 0; i < num ;i++) {
			Window w = findWindowByTitle(dpy, scr, children[i], pattern);
			if (w) {
				XFree(children);
				return w;
			}
		}

		XFree(children);
	}

	/* not found */
	return 0;
}


typedef int (*ErrorFunc)(Display *, XErrorEvent *);

static GLboolean ErrorCaught = GL_FALSE;

static
int handler(Display *dpy, XErrorEvent *xerror)
{
	if (xerror->error_code == BadDrawable) {
		ErrorCaught = GL_TRUE;
		return 0;
	}
	else {
		return 1;
	}
}


static GLboolean
getWindowGeometry(GLint *x, GLint *y, GLint *w, GLint *h)
{
	GLboolean retVal = GL_FALSE;
	ErrorFunc prev = XSetErrorHandler(handler);

	if (!windowtracker_spu.dpy) {
		windowtracker_spu.dpy = XOpenDisplay(windowtracker_spu.display);
	}
	if (windowtracker_spu.dpy) {
		Display *dpy = windowtracker_spu.dpy;
		if (!windowtracker_spu.win
				&& windowtracker_spu.window_title
				&& windowtracker_spu.window_title[0]) {
			crDebug("Window Tracker SPU: Looking for window %s",
							windowtracker_spu.window_title);
			windowtracker_spu.win = findWindowByTitle(dpy,
																								DefaultScreen(dpy),
																								DefaultRootWindow(dpy),
																								windowtracker_spu.window_title);
			if (windowtracker_spu.win) {
				crDebug("Window Tracker SPU: found window ID %u (0x%x)",
								(unsigned int) windowtracker_spu.win,
								(unsigned int) windowtracker_spu.win);
			}
		}
		if (windowtracker_spu.win) {
			Window root, child;
			unsigned int width, height, border, depth;
			if (XGetGeometry(windowtracker_spu.dpy, windowtracker_spu.win, &root,
											 x, y, &width, &height, &border, &depth)) {
				int rx, ry;
				if (XTranslateCoordinates(dpy,
																	windowtracker_spu.win,  /* from */
																	DefaultRootWindow(dpy), /* to */
																	*x, *y, &rx, &ry, &child)) {
					*x = rx;
					*y = ry;
				}
				*w = width;
				*h = height;
				retVal = GL_TRUE;
			}
		}
	}

	XSetErrorHandler(prev);

	if (ErrorCaught) {
		crError("Window Tracker SPU: invalid window handle.  Exiting.");
	}

	return retVal;
}

#else /* GLX */

static GLboolean
getWindowGeometry(GLint *x, GLint *y, GLint *w, GLint *h)
{
	crWarning("Window Tracker SPU: Not supported on this system.");
	*x = *y = *w = *h = -1;
}

#endif /*GLX */



static void WINDOWTRACKERSPU_APIENTRY
windowtrackerspuGetChromiumParametervCR(GLenum target, GLuint index,
																				GLenum type, GLsizei count,
																				GLvoid *values)
{
	switch (target) {
	case GL_WINDOW_SIZE_CR:
		{
			GLint x, y, *size = (GLint *) values;
			if (!getWindowGeometry(&x, &y, size+0, size+1))
				size[0] = size[1] = -1;
		}
		break;
	case GL_WINDOW_POSITION_CR:
		/* return window position, as a screen coordinate */
		{
			GLint w, h, *pos = (GLint *) values;
			if (!getWindowGeometry(pos+0, pos+1, &w, &h))
				pos[0] = pos[1] = -1;
			/*crDebug("%s returning pos %d %d", __FUNCTION__, pos[0], pos[1]);*/
		}
		break;
	default:
		/* pass-through */
		windowtracker_spu.child.GetChromiumParametervCR(target, index, type,
																										count, values);
	}
}


/**
 * SPU function table
 */
SPUNamedFunctionTable _cr_windowtracker_table[] = {
	{ "GetChromiumParametervCR", (SPUGenericFunction) windowtrackerspuGetChromiumParametervCR },
	{ NULL, NULL }
};
