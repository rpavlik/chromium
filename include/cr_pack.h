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
#include "state/cr_client.h"
#ifdef CHROMIUM_THREADSAFE
#include "cr_threads.h"
#endif

#ifdef WINDOWS
#ifndef DLLDATA 
#define DLLDATA __declspec(dllexport)
#endif
#else
#define DLLDATA
#endif


/* Function inlining */
#if defined(__GNUC__)
#  define INLINE __inline__
#elif defined(__MSC__)
#  define INLINE __inline
#elif defined(_MSC_VER)
#  define INLINE __inline
#elif defined(__ICL)
#  define INLINE __inline
#else
#  define INLINE
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CRPackContext_t CRPackContext;

typedef struct {
	void          *pack;
	unsigned int   size;
	unsigned int   mtu;
	unsigned char *data_start, *data_current, *data_end;
	unsigned char *opcode_start, *opcode_current, *opcode_end;
	GLboolean geometry_only;  /* just used for debugging */
	GLboolean holds_BeginEnd;
	GLboolean in_BeginEnd;
	GLboolean canBarf;
	GLboolean holds_List;
	GLboolean in_List;
	CRPackContext *context;
} CRPackBuffer;

typedef void (*CRPackFlushFunc)(void *arg);
typedef void (*CRPackSendHugeFunc)(CROpcode, void *);
typedef void (*CRPackErrorHandlerFunc)(int line, const char *file, GLenum error, const char *info);

typedef GLvectorf CRBBOXPoint;

struct CRPackContext_t{
	CRPackBuffer buffer;   /* not a pointer, see comments in pack_buffer.c */
	CRPackFlushFunc Flush;
	void *flush_arg;
	CRPackSendHugeFunc SendHuge;
	CRPackErrorHandlerFunc Error;
	CRCurrentStatePointers current;
	CRBBOXPoint bounds_min, bounds_max;
	int updateBBOX;
	int swapping;
	CRPackBuffer *currentBuffer;
	char *file;  /* for debugging only */
	int line;    /* for debugging only */
};


CRPackContext *crPackNewContext(int swapping);
void crPackSetContext( CRPackContext *pc );
CRPackContext *crPackGetContext( void );

void crPackSetBuffer( CRPackContext *pc, CRPackBuffer *buffer );
void crPackSetBufferDEBUG( const char *file, int line, CRPackContext *pc, CRPackBuffer *buffer );
void crPackReleaseBuffer( CRPackContext *pc );
void crPackResetPointers( CRPackContext *pc );

int crPackNumOpcodes( int buffer_size );
int crPackNumData( int buffer_size );
void crPackInitBuffer( CRPackBuffer *buffer, void *buf, int size, int mtu );
void crPackFlushFunc( CRPackContext *pc, CRPackFlushFunc ff );
void crPackFlushArg( CRPackContext *pc, void *flush_arg );
void crPackSendHugeFunc( CRPackContext *pc, CRPackSendHugeFunc shf );
void crPackErrorFunction( CRPackContext *pc, CRPackErrorHandlerFunc errf );
void crPackOffsetCurrentPointers( int offset );
void crPackNullCurrentPointers( void );

void crPackResetBBOX( CRPackContext *pc );

void crPackAppendBuffer( const CRPackBuffer *buffer );
void crPackAppendBoundedBuffer( const CRPackBuffer *buffer, const CRrecti *bounds );
int crPackCanHoldBuffer( const CRPackBuffer *buffer );
int crPackCanHoldBoundedBuffer( const CRPackBuffer *buffer );

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

void crPackExpandDrawArrays(GLenum mode, GLint first, GLsizei count, CRClientState *c);
void crPackExpandDrawArraysSWAP(GLenum mode, GLint first, GLsizei count, CRClientState *c);

void crPackExpandDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, CRClientState *c);
void crPackExpandDrawElementsSWAP(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, CRClientState *c);

void crPackExpandDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices, CRClientState *c);
void crPackExpandDrawRangeElementsSWAP(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices, CRClientState *c);

void crPackExpandArrayElement(GLint index, CRClientState *c);
void crPackExpandArrayElementSWAP(GLint index, CRClientState *c);

void crPackExpandMultiDrawArraysEXT( GLenum mode, GLint *first, GLsizei *count, GLsizei primcount, CRClientState *c );
void crPackExpandMultiDrawArraysEXTSWAP( GLenum mode, GLint *first, GLsizei *count, GLsizei primcount, CRClientState *c );

void crPackExpandMultiDrawElementsEXT( GLenum mode, const GLsizei *count, GLenum type, const GLvoid **indices, GLsizei primcount, CRClientState *c );
void crPackExpandMultiDrawElementsEXTSWAP( GLenum mode, const GLsizei *count, GLenum type, const GLvoid **indices, GLsizei primcount, CRClientState *c );


#ifdef CHROMIUM_THREADSAFE
extern CRtsd _PackerTSD;
#define GET_PACKER_CONTEXT(C) CRPackContext *C = (CRPackContext *) crGetTSD(&_PackerTSD)
#else
extern DLLDATA CRPackContext cr_packer_globals;
#define GET_PACKER_CONTEXT(C) CRPackContext *C = &cr_packer_globals
#endif

static INLINE int crPackCanHoldOpcode( int num_opcode, int num_data )
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

#define GET_BUFFERED_POINTER_NO_BEGINEND_FLUSH( pc, len ) \
  THREADASSERT( pc ); \
  CRASSERT( pc->currentBuffer ); \
  data_ptr = pc->buffer.data_current; \
  if ( !crPackCanHoldOpcode( 1, (len) ) ) \
  { \
    pc->Flush( pc->flush_arg ); \
    data_ptr = pc->buffer.data_current; \
    CRASSERT(crPackCanHoldOpcode( 1, (len) ) ); \
  } \
  pc->buffer.data_current += (len)

#define GET_BUFFERED_POINTER( pc, len ) \
  CRASSERT( pc->currentBuffer ); \
  if ( pc->buffer.holds_BeginEnd && ! pc->buffer.in_BeginEnd ) { \
		pc->Flush( pc->flush_arg ); \
		pc->buffer.holds_BeginEnd = 0; \
  } \
  GET_BUFFERED_POINTER_NO_BEGINEND_FLUSH( pc, len )

#define GET_BUFFERED_COUNT_POINTER( pc, len ) \
  CRASSERT( pc->currentBuffer ); \
  data_ptr = pc->buffer.data_current; \
  if ( !crPackCanHoldOpcode( 1, (len) ) ) \
  { \
    pc->Flush( pc->flush_arg ); \
    data_ptr = pc->buffer.data_current; \
    CRASSERT( crPackCanHoldOpcode( 1, (len) ) ); \
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
