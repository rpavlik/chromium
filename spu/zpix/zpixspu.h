/* Copyright (c) 2003, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef ZPIX_SPU_H
#define ZPIX_SPU_H

#ifdef WINDOWS
#define ZPIXSPU_APIENTRY __stdcall
#else
#define ZPIXSPU_APIENTRY
#endif

#include "cr_spu.h"
#include "cr_server.h"


/* 
     compression types
*/
typedef enum {
        ZNONE =  0,       /* no compression       */
        ZLIB  =  1,       /* Gnu Zlib compression */
        ZRLE  =  2,       /* Run Length Encoding  */
        ZPLE  =  3        /* "packed" rle         */
} ZTYPE;

/* 
     frame buffer types
*/
typedef enum {
        FBCOLOR   = 0,      /* Color   */
        FBDEPTH   = 1,      /* Depth   */
        FBSTENCIL = 2,      /* Stencil */
        FBNUM = 3
} FBTYPE;

/* 
     ple buffer layout

         Like RLE, there are length and data fields but
         unlike RLE the lengths are stored in a vector
         preceding the data and indexed backwards similar
         to chromium's packing buffer (hence the name)

         
*/
typedef  signed char PLErun;
typedef union pledata {
        uint   value;                 /* data value  */
        PLErun run[sizeof(uint)];     /* run length vector
                                         indexed backwards from ple_doff
                                         each byte has a count
                                           -n = string of n single words
                                                (n should be at least 2)
                                           +n = n repeats of next word
                                                (n may be  1)
                                            0 = next value is count of prefval
                                      */
        } PLEdata; 

typedef struct{
/*
   For a given n words, worst case size of a valid PLEbuf occurs
   when values alternate between prefval and something else.
               len = sizeof(PLEbuf) + n + 4*n
*/
        int  len;      /* packed buffer byte len  */
        int  n;        /* words of unpacked data */
        int  beg;      /* index to packed data 
                              starts as sizeof(PLEbuf)+n bytes)
                              but squeezed at end
                       */
        uint prefval;  /* preferred data value, 0 for XOR  */
        int  nruns;    /*XXX JAG debug count of runs */
        PLEdata data[];
        }PLEbuf;

/* 
     ZpixSPU instance data
*/
typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super;
	CRServer *server;
 
        /* config options */
        int verbose;     /* 0 = quiet; 1 = chatty */
        int debug;       /* 0 = normal; 1 = debug */
        int no_diff;     /* 0 = difference; 1 = suppress  */ 
        int ztype;       /* compression type - See ZStateType enum */
        int zparm;       /* parameter for ztype */
 
        /* instance data */

        /* raster positions */
        GLint   rXold, rYold;
        GLint   rXnew, rYnew;

        /* allocated frame buffers */

        GLsizei fbWidth[FBNUM], fbHeight[FBNUM];

        GLsizei fbLen[FBNUM];
        GLvoid  *fBuf[FBNUM];             /* currently visible */ 
        GLvoid  *dBuf[FBNUM];             /* transmitted differences */

        GLsizei zbLen[FBNUM];
        GLvoid  *zBuf[FBNUM];             /* client-side compress */

        /* statistics */
        long n;
        long sum_bytes, sum_zbytes;
        long sum_runs, sum_prefv;

} ZpixSPU;

extern ZpixSPU zpix_spu;

extern SPUNamedFunctionTable _cr_zpix_table[];

extern SPUOptions zpixSPUOptions[];

extern void zpixspuGatherConfiguration( ZpixSPU *zpix_spu );


#endif /* ZPIX_SPU_H */
