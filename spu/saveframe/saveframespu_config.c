/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdlib.h>
#include "saveframespu.h"

#include "cr_mothership.h"
#include "cr_string.h"

#include <stdio.h>

static void __setDefaults( void )
{
    saveframe_spu.framenum = 0;
    saveframe_spu.buffer = NULL;
}

void set_stride( void *foo, const char *response ) 
{
   sscanf(response, "%d", &saveframe_spu.stride);
}

void set_basename( void *foo, const char *response ) 
{
   saveframe_spu.basename = crStrdup(response);
}

void set_single( void *foo, const char *response ) 
{
   sscanf(response, "%ld", &saveframe_spu.single);
}

void set_geometry( void *foo, const char *response ) 
{
   int x,y,w,h;
   sscanf( response, "%d %d %d %d", &x, &y, &w, &h );
   saveframe_spu.x = x;
   saveframe_spu.y = y;
   saveframe_spu.width = w;
   saveframe_spu.height = h;

   if ((saveframe_spu.width != -1) && (saveframe_spu.height != -1))
      ResizeBuffer();
}



/* option, type, nr, default, min, max, title, callback
 */
SPUOptions saveframeSPUOptions[] = {

   { "stride", CR_INT, 1, "1", NULL, NULL, 
     "Stride", (SPUOptionCB)set_stride },

   { "basename", CR_STRING, 1, "frame", NULL, NULL, 
     "Basename", (SPUOptionCB)set_basename },

   { "single", CR_INT, 1, "-1", NULL, NULL, 
     "Single", (SPUOptionCB)set_single },

   { "geometry", CR_INT, 4, "-1 -1 -1 -1", NULL, NULL, /* -1 width ??? */
     "Geometry", (SPUOptionCB)set_geometry },

   { NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },

};

void saveframespuGatherConfiguration( void )
{
    CRConnection *conn;

    __setDefaults();

    /* Connect to the mothership and identify ourselves. */

    conn = crMothershipConnect();
    if (!conn)
    {
        /* The mothership isn't running.  Some SPU's can recover gracefully,
         * some should issue an error here. */
         	crSPUSetDefaultParams( &saveframe_spu, saveframeSPUOptions );
        return;
    }
    crMothershipIdentifySPU(conn, saveframe_spu.id);

    crSPUGetMothershipParams( conn, &saveframe_spu, saveframeSPUOptions );

    crMothershipDisconnect(conn);
}
