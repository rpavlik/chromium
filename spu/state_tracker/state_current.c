#include "cr_glstate.h"
#include "cr_glwrapper.h"

void crStateCurrentInit( CRCurrentState *c )
{
	GLvectorf	default_normal     = {0.0f, 0.0f, 1.0f, 1.0f};
	GLcolorf	default_color	     = {1.0f, 1.0f, 1.0f, 1.0f};
	GLtexcoordf default_texcoord = {0.0f, 0.0f, 0.0f, 1.0f};
	GLvectorf default_rasterpos  = {0.0f, 0.0f, 0.0f, 1.0f};

	c->color	= default_color;
	c->index	= 1.0f;
	c->texCoord = default_texcoord;
	c->normal	= default_normal;

	c->rasterPos = default_rasterpos;
	c->rasterPosPre = c->rasterPos;

	c->rasterDistance = 0.0f;
	c->rasterColor = default_color;
	c->rasterTexture = default_texcoord;
	c->rasterValid = GL_TRUE;
	c->rasterIndex = 1.0f;

	c->edgeFlag = GL_TRUE;
	c->normalize = GL_FALSE;

	c->inBeginEnd = GL_FALSE;
	c->beginEndNum = 0;
#if 0
	c->beginEndMax = cfg->beginend_max;
#endif
	c->mode = 0x10; /* Undefined Mode */
	c->flushOnEnd = 0;

#if 0
	c->current = cfg->current;
#endif
}

void crStateSetCurrentPointers( CRContext *ctx, CRCurrentStatePointers *current )
{
	CRCurrentState *c = &(ctx->current);
	c->current = current;
}

void STATE_APIENTRY crStateBegin( GLenum mode )
{
	CRContext *g = GetCurrentContext();
	CRCurrentState *c = &(g->current);

	if (mode > GL_POLYGON)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "Begin called with invalid mode: %d", mode);
		return;
	}

	if (c->inBeginEnd)
	{
		crStateError( __LINE__, __FILE__, GL_INVALID_OPERATION, "glBegin called inside Begin/End");
		return;
	}

	c->inBeginEnd = GL_TRUE;
	c->mode = mode;
	c->beginEndNum++;
}

void STATE_APIENTRY crStateEnd( void )
{
	CRContext *g = GetCurrentContext();
	CRCurrentState *c = &(g->current);

	if (!c->inBeginEnd)
	{
		crStateError( __LINE__, __FILE__, GL_INVALID_OPERATION, "glEnd called outside Begin/End" );
		return;
	}


	c->inBeginEnd = GL_FALSE;
}

void crStateCurrentSwitch (CRCurrentBits *c, GLbitvalue bitID,
					 CRCurrentState *from, CRCurrentState *to) 
{
	GLbitvalue nbitID = ~bitID;

	if (c->enable & bitID) {
		if (from->normalize != to->normalize) {
			if (to->normalize == GL_TRUE)
				diff_api.Enable(GL_NORMALIZE);
			else
				diff_api.Disable(GL_NORMALIZE);
			c->enable = GLBITS_ONES;
		}
		c->enable &= nbitID;
		c->dirty = GLBITS_ONES;
	}

	if (c->raster & bitID) {
		if (to->rasterValid) {
			if (to->rasterPosPre.x != from->rasterPos.x ||
				to->rasterPosPre.y != from->rasterPos.y) {
					GLvectorf p;
					p.x = to->rasterPosPre.x - from->rasterPos.x;
					p.y = to->rasterPosPre.y - from->rasterPos.y;
					diff_api.Bitmap(0, 0, 0.0f, 0.0f, p.x, p.y, 0);
					c->raster = GLBITS_ONES;
					c->dirty  = GLBITS_ONES;
			}
		}
		c->raster &= nbitID;
	}

	/* Vertex Current State Switch Code */

	/* Its important that we don't do a value check here because
	** current may not actaully have the correct values, I think...
	** We also need to restore the current state tracking pointer
	** since the packing functions will set it.
	*/

	/* NEED TO FIX THIS!!!!!! */
	if (c->color & bitID) {
		if (COMPARE_COLOR(from->color,to->color)) {
			diff_api.Color4fv ((GLfloat *) &(to->color));
			c->color = GLBITS_ONES;
			c->dirty = GLBITS_ONES;
		}
		c->color &= nbitID;
	}

	if (c->index & bitID) {
		if (to->index != from->index) {
			diff_api.Indexf (to->index);
			c->index = GLBITS_ONES;
			c->dirty = GLBITS_ONES;
		}
		c->index &= nbitID;
	}

	if (c->normal & bitID) {
		if (COMPARE_VECTOR (from->normal, to->normal)) {
			diff_api.Normal3fv ((GLfloat *) &(to->normal.x));
			c->normal = GLBITS_ONES;
			c->dirty = GLBITS_ONES;
		}
		c->normal &= nbitID;
	}

	if (c->texCoord & bitID) {
		if (COMPARE_TEXCOORD (from->texCoord, to->texCoordPre)) {
			diff_api.TexCoord4fv ((GLfloat *) &(to->texCoord.s));
			c->normal = GLBITS_ONES;
			c->dirty = GLBITS_ONES;
		}
		c->texCoord &= nbitID;
	}

	c->dirty &= nbitID;
}

