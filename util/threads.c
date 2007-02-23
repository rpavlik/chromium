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


/**
 * Create a new thread.
 * \param thread  returns the new thread's info/handle
 * \param flags  optional flags (none at this time)
 * \param threadFunc  the function to run in new thread
 * \param arg  argument to pass to threadFunc
 * \return 0 for success, non-zero if error
 */
int
crCreateThread(CRthread *thread, int flags,
							 crThreadProc threadFunc, void *arg)
{
#ifdef WINDOWS
  return CreateThread(NULL, 0, threadFunc, arg, 0, thread) != NULL;
#else
	int i = pthread_create(thread, NULL, threadFunc, arg);
	return i;
#endif
}



/**
 * Return ID of calling thread.
 */
CRthread
crThreadID(void)
{
#ifdef WINDOWS
	return GetCurrentThreadId();
#else
	return pthread_self();
#endif
}

/**
 * Joins the given thread
 */
void 
crThreadJoin(CRthread thread) 
{
#ifdef WINDOWS
  HANDLE h = OpenThread(SYNCHRONIZE, 1, thread);
  WaitForSingleObject(h, INFINITE);
  CloseHandle(h);
#else
  pthread_join(thread, NULL);
#endif
}

/**
 * Atomical exchange of pointer values
 */
void* 
crInterlockedExchangePointer(void** target, void* value) {
#ifdef WINDOWS
  return InterlockedExchangePointer(target, value);
#else
  void* ret;
  
  asm ( 
    "lock; xchgl %0, (%1)"
    : "=r" (ret)
    : "r" (target), "0" (value) : "memory" 
  );

  return ret;
#endif
}


/**
 * Initialize a thread-specific data handle, with destructor function.
 */
void crInitTSDF(CRtsd *tsd, void (*destructor)(void *))
{
	if (tsd->initMagic != (int) INIT_MAGIC) {
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
	}
	tsd->initMagic = INIT_MAGIC;
}


/**
 * Initialize a thread-specific data handle.
 */
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
	CRASSERT(tsd->initMagic == (int) INIT_MAGIC);
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


void crInitCondition(CRcondition *cond)
{
#ifdef WINDOWS
	/* XXX fix me */
	(void) cond;
#else
	int err = pthread_cond_init(cond, NULL);
	if (err) {
		crError("crInitCondition failed");
	}
#endif
}


void crFreeCondition(CRcondition *cond)
{
#ifdef WINDOWS
	/* XXX fix me */
	(void) cond;
#else
	int err = pthread_cond_destroy(cond);
	if (err) {
		crError("crFreeCondition error (threads waiting on the condition?)");
	}
#endif
}

/**
 * We're basically just wrapping the pthread condition var interface.
 * See the man page for pthread_cond_wait to learn about the mutex parameter.
 */
void crWaitCondition(CRcondition *cond, CRmutex *mutex)
{
#ifdef WINDOWS
	/* XXX fix me */
	(void) cond;
	(void) mutex;
#else
	pthread_cond_wait(cond, mutex);
#endif
}


void crSignalCondition(CRcondition *cond)
{
#ifdef WINDOWS
	/* XXX fix me */
	(void) cond;
#else
	pthread_cond_signal(cond);
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


void crFreeBarrier(CRbarrier *b)
{
	/* XXX anything to do? */
}


void crWaitBarrier(CRbarrier *b)
{
#ifdef WINDOWS
	DWORD dwEvent
		= WaitForMultipleObjects( b->count, b->hEvents, FALSE, INFINITE );
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


void crInitSemaphore(CRsemaphore *s, unsigned int count)
{
#ifdef WINDOWS
	crWarning("CRsemaphore functions not implemented on Windows");
#else
	sem_init(s, 0, count);
#endif
}


void crWaitSemaphore(CRsemaphore *s)
{
#ifdef WINDOWS
	/* to do */
#else
	sem_wait(s);
#endif
}


void crSignalSemaphore(CRsemaphore *s)
{
#ifdef WINDOWS
	/* to do */
#else
	sem_post(s);
#endif
}

