/* cpg - 5/22/02 */

#include "chromium.h"
#include "api_templates.h"

/* This is necessary because OSF1 disagrees with Windows about arg 2. */

void glEdgeFlagPointer( GLsizei stride, const GLboolean *pointer )
{
	glim.EdgeFlagPointer( stride, pointer );
}

