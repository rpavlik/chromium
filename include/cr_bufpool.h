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

typedef struct CRBufferPool_t CRBufferPool;

CRBufferPool *crBufferPoolInit( unsigned int maxBuffers );
void  crBufferPoolFree( CRBufferPool *pool );

void  crBufferPoolPush( CRBufferPool *pool, void *buf, unsigned int bytes );
void *crBufferPoolPop( CRBufferPool *pool, unsigned int bytes );

#ifdef __cplusplus
}
#endif

#endif /* CR_BUFPOOL_H */
