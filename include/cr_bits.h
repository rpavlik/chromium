/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/* Chromium sources include this file instead of including
 * the GL/gl.h and GL/glext.h headers directly.
 */

#ifndef CR_BITS_H
#define CR_BITS_H



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


#define CR_MAX_CONTEXTS      512
#define CR_MAX_BITARRAY      (CR_MAX_CONTEXTS / 32) /* 32 contexts per uint */


static INLINE void DIRTY( unsigned int *b, const unsigned int *d )
{
	int j;
	for (j=0;j<CR_MAX_BITARRAY;j++)
		b[j] = d[j];
}
static INLINE void FILLDIRTY( unsigned int *b )
{
	int j;
	for (j=0;j<CR_MAX_BITARRAY;j++)
		b[j] = 0xffffffff;
}
static INLINE void INVERTDIRTY( unsigned int *b, const unsigned int *d )
{
	int j;
	for (j=0;j<CR_MAX_BITARRAY;j++)
		b[j] &= d[j];
}
static INLINE int CHECKDIRTY( const unsigned int *b, const unsigned int *d )
{
	int j;

	for (j=0;j<CR_MAX_BITARRAY;j++)
		if (b[j] & d[j])
			return 1;

	return 0;
}



#endif /* CR_BITS_H */