#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "OGLwin.h"

static int cindex_8_att[] =
    { GLX_BUFFER_SIZE, 8, GLX_DEPTH_SIZE, 1, None };
static int cindex_8_datt[] =
    { GLX_BUFFER_SIZE, 8, GLX_DEPTH_SIZE, 1, GLX_DOUBLEBUFFER, None };
static int cindex_12_att[] =
    { GLX_BUFFER_SIZE, 12, GLX_DEPTH_SIZE, 1, None };
static int cindex_12_datt[] =
    { GLX_BUFFER_SIZE, 12, GLX_DEPTH_SIZE, 1, GLX_DOUBLEBUFFER, None };
static int rgba_att[] =
    { GLX_RGBA, GLX_DEPTH_SIZE, 1, GLX_RED_SIZE, 4, GLX_GREEN_SIZE, 4,
GLX_BLUE_SIZE, 4, None };
static int rgba_datt[] =
    { GLX_RGBA, GLX_DEPTH_SIZE, 1, GLX_RED_SIZE, 4, GLX_GREEN_SIZE, 4,
    GLX_BLUE_SIZE, 4, GLX_DOUBLEBUFFER, None
};

static int rgba12_att[] =
    { GLX_RGBA, GLX_DEPTH_SIZE, 1, GLX_RED_SIZE, 12, GLX_GREEN_SIZE, 12,
GLX_BLUE_SIZE, 12, None };
static int rgba12_datt[] =
    { GLX_RGBA, GLX_DEPTH_SIZE, 1, GLX_RED_SIZE, 12, GLX_GREEN_SIZE, 12,
GLX_BLUE_SIZE, 12, GLX_DOUBLEBUFFER, None };

#define XFONT_STRING "-misc-fixed-medium-r-*-*-20-*-75-75-*-*-*-*"

static int *attributes[] = {
    cindex_8_att,
    cindex_8_datt,
    cindex_12_att,
    cindex_12_datt,
    rgba_att,
    rgba_datt
};

Display *OGLwin_global_display = NULL;
int OGLwin_direct_context = GL_TRUE;
int OGLwin_parent_border = 0;
int OGLwin_override_redirect = 0;

static Colormap getColormap(Display * dsp, XVisualInfo * vi,
			    WINTYPE Req_Win_Type)
{
    Status status;
    XStandardColormap *standardCmaps;
    int i, numCmaps;

    switch (vi->class) {
    case PseudoColor:

	if (Req_Win_Type >= 4) {

	    if (MaxCmapsOfScreen(DefaultScreenOfDisplay(dsp)) == 1
		&& vi->visual == DefaultVisual(dsp, vi->screen)) {

		/*
		 * Share the root colormap. 
		 */
		return (DefaultColormap(dsp, vi->screen));

	    } else {
		/*
		 * Get our own PseudoColor colormap. 
		 */
		return (XCreateColormap(dsp, RootWindow(dsp, vi->screen),
					vi->visual, AllocNone));
	    }

	} else {
	    /*
	     * CI mode, real GLX never returns a PseudoColor visual
	     * for RGB mode. 
	     */
	    return (XCreateColormap(dsp, RootWindow(dsp, vi->screen),
				    vi->visual, AllocAll));
	}
	break;

    case TrueColor:
    case DirectColor:

	status = XmuLookupStandardColormap(dsp,
					   vi->screen, vi->visualid,
					   vi->depth, XA_RGB_DEFAULT_MAP,
					   /*
					    * replace 
					    */ False, /* retain */ True);
	if (status == 1) {
	    status = XGetRGBColormaps(dsp, RootWindow(dsp, vi->screen),
				      &standardCmaps, &numCmaps,
				      XA_RGB_DEFAULT_MAP);
	    if (status == 1)
		for (i = 0; i < numCmaps; i++)
		    if (standardCmaps[i].visualid == vi->visualid) {
			Colormap cmap;

			cmap = standardCmaps[i].colormap;
			XFree(standardCmaps);
			return (cmap);
		    }
	}
	/*
	 * If no standard colormap but TrueColor, just make a
	 * private one. 
	 */
	return (XCreateColormap(dsp, RootWindow(dsp, vi->screen),
				vi->visual, AllocNone));
	break;

    case StaticColor:
    case StaticGray:
    case GrayScale:
	/*
	 * Mesa supports these visuals 
	 */
	return (XCreateColormap(dsp, RootWindow(dsp, vi->screen),
				vi->visual, AllocNone));
	break;

    default:
	fprintf(stderr,
		"could not allocate colormap for visual type: %d.",
		vi->class);
	exit(-1);
    }

    return ((Colormap)NULL);

}

