#include "cr_packfunctions.h"
#include "cr_pack.h"
#include "cr_glwrapper.h"
#include "cr_opcodes.h"

void PACK_APIENTRY crPackClipPlane( GLenum plane, const GLdouble *equation )
{
	unsigned char *data_ptr;
	int packet_length = sizeof( plane ) + 4*sizeof(*equation);
	GET_BUFFERED_POINTER( packet_length );
	WRITE_DATA( 0, GLenum, plane );
	WRITE_DOUBLE( 4, equation[0] );
	WRITE_DOUBLE( 12, equation[1] );
	WRITE_DOUBLE( 20, equation[2] );
	WRITE_DOUBLE( 28, equation[3] );
	WRITE_OPCODE( CR_CLIPPLANE_OPCODE );
}
