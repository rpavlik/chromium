/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mem.h"
#include "cr_string.h"
#include "packer.h"
#include "cr_error.h"
#include "cr_protocol.h"
#include "cr_unpack.h"

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

/*
 * We need the packer to run as efficiently as possible.  To avoid one
 * pointer dereference from the CRPackContext to the current CRPackBuffer,
 * we keep a _copy_ of the current CRPackBuffer in the CRPackContext and
 * operate on the fields in CRPackContext, rather than the CRPackBuffer.
 *
 * To keep things in sync, when we change a context's
 * buffer, we have to use the crPackSet/GetBuffer() functions.
 */

void crPackSetBuffer( CRPackContext *pc, CRPackBuffer *buffer )
{
	CRASSERT( pc );
	CRASSERT( buffer );

	if (pc->currentBuffer == buffer)
		return; /* re-bind is no-op */

	CRASSERT( pc->currentBuffer == NULL);  /* release if NULL? */
	CRASSERT( buffer->context == NULL );

	/* bind context to buffer */
	pc->currentBuffer = buffer;
	buffer->context = pc;

	/* update the context's packing fields with those from the buffer */
	pc->buffer = *buffer;  /* struct copy */
}

/* This is useful for debugging packer problems */
void crPackSetBufferDEBUG( const char *file, int line,
													 CRPackContext *pc, CRPackBuffer *buffer)
						   
{
	crPackSetBuffer( pc, buffer );
	/* record debugging info */
	pc->file = crStrdup(file);
	pc->line = line;
}


/*
 * Release the buffer currently attached to the context.
 * Update/resync data structures.
 */
