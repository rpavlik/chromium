#include "OGLwin.h"

Display *Dsp;
Window XWindow;


Display *global_display;
int direct_context;
int parent_border;
int override_redirect;

GLXContext Cxt;
Colormap Cmap;
int fontBase;
XEvent event;
XVisualInfo *Vi;
