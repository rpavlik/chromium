/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/* 
 * This program fakes an application into using the Chromium client library.
 * It's mostly based on the 'wgl' application from WireGL, which in turn was
 * based on the 'pomgl' application, written by Matthew Eldridge for
 * his Pomegranate simulator.
 */


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#include <direct.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#endif

#include "cr_mothership.h"
#include "cr_mem.h"
#include "cr_environment.h"
#include "cr_string.h"
#include "cr_error.h"

#ifdef _WIN32

#define DEFAULT_TMP_DIR "C:\\"
#define OPENGL_CLIENT_LIB "crfaker.dll"

static const char *libgl_names[] = {
	"opengl32.dll"
};

#else /* _WIN32 */

#define DEFAULT_TMP_DIR "/tmp"
#if defined(AIX)
#define OPENGL_CLIENT_LIB "libcrfaker.a"
#else
#define OPENGL_CLIENT_LIB "libcrfaker.so"
#endif

#if defined(IRIX) || defined(IRIX64)
#define SYSTEM_LIB_DIR  "/usr/lib32"
#else
#define SYSTEM_LIB_DIR  "/usr/lib"
#endif

static const char *libgl_names[] = {
#ifdef AIX
	"libGL.a"
#else
	"libGL.so"
#endif
};

#endif /* _WIN32 */

static const char *progname   = "app_faker";
static const char *cr_lib     = NULL;
static int         verbose    = 0;
CRConnection *mothership_conn;

#ifdef _WIN32

void print_argv( char **argv )
{
	char **temp;
	for (temp = argv; *temp; temp++)
	{
		fprintf( stderr, " %s", *temp );
	}
	fprintf( stderr, "\n" );
}

const char *error_string( int err )
{
	char *buf;

	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM, NULL, err,
			MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
			(LPTSTR) &buf, 0, NULL );
	return buf;
}

#endif

void debug( const char *format, ... )
{
	if ( verbose ) {
		va_list args;
		fprintf( stderr, "%s: ", progname );
		va_start( args, format );
		vfprintf( stderr, format, args );
		va_end( args );
	}
}

const char *basename( const char *path )
{
	char *last;
	last = crStrrchr( path, '/' );
	if ( !last )
		return path;
	else
		return last + 1;
}

typedef struct List {
	char           *name;
	struct List *next;
} List;

static List *temp_files = NULL;
static List *temp_dirs  = NULL;

void add_to_list( List **list, const char *name )
{
	List *node = (List *) crAlloc( sizeof(*node) );

	node->name = crStrdup( name );
	node->next = *list;
	*list = node;
}

void add_file_to_temp_list( const char *filename )
{
	add_to_list( &temp_files, filename );
}

void delete_temp_files( void )
{
	List *list;

#ifdef _WIN32
	/* we seem to be in a race with a file lock, see if this fixes the
	 * problem */
	Sleep( 100 /* milliseconds */ );
#endif

	for ( list = temp_files; list; list = list->next ) {
#ifdef _WIN32
		if ( !DeleteFile( list->name ) )
			crError( "DeleteFile \"%s\"", list->name );
#else
		if ( unlink( list->name ) )
			crError( "unlink \"%s\"", list->name );
#endif
	}
}

void add_dir_to_temp_list( const char *dirname )
{
	add_to_list( &temp_dirs, dirname );
}

void delete_temp_dirs( void )
{
	List *list;
	for ( list = temp_dirs; list; list = list->next ) {
#ifdef _WIN32
		if ( !RemoveDirectory( list->name ) )
			crError( "RemoveDirectory \"%s\"", list->name );
#else
		if ( rmdir( list->name ) )
			crError( "rmdir \"%s\"", list->name );
#endif
	}
}

#ifdef _WIN32

char *find_file_on_path( const char *name )
{
	int   len;
	char *path, *tail;

	len = SearchPath( NULL, name, NULL, 0, NULL, &tail );
	if ( len == 0 ) {
		crError( "Couldn't find \"%s\" on the path.", name );
		return NULL;
	}

	len += 2;
	path = (char *) crAlloc( len + 12 );

	if ( !SearchPath( NULL, name, NULL, len, path, &tail ) ) {
		crError( "Couldn't find \"%s\" on the path", name );
		return NULL;
	}

	return path;
}

