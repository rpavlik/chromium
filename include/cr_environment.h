/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_ENVIRONMENT_H
#define CR_ENVIRONMENT_H

#ifdef __cplusplus
extern "C" {
#endif

extern void crSetenv( const char *var, const char *value );
extern const char *crGetenv( const char *var );

#ifdef __cplusplus
}
#endif

#endif /* CR_ENVIRONMENT_H */
