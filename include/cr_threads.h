/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */
	
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
#endif
#include <stdio.h>
#include "cr_error.h"


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
extern void crSetTSD(CRtsd *tsd, void *ptr);
extern void *crGetTSD(CRtsd *tsd);
extern unsigned long crThreadID(void);


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
extern void crWaitBarrier(CRbarrier *b);


#ifdef __cplusplus
}
#endif

#endif /* CR_THREADS_H */
