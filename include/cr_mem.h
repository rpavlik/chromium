#ifndef CR_MEM_H
#define CR_MEM_H

void *crAlloc( unsigned int nbytes );
void crRealloc( void **ptr, unsigned int bytes );
void crFree( void *ptr );

#endif /* CR_MEM_H */
