#ifndef CR_HASH_H
#define CR_HASH_H

#define CR_NUM_BUCKETS 1047

typedef struct CRHashNode {
        unsigned int key;
        void *data;
        struct CRHashNode *next;
} CRHashNode;

typedef struct CRHashTable {
        unsigned int num_elements;
        CRHashNode *buckets[CR_NUM_BUCKETS];
} CRHashTable;

CRHashTable *crAllocHashtable( void );
void crFreeHashtable( CRHashTable *hash );
void crHashtableAdd( CRHashTable *h, unsigned int key, void *data );
void crHashtableStringAdd( CRHashTable *h, char *key, void *data );
void crHashtableDelete( CRHashTable *h, unsigned int key );
void crHashtableStringDelete( CRHashTable *h, char *key );
void *crHashtableSearch( CRHashTable *h, unsigned int key );
void *crHashtableStringSearch( CRHashTable *h, char * key );
void crHashtableReplace( CRHashTable *h, unsigned int key, void *data, int free_mem );
void crHashtableStringReplace( CRHashTable *h, char * key, void *data, int free_mem );
unsigned int crHashtableNumElements( CRHashTable *h) ;

#define CR_HASHTABLE_WALK( h, t ) {         \
  CRHashNode *t;                            \
  int _;                                        \
  for (_ = 0 ; _ < CR_NUM_BUCKETS ; _++) {  \
    for (t = h->buckets[_] ; t; t = t->next)   {


#define CR_HASHTABLE_WALK_END( h )          \
  }                                             \
 }                                              \
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CR_HASH_H */
