#ifndef PRINTSPU_H
#define PRINTSPU_H

#include "spu_dispatch_table.h"

#include <stdio.h>

typedef struct {
	int id;
	SPUDispatchTable passthrough;
	FILE *fp;
} PrintSpu;

extern PrintSpu print_spu;

void printspuGatherConfiguration( void );
char *printspuEnumToStr( GLenum e );

#endif /* PRINTSPU_H */
