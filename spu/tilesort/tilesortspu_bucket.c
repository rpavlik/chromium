/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "tilesortspu.h"
#include "cr_bbox.h"
#include "cr_glstate.h"
#include "cr_pack.h"
#include "cr_packfunctions.h"
#include "cr_mem.h"
#include "cr_rand.h"
#include "cr_applications.h"

/*
 * Data structures for hash-based bucketing algorithm.
 */
typedef struct BucketRegion *BucketRegion_ptr;
typedef struct BucketRegion {
	GLbitvalue       id[CR_MAX_BITARRAY];
	GLrecti          extents;
	BucketRegion_ptr right;
	BucketRegion_ptr up;
} BucketRegion;

#define HASHRANGE 256
static BucketRegion *rhash[HASHRANGE][HASHRANGE];
/*static BucketRegion *rlist;*/

#define BKT_DOWNHASH(a, range) ((a)*HASHRANGE/(range))
#define BKT_UPHASH(a, range) ((a)*HASHRANGE/(range) + ((a)*HASHRANGE%(range)?1:0))


/*
 * Data structures for non-regular grid bucketing algorithm.
 * The idea is we have a 2-D grid of rows and columns but the
 * width and height of the columns and rows isn't uniform.
 * This'll often be the case with dynamic tile resizing.
 */
#define MAX_ROWS 128
#define MAX_COLS 128
typedef struct grid_info {
	int rows, columns;
	int rowY1[MAX_ROWS], rowY2[MAX_ROWS]; /* bounds of each row */
	int colX1[MAX_COLS], colX2[MAX_COLS]; /* bounds of each column */
	int server[MAX_ROWS][MAX_COLS];       /* the server for each rect */
} GridInfo;

static GridInfo Grid;  /* XXX move into tilesort_spu struct? */


float _min3f(float a, float b, float c)
{
	if ((a < b) && (a < c))
		return a;
	else
	if ((b < a) && (b < c))
		return b;
	else 
		return c;
}

float _max3f(float a, float b, float c)
{
	if ((a > b) && (a > c))
		return a;
	else
	if ((b > a) && (b > c))
		return b;
	else 
		return c;
}

float _min2f(float a, float b)
{
	if (a < b)
		return a;
	else
		return b;
}

float _max2f(float a, float b)
{
	if (a > b)
		return a;
	else
		return b;
}

float _fabs(float a)
{
	if (a < 0) return -a;

	return a;
}

/*==================================================
 * convex quad-box overlap test, based on the separating
 * axis therom. see Moller, JGT 6(1), 2001 for
 * derivation of the tri case.
 * 
 * return 0 ==> no overlap 
 *        1 ==> overlap
 */
