#ifndef CR_PACK_H
#define CR_PACK_H

#include "cr_error.h"
#include "cr_protocol.h"
#include "cr_opcodes.h"
#include "state/cr_statetypes.h"
#include "state/cr_currentpointers.h"

#ifdef WINDOWS
#define PACK_APIENTRY __stdcall
#else
#define PACK_APIENTRY
#endif

typedef struct {
	void          *pack;
	unsigned int   size;
	unsigned char *data_start, *data_current, *data_end;
	unsigned char *opcode_start, *opcode_current, *opcode_end;
} CRPackBuffer;

typedef void (*CRPackFlushFunc)(void *arg);
typedef void (*CRPackSendHugeFunc)(CROpcode, void *);

typedef struct {
	float x,y,z;
} CRBBOXPoint;

typedef struct {
	CRPackBuffer buffer;
	CRPackFlushFunc Flush;
	void *flush_arg;
	CRPackSendHugeFunc SendHuge;
	CRCurrentStatePointers current;
	CRBBOXPoint bounds_min, bounds_max;
} CRPackGlobals;

extern CRPackGlobals cr_packer_globals;

void crPackSetBuffer( CRPackBuffer *buffer );
void crPackGetBuffer( CRPackBuffer *buffer );
void crPackInitBuffer( CRPackBuffer *buffer, void *buf, int size, int extra );
void crPackResetPointers( int extra );
void crPackFlushFunc( CRPackFlushFunc ff );
void crPackFlushArg( void *flush_arg );
void crPackSendHugeFunc( CRPackSendHugeFunc shf );

void crPackAppendBuffer( CRPackBuffer *buffer );
void crPackAppendBoundedBuffer( CRPackBuffer *buffer, GLrecti *bounds );

#if defined(LINUX) || defined(WINDOWS)
#define CR_UNALIGNED_ACCESS_OKAY
#else
#undef CR_UNALIGNED_ACCESS_OKAY
#endif
void crWriteUnalignedDouble( void *buffer, double d );

void *crPackAlloc( unsigned int len );
void crHugePacket( CROpcode op, void *ptr );
void crPackFree( void *ptr );
void crNetworkPointerWrite( CRNetworkPointer *, void * );

#define GET_BUFFERED_POINTER( len ) \
  data_ptr = cr_packer_globals.buffer.data_current; \
  if (data_ptr + (len) > cr_packer_globals.buffer.data_end ) \
  { \
    cr_packer_globals.Flush( cr_packer_globals.flush_arg ); \
    data_ptr = cr_packer_globals.buffer.data_current; \
    CRASSERT( data_ptr + (len) <= cr_packer_globals.buffer.data_end ); \
  } \
  cr_packer_globals.buffer.data_current += (len)

#define GET_BUFFERED_POINTER_NO_ARGS( ) \
  GET_BUFFERED_POINTER( 4 );  \
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

#define WRITE_OPCODE( opcode ) \
  *(cr_packer_globals.buffer.opcode_current--) = (unsigned char) opcode

#define WRITE_NETWORK_POINTER( offset, data ) \
  crNetworkPointerWrite( (CRNetworkPointer *) ( data_ptr + (offset) ), (data) )

#endif /* CR_PACK_H */
