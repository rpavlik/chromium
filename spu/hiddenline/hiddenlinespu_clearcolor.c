#include "hiddenlinespu.h"

void HIDDENLINESPU_APIENTRY hiddenlinespu_ClearColor( GLclampf r, GLclampf g, GLclampf b, GLclampf a )
{
	GET_CONTEXT(ctx);

	/* Don't pack this, we're going to set it ourselves */
	ctx->clear_color.r = r;
	ctx->clear_color.g = g;
	ctx->clear_color.b = b;
	ctx->clear_color.a = a;

	hiddenline_spu.super.ClearColor( r, g, b, a );
}
