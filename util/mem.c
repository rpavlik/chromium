/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mem.h"
#include "cr_error.h"

#include <stdlib.h>
#include <memory.h>

void *crAlloc( unsigned int nbytes )
{
	void *ret = malloc( nbytes );
	if (!ret) {
		crError( "Out of memory trying to allocate %d bytes!", nbytes );
		abort();
	}
	return ret;
}

void *crCalloc( unsigned int nbytes )
{
	void *ret = malloc( nbytes );
	if (!ret) {
		crError( "Out of memory trying to (c)allocate %d bytes!", nbytes );
		abort();
	}
	crMemset( ret, 0, nbytes );
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

void crMemcpy( void *dst, const void *src, unsigned int bytes )
{
	CRASSERT(dst);
	CRASSERT(src);
	(void) memcpy( dst, src, bytes );
}

void crMemset( void *ptr, int value, unsigned int bytes )
{
	CRASSERT(ptr);
	memset( ptr, value, bytes );
}

void crMemZero( void *ptr, unsigned int bytes )
{
	CRASSERT(ptr);
	memset( ptr, 0, bytes );
}

int crMemcmp( const void *p1, const void *p2, unsigned int bytes )
{
	CRASSERT(p1);
	CRASSERT(p2);
	return memcmp( p1, p2, bytes );
}
