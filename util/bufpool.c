/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mem.h"
#include "cr_error.h"
#include "cr_bufpool.h"
#include <limits.h>


/*
 * New (version 1.4) buffer pool implementation.
 *
 * Now, each buffer in the pool can be a different size.
 * We only return a buffer from crBufferPoolPop() if we have exactly
 * the right size buffer.
 *
 * Note: the old implementation had a 'max buffers' parameter but it
 * really wasn't good for anything since we always grew the buffer pool
 * if we pushed a new buffer that would cause use to exceed the limit.
 * That's gone now.
 *
 * We're just using a simple linked list here.  Since we seldom have
 * more than about 10-15 buffers in the pool, that's OK.  A binary tree
 * would be nicer though.
 *
 * MCH: BufferPoolPop will now return the smallest buffer in the pool that
 *      is >= to the size required.  This fixes BufferPool overruns with lots
 *      of MTUs.
 */


#ifndef NULL
#define NULL  ((void *) 0)
#endif


typedef struct buffer
{
	void *address;
	unsigned int size;
	struct buffer *next;
} Buffer;


struct CRBufferPool_t
{
	unsigned int maxBuffers;
	unsigned int numBuffers;
	struct buffer *head;
};


int
crBufferPoolGetNumBuffers( CRBufferPool *pool )
{
	if ( pool )
		return pool->numBuffers;
	return 0;
}

int
crBufferPoolGetMaxBuffers( CRBufferPool *pool )
{
	if ( pool )
		return pool->maxBuffers;
	return 0;
}

CRBufferPool *
crBufferPoolInit( unsigned int maxBuffers )
{
	CRBufferPool *pool = crCalloc(sizeof(CRBufferPool));
	if (pool) {
		pool->head = NULL;
		pool->maxBuffers = maxBuffers;
		pool->numBuffers = 0;
	}
	return pool;
}

void
crBufferPoolFree( CRBufferPool *pool )
{
	Buffer *b, *next;

	for (b = pool->head; b; b = next) {
		next = b->next;
		crFree(b->address);
		crFree(b);
	}
}

void
crBufferPoolPush( CRBufferPool *pool, void *buf, unsigned int bytes )
{
	Buffer *b = crCalloc(sizeof(Buffer));
	if (b) {
		b->address = buf;
		b->size = bytes;
		b->next = pool->head;
		pool->head = b;
		pool->numBuffers++;
	}
}

void *
crBufferPoolPop( CRBufferPool *pool, unsigned int bytes )
{
	Buffer *b, *prev, *maybe_use;
	unsigned int next_smallest = UINT_MAX;
	unsigned int i;

	prev = NULL;
	maybe_use = NULL;
	for (b = pool->head, i=0; i<pool->numBuffers; b = b->next, i++) {
		if (b->size == bytes) {
			void *p = b->address;
			if (prev) {
				prev->next = b->next;
			}
			else {
				pool->head = b->next;
			}
			crFree(b);
			pool->numBuffers--;
			return p;
		}
		else if(b->size >= bytes){
			if (b->size < next_smallest){
			     maybe_use = b;
			}
		}
		prev = b;
	}
	if(maybe_use != NULL){
	     void *p = maybe_use->address;
	     if (prev) {
		  prev->next = maybe_use->next;
	     }
	     else {
		  pool->head = maybe_use->next;
	     }
	     crFree(maybe_use);
	     pool->numBuffers--;
	     return p;
	}
	return NULL;
}