int quad_overlap(float *quad, 
				float xmin, float ymin, float xmax, float ymax)
{
	int a;
	float v[4][2], f[4][2], c[2], h[2];
	float p0, p1, p2, r;
	
	/* find the center */
	c[0] = (xmin + xmax) * (GLfloat).5;
	c[1] = (ymin + ymax) * (GLfloat).5;

	h[0] = xmax - c[0];
	h[1] = ymax - c[1];
	
	/* translate everything to be at the origin */
	v[0][0] = quad[0] - c[0];
	v[0][1] = quad[1] - c[1];
	v[1][0] = quad[2] - c[0];
	v[1][1] = quad[3] - c[1];
	v[2][0] = quad[4] - c[0];
	v[2][1] = quad[5] - c[1];
	v[3][0] = quad[6] - c[0];
	v[3][1] = quad[7] - c[1];
	
	/* XXX: this should be pre-computed */
	for (a=0; a<2; a++)
	{		
		f[0][a] = v[1][a] - v[0][a];
		f[1][a] = v[2][a] - v[1][a];
		f[2][a] = v[3][a] - v[2][a];
		f[3][a] = v[0][a] - v[3][a];
	}
	
	/* now, test the x & y axes (e0 & e1) */
	if ((_max2f(_max2f(v[0][0], v[1][0]), _max2f(v[2][0], v[3][0])) < -h[0]) ||
		(_min2f(_min2f(v[0][0], v[1][0]), _min2f(v[2][0], v[3][0])) > h[0]))
		return 0;

	if ((_max2f(_max2f(v[0][1], v[1][1]), _max2f(v[2][1], v[3][1])) < -h[1]) ||
		(_min2f(_min2f(v[0][1], v[1][1]), _min2f(v[2][1], v[3][1])) > h[1]))
		return 0;
	
	/* a0* and a1* reduce to e2, so no bother */

	/* a20 = (-f0y, f0x, 0) */
	p0 = v[1][0]*v[0][1] - v[0][0]*v[1][1];
	p1 = v[2][1]*f[0][0] - v[2][0]*f[0][1];
	p2 = v[3][1]*f[0][0] - v[3][1]*f[0][1];
	
	r  = h[0]*_fabs(f[0][1]) + h[1]*_fabs(f[0][0]);
	if ((_min3f(p0, p1, p2) > r) || (_max3f(p0, p1, p2) < -r)) return 0;
		
	/* a21 = (-f1y, f1x, 0) */
	p0 = v[2][0]*v[1][1] - v[1][0]*v[2][1];
	p1 = v[0][1]*f[1][0] - v[0][0]*f[1][1];
	p2 = v[3][1]*f[1][0] - v[3][0]*f[1][1];
	
	r  = h[0]*_fabs(f[1][1]) + h[1]*_fabs(f[1][0]);
	if ((_min3f(p0, p1, p2) > r) || (_max3f(p0, p1, p2) < -r)) return 0;

	/* a22 = (-f2y, f2x, 0) */
	p0 = v[3][0]*v[2][1] - v[2][0]*v[3][1];
	p1 = v[0][1]*f[2][0] - v[0][0]*f[2][1];
	p2 = v[1][1]*f[2][0] - v[1][0]*f[2][1];
	
	r  = h[0]*_fabs(f[2][1]) + h[1]*_fabs(f[2][0]);
	if ((_min3f(p0, p1, p2) > r) || (_max3f(p0, p1, p2) < -r)) return 0;
	
	/* a23 = (-f3y, f3x, 0) */
	p0 = v[0][0]*v[3][1] - v[3][0]*v[0][1];
	p1 = v[1][1]*f[3][0] - v[1][0]*f[3][1];
	p2 = v[2][1]*f[3][0] - v[2][0]*f[3][1];
	
	r  = h[0]*_fabs(f[3][1]) + h[1]*_fabs(f[3][0]);
	if ((_min3f(p0, p1, p2) > r) || (_max3f(p0, p1, p2) < -r)) return 0;

	return 1;
}


/*
 * Initialize the hash table used for optimized bucketing.
 */
