#ifndef OGLwin_H
#define OGLwin_H

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xmu/StdCmap.h>
#include <X11/keysym.h>

typedef enum {

    CINDEX_8_SINGLE = 0,
    CINDEX_8_DOUBLE = 1,
    CINDEX_12_SINGLE = 2,
    CINDEX_12_DOUBLE = 3,
    RGBA_SINGLE = 4,
    RGBA_DOUBLE = 5
} WINTYPE;

typedef enum {
    OGL_EXPOSE,
    OGL_CONFIGURE,
    OGL_MOUSE1,
    OGL_MOUSE2,
    OGL_MOUSE3,
    OGL_MOUSEXY,
    OGL_KEYBOARD,
    OGL_NULL_DEV
} OGL_DEVICE;


void OGLwin_Open_Window(int x, int y, int width, int height, char *name,
                        WINTYPE t, Window parent_win);
void OGLwin_OGLwin(int width, int height, WINTYPE t);
void Close_OGLwin(void);

void OGLwin_SetWin(void);
void OGLwin_SwapBuffers(void);

void OGLwin_DrawString(char *s);

      /*
       * Color Map manipulations 
       */
void OGLwin_SetColor(int ind, short r, short g, short b);
void OGLwin_GetColorVal(int ind, short *r, short *g, short *b);

      /*
       * Events handling 
       */
int OGLwin_QTest(void);
OGL_DEVICE OGLwin_Qread(int *val);


extern Display *Dsp;
extern Window XWindow;


extern Display *global_display;
extern int direct_context;
extern int parent_border;
extern int override_redirect;

extern GLXContext Cxt;
extern Colormap Cmap;
extern int fontBase;
extern XEvent event;
extern XVisualInfo *Vi;
#endif
