/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */
	
#ifndef CR_THREADS_H
#define CR_THREADS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "chromium.h"
#include "cr_bits.h"

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <pthread.h>
#include <semaphore.h>
#endif


/**
 * Thread ID/handle
 */
#ifdef WINDOWS
  typedef DWORD CRthread;
#  define crThreadProc LPTHREAD_START_ROUTINE
#  define CR_THREAD_PROC_DECL static DWORD WINAPI 
#  define CR_THREAD_EXIT(arg) return (arg);
#else
  typedef pthread_t CRthread;
  typedef void *(*crThreadProc) (void *);
#  define CR_THREAD_PROC_DECL static void*
#  define CR_THREAD_EXIT(arg) pthread_exit(arg);
#endif

extern int crCreateThread(CRthread *thread, int flags,
							 crThreadProc threadFunc, void *arg);

extern CRthread crThreadID(void);

extern void crThreadJoin(CRthread thread);

// Atomically assigns value to *target. Returns the former value of *target.
extern void* crInterlockedExchangePointer(void** target, void* value);


/*
 * Handle for Thread-Specific Data
 */
typedef struct {
#ifdef WINDOWS
	DWORD key;
#else
	pthread_key_t key;
#endif
	int initMagic;
} CRtsd;


extern void crInitTSD(CRtsd *tsd);
extern void crInitTSDF(CRtsd *tsd, void (*destructor)(void *));
extern void crFreeTSD(CRtsd *tsd);
extern void crSetTSD(CRtsd *tsd, void *ptr);
#ifdef WINDOWS
#define crGetTSD(TSD) TlsGetValue((TSD)->key)
#else
#define crGetTSD(TSD) pthread_getspecific((TSD)->key)
#endif


/* Mutex datatype */
#ifdef WINDOWS
typedef CRITICAL_SECTION CRmutex;
#else
typedef pthread_mutex_t CRmutex;
#endif

extern void crInitMutex(CRmutex *mutex);
extern void crFreeMutex(CRmutex *mutex);
extern void crLockMutex(CRmutex *mutex);
extern void crUnlockMutex(CRmutex *mutex);


/* Condition variable datatype */
#ifdef WINDOWS
typedef int CRcondition;
#else
typedef pthread_cond_t CRcondition;
#endif

extern void crInitCondition(CRcondition *cond);
extern void crFreeCondition(CRcondition *cond);
extern void crWaitCondition(CRcondition *cond, CRmutex *mutex);
extern void crSignalCondition(CRcondition *cond);


/* Barrier datatype */
typedef struct {
	unsigned int count;
#ifdef WINDOWS
	HANDLE hEvents[CR_MAX_CONTEXTS];
#else
	unsigned int waiting;
	pthread_cond_t cond;
	pthread_mutex_t mutex;
#endif
} CRbarrier;

extern void crInitBarrier(CRbarrier *b, unsigned int count);
extern void crFreeBarrier(CRbarrier *b);
extern void crWaitBarrier(CRbarrier *b);


/* Semaphores */
#ifdef WINDOWS
	typedef int CRsemaphore;
#else
	typedef sem_t CRsemaphore;
#endif

extern void crInitSemaphore(CRsemaphore *s, unsigned int count);
extern void crWaitSemaphore(CRsemaphore *s);
extern void crSignalSemaphore(CRsemaphore *s);


#ifdef __cplusplus
}
#endif

#endif /* CR_THREADS_H */