static void fillBucketingHash(void) 
{
	int i, j, k, m;
	int r_len=0;
	BucketRegion *rlist;
	int id;
	int rlist_alloc = 64 * 128;

	/* rlist_alloc = GLCONFIG_MAX_PROJECTORS*GLCONFIG_MAX_EXTENTS; */
	rlist = (BucketRegion *) crAlloc(rlist_alloc * sizeof (*rlist));

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
			r->extents = tilesort_spu.servers[i].extents[j]; /* x1,y1,x2,y2 */

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


/*
 * Initialize GridInfo data for the non-uniform grid case.
 * If this succeeds, we'll set bucketMode = NON_UNIFORM_GRID (but not here).
 * Return: GL_TRUE - the server's tiles form a non-uniform grid
 *         GL_FALSE - the tiles don't form a non-uniform grid - give it up.
 */
GLboolean tilesortspuInitGridBucketing(void)
{
	int i, j, k;

	CRASSERT(tilesort_spu.muralRows > 0);
	CRASSERT(tilesort_spu.muralColumns > 0);

	/* analyze all the server tiles to determine grid information */
	Grid.rows = 0;
	Grid.columns = 0;

	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		for (j = 0; j < tilesort_spu.servers[i].num_extents; j++) 
		{
			const GLrecti *extent = &(tilesort_spu.servers[i].extents[j]);
			GLboolean found;

			/* look for a row with same y0, y1 */
			found = GL_FALSE;
			for (k = 0; k < Grid.rows; k++) {
				if (Grid.rowY1[k] == extent->y1 &&
					Grid.rowY2[k] == extent->y2)
				{
					found = GL_TRUE;
					break;
				}
			}
			if (!found)
			{
				/* add new row (insertion sort) */
				k = Grid.rows;
				while (k > 0 && extent->y1 < Grid.rowY1[k - 1]) {
					/* move column entry up */
					Grid.rowY1[k] = Grid.rowY1[k - 1];
					Grid.rowY2[k] = Grid.rowY2[k - 1];
					k--;
				}
				Grid.rowY1[k] = extent->y1;
				Grid.rowY2[k] = extent->y2;
				Grid.rows++;
			}

			/* look for a col with same x0, x1 */
			found = GL_FALSE;
			for (k = 0; k < Grid.columns; k++) {
				if (Grid.colX1[k] == extent->x1 &&
					Grid.colX2[k] == extent->x2)
				{
					found = GL_TRUE;
					break;
				}
			}
			if (!found)
			{
				/* add new row (insertion sort) */
				k = Grid.columns;
				while (k > 0 && extent->x1 < Grid.colX1[k - 1]) {
					/* move column entry up */
					Grid.colX1[k] = Grid.colX1[k - 1];
					Grid.colX2[k] = Grid.colX2[k - 1];
					k--;
				}
				Grid.colX1[k] = extent->x1;
				Grid.colX2[k] = extent->x2;
				Grid.columns++;
			}
		}
	}

	/* mural coverage test */
	if (Grid.rowY1[0] != 0)
		return GL_FALSE;   /* first row must start at zero */
	if (Grid.rowY2[Grid.rows - 1] != (int) tilesort_spu.muralHeight)
		return GL_FALSE;   /* last row must and at mural height */
	if (Grid.colX1[0] != 0)
		return GL_FALSE;   /* first column must start at zero */
	if (Grid.colX2[Grid.columns - 1] != (int) tilesort_spu.muralWidth)
		return GL_FALSE;   /* last column must and at mural width */

	/* make sure the rows and columns adjoin without gaps or overlaps */
	for (k = 1; k < Grid.rows; k++)
		if (Grid.rowY2[k - 1] != Grid.rowY1[k])
			return GL_FALSE;  /* found gap/overlap between rows */
	for (k = 1; k < Grid.columns; k++)
		if (Grid.colX2[k - 1] != Grid.colX1[k])
			return GL_FALSE;  /* found gap/overlap between columns */

	/* init Grid.server[][] array to -1 */
	for (i = 0; i < Grid.rows; i++)
		for (j = 0; j < Grid.columns; j++)
			Grid.server[i][j] = -1;

	/* now assign the server number for each rect */
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		for (j = 0; j < tilesort_spu.servers[i].num_extents; j++) 
		{
			const GLrecti *extent = &(tilesort_spu.servers[i].extents[j]);
			int r, c;
			GLboolean found = GL_FALSE;
			for (r = 0; r < Grid.rows && !found; r++)
			{
				if (Grid.rowY1[r] == extent->y1 && Grid.rowY2[r] == extent->y2)
				{
					for (c = 0; c < Grid.columns; c++)
					{
						if (Grid.colX1[c] == extent->x1 && Grid.colX2[c] == extent->x2)
						{
							if (Grid.server[r][c] != -1)
							{
								/* another server already has this tile! */
								return GL_FALSE;
							}
							Grid.server[r][c] = i;
							found = GL_TRUE;
							break;
						}
					}
				}
			}
		}
	}

	/* make sure we have a server for each rect! */
	for (i = 0; i < Grid.rows; i++)
		for (j = 0; j < Grid.columns; j++)
			if (Grid.server[i][j] == -1)
				return GL_FALSE;  /* found a hole in the tiling! */

