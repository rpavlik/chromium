/*

  exec.h

  This is an example of GL_EXT_blend_subtract, described
  on page 92 of the NVidia OpenGL Extension Specifications.

  Christopher Niederauer, ccn@graphics.stanford.edu, 6/29/2001

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

//#define	CCN_DEBUG
#define DISPLAY_LISTS
#define	MULTIPLE_VIEWPORTS
#define	SMOOTH_TEXT

#define	TEST_EXTENSION_STRING	"GL_EXT_blend_subtract"
#ifndef	GL_EXT_blend_subtract
#error	Please update your GL/glext.h header file.
#endif


/* --- Function Prototypes -------------------------------------------------- */

// main.cpp
void	RenderString	( float, float, char* );

// exec.cpp
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
