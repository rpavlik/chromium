/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mem.h"
#include "packer.h"
#include "cr_error.h"
#include "cr_protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

void crWriteUnalignedDouble( void *buffer, double d )
{
	unsigned int *ui = (unsigned int *) buffer;
	ui[0] = ((unsigned int *) &d)[0];
	ui[1] = ((unsigned int *) &d)[1];
}

void crWriteSwappedDouble( void *buffer, double d )
{
	unsigned int *ui = (unsigned int *) buffer;
	ui[0] = SWAP32(((unsigned int *) &d)[1]);
	ui[1] = SWAP32(((unsigned int *) &d)[0]);
}

double crReadUnalignedDouble( void *buffer )
{
	unsigned int *ui = (unsigned int *) buffer;
	double d;
	((unsigned int *) &d)[0] = ui[0];
	((unsigned int *) &d)[1] = ui[1];
	return d;
}

void crPackSetBuffer( CRPackBuffer *buffer )
{
	CRASSERT( buffer != NULL );
	memcpy( &(cr_packer_globals.buffer), buffer, sizeof(*buffer) );
}

void crPackGetBuffer( CRPackBuffer *buffer )
{
	CRASSERT( buffer != NULL );
	memcpy( buffer, &(cr_packer_globals.buffer), sizeof(*buffer) );
}

void crPackFlushFunc( CRPackFlushFunc ff )
{
	cr_packer_globals.Flush = ff;
}

void crPackFlushArg( void *flush_arg )
{
	cr_packer_globals.flush_arg = flush_arg;
}

void crPackSendHugeFunc( CRPackSendHugeFunc shf )
{
	cr_packer_globals.SendHuge = shf;
}

void crPackResetPointers( int extra )
{
	crPackInitBuffer( &(cr_packer_globals.buffer), cr_packer_globals.buffer.pack, cr_packer_globals.buffer.size, extra );
}

void crPackInitBuffer( CRPackBuffer *buf, void *pack, int size, int extra )
{
	unsigned int num_opcodes;

	buf->size = size;
	buf->pack = pack;

	/* Each opcode has at least a 1-word payload, so opcodes can occupy at most 
	 * 20% of the space. */

	/* Don't forget to add one here, thanks to Ken Moreland for finding this */
	num_opcodes = (( buf->size - sizeof(CRMessageOpcodes) ) / 5) + 1;
	num_opcodes = (num_opcodes + 0x3) & (~0x3);

	buf->data_start    = 
		(unsigned char *) buf->pack + num_opcodes + sizeof(CRMessageOpcodes);
	buf->data_current  = buf->data_start;
	buf->data_end      = (unsigned char *) buf->pack + buf->size;

	buf->opcode_start   = buf->data_start - 1;
	buf->opcode_current = buf->opcode_start;
	buf->opcode_end     = buf->opcode_start - num_opcodes;

	buf->data_end -= extra; /* caller may want extra space (sigh) */
}

void crPackAppendBuffer( CRPackBuffer *src )
{
	int num_data = src->data_current - src->data_start;
	int num_opcode;


	if ( cr_packer_globals.buffer.data_current + num_data > cr_packer_globals.buffer.data_end )
		crError( "crPackAppendBuffer: overflowed the destination!" );
	memcpy( cr_packer_globals.buffer.data_current, src->data_start, num_data );
	cr_packer_globals.buffer.data_current += num_data;

	num_opcode = src->opcode_start - src->opcode_current;
	CRASSERT( cr_packer_globals.buffer.opcode_current - num_opcode >= cr_packer_globals.buffer.opcode_end );
	memcpy( cr_packer_globals.buffer.opcode_current + 1 - num_opcode, src->opcode_current + 1,
			num_opcode );
	cr_packer_globals.buffer.opcode_current -= num_opcode;
}


void crPackAppendBoundedBuffer( CRPackBuffer *src, GLrecti *bounds )
{
	int length = src->data_current - ( src->opcode_current + 1 );

	/* 24 is the size of the bounds-info packet... */
	
	if ( cr_packer_globals.buffer.data_current + length + 24 > cr_packer_globals.buffer.data_end )
		crError( "crPackAppendBoundedBuffer: overflowed the destination!" );

	crPackBoundsInfo( bounds, (GLbyte *) src->opcode_current + 1, length,
					src->opcode_start - src->opcode_current );
}

void *crPackAlloc( unsigned int size )
{
	CRPackGlobals *g = &cr_packer_globals;
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
		cr_packer_globals.Flush( cr_packer_globals.flush_arg );
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

	if (cr_packer_globals.swapping)
	{
		*((unsigned int *) data_ptr) = SWAP32(size);
		crDebug( "Just swapped the length, putting %d on the wire!", *((unsigned int *) data_ptr));
	}
	else
	{
		*((unsigned int *) data_ptr) = size;
	}
	return ( data_ptr + 4 );
}

#define IS_BUFFERED( packet ) \
    ((unsigned char *) (packet) >= cr_packer_globals.buffer.data_start && \
	 (unsigned char *) (packet) < cr_packer_globals.buffer.data_end)

void crHugePacket( CROpcode opcode, void *packet )
{
	if ( IS_BUFFERED( packet ) )
		WRITE_OPCODE( opcode );
	else
		cr_packer_globals.SendHuge( opcode, packet );
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
