/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef STATE_H
#define STATE_H

#include "cr_glstate.h"
#ifdef CHROMIUM_THREADSAFE
#include "cr_threads.h"
#endif

extern SPUDispatchTable diff_api;
extern CRStateBits *__currentBits;
extern char *__stateExtensionString;

#define GetCurrentBits() __currentBits

#ifdef CHROMIUM_THREADSAFE
extern CRtsd __contextTSD;
#define GetCurrentContext() (CRContext *) crGetTSD(&__contextTSD)
#else
extern CRContext *__currentContext;
#define GetCurrentContext() __currentContext
#endif

#endif
