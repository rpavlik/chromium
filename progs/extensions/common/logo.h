/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_EXTENSIONS_LOGO_H
#define CR_EXTENSIONS_LOGO_H

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

void crExtensionsDrawLogo( int currentWidth, int currentHeight );

void RenderString(float x, float y, char *string);

int CheckForExtension(const char *extension);

#ifdef __cplusplus
}
#endif

#endif /* CR_EXTENSIONS_LOGO_H */
