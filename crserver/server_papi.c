/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "server_dispatch.h"
#include "server.h"
#include "cr_error.h"
#include "cr_mem.h"
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

void SERVER_DISPATCH_APIENTRY crServerDispatchBarrierCreateCR( GLuint name, GLuint count )
{
	CRBarrier *barrier;
#if DEBUG_BARRIERS
	char debug_buf[4096];
#endif

	if (cr_server.ignore_papi)
	{
		cr_server.head_spu->dispatch_table.BarrierCreateCR( name, count );
		return;
	}

	barrier = (CRBarrier *) crHashtableSearch( cr_barriers, name );

#if DEBUG_BARRIERS
	sprintf( debug_buf, "BarrierCreateCR( %d, %d )", name, count );
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

	if (cr_server.debug_barriers)
		crDebug("crserver: BarrierCreate(id=%d, count=%d)", name, barrier->count);
}

void SERVER_DISPATCH_APIENTRY crServerDispatchBarrierDestroyCR( GLuint name )
{
	if (cr_server.ignore_papi)
	{
		cr_server.head_spu->dispatch_table.BarrierDestroyCR( name );
		return;
	}

	crError( "NO BARRIER DESTROY FOR YOU!  (name=%u)", name );
}

void SERVER_DISPATCH_APIENTRY crServerDispatchBarrierExecCR( GLuint name )
{
	CRBarrier *barrier;
#if DEBUG_BARRIERS
	char debug_buf[4096];
#endif

	if (cr_server.ignore_papi)
	{
		cr_server.head_spu->dispatch_table.BarrierExecCR( name );
		return;
	}

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

		if (cr_server.debug_barriers)
			crDebug("crserver: BarrierExec(id=%d, num_waiting=%d/%d) - release.", name, barrier->num_waiting, barrier->count);

		for ( i = 0; i < barrier->count; i++ )
		{
			barrier->waiting[i]->blocked = 0;
		}
		barrier->num_waiting = 0;
	}
	else if (cr_server.debug_barriers)
		crDebug("crserver: BarrierExec(id=%d, num_waiting=%d/%d) - block", name, barrier->num_waiting, barrier->count);

}

void SERVER_DISPATCH_APIENTRY crServerDispatchSemaphoreCreateCR( GLuint name, GLuint count )
{
	CRSemaphore *sema;

	if (cr_server.ignore_papi)
	{
		cr_server.head_spu->dispatch_table.SemaphoreCreateCR( name, count );
		return;
	}

	sema = crHashtableSearch(cr_semaphores, name);
	if (sema)
	   return; /* already created */

	sema = (CRSemaphore *) crAlloc( sizeof( *sema ) );
	crHashtableAdd( cr_semaphores, name, sema );
	sema->count = count;
	sema->waiting = sema->tail = NULL;
	if (cr_server.debug_barriers)
		crDebug("crserver: SemaphoreCreate(id=%d, count=%d)", name, count);
}

void SERVER_DISPATCH_APIENTRY crServerDispatchSemaphoreDestroyCR( GLuint name )
{
	if (cr_server.ignore_papi)
	{
		cr_server.head_spu->dispatch_table.SemaphoreDestroyCR( name );
		return;
	}

	crError( "NO DESTROY FOR YOU! (name=%u)", name );
}

/* Semaphore wait */
void SERVER_DISPATCH_APIENTRY crServerDispatchSemaphorePCR( GLuint name )
{
	CRSemaphore *sema;

	if (cr_server.ignore_papi)
	{
		cr_server.head_spu->dispatch_table.SemaphorePCR( name );
		return;
	}

	sema = (CRSemaphore *) crHashtableSearch( cr_semaphores, name );
	if (!sema)
	{
		crError( "No such semaphore: %d", name );
	}
	if (sema->count)
	{
		/* go */
		if (cr_server.debug_barriers)
			crDebug("crserver: SemaphoreP(id=%d, count=%d) decrement to %d",
					name, sema->count, sema->count - 1);
		sema->count--;
	}
	else
	{
		/* block */
		wqnode *node;
		if (cr_server.debug_barriers)
			crDebug("crserver: SemaphoreP(id=%d, count=%d) - block.",
					name, sema->count);
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
void SERVER_DISPATCH_APIENTRY crServerDispatchSemaphoreVCR( GLuint name )
{
	CRSemaphore *sema;

	if (cr_server.ignore_papi)
	{
		cr_server.head_spu->dispatch_table.SemaphoreVCR( name );
		return;
	}

	sema = (CRSemaphore *) crHashtableSearch( cr_semaphores, name );
	if (!sema)
	{
		crError( "No such semaphore: %d", name );
	}
	if (sema->waiting)
	{
		wqnode *temp = sema->waiting;
		if (cr_server.debug_barriers)
			crDebug("crserver: SemaphoreV(id=%d, count=%d) - unblock.",
					name, sema->count);
		/* unblock one waiter */
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
		if (cr_server.debug_barriers)
			crDebug("crserver: SemaphoreV(id=%d, count=%d) - increment to %d",
					name, sema->count, sema->count + 1);
		sema->count++;
	}
}
