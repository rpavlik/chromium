/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <math.h>
#include <float.h>
#include "tilesortspu.h"
#include "tilesortspu_proto.h"
#include "cr_bbox.h"
#include "cr_glstate.h"
#include "cr_pack.h"
#include "cr_packfunctions.h"
#include "cr_mem.h"
#include "cr_rand.h"


/* XXX may need to conditionally define this macro for various platforms */
#define FINITE(X) finite(X)


/*
 * Data structures for hash-based bucketing algorithm.
 * All tiles must be the same size!
 *
 * winInfo->bucketInfo points to a HashInfo when winInfo->bucketMode
 * is UNIFORM_GRID.
 */
typedef struct BucketRegion *BucketRegion_ptr;
typedef struct BucketRegion {
	CRbitvalue       id[CR_MAX_BITARRAY];
	CRrecti          extents;
	BucketRegion_ptr right;
	BucketRegion_ptr up;
} BucketRegion;

#define HASHRANGE 256

#define BKT_DOWNHASH(a, range) ((a)*HASHRANGE/(range))
#define BKT_UPHASH(a, range) ((a)*HASHRANGE/(range) + ((a)*HASHRANGE%(range)?1:0))

typedef struct hash_info {
	BucketRegion *rlist;
	BucketRegion_ptr rhash[HASHRANGE][HASHRANGE];
} HashInfo;



/*
 * Data structures for non-uniform grid bucketing algorithm.
 * The idea is we have a 2-D grid of rows and columns but the
 * width and height of the columns and rows isn't uniform.
 * This'll often be the case with dynamic tile resizing.
 *
 * winInfo->bucketInfo points to a GridInfo when winInfo->bucketMode
 * NON_UNIFORM_GRID.
 */
#define MAX_ROWS 128
#define MAX_COLS 128
typedef struct grid_info {
	int rows, columns;
	int rowY1[MAX_ROWS], rowY2[MAX_ROWS]; /* bounds of each row */
	int colX1[MAX_COLS], colX2[MAX_COLS]; /* bounds of each column */
	int server[MAX_ROWS][MAX_COLS];       /* the server for each rect */
} GridInfo;



static float _min3f(float a, float b, float c)
{
	if ((a < b) && (a < c))
		return a;
	else
	if ((b < a) && (b < c))
		return b;
	else 
		return c;
}

static float _max3f(float a, float b, float c)
{
	if ((a > b) && (a > c))
		return a;
	else
	if ((b > a) && (b > c))
		return b;
	else 
		return c;
}

static float _min2f(float a, float b)
{
	if (a < b)
		return a;
	else
		return b;
}

static float _max2f(float a, float b)
{
	if (a > b)
		return a;
	else
		return b;
}

static float _fabs(float a)
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
static int
quad_overlap(float *quad, float xmin, float ymin, float xmax, float ymax)
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
 * Initialize the hash table used for optimized UNIFORM_GRID bucketing.
 * Return: GL_FALSE - invalid tile size(s) or position(s)
 *         GL_TRUE - success!
 */
