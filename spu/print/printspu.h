#ifndef PRINTSPU_H
#define PRINTSPU_H

#include "spu_dispatch_table.h"

#if defined(WINDOWS)
#define PRINT_APIENTRY __stdcall
#else
#define PRINT_APIENTRY
#endif

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
