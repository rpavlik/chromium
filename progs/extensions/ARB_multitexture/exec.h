/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*

  exec.h

  This is an example of GL_ARB_multitexture, described
  on page 18 of the NVidia OpenGL Extension Specifications.

  Christopher Niederauer, ccn@graphics.stanford.edu, 6/25/2001

*/

/* --- Preprocessor --------------------------------------------------------- */

#ifndef EXEC_H
#define EXEC_H

#ifdef WIN32
#pragma warning( push, 3 )
#include <windows.h>
#endif

#include "cr_glwrapper.h"
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glext.h>
#include <iostream.h>


/*#define	CCN_DEBUG */
#define	DISPLAY_LISTS
#define	MULTIPLE_VIEWPORTS
#define	SMOOTH_TEXT

#define	TEST_EXTENSION_STRING	"GL_ARB_multitexture"
#ifndef	GL_ARB_multitexture
#error	Please update your GL/glext.h header file.
#endif


/* --- Function Prototypes -------------------------------------------------- */

/* main.cpp */
void	RenderString	( float, float, char* );

/* exec.cpp */
void	InitGL		( void );
void	InitSpecial	( void );
void	Idle		( void );
void	Display		( void );
void	Reshape		( int, int );
void	Keyboard	( unsigned char, int, int );

#endif /* EXEC_H */
