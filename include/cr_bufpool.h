/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_BUFPOOL_H
#define CR_BUFPOOL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CRBufferPool {
	unsigned int   num;
	unsigned int   max;
	void         **buf;
} CRBufferPool;

void  crBufferPoolInit( CRBufferPool *pool, unsigned int max );
void  crBufferPoolLoad( CRBufferPool *pool, void *mem,
		unsigned int stride, unsigned int count );
void  crBufferPoolPush( CRBufferPool *pool, void *buf );
void *crBufferPoolPop( CRBufferPool *pool );

#ifdef __cplusplus
}
#endif

#endif /* CR_BUFPOOL_H */
