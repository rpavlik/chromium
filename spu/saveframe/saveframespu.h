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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#ifdef JPEG
#include <jpeglib.h>
#endif

#include "cr_spu.h"

void saveframespuGatherConfiguration( const SPU *child_spu );
void ResizeBuffer(void);

typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super;

    int stride;
    int binary;
    int framenum;
    int single;
    char *spec;
    char *format;
    int enabled;

#ifdef JPEG
    struct jpeg_compress_struct cinfo;     /* jpeg compression information */
    struct jpeg_error_mgr       jerr;      /* jpeg compression error handler */
#endif

    GLsizei width, height;
    GLint x, y;
    GLubyte *buffer;
} SaveFrameSPU;

extern SaveFrameSPU saveframe_spu;

#endif /* SAVEFRAME_SPU_H */
