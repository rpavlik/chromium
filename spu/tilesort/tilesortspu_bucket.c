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
#ifdef WINDOWS
#define FINITE(X) _finite(X)
#else
#define FINITE(X) finite(X)
#endif


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
	p2 = v[3][1]*f[0][0] - v[3][0]*f[0][1];
	
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
 * Transform a bounding box from object space to NDC space.
 * This routine is used by most of the bucketing algorithms.
 * Input: winInfo - which window/mural
 *        server - which server (usually not needed)
 *        objMin, objMax - bounds in object space
 * Output: xmin, ymin, zmin, xmax, ymax, zmax - bounds in NDC
 *         ibounds - bounds in screen coords
 * Return: GL_TRUE if the resulting screen-space NDC bbox is visible,
 *         GL_FALSE if the resulting screen-space NDC bbox is not visible.
 */
static GLboolean
TransformBBox(const WindowInfo *winInfo, int server,
							const GLvectorf *objMin, const GLvectorf *objMax,
							float *xmin, float *ymin, float *zmin,
							float *xmax, float *ymax, float *zmax,
							CRrecti *ibounds)
{
	GET_THREAD(thread);
	CRContext *g = thread->currentContext->State;
	CRTransformState *t = &(g->transform);

	if (thread->currentContext->providedBBOX == GL_SCREEN_BBOX_CR) {
		/* objMin, objMax are in NDC screen coords already */
		*xmin = objMin->x;
		*ymin = objMin->y;
		*zmin = objMin->z;
		*xmax = objMax->x;
		*ymax = objMax->y;
		*zmax = objMax->z;
	}
	else if (winInfo->matrixSource == MATRIX_SOURCE_SERVERS) {
		/* Use matrices obtained from the servers */
		const ServerWindowInfo *servWinInfo = winInfo->server + server;
		CRmatrix pv, pvm;

		/* XXX we could multiply this earlier! */
		/* pv = proj matrix * view matrix */
		crMatrixMultiply(&pv, &servWinInfo->projectionMatrix,
														&servWinInfo->viewMatrix);
		/* pvm = pv * model matrix */
		crMatrixMultiply(&pvm, &pv, t->modelViewStack.top);

		/* Transform bbox by modelview * projection, and project to screen */
		crTransformBBox( objMin->x, objMin->y, objMin->z,
										 objMax->x, objMax->y, objMax->z, &pvm,
										 xmin, ymin, zmin, xmax, ymax, zmax );
	}
	else if (winInfo->matrixSource == MATRIX_SOURCE_CONFIG &&
					 thread->currentContext->stereoDestFlags != (EYE_LEFT | EYE_RIGHT)) {
		/* use special left or right eye matrices (set via config options) */
		const int curEye = thread->currentContext->stereoDestFlags - 1;
		CRmatrix pv, pvm;

		CRASSERT(curEye == 0 || curEye == 1);

		/* XXX we could multiply this earlier! */
		/* pv = proj matrix * view matrix */
		crMatrixMultiply(&pv, &tilesort_spu.stereoProjMatrices[curEye],
														&tilesort_spu.stereoViewMatrices[curEye]);
		/* pvm = pv * model matrix */
		crMatrixMultiply(&pvm, &pv, t->modelViewStack.top);

		/* Transform bbox by modelview * projection, and project to screen */
		crTransformBBox( objMin->x, objMin->y, objMin->z,
										 objMax->x, objMax->y, objMax->z, &pvm,
										 xmin, ymin, zmin, xmax, ymax, zmax );
	}
	else {
		const CRmatrix *mvp = &(t->modelViewProjection);

		CRASSERT(winInfo->matrixSource == MATRIX_SOURCE_APP ||
				 thread->currentContext->stereoDestFlags == (EYE_LEFT | EYE_RIGHT));

		/* Check to make sure the transform is valid */
		if (!t->modelViewProjectionValid)
		{
			/* I'm pretty sure this is always the case, but I'll leave it. */
			crStateTransformUpdateTransform(t);
		}

		/* Transform bbox by modelview * projection, and project to screen */
		crTransformBBox( objMin->x, objMin->y, objMin->z,
										 objMax->x, objMax->y, objMax->z, mvp,
										 xmin, ymin, zmin, xmax, ymax, zmax );
	}

	/* trivial rejection test */
	if (*xmin > 1.0f || *ymin > 1.0f || *xmax < -1.0f || *ymax < -1.0f) {
		/* bbox doesn't intersect screen */
		return GL_FALSE;
	}

	/* clamp */
	if (*xmin < -1.0f) *xmin = -1.0f;
	if (*ymin < -1.0f) *ymin = -1.0f;
	if (*xmax > 1.0f) *xmax = 1.0f;
	if (*ymax > 1.0f) *ymax = 1.0f;

	if (ibounds) {
		/* return screen pixel coords too */
		ibounds->x1 = (int) (winInfo->halfViewportWidth * *xmin + winInfo->viewportCenterX);
		ibounds->x2 = (int) (winInfo->halfViewportWidth * *xmax + winInfo->viewportCenterX);
		ibounds->y1 = (int) (winInfo->halfViewportHeight * *ymin + winInfo->viewportCenterY);
		ibounds->y2 = (int) (winInfo->halfViewportHeight * *ymax + winInfo->viewportCenterY);
	}
	return GL_TRUE;
}


