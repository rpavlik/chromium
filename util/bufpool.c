/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mem.h"
#include "cr_error.h"
#include "cr_bufpool.h"


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
	Buffer *b, *prev;
	prev = NULL;
	for (b = pool->head; b; b = b->next) {
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
		prev = b;
	}
	return NULL;
}
