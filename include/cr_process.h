/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_PROCESS_H
#define CR_PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

extern void crSleep( unsigned int seconds );

extern unsigned long crSpawn( const char *command, const char *argv[] );

extern void crKill( unsigned long pid );

extern void crGetProcName( char *name, int maxLen );

extern void crGetCurrentDir( char *dir, int maxLen );

#ifdef __cplusplus
}
#endif

#endif /* CR_PROCESS_H */
