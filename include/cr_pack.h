#ifndef CR_PACK_H
#define CR_PACK_H

#include "cr_error.h"
#include "cr_protocol.h"
#include "cr_opcodes.h"
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

typedef struct {
	CRPackBuffer buffer;
	void (*Flush)(void);
	void (*SendHuge)(CROpcode, void *);
	CRCurrentStatePointers current;
} CRPackGlobals;

extern CRPackGlobals __pack_globals;

void crPackBuffer( CRPackBuffer *buffer );
void crPackResetPointers( void );

#if defined(LINUX) || defined(WINDOWS)
#define CR_UNALIGNED_ACCESS_OKAY
#else
#undef CR_UNALIGNED_ACCESS_OKAY
#endif
void crWriteUnalignedDouble( void *buffer, double d );
double crReadUnalignedDouble( void *buffer );

void *crPackAlloc( unsigned int len );
void crHugePacket( CROpcode op, void *ptr );
void crPackFree( void *ptr );
void crNetworkPointerWrite( CRNetworkPointer *, void * );

#define GET_BUFFERED_POINTER( len ) \
  data_ptr = __pack_globals.buffer.data_current; \
  if (data_ptr + (len) > __pack_globals.buffer.data_end ) \
  { \
    __pack_globals.Flush( ); \
    data_ptr = __pack_globals.buffer.data_current; \
    CRASSERT( data_ptr + (len) <= __pack_globals.buffer.data_end ); \
  } \
  __pack_globals.buffer.data_current += (len)

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
  *(__pack_globals.buffer.opcode_current--) = (unsigned char) opcode

#define WRITE_NETWORK_POINTER( offset, data ) \
  crNetworkPointerWrite( (CRNetworkPointer *) ( data_ptr + (offset) ), (data) )

#endif /* CR_PACK_H */
