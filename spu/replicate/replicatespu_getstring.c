/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "replicatespu.h"
#include "cr_packfunctions.h"
#include "state/cr_statefuncs.h"
#include "cr_string.h"
#include "cr_extstring.h"
#include "replicatespu_proto.h"


/**
 * Get extension string from first active server.
 */
static const GLubyte *
GetExtensions(void)
{
	int i;
	GLubyte return_value[10*1000] = {0};
	const GLubyte *extensions, *ext;
	GET_THREAD(thread);

	return_value[0] = 0; /* null-terminate */

	for (i = 1; i < CR_MAX_REPLICANTS; i++) {
		if (IS_CONNECTED(replicate_spu.rserver[i].conn)) {
			int writeback = 1;

			if (replicate_spu.swap)
				crPackGetStringSWAP( GL_EXTENSIONS, return_value, &writeback );
			else
				crPackGetString( GL_EXTENSIONS, return_value, &writeback );

			replicatespuFlushOne(thread, i);

			while (writeback)
				crNetRecv();

			CRASSERT(crStrlen((char *)return_value) < 10*1000);
			break;
		}
	}

	if (return_value[0] == 0) {
		/* no servers - return default Chromium extension list */
		crStrcpy(return_value, crExtensions);
		(void) crAppOnlyExtensions;
		(void) crChromiumExtensions;
	}

	/* OK, we got the result from the server.  Now we have to
	 * intersect is with the set of extensions that Chromium understands
	 * and tack on the Chromium-specific extensions.
	 */
	extensions = return_value;
	ext = crStateMergeExtensions(1, &extensions);

	return ext;  /* XXX we should return a static string here! */
}


const GLubyte * REPLICATESPU_APIENTRY replicatespu_GetString( GLenum name )
{
	if (name == GL_EXTENSIONS)
	{
		return GetExtensions();
	}
	else
	{
		return crStateGetString(name);
	}
}
