/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */
	
#ifndef CR_API_TEMPLATES_H
#define CR_API_TEMPLATES_H

#include "cr_spu.h"

extern SPUDispatchTable glim;

extern GLint APIENTRY crCreateContext(void *display, GLint visBits);
extern void APIENTRY crDestroyContext(void *display, GLint context);
extern void APIENTRY crMakeCurrent(void *display, GLint drawable, GLint context);
extern void APIENTRY crSwapBuffers(void *display, GLint drawable);

#endif /* CR_API_TEMPLATES_H */
