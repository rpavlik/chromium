/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mem.h"
#include "cr_error.h"

#include <stdlib.h>

void *crAlloc( unsigned int nbytes )
{
	void *ret = malloc( nbytes );
	if (!ret)
		crError( "Out of memory trying to allocate %d bytes!", nbytes );
	return ret;
}

void crRealloc( void **ptr, unsigned int nbytes )
{
	if ( *ptr == NULL )
	{
		*ptr = crAlloc( nbytes );
	}
	else
	{
		*ptr = realloc( *ptr, nbytes );
		if (*ptr == NULL)
			crError( "Couldn't realloc %d bytes!", nbytes );
	}
}

void crFree( void *ptr )
{
	if (ptr)
		free(ptr);
}
