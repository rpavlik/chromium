#include "cr_packfunctions.h"
#include "cr_pack.h"
#include "cr_opcodes.h"
#include <GL/gl.h>

void PACK_APIENTRY crPackMultMatrixd( const GLdouble *m )
{
	unsigned char *data_ptr;
	int packet_length = 16*sizeof( *m );
	GET_BUFFERED_POINTER( packet_length );
	WRITE_DOUBLE( 0*sizeof(double), m[ 0] );
	WRITE_DOUBLE( 1*sizeof(double), m[ 1] );
	WRITE_DOUBLE( 2*sizeof(double), m[ 2] );
	WRITE_DOUBLE( 3*sizeof(double), m[ 3] );
	WRITE_DOUBLE( 4*sizeof(double), m[ 4] );
	WRITE_DOUBLE( 5*sizeof(double), m[ 5] );
	WRITE_DOUBLE( 6*sizeof(double), m[ 6] );
	WRITE_DOUBLE( 7*sizeof(double), m[ 7] );
	WRITE_DOUBLE( 8*sizeof(double), m[ 8] );
	WRITE_DOUBLE( 9*sizeof(double), m[ 9] );
	WRITE_DOUBLE( 10*sizeof(double), m[10] );
	WRITE_DOUBLE( 11*sizeof(double), m[11] );
	WRITE_DOUBLE( 12*sizeof(double), m[12] );
	WRITE_DOUBLE( 13*sizeof(double), m[13] );
	WRITE_DOUBLE( 14*sizeof(double), m[14] );
	WRITE_DOUBLE( 15*sizeof(double), m[15] );
	WRITE_OPCODE( CR_MULTMATRIXD_OPCODE );
}

void PACK_APIENTRY crPackMultMatrixf( const GLfloat *m )
{
	unsigned char *data_ptr;
	int packet_length = 16*sizeof( *m );
	GET_BUFFERED_POINTER( packet_length ); 
	WRITE_DATA( 0*sizeof(float), float, m[ 0] );
	WRITE_DATA( 1*sizeof(float), float, m[ 1] );
	WRITE_DATA( 2*sizeof(float), float, m[ 2] );
	WRITE_DATA( 3*sizeof(float), float, m[ 3] );
	WRITE_DATA( 4*sizeof(float), float, m[ 4] );
	WRITE_DATA( 5*sizeof(float), float, m[ 5] );
	WRITE_DATA( 6*sizeof(float), float, m[ 6] );
	WRITE_DATA( 7*sizeof(float), float, m[ 7] );
	WRITE_DATA( 8*sizeof(float), float, m[ 8] );
	WRITE_DATA( 9*sizeof(float), float, m[ 9] );
	WRITE_DATA( 10*sizeof(float), float, m[10] );
	WRITE_DATA( 11*sizeof(float), float, m[11] );
	WRITE_DATA( 12*sizeof(float), float, m[12] );
	WRITE_DATA( 13*sizeof(float), float, m[13] );
	WRITE_DATA( 14*sizeof(float), float, m[14] );
	WRITE_DATA( 15*sizeof(float), float, m[15] );
	WRITE_OPCODE( CR_MULTMATRIXF_OPCODE );
}

void PACK_APIENTRY crPackLoadMatrixd( const GLdouble *m )
{
	unsigned char *data_ptr;
	int packet_length = 16*sizeof( *m );
	GET_BUFFERED_POINTER( packet_length );
	WRITE_DOUBLE( 0*sizeof(double), m[ 0] );
	WRITE_DOUBLE( 1*sizeof(double), m[ 1] );
	WRITE_DOUBLE( 2*sizeof(double), m[ 2] );
	WRITE_DOUBLE( 3*sizeof(double), m[ 3] );
	WRITE_DOUBLE( 4*sizeof(double), m[ 4] );
	WRITE_DOUBLE( 5*sizeof(double), m[ 5] );
	WRITE_DOUBLE( 6*sizeof(double), m[ 6] );
	WRITE_DOUBLE( 7*sizeof(double), m[ 7] );
	WRITE_DOUBLE( 8*sizeof(double), m[ 8] );
	WRITE_DOUBLE( 9*sizeof(double), m[ 9] );
	WRITE_DOUBLE( 10*sizeof(double), m[10] );
	WRITE_DOUBLE( 11*sizeof(double), m[11] );
	WRITE_DOUBLE( 12*sizeof(double), m[12] );
	WRITE_DOUBLE( 13*sizeof(double), m[13] );
	WRITE_DOUBLE( 14*sizeof(double), m[14] );
	WRITE_DOUBLE( 15*sizeof(double), m[15] );
	WRITE_OPCODE( CR_LOADMATRIXD_OPCODE );
}

void PACK_APIENTRY crPackLoadMatrixf( const GLfloat *m )
{
	unsigned char *data_ptr;
	int packet_length = 16*sizeof( *m );
	GET_BUFFERED_POINTER( packet_length );
	WRITE_DATA( 0*sizeof(float), float, m[ 0] );
	WRITE_DATA( 1*sizeof(float), float, m[ 1] );
	WRITE_DATA( 2*sizeof(float), float, m[ 2] );
	WRITE_DATA( 3*sizeof(float), float, m[ 3] );
	WRITE_DATA( 4*sizeof(float), float, m[ 4] );
	WRITE_DATA( 5*sizeof(float), float, m[ 5] );
	WRITE_DATA( 6*sizeof(float), float, m[ 6] );
	WRITE_DATA( 7*sizeof(float), float, m[ 7] );
	WRITE_DATA( 8*sizeof(float), float, m[ 8] );
	WRITE_DATA( 9*sizeof(float), float, m[ 9] );
	WRITE_DATA( 10*sizeof(float), float, m[10] );
	WRITE_DATA( 11*sizeof(float), float, m[11] );
	WRITE_DATA( 12*sizeof(float), float, m[12] );
	WRITE_DATA( 13*sizeof(float), float, m[13] );
	WRITE_DATA( 14*sizeof(float), float, m[14] );
	WRITE_DATA( 15*sizeof(float), float, m[15] );
	WRITE_OPCODE( CR_LOADMATRIXF_OPCODE );
}
