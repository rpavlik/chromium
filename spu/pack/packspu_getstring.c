#include <stdio.h>
#include "packspu.h"
#include "cr_packfunctions.h"
#include "cr_glwrapper.h"
#include "cr_net.h"
#include "state/cr_statefuncs.h"

const GLubyte * PACKSPU_APIENTRY packspu_GetString( GLenum name )
{
	// It must be this way.
	return crStateGetString( name );
}
