/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "tilesortspu.h"
#include "cr_bbox.h"
#include "cr_glstate.h"
#include "cr_pack.h"
#include "cr_mem.h"

#include "cr_applications.h"

#include <limits.h>
#include <float.h>

typedef struct BucketRegion *BucketRegion_ptr;
typedef struct BucketRegion {
	GLbitvalue       id[CR_MAX_BITARRAY];
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

void __fillBucketingHash (void) 
{
	int i, j, k, m;
	int r_len=0;
	BucketRegion *rlist;
	int id;

	/* Allocate rlist */
	rlist_alloc = 64*128;
	/* rlist_alloc = GLCONFIG_MAX_PROJECTORS*GLCONFIG_MAX_EXTENTS; */
	rlist = (BucketRegion *) crAlloc (rlist_alloc * sizeof (*rlist));

	for (i=0; i<HASHRANGE; i++) 
	{
		for (j=0; j<HASHRANGE; j++) 
		{
			rhash[i][j] = NULL;
		}
	}

	/* Fill hash table */
	id = 0;
	for (i=0; i< tilesort_spu.num_servers; i++)
	{
		int node32 = i >> 5;
		int node = i & 0x1f;
		for (j=0; j < tilesort_spu.servers[i].num_extents; j++) 
		{
			BucketRegion *r = &rlist[id++];
			for (k=0;k<CR_MAX_BITARRAY;k++)
				r->id[k] = 0;
			r->id[node32] = (1 << node);
			r->extents.x1 = tilesort_spu.servers[i].x1[j];
			r->extents.x2 = tilesort_spu.servers[i].x2[j];
			r->extents.y1 = tilesort_spu.servers[i].y1[j];
			r->extents.y2 = tilesort_spu.servers[i].y2[j];
			
			for (k=BKT_DOWNHASH(r->extents.x1, tilesort_spu.muralWidth);
				   k<=BKT_UPHASH(r->extents.x2, (int)tilesort_spu.muralWidth) && k < HASHRANGE;
				   k++) 
			{
				for (m=BKT_DOWNHASH(r->extents.y1, tilesort_spu.muralHeight);
				     m<=BKT_UPHASH(r->extents.y2, (int)tilesort_spu.muralHeight) && m < HASHRANGE;
					   m++) 
				{
					if ( rhash[m][k] == NULL ||
						   (rhash[m][k]->extents.x1 > r->extents.x1 &&
						    rhash[m][k]->extents.y1 > r->extents.y1)) 
					{
						 rhash[m][k] = r;
					}
				}
			}
		}
	}
	r_len = id;

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
			if (r==q) 
			{
				continue;
			}

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

static TileSortBucketInfo *__doBucket( void )
{
	GET_CONTEXT(g);
	static TileSortBucketInfo bucketInfo;
	CRTransformState *t = &(g->transform);
	GLmatrix *m = &(t->transform);

	const GLvectorf zero_vect = {0.0f, 0.0f, 0.0f, 1.0f};
	const GLvectorf one_vect = {1.0f, 1.0f, 1.0f, 1.0f};
	const GLvectorf neg_vect = {-1.0f, -1.0f, -1.0f, 1.0f};
	const GLrecti fullscreen = {-GL_MAXINT, GL_MAXINT, -GL_MAXINT, GL_MAXINT};
	const GLrecti nullscreen = {0, 0, 0, 0};
	float xmin, ymin, xmax, ymax, zmin, zmax;
	GLrecti ibounds;
	int i,j;

	GLbitvalue retval[CR_MAX_BITARRAY];

	/* Init bucketInfo */
	bucketInfo.objectMin = thread->packer->bounds_min;
	bucketInfo.objectMax = thread->packer->bounds_max;
	bucketInfo.screenMin = zero_vect;
	bucketInfo.screenMax = zero_vect;
	bucketInfo.pixelBounds = nullscreen;
	for (j=0;j<CR_MAX_BITARRAY;j++)
	     bucketInfo.hits[j] = 0;

	/* Check to make sure the transform is valid */
	if (!t->transformValid)
	{
		/* I'm pretty sure this is always the case, but I'll leave it. */
		crStateTransformUpdateTransform(t);
	}

	/* we might be broadcasting, *or* we might be 
	 * defining a display list, which goes to everyone 
	 * (currently) */

	if (tilesort_spu.broadcast || g->lists.newEnd)
	{
		bucketInfo.screenMin = neg_vect;
		bucketInfo.screenMax = one_vect;
		bucketInfo.pixelBounds = fullscreen;
		FILLDIRTY(bucketInfo.hits);
		return &bucketInfo;
	}

	xmin = bucketInfo.objectMin.x;
	ymin = bucketInfo.objectMin.y;
	zmin = bucketInfo.objectMin.z;
	xmax = bucketInfo.objectMax.x;
	ymax = bucketInfo.objectMax.y;
	zmax = bucketInfo.objectMax.z;

	if (tilesort_spu.providedBBOX != GL_SCREEN_BBOX_CR)
	{
		crTransformBBox( xmin, ymin, zmin, xmax, ymax, zmax, m,
		                 &xmin, &ymin, &zmin, &xmax, &ymax, &zmax );
	}

	/* Copy for export */
	bucketInfo.screenMin.x = xmin;
	bucketInfo.screenMin.y = ymin;
	bucketInfo.screenMin.z = zmin;
	bucketInfo.screenMax.x = xmax;
	bucketInfo.screenMax.y = ymax;
	bucketInfo.screenMax.z = zmax;

	/* triv reject */
	if (xmin > 1.0f || ymin > 1.0f || xmax < -1.0f || ymax < -1.0f) 
	{
		for (j=0;j<CR_MAX_BITARRAY;j++)
	     		bucketInfo.hits[j] = 0;
		return &bucketInfo;
	}

	/* clamp */
	if (xmin < -1.0f) xmin = -1.0f;
	if (ymin < -1.0f) ymin = -1.0f;
	if (xmax > 1.0f) xmax = 1.0f;
	if (ymax > 1.0f) ymax = 1.0f;

	ibounds.x1 = (int) (tilesort_spu.halfViewportWidth*xmin + tilesort_spu.viewportCenterX);
	ibounds.x2 = (int) (tilesort_spu.halfViewportWidth*xmax + tilesort_spu.viewportCenterX);
	ibounds.y1 = (int) (tilesort_spu.halfViewportHeight*ymin + tilesort_spu.viewportCenterY);
	ibounds.y2 = (int) (tilesort_spu.halfViewportHeight*ymax + tilesort_spu.viewportCenterY);

	bucketInfo.pixelBounds = ibounds;

	for (j=0;j<CR_MAX_BITARRAY;j++)
	     retval[j] = 0;

	if (!tilesort_spu.optimizeBucketing) 
	{
		for (i=0; i < tilesort_spu.num_servers; i++) 
		{
			int node32 = i >> 5;
			int node = i & 0x1f;
			for (j=0; j < tilesort_spu.servers[i].num_extents; j++) 
			{
				if (ibounds.x1 < tilesort_spu.servers[i].x2[j] && 
				    ibounds.x2 >= tilesort_spu.servers[i].x1[j] &&
				    ibounds.y1 < tilesort_spu.servers[i].y2[j] &&
				    ibounds.y2 >= tilesort_spu.servers[i].y1[j]) 
				{
					retval[node32] |= (1 << node);
					break;
				}
			}
		}
	} 
	else 
	{
		BucketRegion *r;
		BucketRegion *q;

		for (r = rhash[BKT_DOWNHASH(0, tilesort_spu.muralHeight)][BKT_DOWNHASH(0, tilesort_spu.muralWidth)];
				 r && ibounds.y2 >= r->extents.y1;
				 r = r->up) 
		{
			for (q=r; q && ibounds.x2 >= q->extents.x1; q = q->right) 
			{
				if (CHECKDIRTY(retval, q->id)) 
				{
					continue;
				}
				if (ibounds.x1 < q->extents.x2 && ibounds.x2 >= q->extents.x1 &&
		 		    ibounds.y1 < q->extents.y2 && ibounds.y2 >= q->extents.y1) 
				{
					for (j=0;j<CR_MAX_BITARRAY;j++)
					     retval[j] |= q->id[j];
				}
			}
		}
	}

	crMemcpy((char*)bucketInfo.hits, (char*)retval,
				sizeof(GLbitvalue) * CR_MAX_BITARRAY);

	return &bucketInfo;
}

TileSortBucketInfo *tilesortspuBucketGeometry(void)
{
	/* First, call the real bucketer */
	TileSortBucketInfo *ret = __doBucket();

	/* Finally, do pinching.  This is unimplemented currently. */

	/* PINCHIT(); */

	return ret;
}

void tilesortspuSetBucketingBounds( int x, int y, unsigned int w, unsigned int h )
{
	tilesort_spu.halfViewportWidth = w/2.0f;
	tilesort_spu.halfViewportHeight = h/2.0f;
	tilesort_spu.viewportCenterX = x + tilesort_spu.halfViewportWidth;
	tilesort_spu.viewportCenterY = y + tilesort_spu.halfViewportHeight;
}

void tilesortspuBucketingInit( void )
{
	tilesortspuSetBucketingBounds( 0, 0, tilesort_spu.muralWidth, tilesort_spu.muralHeight );

	if (tilesort_spu.optimizeBucketing)
	{
		__fillBucketingHash();
	}
}
