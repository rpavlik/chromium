#include "tilesortspu.h"
#include "cr_glstate.h"
#include "cr_pack.h"
#include "cr_mem.h"

#include "cr_applications.h"

#include <limits.h>
#include <float.h>

static float _vmult(float *m, float x, float y, float z) 
{
	return m[0]*x + m[4]*y + m[8]*z + m[12];
}

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

void __fillBucketingHash (void) 
{
	int i, j, k, m;
	int r_len=0;
	BucketRegion *rlist;
	GLbitvalue id;

	/* Allocate rlist */
	rlist_alloc = 64*128;
	// rlist_alloc = GLCONFIG_MAX_PROJECTORS*GLCONFIG_MAX_EXTENTS;
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
		for (j=0; j<tilesort_spu.servers[i].num_extents; j++) 
		{
			BucketRegion *r = &rlist[id++];
			r->id = 1 << i;
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
	static TileSortBucketInfo bucketInfo;
	CRContext *g = tilesort_spu.ctx;
	CRTransformState *t = &(g->transform);
	GLmatrix *m = &(t->transform);

	const GLvectorf zero_vect = {0.0f, 0.0f, 0.0f, 1.0f};
	const GLvectorf one_vect = {1.0f, 1.0f, 1.0f, 1.0f};
	const GLvectorf neg_vect = {-1.0f, -1.0f, -1.0f, 1.0f};
	const GLrecti fullscreen = {-GL_MAXINT, GL_MAXINT, -GL_MAXINT, GL_MAXINT};
	const GLrecti nullscreen = {0, 0, 0, 0};
	float x[8], y[8], z[8], w[8];
	float xmin, ymin, xmax, ymax, zmin, zmax;
	GLrecti ibounds;
	int i,j;

	GLbitvalue retval;

	/*  Here is the arrangement of the bounding box
	 *  
	 *           0 --- 1
	 *           |\    .\
	 *           | 2 --- 3 
	 *           | |   . |
	 *           | |   . |
	 *           4.|...5 |
	 *            \|    .|
	 *             6 --- 7
	 *  
	 *  c array contains the edge connectivitiy list
	 */

	static const int c[8][3] = {	
		{1, 2, 4}, 
		{0, 3, 5}, 
		{0, 3, 6}, 
		{1, 2, 7},
		{0, 5, 6}, 
		{1, 4, 7}, 
		{2, 4, 7}, 
		{3, 5, 6} 
	};

	/* Init bucketInfo */
	bucketInfo.objectMin = cr_packer_globals.bounds_min;
	bucketInfo.objectMax = cr_packer_globals.bounds_max;
	bucketInfo.screenMin = zero_vect;
	bucketInfo.screenMax = zero_vect;
	bucketInfo.pixelBounds = nullscreen;
	bucketInfo.hits = 0;

	/* Check to make sure the transform is valid */
	if (!t->transformValid)
	{
		// I'm pretty sure this is always the case, but I'll leave it.
		crStateTransformUpdateTransform(t);
	}

	// we might be broadcasting, *or* we might be
	// defining a display list, which goes to everyone
	// (currently)

	if (tilesort_spu.broadcast || g->lists.newEnd)
	{
		bucketInfo.screenMin = neg_vect;
		bucketInfo.screenMax = one_vect;
		bucketInfo.pixelBounds = fullscreen;
		bucketInfo.hits = GLBITS_ONES;
		return &bucketInfo;
	}

	xmin = bucketInfo.objectMin.x;
	ymin = bucketInfo.objectMin.y;
	zmin = bucketInfo.objectMin.z;
	xmax = bucketInfo.objectMax.x;
	ymax = bucketInfo.objectMax.y;
	zmax = bucketInfo.objectMax.z;

	if (tilesort_spu.providedBBOX != CR_SCREEN_BBOX_HINT)
	{
		//Now transform the bounding box points
		x[0] = _vmult(&(m->m00), xmin, ymin, zmin);
		x[1] = _vmult(&(m->m00), xmax, ymin, zmin);
		x[2] = _vmult(&(m->m00), xmin, ymax, zmin);
		x[3] = _vmult(&(m->m00), xmax, ymax, zmin);
		x[4] = _vmult(&(m->m00), xmin, ymin, zmax);
		x[5] = _vmult(&(m->m00), xmax, ymin, zmax);
		x[6] = _vmult(&(m->m00), xmin, ymax, zmax);
		x[7] = _vmult(&(m->m00), xmax, ymax, zmax);

		y[0] = _vmult(&(m->m01), xmin, ymin, zmin);
		y[1] = _vmult(&(m->m01), xmax, ymin, zmin);
		y[2] = _vmult(&(m->m01), xmin, ymax, zmin);
		y[3] = _vmult(&(m->m01), xmax, ymax, zmin);
		y[4] = _vmult(&(m->m01), xmin, ymin, zmax);
		y[5] = _vmult(&(m->m01), xmax, ymin, zmax);
		y[6] = _vmult(&(m->m01), xmin, ymax, zmax);
		y[7] = _vmult(&(m->m01), xmax, ymax, zmax);

		z[0] = _vmult(&(m->m02), xmin, ymin, zmin);
		z[1] = _vmult(&(m->m02), xmax, ymin, zmin);
		z[2] = _vmult(&(m->m02), xmin, ymax, zmin);
		z[3] = _vmult(&(m->m02), xmax, ymax, zmin);
		z[4] = _vmult(&(m->m02), xmin, ymin, zmax);
		z[5] = _vmult(&(m->m02), xmax, ymin, zmax);
		z[6] = _vmult(&(m->m02), xmin, ymax, zmax);
		z[7] = _vmult(&(m->m02), xmax, ymax, zmax);

		w[0] = _vmult(&(m->m03), xmin, ymin, zmin);
		w[1] = _vmult(&(m->m03), xmax, ymin, zmin);
		w[2] = _vmult(&(m->m03), xmin, ymax, zmin);
		w[3] = _vmult(&(m->m03), xmax, ymax, zmin);
		w[4] = _vmult(&(m->m03), xmin, ymin, zmax);
		w[5] = _vmult(&(m->m03), xmax, ymin, zmax);
		w[6] = _vmult(&(m->m03), xmin, ymax, zmax);
		w[7] = _vmult(&(m->m03), xmax, ymax, zmax);

		// Now, the object-space bbox has been transformed into
		// clip-space.

		/* Find the 2D bounding box of the 3D bounding box */
		xmin = ymin = zmin = FLT_MAX;
		xmax = ymax = zmax = -FLT_MAX;

		for (i=0; i<8; i++) 
		{
			float xp = x[i];
			float yp = y[i];
			float zp = z[i];
			float wp = w[i];

			/* If corner is to be clipped... */
			if (zp < -wp) 
			{

				/* Point has three edges */
				for (j=0; j<3; j++) 
				{
					/* Handle the clipping... */
					int k = c[i][j];
					float xk = x[k];
					float yk = y[k];
					float zk = z[k];
					float wk = w[k];
					float t = (wp + zp) / (zp+wp-zk-wk);

					if (t < 0.0f || t > 1.0f)
					{
						continue;
					}
					wp = wp + (wk-wp) * t;
					xp = xp + (xk-xp) * t;
					yp = yp + (yk-yp) * t;
					zp = -wp;

					xp /= wp;
					yp /= wp;
					zp /= wp;

					if (xp < xmin) xmin = xp;
					if (xp > xmax) xmax = xp;
					if (yp < ymin) ymin = yp;
					if (yp > ymax) ymax = yp;
					if (zp < zmin) zmin = zp;
					if (zp > zmax) zmax = zp;
				}
			} 
			else 
			{
				/* corner was not clipped.. */
				xp /= wp;
				yp /= wp;
				zp /= wp;
				if (xp < xmin) xmin = xp;
				if (xp > xmax) xmax = xp;
				if (yp < ymin) ymin = yp;
				if (yp > ymax) ymax = yp;
				if (zp < zmin) zmin = zp;
				if (zp > zmax) zmax = zp;
			}
		}
	}

	/* Copy for export */
	bucketInfo.screenMin.x = xmin;
	bucketInfo.screenMin.y = ymin;
	bucketInfo.screenMin.z = zmin;
	bucketInfo.screenMax.x = xmax;
	bucketInfo.screenMax.y = ymax;
	bucketInfo.screenMax.z = zmax;

	/* Now we're down to 4 screen-space points
	 ** (xmin, ymin)
	 ** (xmax, ymin)
	 ** (xmin, ymax)
	 ** (xmax, ymax)
	 */

	/* triv reject */
	if (xmin > 1.0f || ymin > 1.0f || xmax < -1.0f || ymax < -1.0f) 
	{
		bucketInfo.hits = 0;
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

	retval = 0;

	if (!tilesort_spu.optimizeBucketing) 
	{
		for (i=0; i < tilesort_spu.num_servers; i++) 
		{
			for (j=0; j < tilesort_spu.servers[i].num_extents; j++) 
			{
				if (ibounds.x1 < tilesort_spu.servers[i].x2[j] && 
						ibounds.x2 > tilesort_spu.servers[i].x1[j] &&
						ibounds.y1 < tilesort_spu.servers[i].y2[j] &&
						ibounds.y2 > tilesort_spu.servers[i].y1[j]) 
				{
					retval |= 1 << i;
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
				 r && ibounds.y2 > r->extents.y1;
				 r = r->up) 
		{
			for (q=r; q && ibounds.x2 > q->extents.x1; q = q->right) 
			{
				if (retval & q->id) 
				{
					continue;
				}
				if (ibounds.x1 < q->extents.x2 && /* ibounds.x2 > q->extents.x1 && */
						ibounds.y1 < q->extents.y2 && ibounds.y2 > q->extents.y1) 
				{
					retval |= q->id;
				}
			}
		}
	}

	bucketInfo.hits = retval;
	return &bucketInfo;
}

TileSortBucketInfo *tilesortspuBucketGeometry(void)
{
	// First, call the real bucketer
	TileSortBucketInfo *ret = __doBucket();

	// Now, we want to let the state tracker extract the "current" state
	// from the collection of pointers that we have in the geometry 
	// buffer.

	crStateCurrentRecover( &(cr_packer_globals.current) );

	// Finally, do pinching.  This is unimplemented currently.

	// PINCHIT();

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
