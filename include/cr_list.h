#ifndef CR_LIST_H
#define CR_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CRList CRList;
typedef struct CRListIterator CRListIterator;
typedef int ( *CRListCompareFunc ) ( void *element1, void *element2 );
typedef void ( *CRListApplyFunc ) ( void *element, void *arg );

CRList *crAllocList( void );
void crFreeList( CRList *l );

unsigned crListSize( const CRList *l );
int crListIsEmpty( const CRList *l );

void crListInsert( CRList *l, CRListIterator *iter, void *elem );
void crListErase( CRList *l, CRListIterator *iter );
void crListClear( CRList *l );

void crListPushBack( CRList *l, void *elem );
void crListPushFront( CRList *l, void *elem );

void crListPopBack( CRList *l );
void crListPopFront( CRList *l );

void *crListFront( CRList *l );
void *crListBack( CRList *l );

CRListIterator *crListBegin( CRList *l );
CRListIterator *crListEnd( CRList *l );

CRListIterator *crListNext( CRListIterator *iter );
CRListIterator *crListPrev( CRListIterator *iter );
void *crListElement( CRListIterator *iter );

CRListIterator *crListFind( CRList *l, void *element, CRListCompareFunc compare );
void crListApply( CRList *l, CRListApplyFunc apply, void *arg );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CR_LIST_H */
