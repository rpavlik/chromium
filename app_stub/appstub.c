/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/* appstub.c */

#include <stdio.h>
#include <stdlib.h>

#if defined(WINDOWS)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

#define OPENGL_LIBRARY_NAME "opengl32.dll"

#endif

#if defined(IRIX) || defined(IRIX64) || defined(Linux)

#include <GL/gl.h>
#include <dlfcn.h>

#define APIENTRY
#define WINGDIAPI

#define OPENGL_LIBRARY_NAME "libGL.so"

static char *map_dll_error( void )
{
	char *why = dlerror( );
	if ( why[0] == 'd' && why[1] == 'l' && why[2] == 's' && why[3] == 'y' &&
			why[4] == 'm' && why[5] == ':' && why[6] == ' ' )
		why += 7;
	return why;
}

#endif

typedef WINGDIAPI void (APIENTRY * Func_V_U_Ip)( GLuint, GLint * );
typedef WINGDIAPI void (APIENTRY * Func_V_U_U)( GLuint, GLuint );
typedef WINGDIAPI void (APIENTRY * Func_V_U) ( GLuint );
typedef WINGDIAPI void (APIENTRY * Func_V_V) ( void );

static struct {
	Func_V_U_U  glBarrierCreate;
	Func_V_U    glBarrierExec;
	Func_V_U_U  glSemaphoreCreate;
	Func_V_U    glSemaphoreV;
	Func_V_U    glSemaphoreP;
	Func_V_V    crCreateContext;
	Func_V_V    crMakeCurrent;
	Func_V_V    crSwapBuffers;
	/*Func_V_U_Ip wireGLGetIntegerv; 
	 *Func_V_V    wireGLSyncWithL2; 
	 *Func_V_V    wireGLUseSystemGL; */
} fptable;

#define X(a)	{ #a, (void **) &fptable.a }

static struct {
	const char  *name;
	void       **pptr;
} functions[] = {
	X(glBarrierCreate),
	X(glBarrierExec),
	X(glSemaphoreCreate),
	X(glSemaphoreV),
	X(glSemaphoreP),
	X(crCreateContext),
	X(crMakeCurrent),
	X(crSwapBuffers),
	/*X(wireGLGetIntegerv), 
	 *X(wireGLSyncWithL2), 
	 *X(wireGLUseSystemGL) */
};

#undef X

static int first_call = 1;

#if defined(WINDOWS)
typedef HINSTANCE Library;
#else
typedef void     *Library;
#endif

static Library load_library( const char *name )
{
#if defined(WINDOWS)
	Library lib;
	char    current_dir[1024], actual_name[1024], *file_part;

	if ( !GetCurrentDirectory( sizeof(current_dir), current_dir ) )
	{
		int err = GetLastError( );
		fprintf( stderr, "GetCurrentDirectory failed, err=%d\n",
				err );
		exit( 1 );
	}

	if ( !SearchPath( NULL /* DLL search order */, 
				name, 
				NULL /* extension */,
				sizeof(actual_name),
				actual_name,
				&file_part ) )
	{
		int err = GetLastError( );
		fprintf( stderr, "SearchPath \"%s\" failed, err=%d\n",
				name, err );
		exit( 1 );
	}

	fprintf( stderr, "found \"%s\" as \"%s\"\n",
			name, actual_name );

	lib = LoadLibrary( actual_name );

	if ( lib == NULL )
	{
		int err = GetLastError( );
		fprintf( stderr, "LoadLibrary \"%s\" failed, err=%d\n",
				actual_name, err );
		exit( 1 );
	}
#else
	Library lib = dlopen( name, RTLD_NOW );

	if ( lib == NULL )
	{
		fprintf( stderr, "dlopen \"%s\" failed, err=%s\n",
				name, map_dll_error( ) );
		exit( 1 );
	}
#endif
	return lib;
}

static void *find_function( Library lib, const char *name, int must_find )
{
#if defined(WINDOWS)
	FARPROC x = GetProcAddress( lib, name );
	void *ptr = *((void **) &x);

	if ( ptr == NULL && must_find ) {
		int err = GetLastError( );
		fprintf( stderr, "%s: function not found, err=%d\n",
				name, err );
		exit( 1 );
	}
#else
	void *ptr = dlsym( lib, name );

	if ( ptr == NULL && must_find ) {
		fprintf( stderr, "%s: function not found, err=%s\n",
				name, map_dll_error( ) );
		exit( 1 );
	}
#endif
	return ptr;
}

static void init_fptable( int must_find )
{
	Library lib = load_library( OPENGL_LIBRARY_NAME );
	int i;

	for ( i = 0; i < sizeof(functions)/sizeof(functions[0]); i++ )
	{
		*(functions[i].pptr) = find_function( lib, functions[i].name, 
				must_find );
	}
	first_call = 0;
}

void APIENTRY glBarrierCreate( GLuint barrier, GLuint count )
{
	if ( first_call )
		init_fptable( 1 );

	fptable.glBarrierCreate( barrier, count );
}

void APIENTRY glBarrierExec( GLuint barrier )
{
	if ( first_call )
		init_fptable( 1 );

	fptable.glBarrierExec( barrier );
}

void APIENTRY glSemaphoreCreate( GLuint semaphore, GLuint count )
{
	if ( first_call )
		init_fptable( 1 );

	fptable.glSemaphoreCreate( semaphore, count );
}

void APIENTRY glSemaphoreV( GLuint semaphore )
{
	if ( first_call )
		init_fptable( 1 );

	fptable.glSemaphoreV( semaphore );
}

void APIENTRY glSemaphoreP( GLuint semaphore )
{
	if ( first_call )
		init_fptable( 1 );

	fptable.glSemaphoreP( semaphore );
}

void APIENTRY crCreateContext( void )
{
	if ( first_call )
		init_fptable( 1 );

	fptable.crCreateContext( );
}

void APIENTRY crMakeCurrent( void )
{
	if ( first_call )
		init_fptable( 1 );

	fptable.crMakeCurrent( );
}

void APIENTRY crSwapBuffers( void )
{
	if ( first_call )
		init_fptable( 1 );

	fptable.crSwapBuffers( );
}
