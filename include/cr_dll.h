/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_DLL_H
#define CR_DLL_H

#if defined(WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	char *name;
#if defined(WINDOWS)
	HINSTANCE hinstLib;
#elif defined(IRIX) || defined(IRIX64) || defined(Linux) || defined(FreeBSD) || defined(DARWIN) || defined(AIX) || defined(SunOS) || defined(OSF1)
	void *hinstLib;
#else
#error ARCHITECTURE DLL NOT IMPLEMENTED
#endif
} CRDLL;

typedef void (*CRDLLFunc)(void);
CRDLL *crDLLOpen( const char *dllname, int resolveGlobal );
CRDLLFunc crDLLGetNoError( CRDLL *dll, const char *symname );
CRDLLFunc crDLLGet( CRDLL *dll, const char *symname );
void crDLLClose( CRDLL *dll );

#ifdef __cplusplus
}
#endif

#endif /* CR_DLL_H */
