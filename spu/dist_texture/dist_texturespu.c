/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_spu.h"
#include "dist_texturespu.h"

SPUNamedFunctionTable dist_texture_table[] = {
	{ "TexImage2D", (SPUGenericFunction) dist_textureTexImage2D },
	{ NULL, NULL }
};
