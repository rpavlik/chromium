/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_hash.h"
#include "cr_mem.h"
#include "cr_error.h"
#include <stdio.h>

CRHashTable *crAllocHashtable( void )
{
	int i;
	CRHashTable *hash = (CRHashTable *) crAlloc( sizeof( CRHashTable )) ;
	hash->num_elements = 0;
	for (i = 0 ; i < CR_NUM_BUCKETS ; i++)
	{
		hash->buckets[i] = NULL;
	}
	return hash;
}

void crFreeHashtable( CRHashTable *hash )
{
	int i;

	for ( i = 0; i < CR_NUM_BUCKETS; i++ )
	{
		if ( hash->buckets[i] ) 
		{
			crFree( hash->buckets[i]->data );
			crFree( hash->buckets[i] );
		}
	}
	crFree( hash );
}


static unsigned int crHash( unsigned int key )
{
	return key % CR_NUM_BUCKETS;
}

void crHashtableAdd( CRHashTable *h, unsigned int key, void *data )
{
	CRHashNode *node = (CRHashNode *) crAlloc( sizeof( CRHashNode ) );
	node->key = key;
	node->data = data;
	node->next = h->buckets[crHash( key )];
	h->buckets[ crHash( key ) ] = node;
	h->num_elements++;
}

void crHashtableDelete( CRHashTable *h, unsigned int key )
{
	unsigned int index = crHash( key );
	CRHashNode *temp, *beftemp = NULL;
	for ( temp = h->buckets[index]; temp; temp = temp->next )
	{
		if ( temp->key == key )
			break;
		beftemp = temp;
	}
	if ( !temp )
		return; /* not an error */
	if ( beftemp )
		beftemp->next = temp->next;
	else
		h->buckets[index] = temp->next;
	h->num_elements--;
	crFree( temp->data );
	crFree( temp );
}

void *crHashtableSearch( CRHashTable *h, unsigned int key )
{
	unsigned int index = crHash( key );
	CRHashNode *temp;
	for ( temp = h->buckets[index]; temp; temp = temp->next )
	{
		if ( temp->key == key )
			break;
	}
	if ( !temp )
	{
		return NULL;
	}
	return temp->data;
}

void crHashtableReplace( CRHashTable *h, unsigned int key, void *data, int free_mem )
{
	unsigned int index = crHash( key );
	CRHashNode *temp;
	for ( temp = h->buckets[index]; temp; temp = temp->next )
	{
		if ( temp->key == key )
			break;
	}
	if ( !temp )
	{
		crHashtableAdd( h, key, data );
		return;
	}
	if ( free_mem )
	{
		crFree( temp->data );
	}
	temp->data = data;
}

unsigned int crHashtableNumElements( CRHashTable *h) 
{
	return h->num_elements;
}
