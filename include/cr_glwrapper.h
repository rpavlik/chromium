#ifndef CR_GLWRAPPER_H
#define CR_GLWRAPPER_H

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#define GL_GLEXT_PROTOTYPES
#endif

#include <GL/gl.h>
#include <GL/glext.h>

#endif /* CR_GLWRAPPER_H */
