/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>

#include "cr_mem.h"
#include "cr_bufpool.h"

void
crBufferPoolInit( CRBufferPool *pool, unsigned int max )
{
	pool->num = 0;
	pool->max = max;
	pool->buf = (void **) crAlloc( pool->max * sizeof(pool->buf[0]) );
}

void
crBufferPoolFree( CRBufferPool *pool )
{
	crFree( pool->buf );
}

static void
crBufferPoolGrow( CRBufferPool *pool, unsigned int count )
{
	if ( count > pool->max )
	{
		unsigned int n_bytes;
		while ( count > pool->max )
			pool->max <<= 1;
		n_bytes = pool->max * sizeof(pool->buf[0]);
		crRealloc( (void **) &pool->buf, n_bytes );
	}
}

void
crBufferPoolLoad( CRBufferPool *pool, void *mem, unsigned int stride,
					  unsigned int count )
{
	unsigned int   i;
	unsigned char *buf;

	crBufferPoolGrow( pool, pool->num + count );

	buf = (unsigned char *) mem;
	for ( i = 0; i < count; i++ )
	{
		pool->buf[ pool->num++ ] = buf;
		buf += stride;
	}
}

void
crBufferPoolPush( CRBufferPool *pool, void *buf )
{
	if ( pool->num == pool->max )
		crBufferPoolGrow( pool, pool->num + 1 );

	pool->buf[ pool->num++ ] = buf;
}

void *
crBufferPoolPop( CRBufferPool *pool )
{
	void *buf = NULL;

	if ( pool->num )
		buf = pool->buf[ --pool->num ];

	return buf;
}