void crPackReleaseBuffer( CRPackContext *pc )
{
	CRPackBuffer *buf;
	CRASSERT( pc );

	if (!pc->currentBuffer) {
		crWarning("crPackReleaseBuffer called with no current buffer");
		return; /* nothing to do */
	}

	CRASSERT( pc->currentBuffer->context == pc );

	/* buffer to release */
	buf = pc->currentBuffer;

	/* copy context's fields back into the buffer to update it */
	*buf = pc->buffer;        /* struct copy */

	/* unbind buffer from context */
	buf->context = NULL;
	pc->currentBuffer = NULL;

	/* zero-out context's packing fields just to be safe */
	crMemZero(&(pc->buffer), sizeof(pc->buffer));

	/* update the debugging fields */
	if (pc->file)
		crFree(pc->file);
	pc->file = NULL;
	pc->line = -1;
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

/*
 * This basically resets the buffer attached to <pc> to the default, empty
 * state.
 */
void crPackResetPointers( CRPackContext *pc )
{
	const GLboolean geom_only = pc->buffer.geometry_only;   /* save this flag */
	const GLboolean holds_BeginEnd = pc->buffer.holds_BeginEnd;
	const GLboolean in_BeginEnd = pc->buffer.in_BeginEnd;
	const GLboolean canBarf = pc->buffer.canBarf;
	CRPackBuffer *buf = pc->currentBuffer;
	CRASSERT(buf);
	crPackInitBuffer( buf, buf->pack, buf->size, buf->mtu );
	pc->buffer.geometry_only = geom_only;   /* restore the flag */
	pc->buffer.holds_BeginEnd = holds_BeginEnd;
	pc->buffer.in_BeginEnd = in_BeginEnd;
	pc->buffer.canBarf = canBarf;
}

/* Each opcode has at least a 1-word payload, so opcodes can occupy at most 
 * 20% of the space. */
int crPackNumOpcodes( int buffer_size ) {
	/* Don't forget to add one here
	 * in case the buffer size is not divisible by 4
	 * thanks to Ken Moreland for finding this */
	return ((( buffer_size - sizeof(CRMessageOpcodes) ) / 5) + 1 + 0x3)&(~0x3);
												/* round up to multiple of 4 */
}
int crPackNumData( int buffer_size ) {
	return buffer_size - sizeof(CRMessageOpcodes)
		- crPackNumOpcodes(buffer_size);
}

/*
 * Initialize the given buffer.
 * The buffer may or may not be currently bound to a CRPackContext.
 */
void crPackInitBuffer( CRPackBuffer *buf, void *pack, int size, int mtu )
{
	unsigned int num_opcodes;

	CRASSERT(mtu <= size);

	buf->size = size;
	buf->mtu  = mtu;
	buf->pack = pack;

	num_opcodes = crPackNumOpcodes( buf->size );

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

	if (buf->context) {
		/* Also reset context's packing fields */
		CRPackContext *pc = buf->context;
		CRASSERT(pc->currentBuffer == buf);
		/*crMemcpy( &(pc->buffer), buf, sizeof(*buf) );*/
		pc->buffer = *buf;
	}
}

int crPackCanHoldOpcode( int num_opcode, int num_data )
{
	int fitsInMTU, opcodesFit, dataFits;
	GET_PACKER_CONTEXT(pc);

	CRASSERT(pc->currentBuffer);

	fitsInMTU = (((pc->buffer.data_current - pc->buffer.opcode_current - 1
								 + num_opcode + num_data
								 + 0x3 ) & ~0x3) + sizeof(CRMessageOpcodes)
							 <= pc->buffer.mtu);
	opcodesFit = (pc->buffer.opcode_current - num_opcode
								>= pc->buffer.opcode_end);
	dataFits = (pc->buffer.data_current + num_data <= pc->buffer.data_end);

	return fitsInMTU && opcodesFit && dataFits;
}

int crPackCanHoldBuffer( const CRPackBuffer *src )
{
	const int num_data = src->data_current - src->data_start;
	const int num_opcode = src->opcode_start - src->opcode_current;
	return crPackCanHoldOpcode( num_opcode, num_data );
}

int crPackCanHoldBoundedBuffer( const CRPackBuffer *src )
{
	const int len_aligned = (src->data_current - src->opcode_current - 1 + 3) & ~3;
	/* 24 is the size of the bounds-info packet... */
	return crPackCanHoldOpcode( 1, len_aligned + 24 );
}

void crPackAppendBuffer( const CRPackBuffer *src )
{
	GET_PACKER_CONTEXT(pc);
	const int num_data = src->data_current - src->data_start; /* in bytes */
	const int num_opcode = src->opcode_start - src->opcode_current; /* bytes */

	CRASSERT(num_data >= 0);
	CRASSERT(num_opcode >= 0);

	/* don't append onto ourself! */
	CRASSERT(pc->currentBuffer);
	CRASSERT(pc->currentBuffer != src);

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

	/* Copy the buffer data/operands which are at the head of the buffer */
	crMemcpy( pc->buffer.data_current, src->data_start, num_data );
	pc->buffer.data_current += num_data;

	/* Copy the buffer opcodes which are at the tail of the buffer */
	CRASSERT( pc->buffer.opcode_current - num_opcode >= pc->buffer.opcode_end );
	crMemcpy( pc->buffer.opcode_current + 1 - num_opcode, src->opcode_current + 1,
			num_opcode );
	pc->buffer.opcode_current -= num_opcode;
	pc->buffer.holds_BeginEnd |= src->holds_BeginEnd;
	pc->buffer.in_BeginEnd = src->in_BeginEnd;
	pc->buffer.holds_List |= src->holds_List;
}


void crPackAppendBoundedBuffer( const CRPackBuffer *src, const CRrecti *bounds )
{
	GET_PACKER_CONTEXT(pc);
	const GLbyte *payload = (const GLbyte *) src->opcode_current + 1;
	const int num_opcodes = src->opcode_start - src->opcode_current;
	const int length = src->data_current - src->opcode_current - 1;

	CRASSERT(pc);
	CRASSERT(pc->currentBuffer);
	CRASSERT(pc->currentBuffer != src);

	/*
	 * payload points to the block of opcodes immediately followed by operands.
	 */

	if ( !crPackCanHoldBoundedBuffer( src ) )
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
		crPackBoundsInfoCRSWAP( bounds, payload, length, num_opcodes );
	else
		crPackBoundsInfoCR( bounds, payload, length, num_opcodes );

	pc->buffer.holds_BeginEnd |= src->holds_BeginEnd;
	pc->buffer.in_BeginEnd = src->in_BeginEnd;
	pc->buffer.holds_List |= src->holds_List;
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
