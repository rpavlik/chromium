/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */
	
#ifndef CR_API_TEMPLATES_H
#define CR_API_TEMPLATES_H

#include "cr_spu.h" 
extern SPUDispatchTable glim;

extern GLint APIENTRY crCreateContext(const char *dpyName, GLint visBits);
extern void APIENTRY crDestroyContext(GLint context);
extern void APIENTRY crMakeCurrent(GLint drawable, GLint context);
extern void APIENTRY crSwapBuffers(GLint drawable, GLint flags);

extern GLint APIENTRY crWindowCreate(const char *dpyName, GLint visBits);
extern void APIENTRY crWindowDestroy(GLint window);
extern void APIENTRY crWindowSize(GLint window, GLint w, GLint h);
extern void APIENTRY crWindowPosition(GLint window, GLint x, GLint y);

#endif /* CR_API_TEMPLATES_H */
