#ifndef WINDOWS
#include <unistd.h>
#endif
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <memory.h>
#include <assert.h>

#include "cr_error.h"
#include "cr_string.h"
#include "cr_mem.h"

#include "dist_texturespu.h"

static char ppmHeader[]      = "P6\n000000 000000\n255\n" ;
static char ppmHeaderTempl[] = "P6\n%d %d\n255\n" ;
static GLubyte* buffer ;
static int bufferSize ;
#if 0
static int complaint = 0 ;

/* complain when complaint is 0 or a power of 2 */
static int complain_now()
{
	int n = complaint ;
	while ( n && (!( n & 1 )) )
		n >>= 1 ;
	if ( n > 1 )
		return 0 ;
	return 1 ;
}
#endif

static int next_power_of_two( int n )
{
	int power = 1 ;

	if ( n == 0 )
		return 0 ;

	while ( power < n ) {
		power <<= 1 ;
	}
	return power ;
}

static void calc_padded_size( int w, int h, int* pw, int* ph )
{
	*pw = next_power_of_two( w ) ;
	*ph = next_power_of_two( h ) ;
}

#ifdef WINDOWS
static int check_match( HFILE file, char* compare )
#else
static int check_match( int file, char* compare )
#endif
{
#ifdef WINDOWS
	int _readbytes;
#endif
	char c ;
	while (*compare) {
#ifdef WINDOWS
		ReadFile( (HANDLE) file, &c, 1, &_readbytes, NULL );
#else
		read( file, &c, 1 ) ;
#endif
		if ( c != *compare ) {
			crWarning( "File is not a PPM" ) ;
			return 0 ;
		}
		compare++ ;
	}
	return 1 ;
}

