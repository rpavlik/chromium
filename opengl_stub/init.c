/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */
	
#include "cr_spu.h"
#include "api_templates.h"

#ifdef WINDOWS
/* Let me cast function pointers to data pointers, I know what I'm doing. */
#pragma warning( disable: 4054 )
#endif

SPUDispatchTable glim;

void FakerInit( SPU *spu )
{
	crSPUInitDispatchTable( &glim );
	crSPUCopyDispatchTable( &glim, &(spu->dispatch_table) );
}
