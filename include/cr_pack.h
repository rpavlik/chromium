/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_PACK_H
#define CR_PACK_H

#include "cr_error.h"
#include "cr_protocol.h"
#include "cr_opcodes.h"
#include "cr_endian.h"
#include "state/cr_statetypes.h"
#include "state/cr_currentpointers.h"

#ifdef WINDOWS
#ifndef DLLDATA 
#define DLLDATA __declspec(dllimport)
#endif
#else
#define DLLDATA
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	void          *pack;
	unsigned int   size;
	unsigned char *data_start, *data_current, *data_end;
	unsigned char *opcode_start, *opcode_current, *opcode_end;
	GLboolean geometry_only;  /* just used for debugging */
} CRPackBuffer;

typedef void (*CRPackFlushFunc)(void *arg);
typedef void (*CRPackSendHugeFunc)(CROpcode, void *);

typedef GLvectorf CRBBOXPoint;

typedef struct {
	CRPackBuffer buffer;
	CRPackFlushFunc Flush;
	void *flush_arg;
	CRPackSendHugeFunc SendHuge;
	CRCurrentStatePointers current;
	CRBBOXPoint bounds_min, bounds_max;
	int updateBBOX;
	int swapping;
} CRPackContext;

CRPackContext *crPackNewContext(int swapping);
void crPackSetContext( CRPackContext *pc );
CRPackContext *crPackGetContext( void );

void crPackSetBuffer( CRPackContext *pc, CRPackBuffer *buffer );
void crPackGetBuffer( CRPackContext *pc, CRPackBuffer *buffer );
void crPackInitBuffer( CRPackBuffer *buffer, void *buf, int size, int extra );
void crPackResetPointers( CRPackContext *pc, int extra );
void crPackFlushFunc( CRPackContext *pc, CRPackFlushFunc ff );
void crPackFlushArg( CRPackContext *pc, void *flush_arg );
void crPackSendHugeFunc( CRPackContext *pc, CRPackSendHugeFunc shf );
void crPackOffsetCurrentPointers( int offset );
void crPackNullCurrentPointers( void );

void crPackResetBBOX( CRPackContext *pc );

void crPackAppendBuffer( CRPackBuffer *buffer );
void crPackAppendBoundedBuffer( CRPackBuffer *buffer, GLrecti *bounds );

#if defined(LINUX) || defined(WINDOWS)
#define CR_UNALIGNED_ACCESS_OKAY
#else
#undef CR_UNALIGNED_ACCESS_OKAY
#endif
void crWriteUnalignedDouble( void *buffer, double d );
void crWriteSwappedDouble( void *buffer, double d );

void *crPackAlloc( unsigned int len );
void crHugePacket( CROpcode op, void *ptr );
void crPackFree( void *ptr );
void crNetworkPointerWrite( CRNetworkPointer *, void * );

void SanityCheck(void);

#define GET_BUFFERED_POINTER( pc, len ) \
  THREADASSERT( pc ); \
  data_ptr = pc->buffer.data_current; \
  if (data_ptr + (len) > pc->buffer.data_end ) \
  { \
    pc->Flush( pc->flush_arg ); \
    data_ptr = pc->buffer.data_current; \
    CRASSERT( data_ptr + (len) <= pc->buffer.data_end ); \
  } \
  pc->buffer.data_current += (len)

#define GET_BUFFERED_COUNT_POINTER( pc, len ) \
  data_ptr = pc->buffer.data_current; \
  if (data_ptr + (len) > pc->buffer.data_end ) \
  { \
    pc->Flush( pc->flush_arg ); \
    data_ptr = pc->buffer.data_current; \
    CRASSERT( data_ptr + (len) <= pc->buffer.data_end ); \
  } \
  pc->current.vtx_count++; \
  pc->buffer.data_current += (len)

#define GET_BUFFERED_POINTER_NO_ARGS( pc ) \
  GET_BUFFERED_POINTER( pc, 4 );  \
  WRITE_DATA( 0, GLuint, 0xdeadbeef )

#define WRITE_DATA( offset, type, data ) \
  *( (type *) (data_ptr + (offset))) = (data)

#ifdef CR_UNALIGNED_ACCESS_OKAY
#define WRITE_DOUBLE( offset, data ) \
  WRITE_DATA( offset, GLdouble, data )
#else
#define WRITE_DOUBLE( offset, data ) \
  crWriteUnalignedDouble( data_ptr + (offset), (data) )
#endif

#define WRITE_SWAPPED_DOUBLE( offset, data ) \
	crWriteSwappedDouble( data_ptr + (offset), (data) )

#define WRITE_OPCODE( pc, opcode )  \
  *(pc->buffer.opcode_current--) = (unsigned char) opcode

#define WRITE_NETWORK_POINTER( offset, data ) \
  crNetworkPointerWrite( (CRNetworkPointer *) ( data_ptr + (offset) ), (data) )

#ifdef __cplusplus
}
#endif

#endif /* CR_PACK_H */
