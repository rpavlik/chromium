/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_RAND_H
#define CR_RAND_H

#ifdef __cplusplus
extern "C" {
#endif

void crRandSeed( unsigned long seed );
float crRandFloat( float min, float max );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CR_RAND_H */