#ifdef WINDOWS
static void skip_line( HFILE file )
#else
static void skip_line( int file )
#endif
{
#ifdef WINDOWS
	int _readbytes;
	char c ;
	while ( (ReadFile( (HANDLE) file, &c, 1, &_readbytes, NULL ) == 1) &&
#else
	char c ;
	while ( (read(file, &c, 1 ) == 1) &&
#endif
		(c != '\n') ) 
		;
	return ;
}

#ifdef WINDOWS
static int read_int( HFILE file, int* number )
#else
static int read_int( int file, int* number )
#endif
{
#ifdef WINDOWS
	int _readbytes;
#endif
	char c ;
	int digit = -1 ;
	int n = 0 ;
	do {
#ifdef WINDOWS
		if ( ReadFile( (HANDLE) file, &c, 1, &_readbytes, NULL ) != 1) {
#else
		if ( read( file, &c, 1 ) != 1 ) {
#endif
			crWarning( "Read failed trying to get ASCII integer" ) ;
			return -1 ;
		}

		if ( ! isspace( (int)c ) ) {
		     if ( ! isdigit( (int)c ) ) {
				if ( c == '#' )
					skip_line( file ) ;
				else
					return -2 ;
			}
			digit++ ;
			n *= 10 ;
			n += c - '0' ;
		} else {
			if ( digit >= 0 ) {
				*number = n ;
				return 0 ; /* success, we reached the space on the other side */
			}
		}
	} while ( 1 ) ; 

	/*return -1 ;*/
}

static void pad_buffer_texture( GLubyte* dst, const GLubyte* src, int w, int h, int pw, int ph )
{
	int i ;
	for (i=0; i<h; i++)
		crMemcpy( (void*) &dst[ 3*i*pw ], (void*) &src[ 3*i*w ], 3*w ) ;
}

#ifdef WINDOWS
static void pad_file_texture( GLubyte* dst, HFILE file, int w, int h, int pw, int ph)
#else
static void pad_file_texture( GLubyte* dst, int file, int w, int h, int pw, int ph )
#endif
{
#ifdef WINDOWS
	int _readbytes;
#endif
	int i ;
	for (i=0; i<h; i++)
#ifdef WINDOWS
		ReadFile( (HANDLE) file, (void*) &dst[pw*3*i], w*3, &_readbytes, NULL );
#else
		read( file, (void*) &dst[pw*3*i], w*3 ) ;
#endif
}

#if 0
static void pad_gzfile_texture( GLubyte* dst, gzFile file, int w, int h, int pw, int ph )
{
	int i ;
	for (i=0; i<h; i++)
		gzread( f, (void*) &dst[pw*3*i], w*3 ) ;
}
#endif /* 0 */

void DIST_TEXTURESPU_APIENTRY dist_textureTexImage2D( 
		GLenum target,
		GLint level,
		GLint internalformat,
		GLsizei width,
		GLsizei height,
		GLint border,
		GLenum format,
		GLenum type,
		const GLvoid *pixels )
{
	if ( type != GL_TRUE && type != GL_FALSE ) {
		dist_texture_spu.child.TexImage2D(target, level, internalformat,
						  width, height, border, format,
						  type, pixels ) ;
		return ;
	} else {
#ifdef WINDOWS
		int writing = (type == GL_TRUE) ?
				( OF_WRITE | OF_CREATE) :
				OF_READ ;
		HFILE f;
		LPOFSTRUCT OpenBuff = NULL;
		int _writebytes;
#else
		int writing = (type == GL_TRUE) ?
				( O_WRONLY | O_CREAT | O_TRUNC | O_NDELAY ) :
				O_RDONLY ;
		int f ;
#endif
		int filenameLength ;
		int i ;
		int tmp = 0;
		int paddedWidth = 0;
		int paddedHeight = 0;

		assert( pixels ) ;

		filenameLength = crStrlen( pixels ) ;
#ifdef WINDOWS
		f = OpenFile( pixels, OpenBuff, writing );
		if ( f == HFILE_ERROR ) {
#else 
		f = open( pixels, writing, S_IRUSR|S_IWUSR ) ;
		if ( f < 0 ) {
#endif
			crWarning( "Could not open file <%s>", (char*)pixels ) ;
			return ;
		}
		switch (type) {
			case GL_TRUE:
				i=sprintf( ppmHeader, ppmHeaderTempl, width, height );
#ifdef WINDOWS
				WriteFile( (HANDLE) f, ppmHeader, i, &_writebytes, NULL ) ;
				WriteFile( (HANDLE) f, (char*) pixels + filenameLength + 1, width*height*3, &_writebytes, NULL ) ;
#else
				write( f, ppmHeader, i ) ;
				write( f, (char*) pixels + filenameLength + 1, width*height*3 ) ;
#endif
				/* Create padded texture */
				calc_padded_size( width, height, &paddedWidth, &paddedHeight ) ;
				if ( (paddedWidth == width) && (paddedHeight == height) ) {
					tmp = 1 ;
					break ;
				}
				tmp = 0 ;
				i = paddedWidth*paddedHeight*3 ;
				if ( ! buffer ) {
					bufferSize = i ;
					buffer = crAlloc( bufferSize ) ;
				} else if ( bufferSize < i ) {
					bufferSize = i ;
					crRealloc( (void **)&buffer, bufferSize ) ;
				}
				pad_buffer_texture( buffer, (const GLubyte *) pixels + filenameLength + 1,
						width, height, paddedWidth, paddedHeight ) ;
				break ;
			case GL_FALSE:
#ifdef WINDOWS
				if ( ! check_match( f, "P6" ) ) { CloseHandle( (HANDLE) f ) ; return ; }
				if ( read_int( f, &width ) ) { CloseHandle( (HANDLE) f ) ; return ; }
				if ( read_int( f, &height ) ) { CloseHandle( (HANDLE) f ) ; return ; }
				if ( read_int( f, &tmp ) ) { CloseHandle( (HANDLE) f ) ; return ; }
				if ( tmp != 255 ) {
					crWarning( "PPM file isn't GL_UNSIGNED_BYTE format" ) ;
					CloseHandle( (HANDLE) f ) ;
					return ;
				}
#else
				if ( ! check_match( f, "P6" ) ) { close( f ) ; return ; }
				if ( read_int( f, &tmp ) ) { close( f ) ; return ; }
				width = (GLsizei) tmp;
				if ( read_int( f, &tmp ) ) { close( f ) ; return ; }
				height = (GLsizei) tmp;
				if ( read_int( f, &tmp ) ) { close( f ) ; return ; }
				if ( tmp != 255 ) {
					crWarning( "PPM file isn't GL_UNSIGNED_BYTE format" ) ;
					close( f ) ;
					return ;
				}
#endif
				calc_padded_size( width, height, &paddedWidth, &paddedHeight ) ;
				i = paddedWidth*paddedHeight*3 ;
				if ( ! buffer ) {
					bufferSize = i ;
					buffer = crAlloc( bufferSize ) ;
				} else if ( bufferSize < i ) {
					bufferSize = i ;
					crRealloc( (void **)&buffer, bufferSize ) ;
				}
				pad_file_texture( buffer, f, width, height, paddedWidth, paddedHeight ) ;
				break ;
			default:
				crError( "Can't touch this." ) ;
		}
#ifdef WINDOWS
		CloseHandle( (HANDLE) f );
#else
		close( f ) ;
#endif

		if ( (type == GL_TRUE) && tmp )
			/* no need to generate padded texture, since width & height are powers of 2 */
			dist_texture_spu.child.TexImage2D(target, level, internalformat,
			       			  width, height, border, format,
						  GL_UNSIGNED_BYTE, (GLubyte*) pixels + filenameLength + 1 ) ;
		else
			/* had to generate padded texture */
			dist_texture_spu.child.TexImage2D(target, level, internalformat,
			       			  paddedWidth, paddedHeight, border, format,
						  GL_UNSIGNED_BYTE, buffer ) ;
		/*free( buffer ) ;*/
	}
}
