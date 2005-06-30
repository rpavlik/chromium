#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/extensions/dmxext.h>

/* The output of this program is meant to be evaluated by Python.  Thus, it
 * should always output valid Python code. */

static void fatal(char *format, ...);

static void
fatal(char *format, ...)
{
    va_list args;

    va_start(args, format);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    printf("None\n");
    exit(-1);
}

int
main(int argc, char **argv)
{
    Display *display = NULL;
    DMXScreenAttributes satts;
    int     screenCount, i;
    int     event_base, error_base;

    if (argc > 1) {
       display = XOpenDisplay(argv[1]);
        if (!display)
        {
            fatal("Couldn't open display %s", argv[1]);
        }
    }
    else {
        display = XOpenDisplay(NULL);
        if (!display)
        {
            fatal("Couldn't open default X display.");
        }
    }

    if (!DMXQueryExtension(display, &event_base, &error_base))
    {
        fatal("DMX extension not present");
    }

    if (!DMXGetScreenCount(display, &screenCount))
    {
        fatal("Could not get screen count");
    }

    printf("[");
    for (i = 0; i < screenCount; i++)
    {
        if (!DMXGetScreenAttributes(display, i, &satts))
        {
            fatal("Could not get screen attributes for %d\n", i);
        }
        printf("    { 'display': '%s', 'width': %d, 'height': %d, 'xoff': %d, 'yoff': %d, 'screen': %d, 'xorigin': %d, 'yorigin': %d },",
               satts.displayName, satts.rootWindowWidth,
               satts.rootWindowHeight, satts.rootWindowXoffset,
               satts.rootWindowYoffset, satts.logicalScreen,
               satts.rootWindowXorigin, satts.rootWindowYorigin);
    }
    printf("]\n");

    return 0;
}
