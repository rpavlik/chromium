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

/* Magic number to determine if a TSDhandle has been initialized */
#define INIT_MAGIC 0xff8adc98


/* Initialize a TSDhandle */
void crInitTSD(TSDhandle *tsd)
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
void crSetTSD(TSDhandle *tsd, void *ptr)
{
	if (tsd->initMagic != (int) INIT_MAGIC) {
		/* initialize this TSDhandle */
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
void *crGetTSD(TSDhandle *tsd)
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
	/* XXX return calling thread's ID */
	return 0;
#else
	return (unsigned long) pthread_self();
#endif
}

