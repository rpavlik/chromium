#ifndef CR_BUFPOOL_H
#define CR_BUFPOOL_H

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

#endif /* CR_BUFPOOL_H */
