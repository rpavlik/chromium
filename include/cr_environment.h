/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_ENVIRONMENT_H
#define CR_ENVIRONMENT_H

void crSetenv( const char *var, const char *value );
char *crGetenv( const char *var );

#endif /* CR_ENVIRONMENT_H */