char *find_executable_on_path( const char *name, char **tail_ptr )
{
	int   len;
	char *path, *tail;

	len = SearchPath( NULL, name, ".exe", 0, NULL, &tail );
	if ( len == 0 )
		crError( "Couldn't find \"%s\" on the path", name );

	len+=2;
	path = (char *) crAlloc( len + 12 );

	if ( !SearchPath( NULL, name, ".exe", len, path, &tail ) )
		crError( "Couldn't find \"%s\" on the path", name );

	if ( tail_ptr )
		*tail_ptr = tail;

	return path;
}

void make_tmpdir( char *retval )
{
	char *name;

	name = _tempnam( DEFAULT_TMP_DIR, "cr" );
	if ( name == NULL )
		crError( "cannot create a unique directory name\n" );

	debug( "tmpdir=\"%s\"\n", name );

	if ( !CreateDirectory( name, NULL ) )
		crError( "CreateDirectory \"%s\"", name );

	crStrcpy( retval, name );
}

void copy_file( const char *dst_filename, const char *src_filename )
{
	debug( "copying \"%s\" -> \"%s\"\n", src_filename, dst_filename );

	if ( !CopyFile( src_filename, dst_filename, 1 /* fail if exists */ ) )
		crError( "copy \"%s\" -> \"%s\"", src_filename, dst_filename );
}

void do_it( char *argv[] )
{
	char tmpdir[1024], argv0[1024], *tail;
	char response[8096];
	int i, status;

	if ( cr_lib == NULL ) {
		debug( "searching for client library \"%s\" on PATH\n",
				OPENGL_CLIENT_LIB );
		cr_lib = find_file_on_path( OPENGL_CLIENT_LIB );
	}

	if ( cr_lib == NULL ) {
		if (crMothershipGetClientDLL( mothership_conn, response ) )
		{
			cr_lib = crStrdup( response );
		}
	}

	if ( cr_lib == NULL ) {
		crError( "I don't know where to find the client library.  You could "
				"have set CR_FAKER_LIB, but didn't.  You could have used -lib "
				"on the command line, but didn't.  I searched the PATH for "
				"\"%s\", but couldn't find it.  I even asked the configuration "
				"manager!\n", OPENGL_CLIENT_LIB );
	}

	debug( "cr_lib=\"%s\"\n", cr_lib );

	make_tmpdir( tmpdir );
	add_dir_to_temp_list( tmpdir );

	argv[0] = find_executable_on_path( argv[0], &tail );
	crStrcpy( argv0, tmpdir );
#ifdef WINDOWS
	crStrcat( argv0, "\\" );
#else
	crStrcat( argv0, "/" );
#endif
	crStrcat( argv0, tail );

	copy_file( argv0, argv[0] );

	argv[0] = argv0;

	add_file_to_temp_list( argv0 );

	for ( i = 0; i < sizeof(libgl_names)/sizeof(libgl_names[0]); i++ ) {
		char name[1024];
		crStrcpy( name, tmpdir );
		crStrcat( name, "\\" );
		crStrcat( name, libgl_names[i] );
		copy_file( name, cr_lib );
		add_file_to_temp_list( name );
	}

	status = spawnv( _P_WAIT, argv[0], argv );

	if ( status == -1 )
		crError( "Couldn't spawn \"%s\".", argv[0] );
	else if ( status > 0 )
		crWarning( "\"%s\": exited with status=%d\n", argv[0], status );

	delete_temp_files( );
	delete_temp_dirs( );

	exit( status ? 1 : 0 );
}

#else /* _WIN32 */

