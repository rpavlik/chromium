#include "cr_glstate.h"
#include "cr_error.h"

void crStateDiffContext( CRContext *from, CRContext *to )
{
	GLbitvalue bitID = from->bitid;
	GLbitvalue update = to->update;
	CRStateBits *sb = GetCurrentBits();

	crDebug( "Diffing two contexts!" );

	if (update & GLUPDATE_TRANS && sb->transform.dirty & bitID)
	{
		crStateTransformDiff (&(sb->transform), bitID,
							 &(from->transform), &(to->transform));
	}
	if (update & GLUPDATE_PIXEL && sb->pixel.dirty & bitID)
	{
		crStatePixelDiff	(&(sb->pixel), bitID,
							 &(from->pixel), &(to->pixel));
	}
	if (update & GLUPDATE_VIEWPORT && sb->viewport.dirty & bitID)
	{
		crStateViewportDiff	(&(sb->viewport), bitID,
								 &(from->viewport), &(to->viewport));
	}
	if (update & GLUPDATE_FOG && sb->fog.dirty & bitID)
	{
		crStateFogDiff	(&(sb->fog), bitID,
							 &(from->fog), &(to->fog));
	}
	if (update & GLUPDATE_TEXTURE && sb->texture.dirty & bitID)
	{
		crStateTextureDiff	(&(sb->texture), bitID,
							 &(from->texture), &(to->texture));
	}
	if (update & GLUPDATE_LISTS && sb->lists.dirty & bitID)
	{
		crStateListsDiff	(&(sb->lists), bitID,
							 &(from->lists), &(to->lists));
	}
	if (update & GLUPDATE_CLIENT && sb->client.dirty & bitID)
	{
		crStateClientDiff	(&(sb->client), bitID,
							 &(from->client), &(to->client));
	}
	if (update & GLUPDATE_BUFFER && sb->buffer.dirty & bitID)
	{
		crStateBufferDiff	(&(sb->buffer), bitID,
							 &(from->buffer), &(to->buffer));
	}
#if 0
	if (update & GLUPDATE_HINT && sb->hint.dirty & bitID)
	{
		crStateHintDiff	(&(sb->hint), bitID,
							 &(from->hint), &(to->hint));
	}
#endif
	if (update & GLUPDATE_LIGHTING && sb->lighting.dirty & bitID)
	{
		crStateLightingDiff	(&(sb->lighting), bitID,
								 &(from->lighting), &(to->lighting));
	}
	if (update & GLUPDATE_LINE && sb->line.dirty & bitID)
	{
		crStateLineDiff	(&(sb->line), bitID,
							 &(from->line), &(to->line));
	}
	if (update & GLUPDATE_POLYGON && sb->polygon.dirty & bitID)
	{
		crStatePolygonDiff	(&(sb->polygon), bitID,
							 &(from->polygon), &(to->polygon));
	}
	if (update & GLUPDATE_STENCIL && sb->stencil.dirty & bitID)
	{
		crStateStencilDiff	(&(sb->stencil), bitID,
							 &(from->stencil), &(to->stencil));
	}
	if (update & GLUPDATE_EVAL && sb->eval.dirty & bitID)
	{
		crStateEvaluatorDiff	(&(sb->eval), bitID,
							 &(from->eval), &(to->eval));
	}
#if 0
	if (update & GLUPDATE_IMAGING && sb->imaging.dirty & bitID)
	{
		crStateImagingDiff	(&(sb->imaging), bitID,
							 &(from->imaging), &(to->imaging));
	}
	if (update & GLUPDATE_SELECTION && sb->selection.dirty & bitID)
	{
		crStateSelectionDiff	(&(sb->selection), bitID,
								 &(from->selection), &(to->selection));
	}
#endif

	if (update & GLUPDATE_CURRENT && sb->current.dirty & bitID)
	{
		crStateCurrentDiff	(&(sb->current), bitID,
							 &(from->current), &(to->current));
	}
}
