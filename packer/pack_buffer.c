#include "cr_mem.h"
#include "cr_pack.h"
#include "cr_error.h"
#include "cr_protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

void crPackSetBuffer( CRPackBuffer *buffer )
{
	CRASSERT( buffer != NULL );
	memcpy( &(__pack_globals.buffer), buffer, sizeof(*buffer) );
}

void crPackGetBuffer( CRPackBuffer *buffer )
{
	CRASSERT( buffer != NULL );
	memcpy( buffer, &(__pack_globals.buffer), sizeof(*buffer) );
}

void crPackFlushFunc( FlushFunc ff )
{
	__pack_globals.Flush = ff;
}

void crPackSendHugeFunc( SendHugeFunc shf )
{
	__pack_globals.SendHuge = shf;
}

void crPackResetPointers( void )
{
	unsigned int num_opcodes;

	// Each opcode has a 1-word payload, so opcodes can occupy at most
	// 20% of the space.
	num_opcodes = ( __pack_globals.buffer.size - sizeof(CRMessageOpcodes) ) / 5;
	num_opcodes = (num_opcodes + 0x3) & (~0x3);

	__pack_globals.buffer.data_start    = 
		(unsigned char *) __pack_globals.buffer.pack + num_opcodes + sizeof(CRMessageOpcodes);
	__pack_globals.buffer.data_current  = __pack_globals.buffer.data_start;
	__pack_globals.buffer.data_end      = (unsigned char *) __pack_globals.buffer.pack + __pack_globals.buffer.size;

	__pack_globals.buffer.opcode_start   = __pack_globals.buffer.data_start - 1;
	__pack_globals.buffer.opcode_current = __pack_globals.buffer.opcode_start;
	__pack_globals.buffer.opcode_end     = __pack_globals.buffer.opcode_start - num_opcodes;
}

void *crPackAlloc( unsigned int size )
{
	CRPackGlobals *g = &__pack_globals;
	unsigned char *data_ptr;

	/* include space for the length and make the payload word-aligned */
	size = ( size + sizeof(unsigned int) + 0x3 ) & ~0x3;

	if ( g->buffer.data_current + size <= g->buffer.data_end )
	{
		/* we can just put it in the current buffer */
		GET_BUFFERED_POINTER( size );
	}
	else 
	{
		/* Okay, it didn't fit.  Maybe it will after we flush. */
		__pack_globals.Flush( );
		if ( g->buffer.data_current + size <= g->buffer.data_end )
		{
			GET_BUFFERED_POINTER( size );
		}
		else
		{
			/* It's really way too big, so allocate a temporary packet
			 * with space for the single opcode plus the payload &
			 * header */
			data_ptr = (unsigned char *) 
				crAlloc( sizeof(CRMessageOpcodes) + 4 + size );

			/* skip the header & opcode space */
			data_ptr += sizeof(CRMessageOpcodes) + 4;
		}
	}

	*((unsigned int *) data_ptr) = size;
	return ( data_ptr + 4 );
}

#define IS_BUFFERED( packet ) \
    ((unsigned char *) (packet) >= __pack_globals.buffer.data_start && \
	 (unsigned char *) (packet) < __pack_globals.buffer.data_end)

void crHugePacket( CROpcode opcode, void *packet )
{
	if ( IS_BUFFERED( packet ) )
		WRITE_OPCODE( opcode );
	else
		__pack_globals.SendHuge( opcode, packet );
}

void crPackFree( void *packet )
{
	if ( IS_BUFFERED( packet ) )
		return;
	
	/* the pointer passed in doesn't include the space for the single
	 * opcode (4 bytes because of the alignment requirement) or the
	 * length field or the header */
	crFree( (unsigned char *) packet - 8 - sizeof(CRMessageOpcodes) );
}

void crNetworkPointerWrite( CRNetworkPointer *dst, void *src )
{
	dst->ptrAlign[0] = 0xDeadBeef;
	dst->ptrAlign[1] = 0xCafeBabe;
	memcpy( dst, &src, sizeof(src) );
}
