#include "cr_glstate.h"
#include "cr_glwrapper.h"

void crStateCurrentInit( CRCurrentState *c )
{
	GLvectorf	zero_vector = {0.0f, 0.0f, 0.0f, 1.0f};
	GLcolorf	zero_color	= {0.0f, 0.0f, 0.0f, 1.0f};
	GLcolorf	one_color	= {0.0f, 0.0f, 0.0f, 1.0f};
	GLtexcoordf zero_texcoord = {0.0f, 0.0f, 0.0f, 1.0f};

	c->color	= zero_color;
	c->index	= 1.0f;
	c->texCoord = zero_texcoord;
	c->normal	= zero_vector;
	c->normal.z = 1.0f;

	c->numRestore = 0;
	c->wind = 0;
	c->isLoop = 0;

#if 0
    /* Compute the raster pos */
	if (cfg->id >= 0 && cfg->id < cfg->numprojectors) {
		GLrecti *b;
		b = &cfg->bounds[cfg->id][0];
		c->rasterpos.x = (GLfloat) b->x1;
		c->rasterpos.y = (GLfloat) b->y1;
	} else {
		c->rasterpos.x = 0.0f;
		c->rasterpos.y = 0.0f;
	}
	c->rasterpos.z = 0.0f;
	c->rasterpos.w = 1.0f;

	c->rasterpos_pre = c->rasterpos;
#endif

	c->rasterDistance = 0.0f;
	c->rasterColor = one_color;
	c->rasterTexture = zero_texcoord;
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

	c->wind = 0;
	c->isLoop = 0;
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

	if (c->isLoop)
	{
		crError( "Loop?" );
		c->isLoop = 0;
	}

	c->inBeginEnd = GL_FALSE;
}
