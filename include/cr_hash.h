/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_HASH_H
#define CR_HASH_H

#include "chromium.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CRHashTable CRHashTable;

/* Callback function used for freeing/deleting table entries */
typedef void (*CRHashtableCallback)(void *data);
/* Callback function used for walking through table entries */
typedef void (*CRHashtableWalkCallback)(unsigned long key, void *data1, void *data2);

CRHashTable *crAllocHashtable( void );
void crFreeHashtable( CRHashTable *hash, CRHashtableCallback deleteCallback );
void crHashtableAdd( CRHashTable *h, unsigned long key, void *data );
GLuint crHashtableAllocKeys( CRHashTable *h, GLsizei range );
void crHashtableDelete( CRHashTable *h, unsigned long key, CRHashtableCallback deleteCallback );
void crHashtableDeleteBlock( CRHashTable *h, unsigned long key, GLsizei range, CRHashtableCallback deleteFunc );
void *crHashtableSearch( const CRHashTable *h, unsigned long key );
void crHashtableReplace( CRHashTable *h, unsigned long key, void *data, CRHashtableCallback deleteFunc);
unsigned int crHashtableNumElements( const CRHashTable *h) ;
GLboolean crHashtableIsKeyUsed( const CRHashTable *h, GLuint id );
void crHashtableWalk( CRHashTable *hash, CRHashtableWalkCallback walkFunc , void *data);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CR_HASH_H */
