/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_PACKER_H
#define CR_PACKER_H

#define CHROMIUM_THREADSAFE 1


#ifdef WINDOWS
#ifdef DLLDATA
#undef DLLDATA
#endif
#define DLLDATA __declspec(dllexport)
#endif

#include "cr_pack.h"
#include "cr_packfunctions.h"
#include "packer_extensions.h"
#ifdef CHROMIUM_THREADSAFE
#include "cr_threads.h"
#endif

#ifdef CHROMIUM_THREADSAFE
extern CRtsd _PackerTSD;
#define GET_PACKER_CONTEXT(C) CRPackContext *C = (CRPackContext *) crGetTSD(&_PackerTSD)
#else
extern DLLDATA CRPackContext cr_packer_globals;
#define GET_PACKER_CONTEXT(C) CRPackContext *C = &cr_packer_globals
#endif

extern void __PackError( int line, const char *file, GLenum error, char *format, ... );

#endif /* CR_PACKER_H */
