/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_PROCESS_H
#define CR_PROCESS_H

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void crSleep( unsigned int seconds );

#ifdef WINDOWS
extern HANDLE crSpawn( const char *command, const char *argv[] );
extern void crKill( HANDLE pid );
#else
extern unsigned long crSpawn( const char *command, const char *argv[] );
extern void crKill( unsigned long pid );
#endif

extern void crGetProcName( char *name, int maxLen );

extern void crGetCurrentDir( char *dir, int maxLen );

#ifdef __cplusplus
}
#endif

#endif /* CR_PROCESS_H */
