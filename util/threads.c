/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */
	
#include <stdio.h>
#include "cr_threads.h"
#include "cr_error.h"

/* perror() messages */
#define INIT_TSD_ERROR "InitTSD: failed to allocate key"
#define FREE_TSD_ERROR "FreeTSD: failed to destroy key"
#define SET_TSD_ERROR "InitTSD: thread failed to set thread specific data"
#define GET_TSD_ERROR "InitTSD: failed to get thread specific data"

/* Magic number to determine if a CRtsd has been initialized */
#define INIT_MAGIC 0xff8adc98


/* Initialize a CRtsd */
void crInitTSDF(CRtsd *tsd, void (*destructor)(void *))
{
#ifdef WINDOWS
	tsd->key = TlsAlloc();
	if (tsd->key == 0xffffffff) {
		crError("crInitTSD failed!");
	}
	(void) destructor;
#else
	if (pthread_key_create(&tsd->key, destructor) != 0) {
		perror(INIT_TSD_ERROR);
		crError("crInitTSD failed!");
	}
#endif
	tsd->initMagic = INIT_MAGIC;
}


void crInitTSD(CRtsd *tsd)
{
    crInitTSDF(tsd, NULL);
}


void crFreeTSD(CRtsd *tsd)
{
#ifdef WINDOWS
	/* Windows returns true on success, 0 on failure */
	if (TlsFree(tsd->key) == 0) {
		crError("crFreeTSD failed!");
	}
#else
	if (pthread_key_delete(tsd->key) != 0) {
		perror(FREE_TSD_ERROR);
		crError("crFreeTSD failed!");
	}
#endif
	tsd->initMagic = 0x0;
}


/* Set thread-specific data */
void crSetTSD(CRtsd *tsd, void *ptr)
{
	if (tsd->initMagic != (int) INIT_MAGIC) {
		/* initialize this CRtsd */
		crInitTSD(tsd);
	}
#ifdef WINDOWS
	if (TlsSetValue(tsd->key, ptr) == 0) {
		crError("crSetTSD failed!");
	}
#else
	if (pthread_setspecific(tsd->key, ptr) != 0) {
		crError("crSetTSD failed!");
	}
#endif
}


/* Get thread-specific data */
void *crGetTSD(CRtsd *tsd)
{
	if (tsd->initMagic != (int) INIT_MAGIC) {
		crInitTSD(tsd);
	}
#ifdef WINDOWS
	return TlsGetValue(tsd->key);
#else
	return pthread_getspecific(tsd->key);
#endif
}



/* Return ID of calling thread */
unsigned long crThreadID(void)
{
#ifdef WINDOWS
	return (unsigned long) GetCurrentThreadId();
#else
	return (unsigned long) pthread_self();
#endif
}



void crInitMutex(CRmutex *mutex)
{
#ifdef WINDOWS
	InitializeCriticalSection(mutex);
#else
	pthread_mutex_init(mutex, NULL);
#endif
}


void crFreeMutex(CRmutex *mutex)
{
#ifdef WINDOWS
	DeleteCriticalSection(mutex);
#else
	pthread_mutex_destroy(mutex);
#endif
}


void crLockMutex(CRmutex *mutex)
{
#ifdef WINDOWS
	EnterCriticalSection(mutex);
#else
	pthread_mutex_lock(mutex);
#endif
}


void crUnlockMutex(CRmutex *mutex)
{
#ifdef WINDOWS
	LeaveCriticalSection(mutex);
#else
	pthread_mutex_unlock(mutex);
#endif
}



void crInitBarrier(CRbarrier *b, unsigned int count)
{
#ifdef WINDOWS
	unsigned int i;

	for (i = 0; i < count; i++)
		b->hEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
#else
	b->count = count;
	b->waiting = 0;
	pthread_cond_init( &(b->cond), NULL );
	pthread_mutex_init( &(b->mutex), NULL );
#endif
}


void crWaitBarrier(CRbarrier *b)
{
#ifdef WINDOWS
	DWORD dwEvent;

	dwEvent = WaitForMultipleObjects( b->count, b->hEvents, FALSE, INFINITE );
#else
	pthread_mutex_lock( &(b->mutex) );
	b->waiting++;
	if (b->waiting < b->count) {
		pthread_cond_wait( &(b->cond), &(b->mutex) );
	}
	else {
		pthread_cond_broadcast( &(b->cond) );
		b->waiting = 0;
	}
	pthread_mutex_unlock( &(b->mutex) );
#endif
}
