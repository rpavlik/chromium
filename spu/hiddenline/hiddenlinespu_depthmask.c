#include "hiddenlinespu.h"
#include "hiddenlinespu_proto.h"

void HIDDENLINESPU_APIENTRY hiddenlinespu_DepthMask( GLboolean b )
{
	/* Throw this away!  If the application turns off writing to the
	 * depth buffer, our hidden line algorithm won't work.  Oops. */

	(void) b;
}
