/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef PRINTSPU_H
#define PRINTSPU_H

#include "spu_dispatch_table.h"
#include "cr_spu.h"

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

void printspuGatherConfiguration( const SPU *child_spu );
char *printspuEnumToStr( GLenum e );

#endif /* PRINTSPU_H */
