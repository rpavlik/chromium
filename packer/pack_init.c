/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_error.h"
#include "packer.h"
#include <stdio.h>

void crPackInit( int swapping )
{
	crDebug( "INITIALIZATION: %d", swapping );
	cr_packer_globals.swapping = swapping;
	cr_packer_globals.Flush = NULL;
	cr_packer_globals.SendHuge = NULL;
	cr_packer_globals.updateBBOX = 0;
}