static GLboolean
initHashBucketing(WindowInfo *winInfo) 
{
	int i, j, k, m;
	int r_len = 0;
	int id;
	int rlist_alloc = 64 * 128;
	HashInfo *hash;

	hash = (HashInfo *) crCalloc(sizeof(HashInfo));
	if (!hash)
		return GL_FALSE;

	/* First, check that all the tiles are the same size! */
	{
		int reqWidth = 0, reqHeight = 0;

		for (i = 0; i < tilesort_spu.num_servers; i++)
		{
			ServerWindowInfo *servWinInfo = winInfo->server + i;
			for (j = 0; j < servWinInfo->num_extents; j++)
			{
				int x = servWinInfo->extents[j].x1;
				int y = servWinInfo->extents[j].y1;
				int w = servWinInfo->extents[j].x2 - x;
				int h = servWinInfo->extents[j].y2 - y;

				if (reqWidth == 0 || reqHeight == 0)
				{
					reqWidth = w;
					reqHeight = h;
				}
				else if (w != reqWidth || h != reqHeight)
				{
					crWarning("Tile %d on server %d is not the right size!", j, i);
					crWarning("All tiles must be same size when bucket_mode = "
										"'Uniform Grid'.");
					return GL_FALSE;
				}
				else if ((x % reqWidth) != 0 || (y % reqHeight) != 0)
				{
					/* (x,y) should be an integer multiple of the tile size */
					crWarning("Tile %d on server %d is not positioned correctly!", j, i);
					crWarning("Position (%d, %d) is not an integer multiple of the "
										"tile size (%d x %d)", x, y, reqWidth, reqHeight);
					return GL_FALSE;
				}
			}
		}
	}

	/* rlist_alloc = GLCONFIG_MAX_PROJECTORS*GLCONFIG_MAX_EXTENTS; */
	hash->rlist = (BucketRegion *) crAlloc(rlist_alloc * sizeof(BucketRegion));
	if (!hash->rlist) {
		crFree(hash);
		return GL_FALSE;
	}

	for (i=0; i<HASHRANGE; i++) 
	{
		for (j=0; j<HASHRANGE; j++) 
		{
			hash->rhash[i][j] = NULL;
		}
	}

	/* Fill hash table */
	id = 0;
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		ServerWindowInfo *servWinInfo = winInfo->server + i;
		int node32 = i >> 5;
		int node = i & 0x1f;
		for (j = 0; j < servWinInfo->num_extents; j++) 
		{
			BucketRegion *r = &hash->rlist[id++];
			for (k=0;k<CR_MAX_BITARRAY;k++)
				r->id[k] = 0;
			r->id[node32] = (1 << node);
			r->extents = servWinInfo->extents[j]; /* copy x1,y1,x2,y2 */

			for (k=BKT_DOWNHASH(r->extents.x1, winInfo->muralWidth);
				   k<=BKT_UPHASH(r->extents.x2, winInfo->muralWidth) && k < HASHRANGE;
				   k++) 
			{
				for (m=BKT_DOWNHASH(r->extents.y1, winInfo->muralHeight);
				     m<=BKT_UPHASH(r->extents.y2, winInfo->muralHeight) && m < HASHRANGE;
					   m++)
				{
					if ( hash->rhash[m][k] == NULL ||
						   (hash->rhash[m][k]->extents.x1 > r->extents.x1 &&
						    hash->rhash[m][k]->extents.y1 > r->extents.y1)) 
					{
						 hash->rhash[m][k] = r;
					}
				}
			}
		}
	}
	r_len = id;

	/* Initialize links */
	for (i=0; i<r_len; i++) 
	{
		BucketRegion *r = &hash->rlist[i];
		r->right = NULL;
		r->up    = NULL;
	}

	/* Build links */
	for (i=0; i<r_len; i++) 
	{
		BucketRegion *r = &hash->rlist[i];
		for (j=0; j<r_len; j++) 
		{
			BucketRegion *q = &hash->rlist[j];
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

	winInfo->bucketInfo = (void *) hash;
	return GL_TRUE;
}


/*
 * Initialize GridInfo data for the non-uniform grid case.
 * Return: GL_TRUE - the server's tiles form a non-uniform grid
 *         GL_FALSE - the tiles don't form a non-uniform grid - give it up.
 */
static GLboolean
initGridBucketing(WindowInfo *winInfo)
{
	GridInfo *grid;
	int i, j, k;

	grid = (GridInfo *) crCalloc(sizeof(GridInfo));
	if (!grid)
		return GL_FALSE;

	/* analyze all the server tiles to determine grid information */
	grid->rows = 0;
	grid->columns = 0;

	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		ServerWindowInfo *servWinInfo = winInfo->server + i;
		for (j = 0; j < servWinInfo->num_extents; j++) 
		{
			const CRrecti *extent = &(servWinInfo->extents[j]);
			GLboolean found;

			/* look for a row with same y0, y1 */
			found = GL_FALSE;
			for (k = 0; k < grid->rows; k++) {
				if (grid->rowY1[k] == extent->y1 &&	grid->rowY2[k] == extent->y2)
				{
					found = GL_TRUE;
					break;
				}
			}
			if (!found)
			{
				/* add new row (insertion sort) */
				k = grid->rows;
				while (k > 0 && extent->y1 < grid->rowY1[k - 1]) {
					/* move column entry up */
					grid->rowY1[k] = grid->rowY1[k - 1];
					grid->rowY2[k] = grid->rowY2[k - 1];
					k--;
				}
				grid->rowY1[k] = extent->y1;
				grid->rowY2[k] = extent->y2;
				grid->rows++;
			}

			/* look for a col with same x0, x1 */
			found = GL_FALSE;
			for (k = 0; k < grid->columns; k++) {
				if (grid->colX1[k] == extent->x1 && grid->colX2[k] == extent->x2)
				{
					found = GL_TRUE;
					break;
				}
			}
			if (!found)
			{
				/* add new row (insertion sort) */
				k = grid->columns;
				while (k > 0 && extent->x1 < grid->colX1[k - 1]) {
					/* move column entry up */
					grid->colX1[k] = grid->colX1[k - 1];
					grid->colX2[k] = grid->colX2[k - 1];
					k--;
				}
				grid->colX1[k] = extent->x1;
				grid->colX2[k] = extent->x2;
				grid->columns++;
			}
		}
	}

	if (grid->rows == 0 || grid->columns == 0) {
		/* no extent data yet (window not yet mapped) */
		return GL_FALSE;
	}

	/* mural coverage test */
	if (grid->rowY1[0] != 0) {
		crFree(grid);
		return GL_FALSE;   /* first row must start at zero */
	}
	if (grid->rowY2[grid->rows - 1] != (int) winInfo->muralHeight) {
		crFree(grid);
		return GL_FALSE;   /* last row must and at mural height */
	}
	if (grid->colX1[0] != 0) {
		crFree(grid);
		return GL_FALSE;   /* first column must start at zero */
	}
	if (grid->colX2[grid->columns - 1] != (int) winInfo->muralWidth) {
		crFree(grid);
		return GL_FALSE;   /* last column must and at mural width */
	}

	/* make sure the rows and columns adjoin without gaps or overlaps */
	for (k = 1; k < grid->rows; k++) {
		if (grid->rowY2[k - 1] != grid->rowY1[k]) {
			crFree(grid);
			return GL_FALSE;  /* found gap/overlap between rows */
		}
	}
	for (k = 1; k < grid->columns; k++) {
		if (grid->colX2[k - 1] != grid->colX1[k]) {
			crFree(grid);
			return GL_FALSE;  /* found gap/overlap between columns */
		}
	}

	/* init grid->server[][] array to -1 */
	for (i = 0; i < grid->rows; i++)
		for (j = 0; j < grid->columns; j++)
			grid->server[i][j] = -1;

	/* now assign the server number for each rect */
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		ServerWindowInfo *servWinInfo = winInfo->server + i;
		for (j = 0; j < servWinInfo->num_extents; j++) 
		{
			const CRrecti *extent = &(servWinInfo->extents[j]);
			int r, c;
			GLboolean found = GL_FALSE;
			for (r = 0; r < grid->rows && !found; r++)
			{
				if (grid->rowY1[r] == extent->y1 && grid->rowY2[r] == extent->y2)
				{
					for (c = 0; c < grid->columns; c++)
					{
						if (grid->colX1[c] == extent->x1 && grid->colX2[c] == extent->x2)
						{
							if (grid->server[r][c] != -1)
							{
								/* another server already has this tile! */
								crFree(grid);
								return GL_FALSE;
							}
							grid->server[r][c] = i;
							found = GL_TRUE;
							break;
						}
					}
				}
			}
		}
	}

	/* make sure we have a server for each rect! */
	for (i = 0; i < grid->rows; i++) {
		for (j = 0; j < grid->columns; j++) {
			if (grid->server[i][j] == -1) {
				crFree(grid);
				return GL_FALSE;  /* found a hole in the tiling! */
			}
		}
	}

