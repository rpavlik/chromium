/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "cr_error.h"
#include <stdio.h>

void crSPUInitDispatchTable( SPUDispatchTable *table )
{
	table->copy_of = NULL;
	table->copyList = NULL;
	table->mark = 0;
}
