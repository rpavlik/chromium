#include "hiddenlinespu.h"

void HIDDENLINESPU_APIENTRY hiddenlinespu_ClearColor( GLclampf r, GLclampf g, GLclampf b, GLclampf a )
{
	/* Don't pack this, we're going to set it ourselves */
	hiddenline_spu.clear_r = r;
	hiddenline_spu.clear_g = g;
	hiddenline_spu.clear_b = b;

	hiddenline_spu.super.ClearColor( r, g, b, a );
}
