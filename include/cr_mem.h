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

void *crAlloc( unsigned int nbytes );
void crRealloc( void **ptr, unsigned int bytes );
void crFree( void *ptr );

#ifdef __cplusplus
}
#endif

#endif /* CR_MEM_H */
