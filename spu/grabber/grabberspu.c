/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mothership.h"
#include "grabberspu.h"

static void GRABBER_APIENTRY grabberMakeCurrent(GLint crWindow, GLint nativeWindow, GLint ctx)
{
	static GLint lastWindow = 0;

	/* Grab the current window attribute, if we're configured to do so. */
	if (grabber_spu.currentWindowAttributeName) {

	    /* Only grab it if it's changed */
	    if (lastWindow != nativeWindow) {
		/* In base 10, there can be up to 3 digits per byte */
		char buffer[sizeof(int)*3 + 1];

		/* Record the desired window as a Mothership attribute. */
		sprintf(buffer, "%d", nativeWindow);
		crMothershipSetParam(grabber_spu.mothershipConnection, 
		    grabber_spu.currentWindowAttributeName,
		    buffer);
		lastWindow = nativeWindow;
	    }
	}

	/* Continue with normal stuff */
	grabber_spu.super.MakeCurrent(crWindow, nativeWindow, ctx);
}

SPUNamedFunctionTable _cr_grabber_table[] = {
    { "MakeCurrent", (SPUGenericFunction) grabberMakeCurrent},
    { NULL, NULL }
};
