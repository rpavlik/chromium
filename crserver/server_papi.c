#include "server_dispatch.h"
#include "server.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_hash.h"
#include "cr_glstate.h"
#include "cr_applications.h"
#include "state/cr_statetypes.h"

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

	barrier = (CRBarrier *) crHashtableSearch( cr_barriers, name );

	if ( barrier == NULL )
	{
		barrier = (CRBarrier *) crAlloc( sizeof(*barrier) );
		barrier->count = count;
		barrier->num_waiting = 0;
		barrier->waiting = (RunQueue **) 
			crAlloc( count * sizeof(*(barrier->waiting)) );

		crHashtableAdd( cr_barriers, name, barrier );
	}
	else
	{
		/* HACK -- this allows everybody to create a barrier, and all
           but the first creation are ignored, assuming the count
           match. */
		if ( barrier->count != count )
		{
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
	char debug_buf[4096];

	barrier = (CRBarrier *) crHashtableSearch( cr_barriers, name );
	if ( barrier == NULL )
	{
		crError( "No such barrier: %d", name );
	}
	sprintf( debug_buf, "BarrierExec( %d )", name );
	cr_server.head_spu->dispatch_table.Hint( CR_PRINTSPU_STRING_HINT, (GLenum) debug_buf );
	sprintf( debug_buf, "num_waiting = %d", barrier->num_waiting );
	cr_server.head_spu->dispatch_table.Hint( CR_PRINTSPU_STRING_HINT, (GLenum) debug_buf );

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
		sema->count--;
	}
	else
	{
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
		sema->count++;
	}
}
