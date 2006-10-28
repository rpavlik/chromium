/* Copyright (c) 2004-2006, Tungsten Graphics, Inc.
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*#include "buffer.h"*/
#include "cr_threads.h"
#include "vncspu.h"
#include "cr_mem.h"


ScreenBuffer *
AllocScreenBuffer(void)
{
    ScreenBuffer *s = crCalloc(sizeof(ScreenBuffer));
    if (s) {
        s->buffer = (GLubyte *)
            crAlloc(vnc_spu.screen_width * vnc_spu.screen_height * 4);
        if (!s->buffer) {
            crFree(s);
            s = NULL;
        }
    }
    if (!s) {
        crError("VNC SPU: Out of memory allocating %d x %d screen buffer",
                vnc_spu.screen_width, vnc_spu.screen_height);
    }
    return s;
}


void
InitScreenBufferQueue(ScreenBufferQueue *q, const char *name)
{
   crInitMutex(&q->mutex);
   crInitCondition(&q->nonEmpty);
   q->size = 0;
   q->name = name;
}


/**
 * Helper, called when queue's mutex is locked.
 */
static ScreenBuffer *
DequeueLocked(ScreenBufferQueue *q)
{
   ScreenBuffer *b;
   int i;
   CRASSERT(q->size >= 1);
   b = q->buffer[0];
   q->size--;
   /* shift buffer entries toward head/zero */
   for (i = 0; i < q->size; i++) {
      q->buffer[i] = q->buffer[i + 1];
   }
   return b;
}


/**
 * Get buffer from head of queue.
 */
ScreenBuffer *
DequeueBuffer(ScreenBufferQueue *q)
{
   ScreenBuffer *b;
   crLockMutex(&q->mutex);
   while (q->size == 0) {
     /*crDebug("Wait for buffer in '%s' queue", q->name);*/
      crWaitCondition(&q->nonEmpty, &q->mutex);
   }
   b = DequeueLocked(q);
   crUnlockMutex(&q->mutex);
   return b;
}


ScreenBuffer *
DequeueBufferNoBlock(ScreenBufferQueue *q)
{
   ScreenBuffer *b;
   crLockMutex(&q->mutex);
   if (q->size == 0) {
      b = NULL;
   }
   else {
      b = DequeueLocked(q);
   }
   crUnlockMutex(&q->mutex);
   return b;
}


/**
 * If we can't dequeue from q1, dequeue from q2.
 */
ScreenBuffer *
DequeueBuffer2(ScreenBufferQueue *q1, ScreenBufferQueue *q2)
{
   ScreenBuffer *b;
   crLockMutex(&q1->mutex);
   crLockMutex(&q2->mutex);
   if (q1->size > 0) {
     b = DequeueLocked(q1);
     crUnlockMutex(&q1->mutex);
     crUnlockMutex(&q2->mutex);
     return b;
   }
   crUnlockMutex(&q1->mutex);
   crUnlockMutex(&q2->mutex);
   /*crDebug("Dequeue 2");*/
   return DequeueBuffer(q2);
}


int
QueueSize(ScreenBufferQueue *q)
{
   int sz;
   crLockMutex(&q->mutex);
   sz = q->size;
   crUnlockMutex(&q->mutex);
   return sz;
}


/**
 * Put given buffer onto tail of queue.
 */
void
EnqueueBuffer(ScreenBuffer *b, ScreenBufferQueue *q)
{
   crLockMutex(&q->mutex);
   CRASSERT(q->size < QUEUE_MAX);
   q->buffer[q->size++] = b;
   if (q->size == 1) {
      crSignalCondition(&q->nonEmpty);
   }
   crUnlockMutex(&q->mutex);
}

