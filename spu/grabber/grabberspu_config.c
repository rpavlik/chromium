/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdlib.h>
#include "grabberspu.h"

#include "cr_mothership.h"
#include "cr_string.h"
#include "cr_mem.h"

static void set_current_window_attribute_name(GrabberSPU *spu, const char *response)
{
	if (spu->currentWindowAttributeName != NULL) 
	    crFree(spu->currentWindowAttributeName);
	spu->currentWindowAttributeName = crStrdup(response);
}


/* Example:
    { "optionName", <basetype> (CR_INT, CR_BOOL, CR_STRING), arraySize (usually 1),
      "defaultValueString", "minimumValueString", "maximumValueString", "description"},
 */
SPUOptions grabberSPUOptions[] = {
    { "current_window_attribute_name", CR_STRING, 1, NULL, NULL, NULL,
      "The name of the mothership attribute to change when the current window changes",
      (SPUOptionCB) set_current_window_attribute_name},

   { NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};


void grabberGatherConfiguration(GrabberSPU *spu)
{
	CRConnection *conn;

	/* Connect to the mothership and identify ourselves. */
	conn = crMothershipConnect( );
	if (conn == NULL) {
		crError("grabber SPU: cannot connect to mothership -aborting");
		return;
	}

	crMothershipIdentifySPU( conn, spu->id );
	crSPUGetMothershipParams( conn, (void *)spu, grabberSPUOptions );
	/* We don't disconnect the mothership; we'll need the connection
	 * later for normal functioning.
	 */
	spu->mothershipConnection = conn;
}
