/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/*

  exec.h

  This is an example of GL_EXT_blend_color, described
  on page 86 of the NVidia OpenGL Extension Specifications.

  Christopher Niederauer, ccn@graphics.stanford.edu, 6/27/2001

*/

/* --- Preprocessor --------------------------------------------------------- */

#ifndef EXEC_H
#define EXEC_H


#ifdef WIN32
#pragma warning( push, 3 )
#include <windows.h>
#endif

#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glext.h>
#include <iostream>
using namespace std;

#ifndef GLX_ARB_get_proc_address
#ifdef __cplusplus
extern "C" {
#endif
extern void *glXGetProcAddressARB( const GLubyte *name );
#ifdef __cplusplus
}
#endif
#endif


/*#define	CCN_DEBUG */
#define	MULTIPLE_VIEWPORTS
#define	SMOOTH_TEXT

#define	TEST_EXTENSION_STRING	"GL_EXT_blend_color"
#ifndef	GL_EXT_blend_color
#error	Please update your GL/glext.h header file.
#endif


/* --- Global Variables ----------------------------------------------------- */

#ifdef __cplusplus
extern "C"
{
#endif
	/* exec.cpp */
	extern GLuint	currentWidth, currentHeight; /* Window width/height. */
#ifdef __cplusplus
}
#endif


/* --- Function Prototypes -------------------------------------------------- */

/* main.cpp */
void	RenderString	( float, float, char* );

/* logo.c */
#ifdef __cplusplus
extern "C"
{
#endif
	void	DrawLogo	( void );
#ifdef __cplusplus
}
#endif

/* exec.cpp */
void	InitGL		( void );
void	InitSpecial	( void );
void	Idle		( void );
void	Display		( void );
void	Reshape		( int, int );
void	Keyboard	( unsigned char, int, int );
void	Mouse		( int, int, int, int );
void	Motion		( int, int );
void	Special		( int, int, int );

#endif /* EXEC_H */
