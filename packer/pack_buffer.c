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

void crPackSetBuffer( CRPackContext *pc, CRPackBuffer *buffer )
{
	CRASSERT( buffer != NULL );
	crMemcpy( &(pc->buffer), buffer, sizeof(*buffer) );
}

void crPackGetBuffer( CRPackContext *pc, CRPackBuffer *buffer )
{
	CRASSERT( buffer != NULL );
	crMemcpy( buffer, &(pc->buffer), sizeof(*buffer) );
}

void crPackFlushFunc( CRPackContext *pc, CRPackFlushFunc ff )
{
	pc->Flush = ff;
}

void crPackFlushArg( CRPackContext *pc, void *flush_arg )
{
	pc->flush_arg = flush_arg;
}

void crPackSendHugeFunc( CRPackContext *pc, CRPackSendHugeFunc shf )
{
	pc->SendHuge = shf;
}

void crPackResetPointers( CRPackContext *pc )
{
	const GLboolean g = pc->buffer.geometry_only;   /* save this flag */
	const GLboolean holds_BeginEnd = pc->buffer.holds_BeginEnd;
	const GLboolean in_BeginEnd = pc->buffer.in_BeginEnd;
	const GLboolean canBarf = pc->buffer.canBarf;
	crPackInitBuffer( &(pc->buffer), pc->buffer.pack, pc->buffer.size, pc->buffer.mtu );
	pc->buffer.geometry_only = g;   /* restore the flag */
	pc->buffer.holds_BeginEnd = holds_BeginEnd;
	pc->buffer.in_BeginEnd = in_BeginEnd;
	pc->buffer.canBarf = canBarf;
}

void crPackInitBuffer( CRPackBuffer *buf, void *pack, int size, int mtu )
{
	unsigned int num_opcodes;

	buf->size = size;
	buf->mtu  = mtu;
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

	buf->geometry_only = GL_FALSE;
	buf->holds_BeginEnd = 0;
	buf->in_BeginEnd = 0;
	buf->canBarf = 0;
}

int crPackCanHoldOpcode( int num_opcode, int num_data )
{
	GET_PACKER_CONTEXT(pc);
	return (((pc->buffer.data_current - pc->buffer.opcode_current - 1
			+ num_opcode + num_data
			+ 0x3 ) & ~0x3) + sizeof(CRMessageOpcodes)
			<= pc->buffer.mtu
		&& pc->buffer.opcode_current - num_opcode >= pc->buffer.opcode_end
		&& pc->buffer.data_current + num_data <= pc->buffer.data_end );
}

int crPackCanHoldBuffer( CRPackBuffer *src )
{
	int num_data = src->data_current - src->data_start;
	int num_opcode = src->opcode_start - src->opcode_current;
	return crPackCanHoldOpcode( num_opcode, num_data );
}

void crPackAppendBuffer( CRPackBuffer *src )
{
	GET_PACKER_CONTEXT(pc);
	int num_data = src->data_current - src->data_start;
	int num_opcode = src->opcode_start - src->opcode_current;

	if (!crPackCanHoldBuffer(src))
	{
		if (src->holds_BeginEnd)
		{
			crWarning( "crPackAppendBuffer: overflowed the destination!" );
			return;
		}
		else
			crError( "crPackAppendBuffer: overflowed the destination!" );
	}

	crMemcpy( pc->buffer.data_current, src->data_start, num_data );
	pc->buffer.data_current += num_data;

	CRASSERT( pc->buffer.opcode_current - num_opcode >= pc->buffer.opcode_end );
	crMemcpy( pc->buffer.opcode_current + 1 - num_opcode, src->opcode_current + 1,
			num_opcode );
	pc->buffer.opcode_current -= num_opcode;
	pc->buffer.holds_BeginEnd |= src->holds_BeginEnd;
	pc->buffer.in_BeginEnd = src->in_BeginEnd;
}


void crPackAppendBoundedBuffer( CRPackBuffer *src, GLrecti *bounds )
{
	GET_PACKER_CONTEXT(pc);
	int length = src->data_current - src->opcode_current - 1;
	int len_aligned = (length + 3) & ~3;

	/* 24 is the size of the bounds-info packet... */
	
	if ( !crPackCanHoldOpcode( 1, len_aligned + 24 ) )
	{
		if (src->holds_BeginEnd)
		{
			crWarning( "crPackAppendBoundedBuffer: overflowed the destination!" );
			return;
		}
		else
			crError( "crPackAppendBoundedBuffer: overflowed the destination!" );
	}

	if (pc->swapping)
		crPackBoundsInfoCRSWAP( bounds, (GLbyte *) src->opcode_current + 1,length,
														src->opcode_start - src->opcode_current );
	else
		crPackBoundsInfoCR( bounds, (GLbyte *) src->opcode_current + 1, length,
												src->opcode_start - src->opcode_current);

	pc->buffer.holds_BeginEnd |= src->holds_BeginEnd;
	pc->buffer.in_BeginEnd = src->in_BeginEnd;
}

void *crPackAlloc( unsigned int size )
{
	GET_PACKER_CONTEXT(pc);
	unsigned char *data_ptr;

	/* include space for the length and make the payload word-aligned */
	size = ( size + sizeof(unsigned int) + 0x3 ) & ~0x3;

	if ( crPackCanHoldOpcode( 1, size ) )
	{
		/* we can just put it in the current buffer */
		GET_BUFFERED_POINTER(pc, size );
	}
	else 
	{
		/* Okay, it didn't fit.  Maybe it will after we flush. */
		pc->Flush( pc->flush_arg );
		if ( crPackCanHoldOpcode( 1, size ) )
		{
			GET_BUFFERED_POINTER(pc, size );
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

	if (pc->swapping)
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
    ((unsigned char *) (packet) >= pc->buffer.data_start && \
	 (unsigned char *) (packet) < pc->buffer.data_end)

void crHugePacket( CROpcode opcode, void *packet )
{
	GET_PACKER_CONTEXT(pc);
	if ( IS_BUFFERED( packet ) )
		WRITE_OPCODE( pc, opcode );
	else
		pc->SendHuge( opcode, packet );
}

void crPackFree( void *packet )
{
	GET_PACKER_CONTEXT(pc);
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
	crMemcpy( dst, &src, sizeof(src) );
}
