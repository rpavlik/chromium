/* Copyright (c) 2003, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "cr_mem.h"
#include "zpixspu.h"

ZpixSPU zpix_spu;

static SPUFunctions zpix_functions = {
	NULL,													/* CHILD COPY */
	NULL,													/* DATA */
	_cr_zpix_table								/* THE ACTUAL FUNCTIONS */
};

static SPUFunctions *
zpixSPUInit(int id, SPU * child, SPU * self,
						unsigned int context_id, unsigned int num_contexts)
{
	int i = 0;
	(void) self;
	(void) context_id;
	(void) num_contexts;

	self->privatePtr = (void *) &zpix_spu;

	crMemZero(&zpix_spu, sizeof(zpix_spu));

	zpix_spu.id = id;
	zpix_spu.has_child = 0;
	zpix_spu.server = NULL;
	if (child)
	{
		crSPUInitDispatchTable(&(zpix_spu.child));
		crSPUCopyDispatchTable(&(zpix_spu.child), &(child->dispatch_table));
		zpix_spu.has_child = 1;
	}
	crSPUInitDispatchTable(&(zpix_spu.super));
	crSPUCopyDispatchTable(&(zpix_spu.super),
												 &(self->superSPU->dispatch_table));
	zpixspuGatherConfiguration(&zpix_spu);

	/* non-zero instance initialization values */
	crDebug("Zpix SPU - verbose = %d", zpix_spu.verbose);
	zpix_spu.rXold = -1;
	zpix_spu.rYold = -1;

	/* set up shadow buffers */
	for (i = 0; i < FBNUM; i++)
	{
		zpix_spu.b.fbWidth[i] = -1;
		zpix_spu.b.fbHeight[i] = -1;
		zpix_spu.b.fbLen[i] = 0;
		zpix_spu.b.fBuf[i] = NULL;
		zpix_spu.b.dBuf[i] = NULL;
		zpix_spu.zBuf[i] = NULL;
	}

	/* allocate some initial server shadows */

	/*XXX this is just a convenience since storage amount is trivial */
#define DEFAULT_NUMBER_SERVER_SHADOWS 8

	zpix_spu.n_sb = DEFAULT_NUMBER_SERVER_SHADOWS - 1;
	zpix_spu.sb = (SBUFS *) crAlloc((zpix_spu.n_sb + 1) * sizeof(SBUFS));

	/* set highest valid index */

	for (i = 0; i < zpix_spu.n_sb; i++)
	{
		zpix_spu.sb[i] = zpix_spu.b;
	}

	return &zpix_functions;
}

static void
zpixSPUSelfDispatch(SPUDispatchTable * self)
{
	crSPUInitDispatchTable(&(zpix_spu.self));
	crSPUCopyDispatchTable(&(zpix_spu.self), self);

	zpix_spu.server = (CRServer *) (self->server);
}

static int
zpixSPUCleanup(void)
{
	double avg_b, avg_zb;
	double avg_runs, avg_prefv;
	double pcz, pcprefv;
	int i = 0;
	int n = 0;

	if (0 < zpix_spu.n)
	{
		zpix_spu.sum_prefv *= sizeof(GLuint);
		avg_b = zpix_spu.sum_bytes / zpix_spu.n;
		avg_zb = zpix_spu.sum_zbytes / zpix_spu.n;
		pcz = 100 * (1.0 - (avg_zb / avg_b));

		crDebug("Zpix calls %ld, ztype = %d parm = %d, no_diff = %d",
						zpix_spu.n, zpix_spu.ztype, zpix_spu.ztype_parm,
						zpix_spu.no_diff);
		crDebug("Zpix compression %7.2f %% - avg_b %.0f avg_zb %.0f",
						pcz, avg_b, avg_zb);
		if (GL_PLE_COMPRESSION_CR == zpix_spu.ztype)
		{
			avg_runs = zpix_spu.sum_runs / zpix_spu.n;
			avg_prefv = zpix_spu.sum_prefv / zpix_spu.n;
			pcprefv = 100 * (avg_prefv / avg_b);
			crDebug("Zpix unchanged %7.2f %% - avg_runs %.0f, avg_unch %.0f",
							pcprefv, avg_runs, avg_prefv);
		}
	}

	/*
	   Return server shadow buffers
	 */
	for (n = 0; n < zpix_spu.n_sb; n++)
	{
		SBUFS *tsb;
		tsb = &zpix_spu.sb[n];
		for (i = 0; i < FBNUM; i++)
		{
			if (tsb->fBuf[i])
				crFree(tsb->fBuf[i]);
			if (tsb->dBuf[i])
				crFree(tsb->dBuf[i]);
		}
	}
	crFree(zpix_spu.sb);

	/* client compress buffers */
	for (i = 0; i < FBNUM; i++)
	{
		if (zpix_spu.zBuf[i])
			crFree(zpix_spu.zBuf[i]);
		/* mindless neatness */
		zpix_spu.b.fBuf[i] = NULL;
		zpix_spu.b.fbLen[i] = 0;
		zpix_spu.b.dBuf[i] = NULL;
		zpix_spu.zBuf[i] = NULL;
		zpix_spu.zbLen[i] = 0;
	}
	return 1;
}

int
SPULoad(char **name, char **super, SPUInitFuncPtr * init,
				SPUSelfDispatchFuncPtr * self, SPUCleanupFuncPtr * cleanup,
				SPUOptionsPtr * options, int *flags)
{
	*name = "zpix";
	*super = "passthrough";
	*init = zpixSPUInit;
	*self = zpixSPUSelfDispatch;
	*cleanup = zpixSPUCleanup;
	*options = zpixSPUOptions;
	*flags = (SPU_NO_PACKER | SPU_NOT_TERMINAL | SPU_MAX_SERVERS_ZERO);

	return 1;
}