#if 0
	printf("rows:\n");
	for (i = 0; i < Grid.rows; i++)
		printf("  y: %d .. %d\n", Grid.rowY1[i], Grid.rowY2[i]);
	printf("cols:\n");
	for (i = 0; i < Grid.columns; i++)
		printf("  x: %d .. %d\n", Grid.colX1[i], Grid.colX2[i]);
#endif

	return GL_TRUE;  /* a good grid! */
}



/*
 * Compute bounding box/tile intersections.
 * Output:  bucketInfo - results of intersection tests
 */
static void doBucket( TileSortBucketInfo *bucketInfo )
{
	GET_CONTEXT(g);
	CRTransformState *t = &(g->transform);
	GLmatrix *m = &(t->transform);

	const GLvectorf zero_vect = {0.0f, 0.0f, 0.0f, 1.0f};
	const GLvectorf one_vect = {1.0f, 1.0f, 1.0f, 1.0f};
	const GLvectorf neg_vect = {-1.0f, -1.0f, -1.0f, 1.0f};
	const GLrecti fullscreen = {-GL_MAXINT, GL_MAXINT, -GL_MAXINT, GL_MAXINT};
	const GLrecti nullscreen = {0, 0, 0, 0};
	float xmin, ymin, xmax, ymax, zmin, zmax;
	GLrecti ibounds;
	int i, j;

	GLbitvalue retval[CR_MAX_BITARRAY];

	/* Init bucketInfo */
	bucketInfo->objectMin = thread->packer->bounds_min;
	bucketInfo->objectMax = thread->packer->bounds_max;
	bucketInfo->screenMin = zero_vect;
	bucketInfo->screenMax = zero_vect;
	bucketInfo->pixelBounds = nullscreen;
	for (j=0;j<CR_MAX_BITARRAY;j++)
	     bucketInfo->hits[j] = 0;

	/* Check to make sure the transform is valid */
	if (!t->transformValid)
	{
		/* I'm pretty sure this is always the case, but I'll leave it. */
		crStateTransformUpdateTransform(t);
	}

	/* we might be broadcasting, *or* we might be 
	 * defining a display list, which goes to everyone 
	 * (currently) */

	if (tilesort_spu.bucketMode == BROADCAST || g->lists.newEnd)
	{
		bucketInfo->screenMin = neg_vect;
		bucketInfo->screenMax = one_vect;
		bucketInfo->pixelBounds = fullscreen;
		FILLDIRTY(bucketInfo->hits);
		return;
	}

	xmin = bucketInfo->objectMin.x;
	ymin = bucketInfo->objectMin.y;
	zmin = bucketInfo->objectMin.z;
	xmax = bucketInfo->objectMax.x;
	ymax = bucketInfo->objectMax.y;
	zmax = bucketInfo->objectMax.z;

	if (tilesort_spu.providedBBOX != GL_SCREEN_BBOX_CR)
	{
		crTransformBBox( xmin, ymin, zmin, xmax, ymax, zmax, m,
		                 &xmin, &ymin, &zmin, &xmax, &ymax, &zmax );
	}

	/* Copy for export */
	bucketInfo->screenMin.x = xmin;
	bucketInfo->screenMin.y = ymin;
	bucketInfo->screenMin.z = zmin;
	bucketInfo->screenMax.x = xmax;
	bucketInfo->screenMax.y = ymax;
	bucketInfo->screenMax.z = zmax;

	if (tilesort_spu.bucketMode != WARPED_GRID)
	{
		/* triv reject */
		if (xmin > 1.0f || ymin > 1.0f || xmax < -1.0f || ymax < -1.0f) 
		{
			for (j=0;j<CR_MAX_BITARRAY;j++)
	     			bucketInfo->hits[j] = 0;
			return;
		}

		/* clamp */
		if (xmin < -1.0f) xmin = -1.0f;
		if (ymin < -1.0f) ymin = -1.0f;
		if (xmax > 1.0f) xmax = 1.0f;
		if (ymax > 1.0f) ymax = 1.0f;

	}
	ibounds.x1 = (int) (tilesort_spu.halfViewportWidth*xmin + tilesort_spu.viewportCenterX);
	ibounds.x2 = (int) (tilesort_spu.halfViewportWidth*xmax + tilesort_spu.viewportCenterX);
	ibounds.y1 = (int) (tilesort_spu.halfViewportHeight*ymin + tilesort_spu.viewportCenterY);
	ibounds.y2 = (int) (tilesort_spu.halfViewportHeight*ymax + tilesort_spu.viewportCenterY);

	bucketInfo->pixelBounds = ibounds;

	/* Initialize the retval bitvector.
	 */
	for (j = 0; j < CR_MAX_BITARRAY; j++)
	     retval[j] = 0;


	/* Compute the retval bitvector values.
	 * Bit [i] is set if the bounding box intersects any tile on server [i].
	 */
	if (tilesort_spu.bucketMode == TEST_ALL_TILES)
	{
		/* Explicitly test the bounding box (ibounds) against all tiles on
		 * all servers.
		 */
		for (i=0; i < tilesort_spu.num_servers; i++) 
		{
			/* 32 bits (flags) per element in retval */
			const int node32 = i >> 5;
			const int node = i & 0x1f;
			for (j=0; j < tilesort_spu.servers[i].num_extents; j++) 
			{
				if (ibounds.x1 < tilesort_spu.servers[i].extents[j].x2 && 
				    ibounds.x2 >= tilesort_spu.servers[i].extents[j].x1 &&
				    ibounds.y1 < tilesort_spu.servers[i].extents[j].y2 &&
				    ibounds.y2 >= tilesort_spu.servers[i].extents[j].y1) 
				{
					retval[node32] |= (1 << node);
					break;
				}
			}
		}
	} 
	else if (tilesort_spu.bucketMode == UNIFORM_GRID)
	{
		/* Use optimized hash table solution to determine
		 * bounding box / server intersections.
		 */
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
	else if (tilesort_spu.bucketMode == NON_UNIFORM_GRID)
	{
		/* Non-uniform grid bucketing (dynamic tile resize)
		 * Algorithm: we basically march over the tile columns in from the left
		 * and in from the right and the tile rows in from the bottom and in
		 * from the top until we hit the object bounding box.
		 * Then we loop over the intersecting tiles and flag those servers.
		 */
		int bottom, top, left, right;

		/* march from bottom to top */
		bottom = Grid.rows - 1;
		for (i = 0; i < Grid.rows; i++)
		{
			if (Grid.rowY2[i] > ibounds.y1)
			{
				bottom = i;
				break;
			}
		}
		/* march from top to bottom */
		top = 0;
		for (i = Grid.rows - 1; i >= 0; i--)
		{
			if (Grid.rowY1[i] < ibounds.y2)
			{
				top = i;
				break;
			}
		}
		/* march from left to right */
		left = Grid.columns - 1;
		for (i = 0; i < Grid.columns; i++)
		{
			if (Grid.colX2[i] > ibounds.x1)
			{
				left = i;
				break;
			}
		}
		/* march from right to left */
		right = 0;
		for (i = Grid.columns - 1; i >= 0; i--)
		{
			if (Grid.colX1[i] < ibounds.x2)
			{
				right = i;
				break;
			}
		}

		CRASSERT(top >= bottom);
		CRASSERT(right >= left);

		for (i = bottom; i <= top; i++)
		{
			for (j = left; j <= right; j++)
			{
				const int server = Grid.server[i][j];
				const int node32 = server >> 5;
				const int node = server & 0x1f;
				retval[node32] |= (1 << node);
			}
		}
		/*
		printf("bucket result: %d %d\n", retval[0] & 1, (retval[0] >> 1) & 1);
		*/
	}
	else if (tilesort_spu.bucketMode == WARPED_GRID)
	{
		bucketInfo->screenMin = neg_vect;
		bucketInfo->screenMax = one_vect;
		bucketInfo->pixelBounds = fullscreen;

		/* uber-slow overlap testing mode */
		for (i=0; i < tilesort_spu.num_servers; i++) 
		{
			/* 32 bits (flags) per element in retval */
			const int node32 = i >> 5;
			const int node = i & 0x1f;

#if 1
			for (j=0; j < tilesort_spu.servers[i].num_extents; j++) 
			{
				if (quad_overlap(tilesort_spu.servers[i].world_extents[j],
										xmin, ymin, xmax, ymax))
				{
					retval[node32] |= (1 << node);
					break;
				}
			}

#else
			/* XXX: just broadcast now, for debugging */
			retval[node32] |= (1 << node);
#endif			
		}
	}
	else if (tilesort_spu.bucketMode == RANDOM)
	{
		/* Randomly select a server */
		const int server = crRandInt(0, tilesort_spu.num_servers - 1);
		const int node32 = server >> 5;
		const int node = server & 0x1f;

		retval[node32] |= (1 << node);
	}
	else
	{
		crError("Invalid value for tilesort_spu.bucketMode");
	}

	/* XXX why use retval at all?  Why not just use bucketInfo->hits? */
	crMemcpy((char*)bucketInfo->hits, (char*)retval,
				sizeof(GLbitvalue) * CR_MAX_BITARRAY);

	return;
}

void tilesortspuBucketGeometry(TileSortBucketInfo *info)
{
	/* First, call the real bucketer */
	doBucket( info );
	/* Finally, do pinching.  This is unimplemented currently. */
	/* PINCHIT(); */
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

	if (tilesort_spu.bucketMode == UNIFORM_GRID)
	{
		fillBucketingHash();
	}
}


/*
 * Examine the server tile boundaries to compute the overall max
 * viewport dims.  Then send those dims to the servers.
 */
void tilesortspuComputeMaxViewport(void)
{
	ThreadInfo *thread0 = &(tilesort_spu.thread[0]);
	GLint totalDims[2];
	int i;

	/*
	 * It's hard to say what the max viewport size should be.
	 * We've changed this computation a few times now.
	 * For now, we set it to twice the mural size, or at least 4K.
	 * One problem is that the mural size can change dynamically...
	 */
	totalDims[0] = 2 * tilesort_spu.muralWidth;
	totalDims[1] = 2 * tilesort_spu.muralHeight;
	if (totalDims[0] < 4096)
		totalDims[0] = 4096;
	if (totalDims[1] < 4096)
		totalDims[1] = 4096;

	tilesort_spu.limits.maxViewportDims[0] = totalDims[0];
	tilesort_spu.limits.maxViewportDims[1] = totalDims[1];

	/* 
	 * Once we've computed the maximum viewport size, we send
	 * a message to each server with its new viewport parameters.
	 */
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		crPackSetBuffer( thread0->packer, &(thread0->pack[i]) );

		if (tilesort_spu.swap)
			crPackChromiumParametervCRSWAP(GL_SET_MAX_VIEWPORT_CR, GL_INT, 2, totalDims);
		else
			crPackChromiumParametervCR(GL_SET_MAX_VIEWPORT_CR, GL_INT, 2, totalDims);

		crPackGetBuffer( thread0->packer, &(thread0->pack[i]) );

		/* Flush buffer (send to server) */
		tilesortspuSendServerBuffer( i );
	}
}
