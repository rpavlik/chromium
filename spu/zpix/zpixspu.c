/* Copyright (c) 2003, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <zlib.h>
#include <stdio.h>
#include "cr_mem.h"
#include "cr_pixeldata.h"
#include "cr_spu.h"
#include "zpixspu.h"

/*  Forward declarations */

void ZPIXSPU_APIENTRY zpixRasterPos2i( GLint x,  GLint y );

void ZPIXSPU_APIENTRY zpixBitmap( GLsizei width, 
				  GLsizei height,
				  GLfloat xorig,
				  GLfloat yorig,
				  GLfloat xmove,
				  GLfloat ymove,
				  const GLubyte *bitmap );

void ZPIXSPU_APIENTRY zpixDrawPixels( GLsizei  width, 
                                          GLsizei height, 
                                          GLenum format, 
                                          GLenum type, 
                                    const GLvoid *pixels );

void ZPIXSPU_APIENTRY zpixZPix( GLsizei width, 
                                GLsizei height, 
                                GLenum  format, 
                                GLenum  type, 
                                GLenum  ztype, 
                                GLint   zparm, 
                                GLint   length, 
                          const GLvoid  *pixels );
/*
  local functions and data
*/
static char  *FBname[3] = {"COLOR", "DEPTH", "STENCIL"};