char *find_file_on_path( const char *path, const char *basename )
{
	int  i;
	char name[1024];
	struct stat stat_buf;

	assert( path && basename );

	while ( *path ) {

		/* initialize name to the next path element */
		i = 0;
		while ( *path && *path != ':' )
			name[i++] = *path++;

		if ( *path == ':' )
			path++;
		name[i] = 0;

		/* ignore empty paths */
		if ( i == 0 ) continue;

		if ( name[0] == '.' ) {
			if ( !getcwd( name, sizeof(name) ) ) {
				crError( "find_on_path: getcwd" );
				continue;
			}
			i = crStrlen( name );
		}
		else if ( name[0] != '/' ) {
			crError( "find_on_path: relative paths anger me (%s)", name );
			continue;
		}

		if ( name[i-1] != '/' && name[i-1] != '\\' )
			name[i++] = '/';
		crStrcpy( name+i, basename );

		if ( stat( name, &stat_buf ) ) {
			/* continue if the stat fails */
			if ( errno != ENOENT ) {
				/* complain if the problem isn't just a missing file */
				crError( "find_on_path: stat( \"%s\" )", name );
			}
			errno = 0;
			continue;
		}

		return crStrdup( name );
	}

	return NULL;
}

void make_tmpdir( char *name )
{
	char *tmp;
	int   index;

	tmp = crGetenv( "TMP" );
	if ( !tmp )
		tmp = crGetenv( "TEMP" );
	if ( !tmp )
		tmp = DEFAULT_TMP_DIR;

	sprintf( name, "%s/cr%d", tmp, (int) getpid( ) );

	index = 0;
	while ( mkdir( name, 0777 ) )
	{
		index++;

		crError( "mkdir \"%s\"", name );

		if ( errno != EEXIST )
			exit( 1 );

		sprintf( name, "%s/cr%d.%d", tmp, (int) getpid( ), index );
	}

	debug( "tmpdir=\"%s\"\n", name );
}

void prefix_env_var( const char *prefix, const char *varname )
{
	char *val;

	val = crGetenv( varname );
	if ( val ) {
		unsigned long len = crStrlen( prefix ) + 1 + crStrlen( val ) + 1;
		char *buf = (char *) crAlloc( len );
		sprintf( buf, "%s:%s", prefix, val );
		crSetenv( varname, buf );
	} else {
		crSetenv( varname, prefix );
	}
}

int is_a_version_of( const char *basename, const char *libname )
{
	int len = crStrlen( basename );

	/* names must match */
	debug( "%s -> %s (%d)\n", basename, libname, len );
	if ( crStrncmp( libname, basename, len ) ) return 0;

	libname += len;
	/* is this a version? */
	if ( *libname != '.' ) return 0;

	/* remainder of libname should be digits and periods */
	while ( *libname == '.' || isdigit(*libname) )
		libname++;

	/* did we make it to the end of the libname? */
	return ( *libname == '\0' );
}

const char *find_next_version_name( DIR *dir, const char *basename )
{
	struct dirent *entry;

	entry = readdir( dir );
	while ( entry ) {
		if ( is_a_version_of( basename, entry->d_name ) )
		{
			debug( entry->d_name );
			return entry->d_name;
		}
		entry = readdir( dir );
	}

	/* out of entries */
	return NULL;
}

int make_temp_link( const char *dir, const char *name, const char *target )
{
	char link_name[1024];

	crStrcpy( link_name, dir );
	crStrcat( link_name, "/" );
	crStrcat( link_name, name );

	if ( symlink( target, link_name ) ) {
		crError( "link \"%s\" -> \"%s\"", link_name, target );
		return 0;
	}

	add_file_to_temp_list( link_name );
	return 1;
}

