#ifndef CR_ENDIAN_H
#define CR_ENDIAN_H

#define CR_LITTLE_ENDIAN 0
#define CR_BIG_ENDIAN 1

#ifdef __cplusplus 
extern "C" {
#endif

extern char crDetermineEndianness( void );

#ifdef WINDOWS
typedef __int64 CR64BitType;
#else
typedef long long CR64BitType;
#endif

#define SWAP8(x) x
#define SWAP16(x) (short) ((((x) & 0x00FF) << 8) | (((x) & 0xFF00) >> 8))
#define SWAP32(x) ((((x) & 0x000000FF) << 24) | \
		               (((x) & 0x0000FF00) << 8)  | \
									 (((x) & 0x00FF0000) >> 8)  | \
									 (((x) & 0xFF000000) >> 24))
#ifdef WINDOWS
#define SWAP64(x) \
	 ((((x) & 0xFF00000000000000L) >> 56) | \
		(((x) & 0x00FF000000000000L) >> 40) | \
		(((x) & 0x0000FF0000000000L) >> 24) | \
		(((x) & 0x000000FF00000000L) >> 8)  | \
		(((x) & 0x00000000FF000000L) << 8)  | \
		(((x) & 0x0000000000FF0000L) << 24) | \
		(((x) & 0x000000000000FF00L) << 40) | \
		(((x) & 0x00000000000000FFL) << 56))
#else
#define SWAP64(x) \
	 ((((x) & 0xFF00000000000000LL) >> 56) | \
		(((x) & 0x00FF000000000000LL) >> 40) | \
		(((x) & 0x0000FF0000000000LL) >> 24) | \
		(((x) & 0x000000FF00000000LL) >> 8)  | \
		(((x) & 0x00000000FF000000LL) << 8)  | \
		(((x) & 0x0000000000FF0000LL) << 24) | \
		(((x) & 0x000000000000FF00LL) << 40) | \
		(((x) & 0x00000000000000FFLL) << 56))
#endif

double SWAPDOUBLE( double d );

#define SWAPFLOAT(x) ( ((*((int *) &(x)) & 0x000000FF) << 24) | \
                       ((*((int *) &(x)) & 0x0000FF00) << 8) | \
                       ((*((int *) &(x)) & 0x00FF0000) >> 8) | \
                       ((*((int *) &(x)) & 0xFF000000) >> 24))


#ifdef __cplusplus
} /* extern "C" */
#endif
 

#endif /* CR_ENDIAN_H */