void crStateCurrentDiff (CRCurrentBits *c, GLbitvalue bitID,
					 CRCurrentState *from, CRCurrentState *to) 
{
	GLbitvalue nbitID = ~bitID;

	if (c->enable & bitID) {
		if (from->normalize != to->normalize) {
			if (to->normalize == GL_TRUE)
				diff_api.Enable(GL_NORMALIZE);
			else
				diff_api.Disable(GL_NORMALIZE);
			from->normalize = to->normalize;
		}
		c->enable &= nbitID;
	}

	if (c->raster & bitID) {
		from->rasterValid = to->rasterValid;
		if (to->rasterValid) {
			if (to->rasterPosPre.x != from->rasterPos.x ||
				to->rasterPosPre.y != from->rasterPos.y) {
					GLvectorf p;
					p.x = to->rasterPosPre.x - from->rasterPos.x;
					p.y = to->rasterPosPre.y - from->rasterPos.y;
					diff_api.Bitmap(0, 0, 0.0f, 0.0f, p.x, p.y, 0);
			}
			from->rasterPos = to->rasterPos;
		}
		c->raster &= nbitID;
	}

	/* Vertex Current State Sync Code */
	/* Some things to note here:
	** 1) Compare is done against the pre value since the
	**    current value includes the geometry info.
	** 2) Update is done with the current value since
	**    the server will be getting the geometry block
	** 3) Copy is done outside of the compare to ensure
	**    that it happens.
	*/
	if (c->color & bitID) {
		if (COMPARE_COLOR(from->color,to->colorPre)) {
			diff_api.Color4fv ((GLfloat *) &(to->colorPre.r));
		}
		from->color = to->color;
		c->color &= nbitID;
	}

	if (c->index & bitID) {
		if (from->index != to->indexPre) {
			diff_api.Indexf (to->index);
		}
		from->index = to->index;
		c->index &= nbitID;
	}

	if (c->normal & bitID) {
		if (COMPARE_VECTOR (from->normal, to->normalPre)) {
			diff_api.Normal3fv ((GLfloat *) &(to->normalPre.x));
		}
		from->normal = to->normal;
		c->normal &= nbitID;
	}

	if (c->texCoord & bitID) {
		if (COMPARE_TEXCOORD (from->texCoord, to->texCoordPre)) {
			diff_api.TexCoord4fv ((GLfloat *) &(to->texCoordPre.s));
		}
		from->texCoord = to->texCoord;
		c->texCoord &= nbitID;
	}

	if (c->edgeFlag & bitID) {
		if (from->edgeFlag != to->edgeFlagPre) {
			diff_api.EdgeFlag (to->edgeFlagPre);
		}
		from->edgeFlag = to->edgeFlag;
		c->edgeFlag &= nbitID;
	}

	c->dirty &= nbitID;
}
