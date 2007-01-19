/**
 * DMX utility functions.
 */

#include "cr_dmx.h"
#include "cr_error.h"
#include "cr_mem.h"


/* XXX Remove all the old API version 1 stuff someday - it's pretty old */
#ifndef DmxBadXinerama
#error "Looks like your dmxext.h header is too old!"
#endif


/**
 * Try to get an XVisualInfo that satisfies the visAttribs mask.
 * If we can't get multisample, try without.  If we can't get stereo,
 * try without, etc.
 */
static XVisualInfo *
chooseVisualRetry(const crOpenGLInterface *ws, Display *dpy, int screen,
                  GLbitfield visAttribs)
{
	while (1) {
		XVisualInfo *vis = crChooseVisual(ws, dpy, screen, GL_FALSE, visAttribs);
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


/**
 * Allocate/initialize an array of CRDMXBackendWindowInfo objects.
 */
CRDMXBackendWindowInfo *
crDMXAllocBackendWindowInfo(unsigned int numBackendWindows)
{
	CRDMXBackendWindowInfo *backendInfo
		= (CRDMXBackendWindowInfo *)
				crCalloc(numBackendWindows * sizeof(CRDMXBackendWindowInfo));
	if (backendInfo) {
		unsigned i;
		for (i = 0; i < numBackendWindows; i++) {
			backendInfo[i].clipToScreen = GL_TRUE;
		}
	}
	return backendInfo;
}


void
crDMXFreeBackendWindowInfo(unsigned int numBackendWindows,
													 CRDMXBackendWindowInfo *backendWindows)
{
	register unsigned int i;

	for (i = 0; i < numBackendWindows; i++) {
#if 0
		/* Don't destroy the window - its parent was probably
		 * already destroyed!
		 */
		if (backendWindows[i].xsubwin) {
	    XDestroyWindow(backendWindows[i].dpy, backendWindows[i].xsubwin);
		}
#endif
		if (backendWindows[i].dpy) {
	    XCloseDisplay(backendWindows[i].dpy);
		}
	}

	crFree(backendWindows);
}


/**
 * Query the back-end window information for the given DMX window.
 * \param dpy  the DMX Display
 * \param xwin  the DMX Window
 * \param numBackendWindows  the number of backend windows (XXX need this???)
 * \param backendWindows  returns the back-end window information
 * \param openGlInterface  needed in order to access glXChooseVisual, etc.
 * \param subwindowVisBits  OpenGL visual attributes to use when creating
 *                          new backend child windows.
 * \return GL_TRUE if new backend windows were created or detected,
 *         GL_FALSE otherwise.
 */
GLboolean
crDMXGetBackendWindowInfo(Display *dpy,
													GLXDrawable xwin, 
													unsigned int numBackendWindows,
													CRDMXBackendWindowInfo *backendWindows,
													const crOpenGLInterface *openGlInterface,
													GLint subwindowVisBits)
{
	GLboolean newBackendWindows = GL_FALSE; /* return value */
	int numScreens, count, i;
	DMXScreenAttributes *dmxScreenInfo;
	DMXWindowAttributes *dmxWinInfo;

	CRASSERT(dpy);
	DMXGetScreenCount(dpy, &numScreens);
	CRASSERT(numScreens == (int) numBackendWindows);

	dmxScreenInfo = (DMXScreenAttributes *) crAlloc(numScreens * sizeof(*dmxScreenInfo));
	if (!dmxScreenInfo) {
	    crWarning("crDMXGetBackendWindowInfo: failed to allocate DMX sreen info");
	    return GL_FALSE;
	}

	dmxWinInfo = (DMXWindowAttributes *) crAlloc(numScreens * sizeof(*dmxWinInfo));
	if (!dmxWinInfo) {
		crWarning("crDMXGetBackendWindowInfo: failed to allocate DMX window info");
		crFree(dmxScreenInfo);
		return GL_FALSE;
	}

	for (i = 0; i < numScreens; i++) {
		if (!DMXGetScreenAttributes(dpy, i, dmxScreenInfo + i)) {
			crDebug("Could not get screen information for screen %d\n", i);
			crFree(dmxScreenInfo);
			crFree(dmxWinInfo);
			return GL_FALSE;
		}
	}
		
	if (!DMXGetWindowAttributes(dpy, xwin, &count, numScreens, dmxWinInfo))
	{
		crDebug("Could not get window information for 0x%x\n", (int) xwin);
		crFree(dmxScreenInfo);
		crFree(dmxWinInfo);
		return GL_FALSE;
	}

	/* From the DMX info, compute tiling info.
	 * Also setup child X windows for back-end rendering.
	 */
	for (i = 0; i < count; i++) {
		int server = dmxWinInfo[i].screen;
		CRDMXBackendWindowInfo *backend = backendWindows + server;
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
		if (backend->clipToScreen) {
			subwinX = backend->visrect.x1;
			subwinY = backend->visrect.y1;
			subwinW = backend->visrect.x2 - backend->visrect.x1;
			subwinH = backend->visrect.y2 - backend->visrect.y1;
			if (subwinW <= 0)
				subwinW = 1;
			if (subwinH <= 0)
				subwinH = 1;
		}
		else {
			/* Used by dmxdirect */
			subwinX = 0;
			subwinY = 0;
			subwinW = dmxWinInfo[i].pos.width;
			subwinH = dmxWinInfo[i].pos.height;
		}

		if (backend->xwin != 0 && backend->xsubwin == 0) {
			if (openGlInterface != NULL) {
				/* Create a child of the back-end X window.  We do this to work
				 * around a memory allocation problem found with NVIDIA drivers.
				 * See discussion from Feb 2002 on the DMX-devel mailing list.
				 * This also gives us flexibility in choosing the window's visual.
				 */
				XSetWindowAttributes attribs;
				Window root;
				unsigned long attribMask;
				int scr;
				XVisualInfo *visInfo;

				scr = DefaultScreen(backend->dpy);
				root = RootWindow(backend->dpy, scr);

				visInfo = chooseVisualRetry(openGlInterface, backend->dpy, scr, subwindowVisBits);
				CRASSERT(visInfo);

				attribs.background_pixel = 0;
				attribs.border_pixel = 0;
				attribs.colormap = XCreateColormap(backend->dpy, root,
				     visInfo->visual, AllocNone);
				attribMask = CWBorderPixel | CWColormap;

				backend->xsubwin = XCreateWindow(backend->dpy,
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

				/* The return value should reflect that we have new windows */
				newBackendWindows = GL_TRUE;
			}
			else
			{
				/* Don't create a new child window.  Most likely, we're running a
				 * parallel application on a DMX display.  We only want the 0th
				 * tilesort SPU to create child windows.  The other tilesort SPUs
				 * will look for that child window.
				 */
				Window root, parent, *children = NULL;
				unsigned int nChildren;
				Status s = XQueryTree(backend->dpy, backend->xwin, &root, &parent,
															&children, &nChildren);
				if (s && nChildren == 1) {
					backend->xsubwin = children[0];
					newBackendWindows = GL_TRUE;
				}
				if (children)
					XFree(children);
			}
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
		CRDebug("Backend Window %d:  scrn %d  backwin 0x%x  childwin 0x%x:",
					 i, server, (int) backend->xwin, (int) backend->xsubwin);
		CRDebug("  screen offset: %d, %d", dmxScreenInfo[server].xorigin,
			   dmxScreenInfo[server].yorigin);
		CRDebug("  visrect = %d, %d .. %d, %d", 
			   backend->visrect.x1, backend->visrect.y1,
			   backend->visrect.x2, backend->visrect.y2);
#endif
	}

	crFree(dmxWinInfo);
	crFree(dmxScreenInfo);

	return newBackendWindows;
}


int
crDMXSupported(Display *dpy)
{
	int event_base, error_base;
	return DMXQueryExtension(dpy, &event_base, &error_base);
}
