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
    saveframe_spu.stride = 1;
    saveframe_spu.framenum = 0;
    saveframe_spu.basename = "frame";
    saveframe_spu.single = -1;
    saveframe_spu.x = -1;
    saveframe_spu.y = -1;
    saveframe_spu.height = -1;
    saveframe_spu.width = -1;
    saveframe_spu.buffer = NULL;
}

void saveframespuGatherConfiguration( void )
{
    CRConnection *conn;
    char    response[8096];

    __setDefaults();

    /* Connect to the mothership and identify ourselves. */

    conn = crMothershipConnect();
    if (!conn)
    {
        /* The mothership isn't running.  Some SPU's can recover gracefully,
         * some should issue an error here. */
        return;
    }
    crMothershipIdentifySPU(conn, saveframe_spu.id);

    /* CONFIGURATION STUFF HERE */
    if (crMothershipGetSPUParam(conn, response, "stride"))
        sscanf(response, "%d", &saveframe_spu.stride);

    if (crMothershipGetSPUParam(conn, response, "basename"))
        saveframe_spu.basename = crStrdup(response);

    if (crMothershipGetSPUParam(conn, response, "single"))
        sscanf(response, "%ld", &saveframe_spu.single);

    if (crMothershipGetSPUParam(conn, response, "geometry"))
    {
        int x,y,w,h;
        sscanf( response, "%d %d %d %d", &x, &y, &w, &h );
        saveframe_spu.x = x;
        saveframe_spu.y = y;
        saveframe_spu.width = w;
        saveframe_spu.height = h;
    }

    if ((saveframe_spu.width != -1) && (saveframe_spu.height != -1))
    {
        if (saveframe_spu.buffer != NULL)
            free(saveframe_spu.buffer);

        saveframe_spu.buffer =
            (GLubyte *) malloc(sizeof(GLubyte) * saveframe_spu.width *
                               saveframe_spu.height * 4);
    }

    (void)response;

    crMothershipDisconnect(conn);
}
