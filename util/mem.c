/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mem.h"
#include "cr_error.h"

#include <stdlib.h>
#include <memory.h>

#if DEBUG_MEM
#include <stdio.h>
#define THRESHOLD 100 * 1024
#endif

#if DEBUG_MEM
static void *_crAlloc( unsigned int nbytes )
#else
void *crAlloc( unsigned int nbytes )
#endif
{
	void *ret = malloc( nbytes );
	if (!ret) {
		crError( "Out of memory trying to allocate %d bytes!", nbytes );
		abort();
	}
	return ret;
}

void *crAllocDebug( unsigned int nbytes, const char *file, int line )
{
#if DEBUG_MEM
	if (nbytes >= THRESHOLD)
		fprintf(stderr, "crAllocDebug(%d bytes) in %s at %d\n", nbytes, file, line);
	return _crAlloc(nbytes);
#else
	return crAlloc(nbytes);
#endif
}

#if DEBUG_MEM
static void *_crCalloc( unsigned int nbytes )
#else
void *crCalloc( unsigned int nbytes )
#endif
{
	void *ret = malloc( nbytes );
	if (!ret) {
		crError( "Out of memory trying to (c)allocate %d bytes!", nbytes );
		abort();
	}
	crMemset( ret, 0, nbytes );
	return ret;
}

void *crCallocDebug( unsigned int nbytes, const char *file, int line )
{
#if DEBUG_MEM
	if (nbytes >= THRESHOLD)
		fprintf(stderr, "crCallocDebug(%d bytes) in %s at %d\n", nbytes, file, line);
	return _crCalloc(nbytes);
#else
	return crCalloc(nbytes);
#endif
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
