#ifndef PRINTSPU_H
#define PRINTSPU_H

#include "spu_dispatch_table.h"

typedef struct {
	SPUDispatchTable passthrough;
} PrintSpu;

extern PrintSpu print_spu;

extern char *printspuEnumToStr( GLenum e );

#endif /* PRINTSPU_H */