/*
 * Compute bounding box/tile intersections.
 * Output:  bucketInfo - results of intersection tests
 * XXX someday break this function into a bunch of subroutines, one for
 * each bucketing mode.
 */
static void
doBucket( const WindowInfo *winInfo, TileSortBucketInfo *bucketInfo )
{
	static const CRrecti fullscreen = {-CR_MAXINT, CR_MAXINT, -CR_MAXINT, CR_MAXINT};
	static const CRrecti nullscreen = {0, 0, 0, 0};
	GET_CONTEXT(g);

	/* Init bucketInfo results */
	bucketInfo->objectMin = thread->packer->bounds_min;
	bucketInfo->objectMax = thread->packer->bounds_max;
	bucketInfo->pixelBounds = nullscreen;
	/* Initialize the results/hits bitvector.  Each bit represents a server.
	 * If the bit is set, send the geometry to that server.  There are 32
	 * bits per word in the vector.
	 */
	{
		int j;
		for (j = 0; j < CR_MAX_BITARRAY; j++)
			bucketInfo->hits[j] = 0;
	}

	/* We might be broadcasting, *or* we might be defining a display list,
	 * which goes to everyone (currently).
	 */
	if (winInfo->bucketMode == BROADCAST || g->lists.currentIndex) {
		bucketInfo->pixelBounds = fullscreen;
		FILLDIRTY(bucketInfo->hits);
		return;
	}

	/* Check for infinite bounding box values.  If so, broadcast.
	 * This can happen when glVertex4f() is called with w=0.  This
	 * is done in the NVIDIA infinite_shadow_volume demo.
	 */
	if (!FINITE(bucketInfo->objectMin.x) || !FINITE(bucketInfo->objectMax.x)) {
		bucketInfo->pixelBounds = fullscreen;
		FILLDIRTY(bucketInfo->hits);
		return;
	}

	/* When using vertex programs, vertices may get warped/displaced.
	 * This is a way to grow the bounding box a little if needed.
	 */
	if (tilesort_spu.bboxScale != 1.0) {
		GLfloat scale = tilesort_spu.bboxScale;
		/* compute center point of box */
		GLfloat cx = 0.5f * (bucketInfo->objectMin.x + bucketInfo->objectMax.x);
		GLfloat cy = 0.5f * (bucketInfo->objectMin.y + bucketInfo->objectMax.y);
		GLfloat cz = 0.5f * (bucketInfo->objectMin.z + bucketInfo->objectMax.z);
		/* scale bounds relative to center point */
		bucketInfo->objectMin.x = cx + (bucketInfo->objectMin.x - cx) * scale;
		bucketInfo->objectMin.y = cy + (bucketInfo->objectMin.y - cy) * scale;
		bucketInfo->objectMin.z = cz + (bucketInfo->objectMin.z - cz) * scale;
		bucketInfo->objectMax.x = cx + (bucketInfo->objectMax.x - cx) * scale;
		bucketInfo->objectMax.y = cy + (bucketInfo->objectMax.y - cy) * scale;
		bucketInfo->objectMax.z = cz + (bucketInfo->objectMax.z - cz) * scale;
	}

	/*
	 * OK, now do bucketing
	 */
	if (winInfo->bucketMode == TEST_ALL_TILES)
	{
		float xmin, ymin, xmax, ymax, zmin, zmax;
		CRrecti ibounds;
		int i, j;

		if (!TransformBBox(winInfo, 0,
								 /* in */ &bucketInfo->objectMin, &bucketInfo->objectMax,
								 /* out */ &xmin, &ymin, &zmin, &xmax, &ymax, &zmax, &ibounds))
			return; /* not visible, all done */

		/* Explicitly test the bounding box (ibounds) against all tiles on
		 * all servers.
		 */
		for (i=0; i < tilesort_spu.num_servers; i++) 
		{
			const ServerWindowInfo *servWinInfo = winInfo->server + i;
			/* only bother if the stereo left/right flags agree */
			if (servWinInfo->eyeFlags & thread->currentContext->stereoDestFlags) {
				/* 32 bits (flags) per element in hits[] */
				const int node32 = i >> 5;
				const int node = i & 0x1f;

				if (winInfo->matrixSource == MATRIX_SOURCE_SERVERS) {
					/* recompute bounds */
					TransformBBox(winInfo, i,
												&bucketInfo->objectMin,	&bucketInfo->objectMax, /*in*/
												&xmin, &ymin, &zmin, &xmax, &ymax, &ymin, &ibounds);
				}

				bucketInfo->pixelBounds = ibounds;

				for (j = 0; j < servWinInfo->num_extents; j++) 
				{
					if (ibounds.x1 < servWinInfo->extents[j].x2 && 
							ibounds.x2 >= servWinInfo->extents[j].x1 &&
							ibounds.y1 < servWinInfo->extents[j].y2 &&
							ibounds.y2 >= servWinInfo->extents[j].y1) 
					{
						bucketInfo->hits[node32] |= (1 << node);
						break;
					}
				}
			}
		}
		/* all done */
	} 
	else if (winInfo->bucketMode == UNIFORM_GRID)
	{
		/* Use optimized hash table solution to determine
		 * bounding box / server intersections.
		 */
		const HashInfo *hash = (const HashInfo *) winInfo->bucketInfo;
		const BucketRegion *r;
		const BucketRegion *q;
		float xmin, ymin, xmax, ymax, zmin, zmax;
		CRrecti ibounds;

		CRASSERT(winInfo->matrixSource != MATRIX_SOURCE_SERVERS);

	
		/* only need to compute bbox once for all servers */
		if (!TransformBBox(winInfo, 0,
											 /* in */ &bucketInfo->objectMin, &bucketInfo->objectMax,
											 /* out */ &xmin, &ymin, &zmin, &xmax, &ymax, &zmax, &ibounds))
			return; /* not visible, all done */
		
		bucketInfo->pixelBounds = ibounds;

		for (r = hash->rhash[BKT_DOWNHASH(0, winInfo->muralHeight)][BKT_DOWNHASH(0, winInfo->muralWidth)];
				 r && ibounds.y2 >= r->extents.y1;
				 r = r->up) 
		{
			for (q=r; q && ibounds.x2 >= q->extents.x1; q = q->right) 
			{
				if (CHECKDIRTY(bucketInfo->hits, q->id)) 
				{
					continue;
				}
				/* XXX what about stereo eye selection? */
				if (ibounds.x1 < q->extents.x2 && ibounds.x2 >= q->extents.x1 &&
		 		    ibounds.y1 < q->extents.y2 && ibounds.y2 >= q->extents.y1) 
				{
					int j;
					for (j=0;j<CR_MAX_BITARRAY;j++)
						bucketInfo->hits[j] |= q->id[j];
				}
			}
		}
		/* all done */
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
		float xmin, ymin, xmax, ymax, zmin, zmax;
		CRrecti ibounds;
		int i, j;

		CRASSERT(grid);

		if (!TransformBBox(winInfo, 0,
											 /* in */ &bucketInfo->objectMin, &bucketInfo->objectMax,
											 /* out */ &xmin, &ymin, &zmin, &xmax, &ymax, &zmax, &ibounds))
			return; /* not visible, all done */

		bucketInfo->pixelBounds = ibounds;

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
				bucketInfo->hits[node32] |= (1 << node);
			}
		}
		/* all done */
	}
	else if (winInfo->bucketMode == WARPED_GRID)
	{
		float xmin, ymin, xmax, ymax, zmin, zmax;
		int i, j;

		TransformBBox(winInfo, 0,
									/* in */ &bucketInfo->objectMin, &bucketInfo->objectMax,
									/* out */ &xmin, &ymin, &zmin, &xmax, &ymax, &zmax, NULL);

		if (xmin == FLT_MAX || ymin == FLT_MAX || zmin == FLT_MAX ||
			xmax == -FLT_MAX || ymax == -FLT_MAX || zmax == -FLT_MAX)
		{
			/* trivial reject */
			for (j=0;j<CR_MAX_BITARRAY;j++)
				bucketInfo->hits[j] = 0;
			return;
		}

		bucketInfo->pixelBounds = fullscreen;

		/* uber-slow overlap testing mode */
		for (i=0; i < tilesort_spu.num_servers; i++) 
		{
			ServerWindowInfo *servWinInfo = winInfo->server + i;
			/* only bother if the stereo left/right flags agree */
			if (servWinInfo->eyeFlags & thread->currentContext->stereoDestFlags) {
				/* 32 bits (flags) per element in bucketInfo->hits */
				for (j=0; j < servWinInfo->num_extents; j++) 
				{
					if (quad_overlap(servWinInfo->world_extents[j],
													 xmin, ymin, xmax, ymax))
					{
						const int node32 = i >> 5;
						const int node = i & 0x1f;
						bucketInfo->hits[node32] |= (1 << node);
						break;
					}
				}
			}
		}
		/* all done */

	}
	else if (winInfo->bucketMode == FRUSTUM) {
		CRTransformState *t = &(g->transform);
		int i, j;

		/* test 3D bounding box vertices against the view frustum */
		for (i = 0; i < tilesort_spu.num_servers; i++) {
			const ServerWindowInfo *servWinInfo = winInfo->server + i;
			/* only bother if the stereo left/right flags agree */
			if (servWinInfo->eyeFlags & thread->currentContext->stereoDestFlags) {
				const float xmin = bucketInfo->objectMin.x;
				const float ymin = bucketInfo->objectMin.y;
				const float zmin = bucketInfo->objectMin.z;
				const float xmax = bucketInfo->objectMax.x;
				const float ymax = bucketInfo->objectMax.y;
				const float zmax = bucketInfo->objectMax.z;
				CRrecti ibounds;
				int clipAndMask;
				CRmatrix m;
				GLvectorf corner[8];

				/* Compute matrix m = projection * view * modelview. */
				if (winInfo->matrixSource == MATRIX_SOURCE_SERVERS) {
					/* Use projection and view matrices obtained from servers */
					/* XXX use pre-multiplied matrix here */
					crMatrixMultiply(&m, &servWinInfo->projectionMatrix,
																	&servWinInfo->viewMatrix);
				}
				else if (winInfo->matrixSource == MATRIX_SOURCE_CONFIG &&
					 thread->currentContext->stereoDestFlags != (EYE_LEFT | EYE_RIGHT)) {
					/* Use the left or right eye stereo projection/view matrices */
					const int curEye = thread->currentContext->stereoDestFlags - 1;
					CRASSERT(curEye == 0 || curEye == 1);
					/* XXX use pre-multiplied matrix here */
					crMatrixMultiply(&m, &tilesort_spu.stereoProjMatrices[curEye],
																	&tilesort_spu.stereoViewMatrices[curEye]);
				}
				else {
					/* Use the application's projection and server's view matrice */
					crMatrixMultiply(&m, t->projectionStack.top,
																	&servWinInfo->viewMatrix);
				}
				crMatrixMultiply(&m, &m, t->modelViewStack.top);

				/*
				 * Compute clipping flags to determine if the bbox intersects
				 * the viewing frustum.  Actually, check if all 8 vertices are
				 * either above, below, left, right, in front or behind the frustum.
				 * If that's true, the bbox is not visible, else it is.
				 */
				clipAndMask = ~0;
				for (j = 0; j < 8; j++) {
					int clipmask = 0;
					/* generate corner vertex.  xmin, xmax etc are object coords */
					corner[j].x = (j & 1) ? xmin : xmax;
					corner[j].y = (j & 2) ? ymin : ymax;
					corner[j].z = (j & 4) ? zmin : zmax;
					corner[j].w = 1.0;
					/* transform */
					crStateTransformXformPointMatrixf(&m, &corner[j]);
					/* corner is now in clip coordinates */
					/* cliptest */
					if (-corner[j].w > corner[j].x)
						clipmask |= 1; /* left of frustum */
					if (corner[j].x > corner[j].w)
						clipmask |= 2; /* right of frustum */
					if (-corner[j].w > corner[j].y)
						clipmask |= 4; /* below frustum */
					if (corner[j].y > corner[j].w)
						clipmask |= 8; /* above frustum */
					if (-corner[j].w > corner[j].z)
						clipmask |= 16; /* in front of frustum */
					if (corner[j].z > corner[j].w)
						clipmask |= 32; /* beyond frustum */
					clipAndMask &= clipmask;
				}
				if (clipAndMask) {
					/* all corners are beyond one of the frustum's bounding planes,
					 * therefore, the bbox can't intersect the frustum.
					 */
				}
				else {
					/* bounding box intersects the frustum in some manner */
					const int node32 = i >> 5;
					const int node = i & 0x1f;
					bucketInfo->hits[node32] |= (1 << node);
					/* XXX The following fancy bounding box code doesn't work.
					 * The problem is we need to have separate bucketInfo->pixelBounds
					 * data for each server so that when we send the BoundsInfo
					 * function/payload to the servers we actually send the correct
					 * bounds info.
					 * For now, just send bounds which match the mural size.
					 * This is OK since only the appropriate servers will get the
					 * geometry anyway.  It's only for tiled servers that this really
					 * makes any difference.  -Brian
					 */
#if 0
					/* Compute window coords of the corners, and find the bounding
					 * box of those vertices. store in ibounds.
					 */
					for (j = 0; j < 8; j++) {
						float invW = 1.0 / corner[j].w;
						corner[j].x *= invW;
						corner[j].y *= invW;
						corner[j].z *= invW;
						corner[j].x = winInfo->halfViewportWidth * corner[j].x + winInfo->viewportCenterX;
						corner[j].y = winInfo->halfViewportHeight * corner[j].y + winInfo->viewportCenterY;
						if (j == 0) {
							/* init bounding box */
							ibounds.x1 = ibounds.x2 = (int) corner[0].x;
							ibounds.y1 = ibounds.y2 = (int) corner[0].y;
						}
						else {
							/* update bounding box */
							if (corner[j].x < ibounds.x1)
								ibounds.x1 = corner[j].x;
							if (corner[j].x > ibounds.x2)
								ibounds.x2 = corner[j].x;
							if (corner[j].y < ibounds.y1)
								ibounds.y1 = corner[j].y;
							if (corner[j].y > ibounds.y2)
								ibounds.y2 = corner[j].y;
						}
					} /* loop over 8 corners */
#else
					/* set bounds to full window dims */
					ibounds.x1 = 0;
					ibounds.y1 = 0;
					ibounds.x2 = winInfo->muralWidth;
					ibounds.y2 = winInfo->muralHeight;
#endif
					bucketInfo->pixelBounds = ibounds;
				}
			} /* if eye flags match */
		} /* loop over servers */
		/* all done */

	}
	else if (winInfo->bucketMode == RANDOM)
	{
		/* Randomly select a server */
		/* XXX what about stereo eye selection? */
		float xmin, ymin, xmax, ymax, zmin, zmax;
		CRrecti ibounds;
		const int server = crRandInt(0, tilesort_spu.num_servers - 1);
		const int node32 = server >> 5;
		const int node = server & 0x1f;

		/* only need to compute bbox once for all servers */
		if (!TransformBBox(winInfo, 0,
											 /* in */ &bucketInfo->objectMin, &bucketInfo->objectMax,
											 /* out */ &xmin, &ymin, &zmin, &xmax, &ymax, &zmax, &ibounds))
			/*return*/; /* not visible, all done */
		
		bucketInfo->pixelBounds = ibounds;

		bucketInfo->hits[node32] |= (1 << node);
		/* all done */
	}
	else
	{
		crError("Invalid value for winInfo->bucketMode");
	}
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
