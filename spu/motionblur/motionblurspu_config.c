/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "motionblurspu.h"

#include "cr_mothership.h"
#include "cr_string.h"

#include <stdio.h>

static void __setDefaults( void )
{
    motionblur_spu.beginBlurFlag = GL_TRUE;
}


static void set_blur(void *spu, const char *value)
{
    if (crStrcmp(value, "Little") == 0)
        motionblur_spu.accumCoef = 0.25;
    else if (crStrcmp(value, "Medium") == 0)
        motionblur_spu.accumCoef = 0.4;
    else if (crStrcmp(value, "Lots") == 0)
        motionblur_spu.accumCoef = 0.7;
    else if (crStrcmp(value, "Extreme") == 0)
        motionblur_spu.accumCoef = 0.9;
    else
       printf("Bad value: %s\n", value);
}


/* option, type, nr, default, min, max, title, callback
 */
SPUOptions motionblurSPUOptions[] = {
   { "blur", CR_ENUM, 1, "Medium",
     "'Little', 'Medium', 'Lots', 'Extreme'", NULL,
     "Amount of blur", set_blur },

   { NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};


void motionblurspuGatherConfiguration( void )
{
	CRConnection *conn;

	__setDefaults();

	/* Connect to the mothership and identify ourselves. */
	
	conn = crMothershipConnect( );
	if (!conn)
	{
		/* The mothership isn't running.  Some SPU's can recover gracefully, some 
		 * should issue an error here. */
         	crSPUSetDefaultParams( &motionblur_spu, motionblurSPUOptions );
		return;
	}
	crMothershipIdentifySPU( conn, motionblur_spu.id );

	crSPUGetMothershipParams( conn, &motionblur_spu, motionblurSPUOptions );

	crMothershipDisconnect( conn );
}
