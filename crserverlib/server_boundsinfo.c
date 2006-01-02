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
	CRbitvalue       id;
	CRrecti          extents;
	BucketRegion_ptr right;
	BucketRegion_ptr up;
} BucketRegion;

#define HASHRANGE 256

#define BKT_DOWNHASH(a, range) ((a)*HASHRANGE/(range))
#define BKT_UPHASH(a, range) ((a)*HASHRANGE/(range) + ((a)*HASHRANGE%(range)?1:0))

struct BucketingInfo {
	BucketRegion *rhash[HASHRANGE][HASHRANGE];
	BucketRegion *rlist;
};


/*
 * At this point we know that the tiles are uniformly sized so we can use
 * a hash-based bucketing method.  Setup the hash table now.
 */
static GLboolean
fillBucketingHash(CRMuralInfo *mural)
{
	int i, j, k, m;
	int r_len = 0;
	int xinc, yinc;
	int rlist_alloc = 64 * 128;
	BucketRegion *rptr;
	struct BucketingInfo *bucketInfo;

	if (mural->bucketInfo) {
		crFree(mural->bucketInfo->rlist);
		crFree(mural->bucketInfo);
		mural->bucketInfo = NULL;
	}

	bucketInfo = (struct BucketingInfo *) crCalloc(sizeof(struct BucketingInfo));
	if (!bucketInfo)
		return GL_FALSE;

	/* Allocate rlist (don't free it!!!) */
	bucketInfo->rlist = (BucketRegion *) crAlloc(rlist_alloc * sizeof(BucketRegion));

	for ( i = 0; i < HASHRANGE; i++ )
	{
		for ( j = 0; j < HASHRANGE; j++ )
		{
			bucketInfo->rhash[i][j] = NULL;
		}
	}

	/* Fill the rlist */
	xinc = mural->extents[0].imagewindow.x2 - mural->extents[0].imagewindow.x1;
	yinc = mural->extents[0].imagewindow.y2 - mural->extents[0].imagewindow.y1;
	CRASSERT(xinc > 0);
	CRASSERT(yinc > 0);

	rptr = bucketInfo->rlist;
	for (i=0; i < (int) mural->width; i+=xinc) 
	{
		for (j=0; j < (int) mural->height; j+=yinc) 
		{
			for (k=0; k < mural->numExtents; k++) 
			{
				if (mural->extents[k].imagewindow.x1 == i &&
						mural->extents[k].imagewindow.y1 == j) 
				{
					rptr->extents = mural->extents[k].imagewindow; /* x1,y1,x2,y2 */
					rptr->id = k;
					break;
				}
			}
			if (k == mural->numExtents) 
			{
				rptr->extents.x1 = i;
				rptr->extents.y1 = j;
				rptr->extents.x2 = i + xinc;
				rptr->extents.y2 = j + yinc;
				rptr->id = -1;
			}
			rptr++;
		}
	}
	r_len = rptr - bucketInfo->rlist;

	/* Fill hash table */
	for (i = 0; i < r_len; i++)
	{
		BucketRegion *r = &bucketInfo->rlist[i];

		for (k=BKT_DOWNHASH(r->extents.x1, (int)mural->width);
		     k<=BKT_UPHASH(r->extents.x2, (int)mural->width) &&
			     k < HASHRANGE;
		     k++) 
		{
			for (m=BKT_DOWNHASH(r->extents.y1, (int)mural->height);
			     m<=BKT_UPHASH(r->extents.y2, (int)mural->height) &&
				     m < HASHRANGE;
			     m++) 
			{
				if ( bucketInfo->rhash[m][k] == NULL ||
				     (bucketInfo->rhash[m][k]->extents.x1 > r->extents.x1 &&
				      bucketInfo->rhash[m][k]->extents.y1 > r->extents.y1))
				{
					bucketInfo->rhash[m][k] = r;
				}
			}
		}
	}

	/* Initialize links */
	for (i=0; i<r_len; i++) 
	{
		BucketRegion *r = &bucketInfo->rlist[i];
		r->right = NULL;
		r->up    = NULL;
	}

	/* Build links */
	for (i=0; i<r_len; i++) 
	{
		BucketRegion *r = &bucketInfo->rlist[i];
		for (j=0; j<r_len; j++) 
		{
			BucketRegion *q = &bucketInfo->rlist[j];
			if (r==q)
				continue;

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

	mural->bucketInfo = bucketInfo;
	return GL_TRUE;
}


/*
 * Check if the tiles are the same size.  If so, initialize hash-based
 * bucketing.
 */
GLboolean
crServerInitializeBucketing(CRMuralInfo *mural)
{
	int optTileWidth = 0, optTileHeight = 0;
	int i;

	for (i = 0; i < mural->numExtents; i++)
	{
		const int w = mural->extents[i].imagewindow.x2 -
			mural->extents[i].imagewindow.x1;
		const int h = mural->extents[i].imagewindow.y2 -
			mural->extents[i].imagewindow.y1;

		if (optTileWidth == 0 && optTileHeight == 0) {
			/* First tile */
			optTileWidth = w;
			optTileHeight = h;
		}
		else
		{
			/* Subsequent tile - make sure it's the same size as first and
			 * falls on the expected x/y location.
			 */
			if (w != optTileWidth || h != optTileHeight) {
				crWarning("Tile %d, %d .. %d, %d is not the right size!",
									mural->extents[i].imagewindow.x1, mural->extents[i].imagewindow.y1,
									mural->extents[i].imagewindow.x2, mural->extents[i].imagewindow.y2);
				crWarning("All tiles must be same size with optimize_bucket.");
				crWarning("Turning off optimize_bucket for this mural.");
				return GL_FALSE;
			}
			else if ((mural->extents[i].imagewindow.x1 % optTileWidth) != 0 ||
							 (mural->extents[i].imagewindow.x2 % optTileWidth) != 0 ||
							 (mural->extents[i].imagewindow.y1 % optTileHeight) != 0 ||
							 (mural->extents[i].imagewindow.y2 % optTileHeight) != 0)
			{
				crWarning("Tile %d, %d .. %d, %d is not positioned correctly "
									"to use optimize_bucket.",
									mural->extents[i].imagewindow.x1, mural->extents[i].imagewindow.y1,
									mural->extents[i].imagewindow.x2, mural->extents[i].imagewindow.y2);
				crWarning("Turning off optimize_bucket for this mural.");
				return GL_FALSE;
			}
		}
	}

	return fillBucketingHash(mural);
}


void SERVER_DISPATCH_APIENTRY
crServerDispatchBoundsInfoCR( const CRrecti *bounds, const GLbyte *payload,
															GLint len, GLint num_opcodes )
{
	CRMuralInfo *mural = cr_server.curClient->currentMural;
	char *data_ptr = (char*)(payload + ((num_opcodes + 3 ) & ~0x03));
	unsigned int bx, by;

	crUnpackPush();

	bx = BKT_DOWNHASH(bounds->x1, mural->width);
	by = BKT_DOWNHASH(bounds->y1, mural->height);

	if (!mural->viewportValidated) {
		crServerComputeViewportBounds(&(cr_server.curClient->currentCtx->viewport),
																	mural);
	}

	/* Check for out of bounds, and optimizebucket to enable */
	if (mural->optimizeBucket && (bx <= HASHRANGE) && (by <= HASHRANGE))
	{
		const struct BucketingInfo *bucketInfo = mural->bucketInfo;
		const BucketRegion *r;
		const BucketRegion *p;

		CRASSERT(bucketInfo);

		for (r = bucketInfo->rhash[by][bx]; r && bounds->y2 >= r->extents.y1;
		     r = r->up)
		{
			for (p=r; p && bounds->x2 >= p->extents.x1; p = p->right)
			{
				if ( p->id != (unsigned int) -1 &&
					bounds->x1 < p->extents.x2  &&
					bounds->y1 < p->extents.y2 &&
					bounds->y2 >= p->extents.y1 )
				{
					mural->curExtent = p->id;
					if (cr_server.run_queue->client->currentCtx) {
						crServerSetOutputBounds( mural, mural->curExtent );
					}
					crUnpack( data_ptr, data_ptr-1, num_opcodes, &(cr_server.dispatch) );
				}
			}
		}
	} 
	else 
	{
		/* non-optimized bucketing */
		int i;
		for ( i = 0; i < mural->numExtents; i++ )
		{
			CRExtent *extent = &mural->extents[i];

			if (cr_server.localTileSpec ||
			    (extent->imagewindow.x2 > bounds->x1 &&
					 extent->imagewindow.x1 < bounds->x2 &&
					 extent->imagewindow.y2 > bounds->y1 &&
					 extent->imagewindow.y1 < bounds->y2))
			{
				mural->curExtent = i;
				if (cr_server.run_queue->client->currentCtx) {
					crServerSetOutputBounds( mural, i );
				}
				crUnpack( data_ptr, data_ptr-1, num_opcodes, &(cr_server.dispatch) );
			}
		}
	}

	crUnpackPop();
}
