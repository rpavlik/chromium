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


#include <GL/gl.h>
#include <GL/glx.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define MAX_CONTEXTS 100

static int WinWidth = 300, WinHeight = 300;


static void
Render(Display *dpy, Window win, int numCtx, GLXContext context[],
       XVisualInfo *visinfo)
{
   float colors[7][3] = {
      { 1, 1, 1 },
      { 1, 0, 0 },
      { 0, 1, 0 },
      { 0, 0, 1 },
      { 0, 1, 1 },
      { 1, 0, 1 },
      { 1, 1, 0 }
   };
   int failures = 0;
   int frame = 0;
   int i;
   float theta;

   theta = 360.0 / numCtx;

   for (frame = 0; ; frame++) {
      for (i = 0; i < numCtx; i++) {
         if (!glXMakeCurrent(dpy, win, context[i])) {
            fprintf(stderr, "MakeCurrent to context %d failed!\n", i);
            failures++;
            if (failures >= 10)
               return;
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
            glRotatef(frame, 0.0, 0.0, 1.0);
            glBegin(GL_QUADS);
            glVertex3f(0.5, -0.7, 0.5);
            glVertex3f(9, -2, 0.5);
            glVertex3f(9, 2, -0.5);
            glVertex3f(0.5, 0.7, -0.5);
            glEnd();
         glPopMatrix();

      }
      glXSwapBuffers(dpy, win);
      sleep(1);

#if 0  /* just for testing */
      if (frame == 10) {
         /* destroy a context */
         numCtx--;
         glXDestroyContext(dpy, context[numCtx]);
      }

      if (frame == 15) {
         /* recreate the context */
         context[numCtx] = glXCreateContext(dpy, visinfo, NULL, True);
         assert(context[numCtx]);
         numCtx++;
      }
#endif

      /* process any pending events */
      while (XPending(dpy) > 0) {
         XEvent event;
         XNextEvent(dpy, &event);
         if (event.xany.window == win) {
            switch (event.type) {
            case Expose:
               /* ignore */
               break;
            case ConfigureNotify:
               WinWidth = event.xconfigure.width;
               WinHeight = event.xconfigure.height;
               break;
            case KeyPress:
               return;  /* exiting... */
            default:
               /* no-op */ ;
            }
         }
      }

   } /* frame loop */

}



int
main(int argc, char *argv[])
{
   int numContexts;
   int scrnum;
   XSetWindowAttributes attr;
   char *dpyName = NULL;
   Display *dpy;
   Window win, root;
   XVisualInfo *visinfo;
   int mask;
   int attrib[] = { GLX_RGBA,
		    GLX_RED_SIZE, 1,
		    GLX_GREEN_SIZE, 1,
		    GLX_BLUE_SIZE, 1,
		    GLX_DOUBLEBUFFER,
                    GLX_DEPTH_SIZE, 1,
		    None };
   int xpos = 10, ypos = 10;
   const char *name = "Multicontext test";
   int i;
   GLXContext context[MAX_CONTEXTS];

   if (argc > 1 && strcmp(argv[1], "--help") == 0) {
      printf("multicontext: render with N GLX contexts\n");
      printf("Usage:\n");
      printf("  multicontext numContexts\n");
      return 0;
   }

   /* XXX add option for GLX context sharing (display lists, textures) */
   if (argc == 1)
      numContexts = 3;
   else
      numContexts = atoi(argv[1]);

   if (numContexts < 1)
      numContexts = 1;
   else if (numContexts > MAX_CONTEXTS)
      numContexts = MAX_CONTEXTS;

   dpy = XOpenDisplay(dpyName);
   if (!dpy) {
      fprintf(stderr, "Error:  unable to open default display\n");
      return -1;
   }

   scrnum = DefaultScreen(dpy);
   root = RootWindow(dpy, scrnum);

   visinfo = glXChooseVisual(dpy, scrnum, attrib);
   if (!visinfo) {
      fprintf(stderr, "Error:  unable to find RGB, double-buffered visual\n");
      return -1;
   }

   /* Create the window */
   attr.background_pixel = 0;
   attr.border_pixel = 0;
   attr.colormap = XCreateColormap(dpy, root, visinfo->visual, AllocNone);
   attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
   mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

   win = XCreateWindow(dpy, root, xpos, ypos, WinWidth, WinHeight,
		        0, visinfo->depth, InputOutput,
		        visinfo->visual, mask, &attr);
   if (!win) {
      fprintf(stderr, "Error:  couldn't create window\n");
      return -1;
   }

   {
      XSizeHints sizehints;
      sizehints.x = xpos;
      sizehints.y = ypos;
      sizehints.width  = WinWidth;
      sizehints.height = WinHeight;
      sizehints.flags = USSize | USPosition;
      XSetNormalHints(dpy, win, &sizehints);
      XSetStandardProperties(dpy, win, name, name,
                              None, (char **)NULL, 0, &sizehints);
   }

   XMapWindow(dpy, win);

   /* Now create the GLX contexts */
   for (i = 0; i < numContexts; i++) {
      context[i] = glXCreateContext(dpy, visinfo, NULL, True);
      if (!context[i]) {
         fprintf(stderr, "Error: couldn't create GLX context number %d\n", i);
         return -1;
      }
   }

   /* Main rendering loop */
   Render(dpy, win, numContexts, context, visinfo);

   /* Clean up before exit */
   XDestroyWindow(dpy, win);
   for (i = 0; i < numContexts; i++)
      glXDestroyContext(dpy, context[i]);
   XCloseDisplay(dpy);

   return 0;
}
