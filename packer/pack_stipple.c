#include "cr_packfunctions.h"
#include "cr_pack.h"
#include "cr_opengl_types.h"
#include "cr_opcodes.h"

#include <string.h>

void PACK_APIENTRY crPackPolygonStipple( const GLubyte *mask )
{
	unsigned char *data_ptr;
	int packet_length = 32*32/8;
	GET_BUFFERED_POINTER( packet_length );
	memcpy( data_ptr, mask, 32*32/8 );
	WRITE_OPCODE( CR_POLYGONSTIPPLE_OPCODE );
}
