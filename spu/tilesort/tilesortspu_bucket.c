#include "tilesortspu.h"

#include "cr_pack.h" // the bbox is here

TileSortBucketInfo *tilesortspuBucketGeometry(void)
{
	static TileSortBucketInfo bucketinfo;
	CRContext *g = tilesort_spu.ctx;
	CRTransformState *t = &(g->transform);
	GLmatrix *m = &(t->transform);

	(void) g;
	(void) t;
	(void) m;

	bucketinfo.hits = ~0;
	return &bucketinfo;
}
