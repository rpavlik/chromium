#ifndef CR_RENDERSPU_H
#define CR_RENDERSPU_H

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define RENDER_APIENTRY __stdcall
#else
#define RENDER_APIENTRY
#endif

#include "cr_spu.h"

void renderspuGatherConfiguration( void );
void renderspuCreateWindow( void );

void RENDER_APIENTRY renderspuSwapBuffers( void );

#ifdef WINDOWS
typedef HGLRC (RENDER_APIENTRY *wglCreateContextFunc_t)(HDC);
typedef BOOL (RENDER_APIENTRY *wglMakeCurrentFunc_t)(HDC,HGLRC);
typedef BOOL (RENDER_APIENTRY *wglSwapBuffersFunc_t)(HDC);
#else
#error FOO
#endif

typedef struct {
	SPUDispatchTable *dispatch;
	int id;
	int window_x, window_y;
	unsigned int window_width, window_height;
	unsigned int actual_window_width, actual_window_height;
	int use_L2;
	int fullscreen;
	int depth_bits, stencil_bits;
#ifdef WINDOWS
	HWND         hWnd;
	HGLRC        hRC;
	HDC          device_context;
	wglCreateContextFunc_t wglCreateContext;
	wglMakeCurrentFunc_t wglMakeCurrent;
	wglSwapBuffersFunc_t wglSwapBuffers;
#else
	Display     *dpy;
	XVisualInfo *visual;
	GLXContext   context;
	Window       window;
	char        *display_string;
	int          try_direct;
	int          force_direct;
	int          sync;
#endif
} RenderSPU;

extern RenderSPU render_spu;

#endif /* CR_RENDERSPU_H */
