#include "hiddenlinespu.h"
#include "hiddenlinespu_proto.h"

void HIDDENLINESPU_APIENTRY hiddenlinespu_Clear( GLbitfield buffer )
{
	/* If single_clear is set (the default), we'll ignore the user's
	 * calls to glClear() and do it ourself later when we replay the
	 * frame.
	 */
	if (!hiddenline_spu.single_clear)
		hiddenline_spu.super.Clear( buffer );
}
