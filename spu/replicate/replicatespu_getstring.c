/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "replicatespu.h"
#include "cr_packfunctions.h"
#include "state/cr_statefuncs.h"
#include "cr_string.h"
#include "replicatespu_proto.h"

static const GLubyte *
GetExtensions(void)
{
	GLubyte return_value[10*1000];
	const GLubyte *extensions, *ext;
	GET_THREAD(thread);
	int writeback = 1;

	thread->broadcast = 0;

	if (replicate_spu.swap)
	{
		crPackGetStringSWAP( GL_EXTENSIONS, return_value, &writeback );
	}
	else
	{
		crPackGetString( GL_EXTENSIONS, return_value, &writeback );
	}
	replicatespuFlush( (void *) thread );

	while (writeback)
		crNetRecv();

	thread->broadcast = 1;

	CRASSERT(crStrlen((char *)return_value) < 10*1000);

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
