/*
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
    shadow buffers (each client has one set
                       a server could have many 

   XXX Note: It really isn't necessary to have
             more than one global difference buffer,
             as long as one tracked the maximum
             observed need, but the code evolved from
             a place which made this simpler.
*/
typedef struct {
        GLsizei  fbWidth[FBNUM], fbHeight[FBNUM];
        int      fbLen[FBNUM];             /* allocated size */

        GLvoid  *fBuf[FBNUM];             /* currently visible */ 
        GLvoid  *dBuf[FBNUM];             /* transmitted differences */

} SBUFS;

/* 
     buffer layout for PLE Compression

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
        PLEdata data[0];
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
        int    client_id;   /* client identification index */
        int    verbose;     /* 0 = quiet; 1 = chatty */
        int    debug;       /* 0 = normal; 1 = debug */
        int    no_diff;     /* 0 = difference; 1 = suppress  */ 
        ZTYPE  ztype;       /* compression type - See ZStateType enum */
        int    ztype_parm;  /* parameter for ztype */
 
        /* instance data */

        /* raster positions */
        GLint   rXold, rYold;
        GLint   rXnew, rYnew;

        /* shadow buffers */

        SBUFS   b;            /* currently active buffer set */

        /* servers may have multipe shadow buffers */
        int     n_sb;        /* highest current index */
        SBUFS  *sb;

        /* clients have a work area buffer to compress in */
        int      zbLen[FBNUM];
        GLvoid  *zBuf[FBNUM];             


        /* statistics */
        long n;
        long sum_runs;
        double sum_bytes, sum_zbytes, sum_prefv;

} ZpixSPU;

extern ZpixSPU zpix_spu;

extern SPUNamedFunctionTable _cr_zpix_table[];

extern SPUOptions zpixSPUOptions[];

extern void zpixspuGatherConfiguration( ZpixSPU *zpix_spu );


#endif /* ZPIX_SPU_H */
