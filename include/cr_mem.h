/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_MEM_H
#define CR_MEM_H

#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG_MEM 0

void *crAllocDebug( unsigned int nbytes, const char *file, int line );
void *crCallocDebug( unsigned int nbytes, const char *file, int line );
#if DEBUG_MEM
#define crAlloc(N) crAllocDebug(N, __FILE__, __LINE__)
#define crCalloc(N) crCallocDebug(N, __FILE__, __LINE__)
#else
void *crAlloc( unsigned int nbytes );
void *crCalloc( unsigned int nbytes );
#endif
void crRealloc( void **ptr, unsigned int bytes );
void crFree( void *ptr );
void crMemcpy( void *dst, const void *src, unsigned int bytes );
void crMemset( void *ptr, int value, unsigned int bytes );
void crMemZero( void *ptr, unsigned int bytes );
int  crMemcmp( const void *p1, const void *p2, unsigned int bytes );

#ifdef __cplusplus
}
#endif

#endif /* CR_MEM_H */