#if 0
	printf("rows:\n");
	for (i = 0; i < grid->rows; i++)
		printf("  y: %d .. %d\n", grid->rowY1[i], grid->rowY2[i]);
	printf("cols:\n");
	for (i = 0; i < grid->columns; i++)
		printf("  x: %d .. %d\n", grid->colX1[i], grid->colX2[i]);
#endif

	winInfo->bucketInfo = (void *) grid;
	return GL_TRUE;  /* a good grid! */
}



/*
 * Compute bounding box/tile intersections.
 * Output:  bucketInfo - results of intersection tests
 */
static void
doBucket( const WindowInfo *winInfo, TileSortBucketInfo *bucketInfo )
{
	static const GLvectorf zero_vect = {0.0f, 0.0f, 0.0f, 1.0f};
	static const GLvectorf one_vect = {1.0f, 1.0f, 1.0f, 1.0f};
	static const GLvectorf neg_vect = {-1.0f, -1.0f, -1.0f, 1.0f};
	static const CRrecti fullscreen = {-CR_MAXINT, CR_MAXINT, -CR_MAXINT, CR_MAXINT};
	static const CRrecti nullscreen = {0, 0, 0, 0};
	GET_CONTEXT(g);
	CRTransformState *t = &(g->transform);
	const CRmatrix *mvp = &(t->modelViewProjection);
	float xmin, ymin, xmax, ymax, zmin, zmax;
	CRrecti ibounds;
	int i, j;

	CRbitvalue retval[CR_MAX_BITARRAY];

	/* Init bucketInfo */
	bucketInfo->objectMin = thread->packer->bounds_min;
	bucketInfo->objectMax = thread->packer->bounds_max;
	bucketInfo->screenMin = zero_vect;
	bucketInfo->screenMax = zero_vect;
	bucketInfo->pixelBounds = nullscreen;
	for (j=0;j<CR_MAX_BITARRAY;j++)
	     bucketInfo->hits[j] = 0;

	/* Check to make sure the transform is valid */
	if (!t->modelViewProjectionValid)
	{
		/* I'm pretty sure this is always the case, but I'll leave it. */
		crStateTransformUpdateTransform(t);
	}

	/* we might be broadcasting, *or* we might be defining a display list,
	 * which goes to everyone (currently).
	 */
	if (winInfo->bucketMode == BROADCAST || g->lists.currentIndex)
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

	if (thread->currentContext->providedBBOX != GL_SCREEN_BBOX_CR)
	{
		/* The current bounds were either provided by the user with
		 * GL_OBJECT_BBOX_CR or computed automatically by Chromium (in
		 * object coords.  Transform the box to window coords.
		 */

		/* Check for infinite bounding box values.  If so, broadcast.
		 * This can happen when glVertex4f() is called with w=0.  This
		 * is done in the NVIDIA infinite_shadow_volume demo.
		 */
		if (!FINITE(xmin) || !FINITE(xmax)) {
			bucketInfo->screenMin = neg_vect;
			bucketInfo->screenMax = one_vect;
			bucketInfo->pixelBounds = fullscreen;
			FILLDIRTY(bucketInfo->hits);
			return;
		}

		crTransformBBox( xmin, ymin, zmin, xmax, ymax, zmax, mvp,
		                 &xmin, &ymin, &zmin, &xmax, &ymax, &zmax );
	}

	/* Copy for export
	 * These are NDC coords in [0,1]x[0,1]
	 */
	bucketInfo->screenMin.x = xmin;
	bucketInfo->screenMin.y = ymin;
	bucketInfo->screenMin.z = zmin;
	bucketInfo->screenMax.x = xmax;
	bucketInfo->screenMax.y = ymax;
	bucketInfo->screenMax.z = zmax;

	if (winInfo->bucketMode == WARPED_GRID)
	{
		if (xmin == FLT_MAX || ymin == FLT_MAX || zmin == FLT_MAX ||
			xmax == -FLT_MAX || ymax == -FLT_MAX || zmax == -FLT_MAX)
		{
			/* trivial reject */
			for (j=0;j<CR_MAX_BITARRAY;j++)
	     			bucketInfo->hits[j] = 0;
			return;
		}
	}
	else
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

	/* compute window coords by applying the viewport transformation */
	ibounds.x1 = (int) (winInfo->halfViewportWidth * xmin + winInfo->viewportCenterX);
	ibounds.x2 = (int) (winInfo->halfViewportWidth * xmax + winInfo->viewportCenterX);
	ibounds.y1 = (int) (winInfo->halfViewportHeight * ymin + winInfo->viewportCenterY);
	ibounds.y2 = (int) (winInfo->halfViewportHeight * ymax + winInfo->viewportCenterY);

	bucketInfo->pixelBounds = ibounds;

	/* Initialize the retval bitvector.
	 */
	for (j = 0; j < CR_MAX_BITARRAY; j++)
	     retval[j] = 0;


	/* Compute the retval bitvector values.
	 * Bit [i] is set if the bounding box intersects any tile on server [i].
	 */
	if (winInfo->bucketMode == TEST_ALL_TILES)
	{
		/* Explicitly test the bounding box (ibounds) against all tiles on
		 * all servers.
		 */
		for (i=0; i < tilesort_spu.num_servers; i++) 
		{
			ServerWindowInfo *servWinInfo = winInfo->server + i;
			/* 32 bits (flags) per element in retval */
			const int node32 = i >> 5;
			const int node = i & 0x1f;
			for (j = 0; j < servWinInfo->num_extents; j++) 
			{
				if (ibounds.x1 < servWinInfo->extents[j].x2 && 
				    ibounds.x2 >= servWinInfo->extents[j].x1 &&
				    ibounds.y1 < servWinInfo->extents[j].y2 &&
				    ibounds.y2 >= servWinInfo->extents[j].y1) 
				{
					retval[node32] |= (1 << node);
					break;
				}
			}
		}
	} 
	else if (winInfo->bucketMode == UNIFORM_GRID)
	{
		/* Use optimized hash table solution to determine
		 * bounding box / server intersections.
		 */
		const HashInfo *hash = (const HashInfo *) winInfo->bucketInfo;
		BucketRegion *r;
		BucketRegion *q;

		for (r = hash->rhash[BKT_DOWNHASH(0, winInfo->muralHeight)][BKT_DOWNHASH(0, winInfo->muralWidth)];
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
	else if (winInfo->bucketMode == NON_UNIFORM_GRID)
	{
		/* Non-uniform grid bucketing (dynamic tile resize)
		 * Algorithm: we basically march over the tile columns in from the left
		 * and in from the right and the tile rows in from the bottom and in
		 * from the top until we hit the object bounding box.
		 * Then we loop over the intersecting tiles and flag those servers.
		 */
		const GridInfo *grid = (const GridInfo *) winInfo->bucketInfo;
		int bottom, top, left, right;

		CRASSERT(grid);

		/* march from bottom to top */
		bottom = grid->rows - 1;
		for (i = 0; i < grid->rows; i++)
		{
			if (grid->rowY2[i] > ibounds.y1)
			{
				bottom = i;
				break;
			}
		}
		/* march from top to bottom */
		top = 0;
		for (i = grid->rows - 1; i >= 0; i--)
		{
			if (grid->rowY1[i] < ibounds.y2)
			{
				top = i;
				break;
			}
		}
		/* march from left to right */
		left = grid->columns - 1;
		for (i = 0; i < grid->columns; i++)
		{
			if (grid->colX2[i] > ibounds.x1)
			{
				left = i;
				break;
			}
		}
		/* march from right to left */
		right = 0;
		for (i = grid->columns - 1; i >= 0; i--)
		{
			if (grid->colX1[i] < ibounds.x2)
			{
				right = i;
				break;
			}
		}

		/*CRASSERT(top >= bottom);*/
		/*CRASSERT(right >= left);*/

		for (i = bottom; i <= top; i++)
		{
			for (j = left; j <= right; j++)
			{
				const int server = grid->server[i][j];
				const int node32 = server >> 5;
				const int node = server & 0x1f;
				retval[node32] |= (1 << node);
			}
		}
		/*
		printf("bucket result: %d %d\n", retval[0] & 1, (retval[0] >> 1) & 1);
		*/
	}
	else if (winInfo->bucketMode == WARPED_GRID)
	{
		bucketInfo->screenMin = neg_vect;
		bucketInfo->screenMax = one_vect;
		bucketInfo->pixelBounds = fullscreen;

		/* uber-slow overlap testing mode */
		for (i=0; i < tilesort_spu.num_servers; i++) 
		{
			ServerWindowInfo *servWinInfo = winInfo->server + i;
			/* 32 bits (flags) per element in retval */
			const int node32 = i >> 5;
			const int node = i & 0x1f;

#if 1
			for (j=0; j < servWinInfo->num_extents; j++) 
			{
				if (quad_overlap(servWinInfo->world_extents[j],
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
	else if (winInfo->bucketMode == RANDOM)
	{
		/* Randomly select a server */
		const int server = crRandInt(0, tilesort_spu.num_servers - 1);
		const int node32 = server >> 5;
		const int node = server & 0x1f;

		retval[node32] |= (1 << node);
	}
	else
	{
		crError("Invalid value for winInfo->bucketMode");
	}

	/* XXX why use retval at all?  Why not just use bucketInfo->hits? */
	crMemcpy((char*)bucketInfo->hits, (char*)retval,
				sizeof(CRbitvalue) * CR_MAX_BITARRAY);

	return;
}

void tilesortspuBucketGeometry(WindowInfo *winInfo, TileSortBucketInfo *info)
{
	/* First, call the real bucketer */
	doBucket( winInfo, info );
	/* Finally, do pinching.  This is unimplemented currently. */
	/* PINCHIT(); */
}

void tilesortspuSetBucketingBounds( WindowInfo *winInfo, int x, int y, unsigned int w, unsigned int h )
{
	winInfo->halfViewportWidth = w / 2.0f;
	winInfo->halfViewportHeight = h / 2.0f;
	winInfo->viewportCenterX = x + winInfo->halfViewportWidth;
	winInfo->viewportCenterY = y + winInfo->halfViewportHeight;
}


/*
 * Prepare bucketing.  This needs to be called:
 *  - upon initialising the default window
 *  - when changing the bucketing mode
 *  - when the tile layout changes
 */
void tilesortspuBucketingInit( WindowInfo *winInfo )
{
	tilesortspuSetBucketingBounds( winInfo, 0, 0,
																 winInfo->muralWidth, winInfo->muralHeight);

	if (winInfo->bucketInfo) {
		/* XXX probably need something better here */
		if (winInfo->bucketMode == UNIFORM_GRID) {
			HashInfo *hash = (HashInfo *) winInfo->bucketInfo;
			crFree(hash->rlist);
		}
		crFree(winInfo->bucketInfo);
		winInfo->bucketInfo = NULL;
	}

	/* Set the window's bucket mode to the default.  If for some reason
	 * we can't use that mode (invalid tile sizes or arrangement) we'll fall
	 * back to the next best bucket mode.
	 */
	winInfo->bucketMode = tilesort_spu.defaultBucketMode;

	if (winInfo->bucketMode == UNIFORM_GRID)
	{
		if (!initHashBucketing(winInfo))
		{
			/* invalid tiles for hash-mode bucketing, try non-uniform */
			crWarning("Falling back to Non-Uniform Grid mode.");
			winInfo->bucketMode = NON_UNIFORM_GRID;
		}
	}

	if (winInfo->bucketMode == NON_UNIFORM_GRID)
	{
		if (!initGridBucketing(winInfo))
		{
			/* The tiling isn't really non-uniform! */
			crWarning("Tilesort bucket_mode = Non-Uniform Grid, but tiles don't "
								"form a non-uniform grid!  "
								"Falling back to Test All Tiles mode.");
			winInfo->bucketMode = TEST_ALL_TILES;
		}
	}
}