static Bool WaitForUnmapNotify(Display * d, XEvent * e, char *arg)
{
    if ((e->type == UnmapNotify) && (e->xmap.window == (Window) arg)) {
	return GL_TRUE;
    }
    return GL_FALSE;
}

static Bool WaitForMapNotify(Display * d, XEvent * e, char *arg)
{
    if ((e->type == MapNotify) && (e->xmap.window == (Window) arg)) {
	return GL_TRUE;
    }
    return GL_FALSE;
}

static int MakeFont(Display * xdisplay, char *fn)
{
    XFontStruct *fontInfo;
    Font id;
    unsigned int first, last;
    GLuint fontBase;

    fontInfo = XLoadQueryFont(xdisplay, fn);
    if (fontInfo == NULL) {
	printf("no font found\n'%s'\n", fn);
	exit(0);
    }

    id = fontInfo->fid;
    first = fontInfo->min_char_or_byte2;
    last = fontInfo->max_char_or_byte2;

    fontBase = glGenLists((GLuint) last + 1);
    if (fontBase == 0) {
	printf("out of display lists\n");
	exit(0);
    }
    glXUseXFont(id, first, last - first + 1, fontBase + first);

    return (fontBase);
}

void OGLwin_Open_Window(int x, int y, int width, int height, char *name,
			WINTYPE t, Window parent_win)
{
    XSetWindowAttributes swa;
    XEvent event;
    unsigned long attr_mask;
    Window parent;
    Window root_win;

    if (!Dsp) {
	if (global_display != NULL) {
	    Dsp = global_display;
	} else {
	    Dsp = XOpenDisplay(0);
	}
    }

    if (getenv("OGLWIN_USE12")) {
	attributes[RGBA_SINGLE] = rgba12_att;
	attributes[RGBA_DOUBLE] = rgba12_datt;
    }

    Vi = glXChooseVisual(Dsp, DefaultScreen(Dsp), attributes[t]);
    Cmap = getColormap(Dsp, Vi, t);

    swa.border_pixel = 0;
    swa.colormap = Cmap;
    swa.event_mask =
	ExposureMask | StructureNotifyMask | VisibilityChangeMask |
	PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
	KeyPressMask | KeyReleaseMask;
    swa.override_redirect = OGLwin_override_redirect;
    attr_mask =
	CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect;

    parent = parent_win;
    root_win = RootWindow(Dsp, Vi->screen);

    if (!parent)
	parent = root_win;

    if (parent_border) {
	parent = XCreateSimpleWindow(Dsp, parent, x, y, width, height,
				     0, 0, 0);
	XSelectInput(Dsp, parent, StructureNotifyMask);
	x = y = parent_border;
	width -= parent_border;
	height -= parent_border;
    }

    XWindow = XCreateWindow(Dsp, parent, x, y,
			    width, height,
			    0, Vi->depth, InputOutput, Vi->visual,
			    attr_mask, &swa);

    if (parent_win == root_win) {
	Window top_win = (parent_border ? parent : XWindow);

	/*
	 * fix size/position/aspect of new swWindow  
	 */
	{
	    XSizeHints *size_hints;

	    size_hints = XAllocSizeHints();
	    size_hints->flags = USSize | PSize | USPosition | PPosition;
	    XSetWMNormalHints(Dsp, top_win, size_hints);
	    XFree(size_hints);
	}

	XSetWMColormapWindows(Dsp, XWindow, &XWindow, 1);
	XStoreName(Dsp, top_win, name);
	XMapSubwindows(Dsp, top_win);
	XMapWindow(Dsp, top_win);
	XIfEvent(Dsp, &event, WaitForMapNotify, (char *) top_win);
    } else {
	XMapWindow(Dsp, XWindow);
	XIfEvent(Dsp, &event, WaitForMapNotify, (char *) XWindow);
    }

    Cxt = glXCreateContext(Dsp, Vi, 0, direct_context);

    //SetWin();

    fontBase = -1;
}


