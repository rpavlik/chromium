#include "cr_mem.h"
#include "cr_error.h"

#include <stdlib.h>

void *CRAlloc( unsigned int nbytes )
{
	void *ret = malloc( nbytes );
	if (!ret)
		CRError( "Out of memory trying to allocate %d bytes!", nbytes );
	return ret;
}

void CRFree( void *ptr )
{
	if (ptr)
		free(ptr);
}
