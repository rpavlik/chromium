/* 
 * Taken from the GLX multicontext program in progs/glx/multicontext
 * and adapted for Windows WGL
 *
 * Written by Alan Hourihane
 * 1 February 2002
 */

/*
 * Exercise multiple GLX contexts rendering into one window.
 *
 * For each frame we loop over the rendering contexts, drawing one
 * quadrilateral with each context.  The quads form a propeller/fan shape.
 * If state tracking across context switches is broken, the modelview
 * transformations and quad colors will likely be messed up.
 *
 * By default, we create three GLX contexts.  Different numbers of
 * contexts can be specified on the command line.
 *
 * Written by Brian Paul
 * 5 November 2001
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

static int WinWidth = 300, WinHeight = 300;
#define MAX_CONTEXTS 100

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#include <stdio.h>
#include <stdlib.h>

LPWSTR * WINAPI CommandLineToArgvW(LPCWSTR, int *);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine, int iCmdShow)
{
	WNDCLASS wc;
	HWND hWnd;
	HDC hDC;
	HGLRC hRC[MAX_CONTEXTS];
	MSG msg;
	int argc;
	LPWSTR *argv;
	BOOL exit = FALSE;
   	float colors[7][3] = {
			      	{ 1, 1, 1 },
      				{ 1, 0, 0 },
      				{ 0, 1, 0 },
      				{ 0, 0, 1 },
      				{ 0, 1, 1 },
      				{ 1, 0, 1 },
      				{ 1, 1, 0 }
   				};
   	float theta;
	int numContexts;
   	int frame = 0;
	int i;
	PIXELFORMATDESCRIPTOR pfd;
	int format;

	(void) iCmdShow;
	(void) hPrevInstance;

	argv = CommandLineToArgvW((LPCWSTR)lpCmdLine, &argc);

   	if (argc == 0)
      		numContexts = 3;
   	else
      		numContexts = atoi((const char *)argv[0]);

   	if (numContexts < 1)
      		numContexts = 1;
   	else if (numContexts > MAX_CONTEXTS)
      		numContexts = MAX_CONTEXTS;

   	theta = (float) 360.0 / numContexts;
	
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "multicontext";
	RegisterClass( &wc );
	
	hWnd = CreateWindow( 
		"multicontext", "multicontext", 
		WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE,
		0, 0, WinWidth, WinHeight + 30,
		NULL, NULL, hInstance, NULL );
	
	hDC = GetDC( hWnd );
	
	ZeroMemory( &pfd, sizeof( pfd ) );
	pfd.nSize = sizeof( pfd );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	format = ChoosePixelFormat( hDC, &pfd );
	SetPixelFormat( hDC, format, &pfd );
	
	for (i = 0; i < numContexts; i++)
		hRC[i] = wglCreateContext( hDC );

	wglMakeCurrent( hDC, hRC[0] );
	
	while ( !exit )
	{
		if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE )  )
		{
			if ( msg.message == WM_QUIT ) 
			{
				exit = TRUE;
			} 
			else 
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}
		else 
		{
   			int failures = 0;

      			for (i = 0; i < numContexts; i++) {
         			if (!wglMakeCurrent(hDC, hRC[i])) {
            				failures++;
            				if (failures >= 10)
               					return 0;
         			}

         			/* one-time context setup */
         			if (frame == 0 /*|| frame == 16*/) {
            				glEnable(GL_DEPTH_TEST);
            				glShadeModel(GL_SMOOTH);
            				glMatrixMode(GL_PROJECTION);
            				glLoadIdentity();
            				glOrtho(-10.0, 10.0, -10.0, 10.0, -1.0, 1.0);
            				glMatrixMode(GL_MODELVIEW);
            				glLoadIdentity();
            				glColor3fv(colors[i % 7]);
            				glRotatef(i * theta, 0.0, 0.0, 1.0);
         			}

         			/* clear the window using 0th context */
         			if (i == 0) {
            				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
         			}

         			glViewport(0, 0, WinWidth, WinHeight);

         			glPushMatrix();
         			/*
            			  glRotatef(frame + i * theta, 0.0, 0.0, 1.0);
         			*/
            			glRotatef((float)frame, 0.0, 0.0, 1.0);
            			glBegin(GL_QUADS);
            			glVertex3d(0.5, -0.7, 0.5);
            			glVertex3d(9, -2, 0.5);
            			glVertex3d(9, 2, -0.5);
            			glVertex3d(0.5, 0.7, -0.5);
            			glEnd();
         			glPopMatrix();
			}
      			SwapBuffers( hDC );
			frame++;
		}
	}
	
	wglMakeCurrent( NULL, NULL );

	for (i = 0; i < numContexts; i++)
		wglDeleteContext( hRC[i] );

	ReleaseDC( hWnd, hDC );

	DestroyWindow( hWnd );
	
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		return 0;
	case WM_CLOSE:
		PostQuitMessage( 0 );
		return 0;
	case WM_DESTROY:
		return 0;
	case WM_KEYDOWN:
		switch ( wParam )
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			return 0;
		}
		return 0;
	default:
		return DefWindowProc( hWnd, message, wParam, lParam );
	}
}