void do_it( char *argv[] )
{
	pid_t pid;
	int status;
	char tmpdir[1024];
	int i;
	char response[8096];
	struct stat stat_buf;

	if ( cr_lib == NULL ) {
		const char *path;

#if defined(IRIX) || defined(IRIX64)
		path = crGetenv( "LD_LIBRARYN32_PATH" );
		if ( !path )
			path = crGetenv( "LD_LIBRARY_PATH" );
#else
		path = crGetenv( "LD_LIBRARY_PATH" );
#if defined(AIX)
		if(!path) path = crGetenv("LIBPATH");
		fprintf(stderr,"libpath=%s\n",path);
#endif
#endif
		if ( path ) {
			debug( "searching for client library \"%s\" in \"%s\"\n",
					OPENGL_CLIENT_LIB, path );
			cr_lib = find_file_on_path( path, OPENGL_CLIENT_LIB );
		}
	}

	if ( cr_lib == NULL ) {
		if (crMothershipGetClientDLL( mothership_conn, response ) )
		{
			cr_lib = crStrdup( response );
		}
	}

	if ( cr_lib == NULL ) {
		crError( "I don't know where to find the client library.  You could "
				"have set CR_FAKER_LIB, but didn't.  You could have used -lib "
				"on the command line, but didn't.  I searched the LD_LIBRARY_PATH "
#if defined(IRIX) || defined(IRIX64)
				"(actually, I looked in LD_LIBRARYN32_PATH first) "
#endif
				"for \"%s\", but couldn't find it.  I even asked the "
				"mothership!\n", OPENGL_CLIENT_LIB );
	}

	debug( "cr_lib=\"%s\"\n", cr_lib );

	if ( stat( cr_lib, &stat_buf ) )
		crError( "\"%s\"", cr_lib );

	if ( !S_ISREG(stat_buf.st_mode) )
		crError( "\"%s\" isn't a regular file?\n", cr_lib );

	make_tmpdir( tmpdir );

	add_dir_to_temp_list( tmpdir );

	for ( i = 0; i < sizeof(libgl_names)/sizeof(libgl_names[0]); i++ ) {

		DIR *dir;

		make_temp_link( tmpdir, libgl_names[i], cr_lib );

		dir = opendir( SYSTEM_LIB_DIR );
		if ( dir ) {
			const char *version_name;
			version_name = find_next_version_name( dir, libgl_names[i] );
			while ( version_name ) {
				debug( "found version of \"%s\" as \"%s\", linking it\n", 
						libgl_names[i], version_name );
				make_temp_link( tmpdir, version_name, cr_lib );
				version_name = find_next_version_name( dir, libgl_names[i] );
			}
			closedir( dir );
		} else {
			crError( "opendir( \"%s\" )", SYSTEM_LIB_DIR );
		}
	}

#ifdef AIX
	prefix_env_var( tmpdir, "LIBPATH" );
#else
	prefix_env_var( tmpdir, "LD_LIBRARY_PATH" );
#endif

#if defined(IRIX) || defined(IRIX64)
	/* if these are set then they'll be used by rld, so we have to
     * make sure to modify them. */
	if ( crGetenv( "LD_LIBRARYN32_PATH" ) )
		prefix_env_var( tmpdir, "LD_LIBRARYN32_PATH" );

	if ( crGetenv( "LD_LIBRARY64_PATH" ) )
		prefix_env_var( tmpdir, "LD_LIBRARY64_PATH" );
#endif

    debug( "contacting mothership to determine SPU directory \n");
	if (crMothershipGetSPUDir( mothership_conn, response ))
	{
		crSetenv( "SPU_DIR", response );
	}

	pid = fork( );
	if ( pid < 0 )
		crError( "fork" );

	if ( pid == 0 ) {
		execvp( argv[0], argv );
		crError( "execvp \"%s\" failed (verify program name is correct)", argv[0] );
	}

	debug( "started \"%s\" [%d]\n", argv[0], pid );

	do {
		while ( wait( &status ) != pid )
			;
	} while ( WIFSTOPPED(status) );

	delete_temp_files( );
	delete_temp_dirs( );

	if ( WIFEXITED(status) ) {
		debug( "\"%s\" exited, status=%d\n", argv[0], WEXITSTATUS(status) );
		exit( WEXITSTATUS(status) );
	}

	if ( WIFSIGNALED(status) ) {
		debug( "\"%s\" uncaught signal=%d\n", argv[0], WTERMSIG(status) );
		exit( 1 );
	}

	debug( "\"%s\" exited, not sure why\n", argv[0] );
	exit( 1 );
}

#endif /* _WIN32 */

	void
