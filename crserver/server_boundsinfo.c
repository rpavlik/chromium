/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "server_dispatch.h"
#include "server.h"
#include "cr_error.h"
#include "cr_unpack.h"
#include "cr_mem.h"
#include "state/cr_statetypes.h"

/* This code copied from the tilesorter (fooey) */

typedef struct BucketRegion *BucketRegion_ptr;
typedef struct BucketRegion {
	GLbitvalue       id;
	GLrecti          extents;
	BucketRegion_ptr right;
	BucketRegion_ptr up;
} BucketRegion;

#define HASHRANGE 256
BucketRegion *rhash[HASHRANGE][HASHRANGE];
BucketRegion *rlist;
int rlist_alloc = 0;

#define BKT_DOWNHASH(a, range) ((a)*HASHRANGE/(range))
#define BKT_UPHASH(a, range) ((a)*HASHRANGE/(range) + ((a)*HASHRANGE%(range)?1:0))

void crServerFillBucketingHash(void) 
{
    int i, j, k, m;
    int r_len=0;
    int xinc, yinc;

    BucketRegion *rlist;
    BucketRegion *rptr;

    /* Allocate rlist */
    rlist_alloc = 64*128;
    /*rlist_alloc = GLCONFIG_MAX_PROJECTORS*GLCONFIG_MAX_EXTENTS; */
    rlist = (BucketRegion *) crAlloc( rlist_alloc * sizeof(*rlist) );

    for ( i = 0; i < HASHRANGE; i++ )
    {
        for ( j = 0; j < HASHRANGE; j++ )
        {
            rhash[i][j] = NULL;
        }
    }

    /* Fill the rlist */
    xinc = cr_server.x2[0] - cr_server.x1[0];
    yinc = cr_server.y2[0] - cr_server.y1[0];

    rptr = rlist;
    for (i=0; i < (int) cr_server.muralWidth; i+=xinc) 
		{
        for (j=0; j < (int) cr_server.muralHeight; j+=yinc) 
				{
            for (k=0; k < cr_server.numExtents; k++) 
						{
                if (cr_server.x1[k] == i && cr_server.y1[k] == j) 
								{
                    rptr->extents.x1 = cr_server.x1[k];
                    rptr->extents.x2 = cr_server.x2[k];
                    rptr->extents.y1 = cr_server.y1[k];
                    rptr->extents.y2 = cr_server.y2[k];
                    rptr->id = k;
                    break;
                }
            }
            if (k == cr_server.numExtents) 
						{
                rptr->extents.x1 = i;
                rptr->extents.y1 = j;
                rptr->extents.x2 = i+xinc;
                rptr->extents.y2 = j+yinc;
                rptr->id = -1;
            }
            rptr++;
        }
    }
    r_len = rptr - rlist;

    /* Fill hash table */
    for ( i = 0; i < r_len; i++ )
    {
        BucketRegion *r = &rlist[i];

        for (k=BKT_DOWNHASH(r->extents.x1, (int)cr_server.muralWidth);
             k<=BKT_UPHASH(r->extents.x2, (int)cr_server.muralWidth) &&
                 k < HASHRANGE;
             k++) 
				{
            for (m=BKT_DOWNHASH(r->extents.y1, (int)cr_server.muralHeight);
                 m<=BKT_UPHASH(r->extents.y2, (int)cr_server.muralHeight) &&
                     m < HASHRANGE;
                 m++) 
						{
                if ( rhash[m][k] == NULL ||
                    (rhash[m][k]->extents.x1 > r->extents.x1 &&
                     rhash[m][k]->extents.y1 > r->extents.y1)) {
                     rhash[m][k] = r;
                }
            }
        }
    }

    /* Initialize links */
    for (i=0; i<r_len; i++) 
		{
        BucketRegion *r = &rlist[i];
        r->right = NULL;
        r->up    = NULL;
    }

    /* Build links */
    for (i=0; i<r_len; i++) 
		{
        BucketRegion *r = &rlist[i];
        for (j=0; j<r_len; j++) 
				{
            BucketRegion *q = &rlist[j];
            if (r==q) continue;

            /* Right Edge */
            if (r->extents.x2 == q->extents.x1 &&
                r->extents.y1 == q->extents.y1 &&
                r->extents.y2 == q->extents.y2) 
						{
                r->right = q;
            }

            /* Upper Edge */
            if (r->extents.y2 == q->extents.y1 &&
                r->extents.x1 == q->extents.x1 &&
                r->extents.x2 == q->extents.x2) 
						{
                r->up = q;
            }
        }
    }
}

void crServerSetOutputBounds( CRContext *ctx, 
												const GLrecti *outputwindow, 
												const GLrecti *imagespace, 
												const GLrecti *imagewindow )
{
	GLrecti p,q;

	crServerSetViewportBounds( &(ctx->viewport), outputwindow, imagespace, imagewindow, &p, &q );
	crServerRecomputeBaseProjection( &(cr_server.curClient->baseProjection), ctx->viewport.viewportX, ctx->viewport.viewportY, ctx->viewport.viewportW, ctx->viewport.viewportH );
	crServerApplyBaseProjection();
	
}

void SERVER_DISPATCH_APIENTRY
crServerDispatchBoundsInfo( GLrecti *bounds, GLbyte *payload, GLint len,
                            GLint num_opcodes )
{
	char *data_ptr = (char*)(payload + ((num_opcodes + 3 ) & ~0x03));
	int i;
	crUnpackPush();
	if ( cr_server.optimizeBucket )
	{
		BucketRegion *r;
		BucketRegion *p;

		for (r = rhash[BKT_DOWNHASH(bounds->y1, cr_server.muralHeight)][BKT_DOWNHASH(bounds->x1, cr_server.muralWidth)];
				r && bounds->y2 >= r->extents.y1;
				r = r->up)
		{
			for (p=r; p && bounds->x2 >= p->extents.x1; p = p->right)
			{
				if ( p->id != -1 &&
					bounds->x1 < p->extents.x2  &&
					bounds->y1 < p->extents.y2 &&
					bounds->y2 >= p->extents.y1 )
				{
					int c;
					cr_server.curExtent = p->id;
					for (c = 0; c < CR_MAX_CONTEXTS; c++)
					{
						if (run_queue->client->context[c])
						{
							crServerSetOutputBounds( run_queue->client->context[c],
													 &run_queue->extent[p->id].outputwindow,
													 &run_queue->imagespace,
													 &run_queue->extent[p->id].imagewindow );
						}
					}
					crUnpack( data_ptr, data_ptr-1, num_opcodes, &(cr_server.dispatch) );
				}
			}
		}
	} 
	else 
	{
		for ( i = 0; i < run_queue->numExtents; i++ )
		{
			CRRunQueueExtent *extent = &run_queue->extent[i];
			int c;

			if ( !( extent->imagewindow.x2 > bounds->x1 &&
				extent->imagewindow.x1 < bounds->x2 &&
				extent->imagewindow.y2 > bounds->y1 &&
				extent->imagewindow.y1 < bounds->y2 ) )
			{
				continue;
			}

			cr_server.curExtent = i;
			for (c = 0; c < CR_MAX_CONTEXTS; c++)
			{
				if (run_queue->client->context[c])
				{
					crServerSetOutputBounds( run_queue->client->context[c],
											 &extent->outputwindow,
											 &run_queue->imagespace,
											 &extent->imagewindow );
				}
			}

			crUnpack( data_ptr, data_ptr-1, num_opcodes, &(cr_server.dispatch) );
		}
	}
	crUnpackPop();
}
