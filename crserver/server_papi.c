/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "server_dispatch.h"
#include "server.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_hash.h"
#include "cr_glstate.h"
#include "cr_applications.h"
#include "state/cr_statetypes.h"

#define DEBUG_BARRIERS 1

/* Semaphore wait queue node */
typedef struct _wqnode {
	RunQueue *q;
	struct _wqnode *next;
} wqnode;

typedef struct {
	GLuint count;
	GLuint num_waiting;
	RunQueue **waiting;
} CRBarrier;

typedef struct {
	GLuint count;
	wqnode *waiting, *tail;
} CRSemaphore;

CRHashTable *cr_barriers, *cr_semaphores;

void SERVER_DISPATCH_APIENTRY crServerDispatchBarrierCreate( GLuint name, GLuint count )
{
	CRBarrier *barrier;
#if DEBUG_BARRIERS
	char debug_buf[4096];
#endif

	barrier = (CRBarrier *) crHashtableSearch( cr_barriers, name );

#if DEBUG_BARRIERS
	sprintf( debug_buf, "BarrierCreate( %d, %d )", name, count );
	cr_server.head_spu->dispatch_table.ChromiumParametervCR( GL_PRINT_STRING_CR, GL_UNSIGNED_BYTE, sizeof(debug_buf), debug_buf );
#endif
	if (count == 0)
	{
		count = cr_server.numClients;
#if DEBUG_BARRIERS
		sprintf( debug_buf, "changing count to %d", count );
		cr_server.head_spu->dispatch_table.ChromiumParametervCR( GL_PRINT_STRING_CR, GL_UNSIGNED_BYTE, sizeof(debug_buf), debug_buf );
#endif
	}


	/* we use maxBarrierCount in Clear() and SwapBuffers() and also use it
	 * in __getNextClient() for deadlock detection.  The issue is that all
	 * the existing clients may be blocked, but we might soon get another
	 * client connection to break the apparent deadlock.
	 */
	if (count > cr_server.maxBarrierCount)
		cr_server.maxBarrierCount = count;

	if ( barrier == NULL )
	{
		barrier = (CRBarrier *) crAlloc( sizeof(*barrier) );
		barrier->count = count;
		barrier->num_waiting = 0;
		barrier->waiting = (RunQueue **) 
			crAlloc( count * sizeof(*(barrier->waiting)) );

		crHashtableAdd( cr_barriers, name, barrier );
#if DEBUG_BARRIERS
		sprintf( debug_buf, "This was a new barrier!" );
		cr_server.head_spu->dispatch_table.ChromiumParametervCR( GL_PRINT_STRING_CR, GL_UNSIGNED_BYTE, sizeof(debug_buf), debug_buf );
#endif
	}
	else
	{
		/* HACK -- this allows everybody to create a barrier, and all
           but the first creation are ignored, assuming the count
           match. */
#if DEBUG_BARRIERS
		sprintf( debug_buf, "I already knew about this barrier." );
		cr_server.head_spu->dispatch_table.ChromiumParametervCR( GL_PRINT_STRING_CR, GL_UNSIGNED_BYTE, sizeof(debug_buf), debug_buf );
#endif
		if ( barrier->count != count )
		{
#if DEBUG_BARRIERS
			sprintf( debug_buf, "And someone messed up the count!." );
			cr_server.head_spu->dispatch_table.ChromiumParametervCR( GL_PRINT_STRING_CR, GL_UNSIGNED_BYTE, sizeof(debug_buf), debug_buf );
#endif
			crError( "Barrier name=%u created with count=%u, but already "
						 "exists with count=%u", name, count, barrier->count );
		}
	}
}

void SERVER_DISPATCH_APIENTRY crServerDispatchBarrierDestroy( GLuint name )
{
	crError( "NO BARRIER DESTROY FOR YOU!  (name=%u)", name );
}

void SERVER_DISPATCH_APIENTRY crServerDispatchBarrierExec( GLuint name )
{
	CRBarrier *barrier;
#if DEBUG_BARRIERS
	char debug_buf[4096];
#endif

	barrier = (CRBarrier *) crHashtableSearch( cr_barriers, name );
	if ( barrier == NULL )
	{
		crError( "crServerDispatchBarrierExec: No such barrier: %d", name );
	}

#if DEBUG_BARRIERS
	sprintf( debug_buf, "BarrierExec( %d )", name );
	cr_server.head_spu->dispatch_table.ChromiumParametervCR( GL_PRINT_STRING_CR, GL_UNSIGNED_BYTE, sizeof(debug_buf), debug_buf );
	sprintf( debug_buf, "num_waiting = %d", barrier->num_waiting );
	cr_server.head_spu->dispatch_table.ChromiumParametervCR( GL_PRINT_STRING_CR, GL_UNSIGNED_BYTE, sizeof(debug_buf), debug_buf );
#endif

	barrier->waiting[barrier->num_waiting++] = run_queue;

	run_queue->blocked = 1;

	if ( barrier->num_waiting == barrier->count )
	{
		GLuint i;

		for ( i = 0; i < barrier->count; i++ )
		{
			barrier->waiting[i]->blocked = 0;
		}
		barrier->num_waiting = 0;
	}
}

void SERVER_DISPATCH_APIENTRY crServerDispatchSemaphoreCreate( GLuint name, GLuint count )
{
	CRSemaphore *sema;

	sema = (CRSemaphore *) crAlloc( sizeof( *sema ) );
	crHashtableAdd( cr_semaphores, name, sema );
	sema->count = count;
	sema->waiting = sema->tail = NULL;
}

void SERVER_DISPATCH_APIENTRY crServerDispatchSemaphoreDestroy( GLuint name )
{
	crError( "NO DESTROY FOR YOU! (name=%u)", name );
}

/* Semaphore wait */
void SERVER_DISPATCH_APIENTRY crServerDispatchSemaphoreP( GLuint name )
{
	CRSemaphore *sema;

	sema = (CRSemaphore *) crHashtableSearch( cr_semaphores, name );
	if (!sema)
	{
		crError( "No such semaphore: %d", name );
	}
	if (sema->count)
	{
		/* go */
		sema->count--;
	}
	else
	{
		/* block */
		wqnode *node;
		run_queue->blocked = 1;
		node = (wqnode *) crAlloc( sizeof( *node ) );
		node->q = run_queue;
		node->next = NULL;
		if (sema->tail)
		{
			sema->tail->next = node;
		}
		else
		{
			sema->waiting = node;
		}
		sema->tail = node;
	}
}

/* Semaphore signal */
void SERVER_DISPATCH_APIENTRY crServerDispatchSemaphoreV( GLuint name )
{
	CRSemaphore *sema;

	sema = (CRSemaphore *) crHashtableSearch( cr_semaphores, name );
	if (!sema)
	{
		crError( "No such semaphore: %d", name );
	}
	if (sema->waiting)
	{
		/* unblock one waiter */
		wqnode *temp = sema->waiting;
		temp->q->blocked = 0;
		sema->waiting = temp->next;
		crFree( temp );
		if (!sema->waiting)
		{
			sema->tail = NULL;
		}
	}
	else
	{
		/* nobody's waiting */
		sema->count++;
	}
}