usage( void )
{
	char *cr_lib;

	fprintf( stderr,
			"usage: %s [options] <cmd> [args]\n"
			"options:\n"
			"   -lib    <file> : name of the replacement opengl32.dll\n"
			"   -v[erbose]\n"
			"   -q[uiet]\n"
			"   -h[elp]        : this message\n"
			"\n"
			"The environment variable CR_FAKER_LIB specifies the default -lib,\n",
			progname );

	cr_lib = crGetenv( "CR_FAKER_LIB" );
	if ( cr_lib )
		fprintf( stderr, "currently set to \"%s\".\n", cr_lib );
	else
		fprintf( stderr, "currently not set.\n" );

	fputs( "If CR_FAKER_LIB isn't set the "
#if defined(_WIN32)
			"PATH "
#elif defined(IRIX) || defined(IRIX64)
			"LD_LIBRARYN32_PATH (and LD_LIBRARY_PATH) "
#else
			"LD_LIBRARY_PATH "
#endif
			"is \nsearched for \"" OPENGL_CLIENT_LIB "\".\n", stderr );
}

int main( int argc, char **argv )
{
	int i;
	char *mothership;
	char **faked_argv = NULL;
	char response[1024];
	char **chain;

	progname = basename( argv[0] );

	cr_lib = NULL;
	mothership = NULL;

	for ( i = 1; i < argc; i++ ) {

		if ( argv[i][0] != '-' )
			break;

		if ( !crStrcmp( argv[i], "-lib" ) ) {

			i++;
			if ( i == argc )
				crError( "%s expects argument\n", argv[i-1] );

			cr_lib = argv[i];
		}
		else if ( !crStrcmp( argv[i], "-mothership" ) ||
				!crStrcmp( argv[i], "-m" )) {
			i++;
			if ( i == argc )
				crError( "%s expects argument\n", argv[i-1] );

			mothership = argv[i];
		}
		else if ( !crStrcmp( argv[i], "-mesa" ) ) 
		{
			char *mesa_path = crGetenv( "CR_MESA_LIB_PATH" );
			if (mesa_path)
			{
				crSetenv( "CR_SYSTEM_GL_PATH", mesa_path );
			}
		}
		else if ( !crStrcmp( argv[i], "-v" ) ||
				!crStrcmp( argv[i], "-verbose" ) ) {
			verbose = 1;
		}
		else if ( !crStrcmp( argv[i], "-q" ) ||
				!crStrcmp( argv[i], "-quiet" ) ) {
			verbose = 0;
		}
		else if ( !crStrcmp( argv[i], "-h" ) ||
				!crStrcmp( argv[i], "-help" ) ) {

			usage( );
			exit( 0 );
		}
		else {

			crError( "unknown argument: %s\n", argv[i] );
			usage( );
			exit( 1 );
		}
	}

	argc -= i;
	argv += i;


	if (mothership)
		crSetenv( "CRMOTHERSHIP", mothership );
	mothership_conn = crMothershipConnect( );
	if (!mothership_conn)
	{
		crError( "Couldn't connect to the mothership -- I have no idea what to do!" );
	}

	crMothershipIdentifyFaker( mothership_conn, response );
	chain = crStrSplitn( response, " ", 1 );

	/* the OpenGL faker DLL needs to have a unique ID in case there 
	* are multiple apps running on the same host. */
	crSetenv( "CR_APPLICATION_ID_NUMBER", chain[0] );

	if ( argc < 1 )
	{
		/* No command specified, contact the configuration server to 
		* ask what I should do. */

		faked_argv = crStrSplit( chain[1], " " );

		crMothershipGetStartdir( mothership_conn, response );

		if (chdir( response ))
		{
			crError( "Couldn't change to the starting directory: %s", response );
		}
	}
	else
	{
		faked_argv = argv;
	}

	if ( cr_lib == NULL ) {
		cr_lib = crGetenv( "CR_FAKER_LIB" );
		if ( cr_lib )
			debug( "using CR_FAKER_LIB=\"%s\" for library\n", cr_lib );
	}

	if ( !crGetenv("DISPLAY") ) {
		debug( "DISPLAY not set, defaulting to :0.0\n" );
		crSetenv( "DISPLAY", ":0.0" );
	}

	/* This will let SPUS do things differently if they want. */
	crSetenv( "__CR_LAUNCHED_FROM_APP_FAKER", "yes" );
	do_it( faked_argv );

	return 0;
}
