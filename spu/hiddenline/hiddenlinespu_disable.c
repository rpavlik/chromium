#include "hiddenlinespu.h"
#include "hiddenlinespu_proto.h"
#include "cr_packfunctions.h"

void HIDDENLINESPU_APIENTRY hiddenlinespu_Disable( GLenum cap )
{
	/* We can't let the application disable the depth test, or 
	 * the wrong thing will happen.  Sorry. */

	if (cap != GL_DEPTH_TEST)
	{
		crPackDisable( cap );
	}
}
