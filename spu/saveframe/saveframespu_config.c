/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdlib.h>
#ifndef WINDOWS
#include <unistd.h>
#endif
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

void set_binary( void *foo, const char *response ) 
{
   sscanf(response, "%d", &saveframe_spu.binary);
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

   { "basename", CR_STRING, 1, "frame", NULL, NULL, 
     "Filename Basename", (SPUOptionCB)set_basename },

   { "stride", CR_INT, 1, "1", "1", NULL, 
     "Filename Number Stride", (SPUOptionCB)set_stride },

   { "single", CR_INT, 1, "-1", "-1", NULL, 
     "Single Frame Number", (SPUOptionCB)set_single },

   { "binary", CR_BOOL, 1, "1", NULL, NULL,
     "Binary PPM format", (SPUOptionCB)set_binary },

   { "geometry", CR_INT, 4, "0, 0, 100, 100", "0, 0, 1, 1", NULL,
     "Geometry (x, y, w, h)", (SPUOptionCB)set_geometry },

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
