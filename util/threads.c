/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */
	
#include <stdio.h>
#include <string.h>
#include "cr_threads.h"
#include "cr_error.h"

/* perror() messages */
#define INIT_TSD_ERROR "InitTSD: failed to allocate key"
#define SET_TSD_ERROR "InitTSD: thread failed to set thread specific data"
#define GET_TSD_ERROR "InitTSD: failed to get thread specific data"

/* Magic number to determine if a CRtsd has been initialized */
#define INIT_MAGIC 0xff8adc98


/* Initialize a CRtsd */
void crInitTSD(CRtsd *tsd)
{
#ifdef WINDOWS
	tsd->key = TlsAlloc();
	if (tsd->key == 0xffffffff) {
		crError("crInitTSD failed!");
	}
#else
	if (pthread_key_create(&tsd->key, NULL/*free*/) != 0) {
		perror(INIT_TSD_ERROR);
		crError("crInitTSD failed!");
	}
#endif
	tsd->initMagic = INIT_MAGIC;
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



#ifdef WINDOWS
void crInitMutex(CRITICAL_SECTION *mutex)
{
	InitializeCriticalSection(mutex);
}
#else
void crInitMutex(CRmutex *mutex)
{
	pthread_mutex_init(mutex, NULL);
}
#endif


#ifdef WINDOWS
void crLockMutex(CRITICAL_SECTION *mutex)
{
	EnterCriticalSection(mutex);
}
#else
void crLockMutex(CRmutex *mutex)
{
	pthread_mutex_lock(mutex);
}
#endif


#ifdef WINDOWS
void crUnlockMutex(CRITICAL_SECTION *mutex)
{
	LeaveCriticalSection(mutex);
}
#else
void crUnlockMutex(CRmutex *mutex)
{
	pthread_mutex_unlock(mutex);
}
#endif



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
