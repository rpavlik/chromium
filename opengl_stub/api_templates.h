/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */
	
#ifndef CR_API_TEMPLATES_H
#define CR_API_TEMPLATES_H

#include "cr_spu.h"

extern SPUDispatchTable glim;

extern void APIENTRY crCreateContext(void);
extern void APIENTRY crMakeCurrent(void);
extern void APIENTRY crSwapBuffers(void);

#endif /* CR_API_TEMPLATES_H */