static void  FriskPLE( PLEbuf *p_plebuf) 
{
             int       n, nrun, npref, r, runt;
             GLuint      *p_val;
             PLErun    *p_run;

             /* point again at first value and count */
             p_run    = (PLErun *) p_plebuf + p_plebuf->beg;
             p_val    = (GLuint *) p_run;
             n        = 0;
             nrun     = 0;
             npref    = 0;
             while (nrun < p_plebuf->nruns)
             {
                nrun++;
                r = *(p_run - nrun);
                runt = (r < 0) ? -1 : r != 0 ;   /* extract sign */
                switch (runt) {
                case -1:
                     n -= r;
                     CRASSERT(r >=  -128);
                     p_val -= r;
                     break;

                case 0:
                     r = *p_val;
                     CRASSERT(r > 0);
                     n += r;
                     npref += r;
                     p_val++;
                     break; 
                     
                case 1:
                     n += r;
                     CRASSERT(r < 128);
                     p_val ++;
                     break;
                }
            }   
             crDebug("Zpix Debug: call %ld, runs %d, prefs %d, zb %d",
                      zpix_spu.n, nrun, npref, p_plebuf->len);
             CRASSERT(nrun == p_plebuf->nruns);
             CRASSERT(n == p_plebuf->n);
             CRASSERT( 1 == *(p_run - nrun - 1));
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                         CLIENT SIDE

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*----------------------------------------------------------
  RasterPos2i:   Record x,y  from glRasterPos2i
----------------------------------------------------------*/
void ZPIXSPU_APIENTRY zpixRasterPos2i( GLint x, GLint y) 
{
        /*XXX make sure it's what we expect */
        CRASSERT(0 == x) ;
        CRASSERT(0 == y) ;

        /*XXX remember and pass it through for now */
        zpix_spu.rXnew = x;
        zpix_spu.rYnew = y;
	zpix_spu.child.RasterPos2i( x, y );
}

/*----------------------------------------------------------
  MCH:
  Bitmap:   Record xmove, ymove from Bitmap
  This is do deal with the hacks in which we set
  the raster position with glBitmap instead of glRasterPos2i.
  This makes me a little ill...
----------------------------------------------------------*/
/*void ZPIXSPU_APIENTRY zpixBitmap( GLsizei width, 
				  GLsizei height,
				  GLfloat xorig,
				  GLfloat yorig,
				  GLfloat xmove,
				  GLfloat ymove,
				  const GLubyte *bitmap )
{
     if(bitmap == NULL){
	  zpix_spu.rXnew = (int)xmove;
	  zpix_spu.rYnew = (int)ymove;
     }
     zpix_spu.child.Bitmap( width, height, xorig, yorig, xmove, ymove, bitmap );
}*/

/*----------------------------------------------------------
  zpixDrawPixels:  Compress the data from glDrawPixels
------------------------------------------------------------*/       

void ZPIXSPU_APIENTRY zpixDrawPixels( GLsizei width, 
                                          GLsizei height, 
                                          GLenum format, 
                                          GLenum type, 
                                    const GLvoid *pixels )
{
        int       bufi = 0;
        int       bufw;
        GLuint     *p_old;
        GLuint     *p_new;
        GLuint     *p_dif;
        int       pixsize;
        GLuint      alen, plen;
        int       r, rc;
        ulong     zliblen;
        int       zlen;
        ZTYPE     ztype;
        FBTYPE    FBtype;
        GLint     zclient;

        PLEbuf    *p_plebuf, pletmp;
  const GLuint      prefv = 0;
        GLuint      *p_pix, *p_val, v_curr, v_peek;
        PLErun    *p_run, *p_left, *p_data;

        zpix_spu.n++;
        ztype     = zpix_spu.ztype;
        zclient   = zpix_spu.client_id;
        pixsize   = crPixelSize(format, type);
        plen      =  pixsize * width * height;
        if (1 == zpix_spu.verbose)
        {
          crDebug("zpixDrawPixels: client %d  %d x %d, format %x, type %d, plen %d ", 
                                   zclient, width,height,format,type,plen);
        } 
        /*  
             Set buffer type index
        */
        switch (format) {
        case GL_STENCIL_INDEX:
             FBtype = FBSTENCIL;
             break;
        case GL_DEPTH_COMPONENT:
             FBtype = FBDEPTH;
             break;
         default:
             FBtype = FBCOLOR;
         }


        if (zpix_spu.rXold != zpix_spu.rXnew ||
            zpix_spu.rYold != zpix_spu.rYnew ||
            zpix_spu.b.fbWidth[FBtype] != width ||
            zpix_spu.b.fbHeight[FBtype] != height ||
            zpix_spu.b.fbLen[FBtype] < (int) plen )
        {
        /* new or changed frame buffer attributes */

        /* free any old buffers */
        if (zpix_spu.b.fBuf[FBtype] ) crFree(zpix_spu.b.fBuf[FBtype]);
        if (zpix_spu.b.dBuf[FBtype] ) crFree(zpix_spu.b.dBuf[FBtype]);
        /* set up new buffers */
        zpix_spu.rXold = zpix_spu.rXnew;
        zpix_spu.rYold = zpix_spu.rYnew;

        zpix_spu.b.fbWidth[FBtype] = width;
        zpix_spu.b.fbHeight[FBtype] = height;

        alen = (plen + 7)& -sizeof(GLuint);     /* trim size up to doubleword */
        zpix_spu.b.fbLen[FBtype] = alen;
        zpix_spu.b.fBuf[FBtype] = crAlloc(alen);
        crMemZero(zpix_spu.b.fBuf[FBtype],alen);
        zpix_spu.b.dBuf[FBtype] = crAlloc(alen);

/*
     The compress buffer must be big enough 
     for the worst case of any buffer's compress algorithm
     rounded to word boundaries

     zlib = 12 + 1.01*plen
     ple  = 2 + sizeof(PLEbuf) + 1.25*plen

    so ple is the biggest
*/
        zlen = 2 + sizeof(PLEbuf) + plen + ((plen+3) >> 2);
        zlen = (zlen-1+sizeof(GLuint)) & -sizeof(GLuint);
        if (zpix_spu.zbLen[FBtype] < zlen)
          {
           if (zpix_spu.zBuf[FBtype]) crFree(zpix_spu.zBuf[FBtype]);
           zpix_spu.zbLen[FBtype] = zlen;
           zpix_spu.zBuf[FBtype] = crAlloc(zlen);
          }

        crDebug("zpixDrawPixels: client %d sb %s at  %d, %d dimension %d x %d", 
                                      zpix_spu.client_id,
                                      FBname[FBtype], 
                                      zpix_spu.rXnew, 
                                      zpix_spu.rYnew, 
                                      width, height);
        crDebug("zpixDrawPixels: ztype %d, format %x, plen %d, zlen %d", 
                                      zpix_spu.ztype, 
                                      format, 
                                      plen, zlen);
        crDebug("zpixDrawPixels: %d bytes f @ %p, d @ %p, z %d bytes @ %p", 
                                      plen, 
                                      zpix_spu.b.fBuf[FBtype],
                                      zpix_spu.b.dBuf[FBtype],
                                      zlen,
                                      zpix_spu.zBuf[FBtype]
                                      );

        
       }
      
      /*  Set buffer manipulation values */

      bufw  = (plen + 3) / sizeof(GLuint);
      p_old = zpix_spu.b.fBuf[FBtype];
      p_new = (GLuint *) pixels;
      p_dif = zpix_spu.b.dBuf[FBtype];

      /* initialize compressed length to max buffer len available */
      zlen = zpix_spu.zbLen[FBtype];

      if (1 == zpix_spu.no_diff)  
      {
        /*********************************************************
          Frames only compressed - not differenced
               probably many clients feeding one server
        **********************************************************/

         /*XXX copies could be avoided - allows differencing later */
         crMemcpy(p_dif,p_new,plen);
         crMemcpy(p_old,p_new,plen);
      }
      else  /* compute difference of new and old buffer */
      {
       
       /********************************************************
          Create difference buffer by XOR of pixels with old pixels 
           and replace the old pixels with the new ones
        ********************************************************/
  
        for ( bufi = 0; bufi < bufw ; bufi++ )
          {
           GLuint   xpix;
           xpix = (*(p_new + bufi)) ^ *(p_old + bufi) ;
           *(p_dif + bufi) = xpix;
           *(p_old + bufi) = *(p_new + bufi);
          }
       } 
       /********************************************************
          Compress the difference buffer for transmission
        ********************************************************/

        switch (ztype) {

        case ZNONE: /* no compression - no use except debugging */
                                 
             /* update statistics */
             zpix_spu.sum_bytes += plen;
             zpix_spu.sum_zbytes += plen;
       
             zpix_spu.child.ZPix( width,
                                  height,
                                  format,
                                  type,
                                  ztype,
                                  zclient,
                                  plen,
                                  p_dif);
             break;


        case ZLIB:  /* use gnu zlib compression */

             zliblen = (ulong) zlen;
             rc = compress2(zpix_spu.zBuf[FBtype], &zliblen,
                            zpix_spu.b.dBuf[FBtype],
                            plen, zpix_spu.ztype_parm);

             if (Z_OK != rc ) 
                 crError("zpixDrawpixels: zlib compress2 rc = %d", rc);

             /* zlen now has actual size of compressed data */
             zlen = (int) zliblen;
             if (1 == zpix_spu.verbose)
             {
               crDebug("zpixDrawPixels: bufi %d, fb_end %p", bufi, p_old+bufi ); 
               crDebug("zpixDrawPixels: dBuf %p - %p", p_dif, p_dif+bufi ); 
               crDebug("zpixDrawPixels: plen = %u zlen = %d zused = %d ", 
                                   plen, zpix_spu.zbLen[FBtype], zlen);
             }
                                 
             /* update statistics */
             zpix_spu.sum_bytes += plen;
             zpix_spu.sum_zbytes += zlen;
       
             zpix_spu.child.ZPix( width,
                                  height,
                                  format,
                                  type,
                                  ztype,
                                  zclient,
                                  zlen,
                                  zpix_spu.zBuf[FBtype]);
             break;


        case ZRLE:  /* classic run length encoding */

             crError("Zpix - RLE unimplemented  ztype = %d",ztype);
             break;


        case ZPLE: 

             p_plebuf = (PLEbuf *) zpix_spu.zBuf[FBtype];
             p_plebuf->len     = zlen;
             p_plebuf->n       = bufw;
             p_plebuf->beg     = (bufw + sizeof(PLEbuf) + 3) & -sizeof(GLuint) ;
             p_plebuf->prefval = prefv;
             p_plebuf->nruns   = 0;

             pletmp = *p_plebuf;

             /* point at first value and count */
             p_run    = (PLErun *) p_plebuf + p_plebuf->beg;
             p_data  = p_run;
             p_val    = (GLuint *) p_data;
             p_run-- ;
             p_pix    = zpix_spu.b.dBuf[FBtype];

           
             if (1 == zpix_spu.verbose)
                 crDebug("bufw %d, p_plebuf %p, p_val %p", bufw, p_plebuf, p_val
                 ) ;

             /*
                     Create the runs
             */
             bufi = 0;
             while ( bufi < bufw ) 
             {  
               int rt;                       /* debugging: run type */
               p_plebuf->nruns++;            /*XXXX debugging run counts */
               zpix_spu.sum_runs++;
               r = -1;                       /* in case last input word */
               v_curr = *(p_pix + bufi);
               bufi++; 
               *p_val++ = v_curr;

               /* 
                      Create a run of uniques, matches, or preferred values
               */
               if ( bufi < bufw )          
               {  
                 v_peek = *(p_pix + bufi);  

                 if (v_curr != v_peek) 
                 /* 
                    run of unique values (count is negative)
                 */
                 {
                    rt = -1;
                    r = -2;                    /* run so far */
                    v_curr = v_peek;
                    *p_val++ = v_curr;
                    bufi++; 
                    while ( bufi < bufw && r > -128 && v_curr != (v_peek = *(p_pix + bufi))  )
                    {
                      r--;
                      v_curr = v_peek;
                      *p_val++ = v_curr;      /* set value in sink */
                      bufi++;
                    }
                   /* if ended because of matched run, back up one */
                   if (bufi < bufw && v_curr == v_peek)
                   {
                      p_val--;              /* undo last store */
                      bufi--;                /* to start a matched run */
                      r++;                   /* adjust run count */
                      CRASSERT(r < 0);
                   } 
                 }
                 else                       
                 {
                 /* 
                    run of matched values (count is positive)
                 */
                   rt = 1;
                   r = 2;
                   v_curr = *(p_pix + bufi); 
                   bufi++;
                   if (v_curr != prefv )   /* ordinary matches */
                   {
                     while ( bufi < bufw && r < 127 && v_curr == *(p_pix + bufi) )
                     {
                       r++;
                       bufi++;
                     }
                   }
                   else                    
                   {
                   /* 
                       run of preferred values (count is saved in value)
                   */
                     rt = 0 ;
                     v_curr = r;           /* real run count variable */
                     r = 0;                /* set special count flag  */
                     while ( bufi < bufw && prefv == *(p_pix + bufi) )
                     {
                       v_curr++;          /* increment real run count */
                       bufi++;
                     }
                     zpix_spu.sum_prefv += v_curr;
                     *(p_val - 1) = v_curr;  /* store count over value */
                   }
                 }
               }  

               /* end of run - store length and decrement pointer */
               CRASSERT(-128 <= r && r <= 127);
               CRASSERT(rt ==  (r < 0) ? -1 : r != 0);   /* extract sign */
               *p_run-- = (PLErun) r ;
              }

              /* end of packing */
              *p_run = 1;                /*XXX DEBUG mark end with
                                                     impossible count */

             /* check for buffer underrun */

             /* round down left edge to word boundary */
             /*  the following atrocity makes IRIX happy */ 
             *((GLuint *) &p_run) &=  (GLuint) -sizeof (GLuint); 

             CRASSERT( (GLuint) p_run >=  (GLuint) &(p_plebuf->data) );
             p_left = p_run - sizeof(PLEbuf);
             
             /* scoot header up against run lengths */
             pletmp = *p_plebuf;
             p_plebuf = (PLEbuf *) p_left;
         
             pletmp.beg = (GLuint) p_data - (GLuint) p_plebuf;
             
             zlen = (int) p_val - (int) p_plebuf;
             pletmp.len = zlen;

             *p_plebuf = pletmp;

             CRASSERT(zlen <= zpix_spu.zbLen[FBtype] ) ;

             /*XXX JAG DEBUG see if packing is OK */ 
             if (1 == zpix_spu.debug) FriskPLE(p_plebuf);
             if (1 == zpix_spu.verbose)
             {
                crDebug("zpixDrawPixels: bufi %d, fb_end %p", bufi, p_old+bufi ); 
                crDebug("zpixDrawPixels: dBuf %p - %p", p_dif, p_dif+bufi ); 
                crDebug("zpixDrawPixels: plen = %u zlen = %d zused = %d ", 
                                   plen, zpix_spu.zbLen[FBtype], zlen);
             }
                                 
             /* update statistics */
             zpix_spu.sum_bytes += plen;
             zpix_spu.sum_zbytes += zlen;
       
             zpix_spu.child.ZPix( width,
                                  height,
                                  format,
                                  type,
                                  ztype,
                                  zclient,
                                  zlen,
                                  p_plebuf);
             break;
 
        default:
            crError("Zpix - invalid or unimplemented  ztype = %d",ztype);
        }

}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                         SERVER SIDE

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*--------------------------------------------------------
      Decompress data and then glDrawPixels 
----------------------------------------------------------*/       
void ZPIXSPU_APIENTRY zpixZPix( GLsizei width, 
                                GLsizei height, 
                                GLenum  format, 
                                GLenum  type, 
                                GLenum  ztype, 
                                GLint   zclient, 
                                GLint   zlen, 
                                const GLvoid  *zpixels )
{
        ulong   zldlen, zliblen;
        GLuint    alen, plen;
        GLuint    *p_fb;
        GLuint    *p_dif;
        unsigned int     bufi = 0;
        int     dlen;
        int     pixsize;
        FBTYPE  FBtype;           /* frame buffer type */

        zpix_spu.n++;

        if (1 == zpix_spu.verbose)
        {
           crDebug("zpixZPix: client %d call %ld, format %x, ztype  %d zlen  %d ", 
                              zclient, zpix_spu.n, format, ztype, zlen);
        }

        /*  
             Set buffer type index
        */
        switch (format) {
        case GL_STENCIL_INDEX:
             FBtype = FBSTENCIL;
             break;
        case GL_DEPTH_COMPONENT:
             FBtype = FBDEPTH;
             break;
         default:
             FBtype = FBCOLOR;
         }

        pixsize = crPixelSize(format, type);
        plen =  pixsize * width * height;
        zpix_spu.ztype = (ZTYPE)ztype; 

        /* Select client shadow buffers */
        
        CRASSERT(zclient >= 0);

        if ( zclient > zpix_spu.n_sb )
        {
          
          SBUFS *osb = zpix_spu.sb ;         /* retrieve current list */
          int   osbn = zpix_spu.n_sb ;
          int   i = 0;

          /* reallocate shadow buffer pointer vector */
          crDebug("ZPix: realloc shadow pointers - max client %d", zclient);

          zpix_spu.sb = (SBUFS *) crAlloc((zclient+1)*sizeof(SBUFS));
          zpix_spu.n_sb = zclient;

          /* set up a temporary template shadow buffer to copy */

          for (i = 0; i < FBNUM; i++)
          {
            zpix_spu.b.fbWidth[i] = -1;
            zpix_spu.b.fbHeight[i] = -1;
            zpix_spu.b.fbLen[i] = 0;
            zpix_spu.b.fBuf[i] = NULL;
            zpix_spu.b.dBuf[i] = NULL;
           }

           /* copy existing entries */
           for (i = 0; i < osbn+1 ; i++)
           {
  	    zpix_spu.sb[i] = *(osb + i); 
           }

           /* template the new entries */
           for (i = osbn+1; i < zpix_spu.n_sb+1 ; i++)
           {
            zpix_spu.sb[i] = zpix_spu.b;       /* copy template */
           }
  
         crFree(osb);                         /* out with the old */
        }
        
        /*  set shadow buffers to current client */
        zpix_spu.b = zpix_spu.sb[zclient];

        if (zpix_spu.rXold != zpix_spu.rXnew ||
            zpix_spu.rYold != zpix_spu.rYnew ||
            zpix_spu.b.fbWidth[FBtype] != width ||
            zpix_spu.b.fbHeight[FBtype] != height ||
            zpix_spu.b.fbLen[FBtype] < (int) plen )
        {
        /* new or changed frame buffer attributes */

        /* free any old buffers */
          if (zpix_spu.b.fBuf[FBtype] ) crFree(zpix_spu.b.fBuf[FBtype]);
          if (zpix_spu.b.dBuf[FBtype] ) crFree(zpix_spu.b.dBuf[FBtype]);
        /* set up new buffers */
          zpix_spu.rXold = zpix_spu.rXnew;
          zpix_spu.rYold = zpix_spu.rYnew;

          zpix_spu.b.fbWidth[FBtype] = width;
          zpix_spu.b.fbHeight[FBtype] = height;

          alen = (plen + 7)& -sizeof(GLuint);     /* trim size up to doubleword */
          zpix_spu.b.fbLen[FBtype] = alen;

          zpix_spu.b.fBuf[FBtype] = crAlloc(alen);
          crMemZero(zpix_spu.b.fBuf[FBtype],alen);
          zpix_spu.b.dBuf[FBtype] = crAlloc(alen);

          crDebug("zpixZPix: client %d sb %s at %d  %d,  %d x %d, %d", 
                                      zclient, 
                                      FBname[FBtype], 
                                      zpix_spu.rXnew, 
                                      zpix_spu.rYnew, 
                                      width, height, plen);
          if (1 == zpix_spu.verbose)
          { 
            crDebug("zpixZPix: fBuf %p-%p, dBuf %p-%p", 
                                      zpix_spu.b.fBuf[FBtype],
                                      (char *) zpix_spu.b.fBuf[FBtype]+alen, 
                                      zpix_spu.b.dBuf[FBtype], 
                                      (char *) zpix_spu.b.dBuf[FBtype]+alen
                                      );
        
          }
        }              /* end buffer (re)allocation */
       
       /* remember shadow buffers for this client */

       zpix_spu.sb[zclient] = zpix_spu.b;

       dlen = zpix_spu.b.fbLen[FBtype];     /* available space */

       switch (ztype) {

            int    n, rc, run, runt;
            int    nrun;                /*XXX JAG debugging */

            PLEbuf *p_plebuf;
            GLuint   *p_val, prefv, val;
            PLErun *p_run;

       case ZNONE:
            /* no compression - only useful for debugging */
            crMemcpy(zpix_spu.b.dBuf[FBtype],zpixels,zlen);
            break;


       case ZLIB:
           /* Decompress and then DrawPixels */

            zliblen = (ulong) zlen;
            zldlen  = (ulong) dlen;

            rc = uncompress(zpix_spu.b.dBuf[FBtype], &zldlen, zpixels, zliblen);

            if (Z_OK != rc )
               crError("zpixZPix: zlib uncompress failure, rc = %d", rc);
            break;

       case ZRLE:  /* classic run length encoding */

            crError("Zpix - RLE unimplemented  ztype = %d",ztype);
            break;

       case ZPLE:
           /* Decode runs and then DrawPixels */
           p_plebuf = (PLEbuf *) zpixels;
           CRASSERT(p_plebuf->len == zlen) ;
           
           if (1 == zpix_spu.debug) FriskPLE(p_plebuf);

           nrun = 0;
           n    = p_plebuf->n;  
           dlen =  n * sizeof(GLuint);
           CRASSERT(zpix_spu.b.fbLen[FBtype] >= (int) (n*sizeof(GLuint))) ;

           prefv = p_plebuf->prefval;

           /* point at first value and count */
           p_run    = (PLErun *) p_plebuf + p_plebuf->beg;
           p_val    = (GLuint *) p_run;
           p_run-- ;
           /* output buffer */
           p_dif = zpix_spu.b.dBuf[FBtype];

           /* Now march left and right decoding runs */
           while (n > 0 )
           {
             nrun++;
             run  = *p_run-- ;
             zpix_spu.sum_runs++;
             runt = (run < 0) ? -1 : run != 0 ;   /* extract sign */

             switch (runt) {
                 
             case -1:   /* negative runs are unique values */
                  CRASSERT(run >= -128 );
                  n += run;                         /* reduce global count */
                  while (run < 0 )
                  {
                  val = *p_val++;
                  *p_dif = val;
                  p_dif++;
                  run++;
                  }
                  break;

             case 0:    /* zero  runs are repeats of preferred values */
                  run = (int) *p_val++;            /* count instead of data */
                  CRASSERT(run >0);
                  CRASSERT(run <= n);
                  n -= run;
                  zpix_spu.sum_prefv += run;
                  while (run > 0 )
                  {
                  *p_dif = prefv;
                  run--;
                  p_dif++;
                  }
                  break;

             case 1:    /* positive runs are repeats of values */
                  CRASSERT(run < 128 );
                  n -= run;                         /* reduce global count */
                  val = *p_val++;                   /* fetched repeated val */
                  while (run > 0 )
                  {
                  *p_dif = val;
                  run--;
                  p_dif++;
                  }
                  break;
                  
             }
             
           }
             CRASSERT( nrun == p_plebuf->nruns );
             CRASSERT( n == 0 );
                 
             /* Check left and right edges */
             CRASSERT(sizeof(PLEbuf) <= (GLuint) p_run - (GLuint) p_plebuf );


/*
             CRASSERT(zlen >= (GLuint) p_val - (GLuint) p_plebuf );
*/
            break;

       default:
            crError("Zpix - invalid or unimplemented ztype = %d",ztype);
       }

       /* update statistics */
       zpix_spu.sum_bytes += dlen;
       zpix_spu.sum_zbytes += zlen;
       
/*****************************************************************
       Buffer is decompressed - continue based on differencing
******************************************************************/

      if (1 == zpix_spu.no_diff)  
      {
        /*********************************************************
          Frames only compressed - not differenced
               probably many clients feeding one server
        **********************************************************/
         /* XXX copy could be avoided - allows differencing later */
         crMemcpy(zpix_spu.b.fBuf[FBtype],zpix_spu.b.dBuf[FBtype],plen);
      }
      else
      {
         /********************************************************
              XOR decompressed difference pixels with old pixels
            ********************************************************/
            p_fb = zpix_spu.b.fBuf[FBtype];
            p_dif = zpix_spu.b.dBuf[FBtype];

            for ( bufi = 0; bufi < (plen / sizeof(GLuint)) ; bufi++ )
            {
							GLuint xpix;
							xpix =  *(p_dif + bufi) ^ *(p_fb + bufi) ;
							*(p_fb + bufi) = xpix ;
            }
       }

       zpix_spu.child.DrawPixels( width,
                                  height,
                                  format,
                                  type,
                                  zpix_spu.b.fBuf[FBtype]); 
}


SPUNamedFunctionTable _cr_zpix_table[] = {
  { "RasterPos2i",    (SPUGenericFunction) zpixRasterPos2i },
  { "DrawPixels", (SPUGenericFunction) zpixDrawPixels },
  /*{ "Bitmap", (SPUGenericFunction) zpixBitmap },*/
  { "ZPix", (SPUGenericFunction) zpixZPix },
  { NULL, NULL }
}; 
