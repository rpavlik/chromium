/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */
	
#ifndef CR_THREADS_H
#define CR_THREADS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WINDOWS
#include <pthread.h>
#endif
#include <stdio.h>
#include <string.h>
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
} TSDhandle;


extern void crInitTSD(TSDhandle *tsd);
extern void crSetTSD(TSDhandle *tsd, void *ptr);
extern void *crGetTSD(TSDhandle *tsd);
extern unsigned long crThreadID(void);


#ifdef __cplusplus
}
#endif

#endif /* CR_THREADS_H */
