#ifndef CR_UNPACK_H
#define CR_UNPACK_H

#include "cr_spu.h"
#include "cr_protocol.h"

#ifdef WINDOWS
#ifndef DLLDATA 
#define DLLDATA __declspec(dllimport)
#endif
#else
#define DLLDATA
#endif

extern DLLDATA unsigned char *cr_unpackData;
extern SPUDispatchTable cr_unpackDispatch;

void crUnpackSetReturnPointer( CRNetworkPointer *ptr );
void crUnpackSetWritebackPointer( CRNetworkPointer *ptr );
void crUnpack( void *data, void *opcodes, unsigned int num_opcodes, SPUDispatchTable *table );

extern CRNetworkPointer *return_ptr, *writeback_ptr;

#if defined(LINUX) || defined(WINDOWS)
#define CR_UNALIGNED_ACCESS_OKAY
#else
#undef CR_UNALIGNED_ACCESS_OKAY
#endif
double crReadUnalignedDouble( void *buffer );

#define READ_DATA( offset, type ) \
	*( (type *) (cr_unpackData + (offset)))

#ifdef CR_UNALIGNED_ACCESS_OKAY
#define READ_DOUBLE( offset ) \
	READ_DATA( offset, GLdouble )
#else
#define READ_DOUBLE( offset ) \
	crReadUnalignedDouble( cr_unpackData + (offset) )
#endif

#define READ_NETWORK_POINTER( offset ) \
  ( cr_unpackData + (offset) )

#define DATA_POINTER( offset, type ) \
	( (type *) (cr_unpackData + (offset)) )

#define INCR_DATA_PTR( delta ) \
	cr_unpackData += (delta)

#define INCR_DATA_PTR_NO_ARGS() \
	INCR_DATA_PTR( 4 )

#define INCR_VAR_PTR() \
	INCR_DATA_PTR( *((int *) cr_unpackData ) )

#define SET_RETURN_PTR( offset ) \
	memcpy( return_ptr, cr_unpackData + (offset), sizeof( *return_ptr ) );

#define SET_WRITEBACK_PTR( offset ) \
	memcpy( writeback_ptr, cr_unpackData + (offset), sizeof( *writeback_ptr ) );

#endif /* CR_UNPACK_H */