void OGLwin_OGLwin(int width, int height, WINTYPE t)
{
    Dsp = NULL;
    OGLwin_Open_Window(0, 0, width, height, "OGLwin", t, 0);
}


void Close_OGLwin(void)
{

    //
    // wait for the window to be unmapped
    //
    XEvent e;

    XUnmapWindow(Dsp, XWindow);
    XIfEvent(Dsp, &e, WaitForUnmapNotify, (char *) XWindow);

    glXDestroyContext(Dsp, Cxt);
    XDestroyWindow(Dsp, XWindow);
}

void OGLwin_SetWin(void)
{
    if (!glXMakeCurrent(Dsp, XWindow, Cxt)) {
	fprintf(stderr, "Can't make Window current to context\n");
	return;
    }
}

void OGLwin_SwapBuffers(void)
{
    glXSwapBuffers(Dsp, XWindow);
}

void OGLwin_DrawString(char *s)
{
    if (fontBase < 0)
	fontBase = MakeFont(Dsp, XFONT_STRING);

    glPushAttrib(GL_LIST_BIT);
    glListBase(fontBase);
    glCallLists(strlen(s), GL_UNSIGNED_BYTE, (GLubyte *) s);
    glPopAttrib();
}

void OGLwin_SetColor(int ind, short r, short g, short b)
{
    XColor color;

    color.pixel = ind;
    color.red = r;
    color.green = g;
    color.blue = b;
    color.flags = DoRed | DoGreen | DoBlue;

    XStoreColor(Dsp, Cmap, &color);

    XFlush(Dsp);
}


void OGLwin_GetColorVal(int ind, short *r, short *g, short *b)
{
    XColor color;

    color.pixel = ind;
    XQueryColor(Dsp, Cmap, &color);

    *r = color.red;
    *g = color.green;
    *b = color.blue;
}

static Bool CheckForWindowEvent(Display * d, XEvent * e, XPointer arg)
{
    if (e->xany.window == (Window) arg) {
	return GL_TRUE;
    }
    return GL_FALSE;
}

int OGLwin_QTest(void)
{
    XEvent e;

    if (XCheckIfEvent(Dsp, &e, CheckForWindowEvent, (XPointer) XWindow)) {
	XPutBackEvent(Dsp, &e);
	return (1);
    } else
	return (0);

}

OGL_DEVICE OGLwin_Qread(int *val)
{
    OGL_DEVICE dev = 0;
    char buf[100];

    XIfEvent(Dsp, &event, CheckForWindowEvent, (XPointer) XWindow);

    switch (event.type) {

    case Expose:
	dev = OGL_EXPOSE;
	break;

    case ConfigureNotify:
	dev = OGL_CONFIGURE;
	*(val) = event.xconfigure.width;
	*(val + 1) = event.xconfigure.height;
	break;

    case ButtonPress:
    case ButtonRelease:
	switch (event.xbutton.button) {
	case 1:
	    dev = OGL_MOUSE1;
	    break;
	case 2:
	    dev = OGL_MOUSE2;
	    break;
	case 3:
	    dev = OGL_MOUSE3;
	    break;
	}

	if (event.type == ButtonPress)
	    *val = 1;
	else
	    *val = 0;

	break;

    case MotionNotify:
	dev = OGL_MOUSEXY;
	*val = event.xmotion.x;
	*(val + 1) = event.xmotion.y;
	break;

    case KeyPress:
    case KeyRelease:
	{
	    KeySym ks;

	    dev = OGL_KEYBOARD;
	    XLookupString(&event.xkey, buf, sizeof(buf), &ks, 0);
	    *val = (int) ks;

	    if (event.type == KeyPress)
		*(val + 1) = 1;
	    else
		*(val + 1) = 0;

	}
	break;


    default:
	dev = OGL_NULL_DEV;
	break;
    }

    return (dev);
}
