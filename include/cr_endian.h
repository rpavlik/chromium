#ifndef CR_ENDIAN_H
#define CR_ENDIAN_H

#define CR_LITTLE_ENDIAN 0
#define CR_BIG_ENDIAN 1

#ifdef __cplusplus 
extern "C" {
#endif

extern char crDetermineEndianness( void );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CR_ENDIAN_H */
