/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_STATE_ERROR_H
#define CR_STATE_ERROR_H

#include "chromium.h"

void crStateError( int line, char *file, GLenum err, char *format, ... );

#endif /* CR_STATE_ERROR_H */
