/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef SAVEFRAME_SPU_H
#define SAVEFRAME_SPU_H

#ifdef WINDOWS
#define SAVEFRAMESPU_APIENTRY __stdcall
#else
#define SAVEFRAMESPU_APIENTRY
#endif

#include "cr_spu.h"

void saveframespuGatherConfiguration( void );

typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super;

    int stride;
    long framenum;
    long single;
    char *basename;
    GLsizei width, height;
    GLint x, y;
    GLubyte *buffer;
} SaveFrameSPU;

extern SaveFrameSPU saveframe_spu;

#endif /* SAVEFRAME_SPU_H */
