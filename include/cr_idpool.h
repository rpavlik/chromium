/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */


/*
 * For allocating display IDs, texture IDs, vertex program IDs, etc.
 */


#ifndef CR_IDPOOL_H
#define CR_IDPOOL_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct CRIdPoolRec CRIdPool;


extern CRIdPool *crAllocIdPool( void );

extern void crFreeIdPool( CRIdPool *pool );


extern GLuint crIdPoolAllocBlock( CRIdPool *pool, GLuint count );

extern void crIdPoolFreeBlock( CRIdPool *pool, GLuint first, GLuint count );

extern void crIdPoolAllocId( CRIdPool *pool, GLuint id );

extern GLboolean crIdPoolIsIdFree( const CRIdPool *pool, GLuint id );

extern GLboolean crIdPoolIsIdUsed( const CRIdPool *pool, GLuint id );


#ifdef __cplusplus
}
#endif


#endif /* CR_IDPOOL_H */
