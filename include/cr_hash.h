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

CRHashTable *crAllocHashtable( void );
void crFreeHashtable( CRHashTable *hash, CRHashtableCallback deleteCallback );
void crHashtableAdd( CRHashTable *h, unsigned int key, void *data );
GLuint crHashtableAllocKeys( CRHashTable *h, GLsizei range );
void crHashtableDelete( CRHashTable *h, unsigned int key, CRHashtableCallback deleteCallback );
void crHashtableDeleteBlock( CRHashTable *h, unsigned int key, GLsizei range, CRHashtableCallback deleteFunc );
void *crHashtableSearch( const CRHashTable *h, unsigned int key );
void crHashtableReplace( CRHashTable *h, unsigned int key, void *data, int free_mem );
unsigned int crHashtableNumElements( const CRHashTable *h) ;
GLboolean crHashtableIsKeyUsed( const CRHashTable *h, GLuint id );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CR_HASH_H */
