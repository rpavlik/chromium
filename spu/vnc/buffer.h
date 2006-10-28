/* Copyright (c) 2004-2006, Tungsten Graphics, Inc.
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef BUFFER_H
#define BUFFER_H

#include "cr_threads.h"
#include "vncspu.h"


/**
 * The screen buffer contains the pixels we'll encode and send to clients.
 */
typedef struct {
	GLubyte *buffer;  /* screen_width * screen_height * 4 */
	RegionRec dirtyRegion;
	GLboolean regionSent;
} ScreenBuffer;



#define QUEUE_MAX 5

/**
 * Queue of ScreenBuffers, thread-safe.
 */
typedef struct
{
	CRmutex mutex;
	CRcondition nonEmpty;
	int size;
	ScreenBuffer *buffer[QUEUE_MAX];
	const char *name;
} ScreenBufferQueue;


extern ScreenBuffer *
AllocScreenBuffer(void);

extern void
InitScreenBufferQueue(ScreenBufferQueue *q, const char *name);

extern ScreenBuffer *
DequeueBuffer(ScreenBufferQueue *q);

extern ScreenBuffer *
DequeueBufferNoBlock(ScreenBufferQueue *q);

extern ScreenBuffer *
DequeueBuffer2(ScreenBufferQueue *q1, ScreenBufferQueue *q2);

extern int
QueueSize(ScreenBufferQueue *q);

extern void
EnqueueBuffer(ScreenBuffer *b, ScreenBufferQueue *q);


#endif
