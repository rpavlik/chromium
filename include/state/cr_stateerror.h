#ifndef CR_STATE_ERROR_H
#define CR_STATE_ERROR_H

#include "cr_opengl_types.h"

void crStateError( int line, char *file, GLenum err, char *format, ... );

#endif /* CR_STATE_ERROR_H */
