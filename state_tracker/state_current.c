/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include <stdlib.h>
#include "cr_glstate.h"
#include "cr_glwrapper.h"
#include "state_internals.h"

void crStateCurrentInit( CRCurrentState *c )
{
	GLvectorf	default_normal     = {0.0f, 0.0f, 1.0f, 1.0f};
	GLcolorf	default_color	     = {1.0f, 1.0f, 1.0f, 1.0f};
	GLcolorf	default_secondaryColor = {0.0f, 0.0f, 0.0f, 0.0f};
	GLtexcoordf default_texcoord = {0.0f, 0.0f, 0.0f, 1.0f};
	GLvectorf default_rasterpos  = {0.0f, 0.0f, 0.0f, 1.0f};
	int i;

	c->color	= default_color;
	c->secondaryColor = default_secondaryColor;
	c->index	= 1.0f;
	for (i = 0 ; i < CR_MAX_TEXTURE_UNITS ; i++)
	{
		c->texCoord[i] = default_texcoord;
	}
	c->normal	= default_normal;

	c->rasterPos = default_rasterpos;
	c->rasterPosPre = c->rasterPos;

	c->rasterDistance = 0.0f;
	c->rasterColor = default_color;
	c->rasterSecondaryColor = default_secondaryColor;
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

void STATE_APIENTRY crStateColor3f( GLfloat r, GLfloat g, GLfloat b )
{
	crStateColor4f(r, g, b, 1.0F);
}

void STATE_APIENTRY crStateColor3fv( const GLfloat *color )
{
	crStateColor4f( color[0], color[1], color[2], 1.0F );
}

void STATE_APIENTRY crStateColor4f( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{
	CRContext *g = GetCurrentContext();
	CRCurrentState *c = &(g->current);
	CRStateBits *sb = GetCurrentBits();
	CRCurrentBits *cb = &(sb->current);

	FLUSH();

        c->color.r = red;
        c->color.g = green;
        c->color.b = blue;
        c->color.a = alpha;

        DIRTY(cb->dirty, g->neg_bitid);
        DIRTY(cb->color, g->neg_bitid);
}

void STATE_APIENTRY crStateColor4fv( const GLfloat *color )
{
	crStateColor4f( color[0], color[1], color[2], color[3] );
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

void crStateCurrentSwitch( GLuint maxTextureUnits,
						   CRCurrentBits *c, GLbitvalue *bitID,
						   CRCurrentState *from, CRCurrentState *to )
{
	unsigned int i,j;
	GLbitvalue nbitID[CR_MAX_BITARRAY];

	for (j=0;j<CR_MAX_BITARRAY;j++)
		nbitID[j] = ~bitID[j];

	if (CHECKDIRTY(c->enable, bitID)) {
		if (from->normalize != to->normalize) {
			if (to->normalize == GL_TRUE)
				diff_api.Enable(GL_NORMALIZE);
			else
				diff_api.Disable(GL_NORMALIZE);
			FILLDIRTY(c->enable);
			FILLDIRTY(c->dirty);
		}
		INVERTDIRTY(c->enable, nbitID);
	}

	if (CHECKDIRTY(c->raster, bitID)) {
		if (to->rasterValid) {
			if (to->rasterPosPre.x != from->rasterPos.x ||
				to->rasterPosPre.y != from->rasterPos.y) {
					GLvectorf p;
					p.x = to->rasterPosPre.x - from->rasterPos.x;
					p.y = to->rasterPosPre.y - from->rasterPos.y;
					diff_api.Bitmap(0, 0, 0.0f, 0.0f, p.x, p.y, 0);
					FILLDIRTY(c->raster);
					FILLDIRTY(c->dirty);
			}
		}
		INVERTDIRTY(c->raster, nbitID);
	}

	/* Vertex Current State Switch Code */

	/* Its important that we don't do a value check here because
	** current may not actaully have the correct values, I think...
	** We also need to restore the current state tracking pointer
	** since the packing functions will set it.
	*/

	/* NEED TO FIX THIS!!!!!! */
	if (CHECKDIRTY(c->color, bitID)) {
           if (COMPARE_COLOR(from->color,to->color)) {
			diff_api.Color4fv ((GLfloat *) &(to->color));
			FILLDIRTY(c->color);
			FILLDIRTY(c->dirty);
		}
		INVERTDIRTY(c->color, nbitID);
	}

	/* NEED TO FIX THIS, ALSO?!!!!! */
#ifdef CR_EXT_secondary_color
	if (CHECKDIRTY(c->secondaryColor, bitID)) {
		if (COMPARE_COLOR(from->secondaryColor,to->secondaryColor)) {
			diff_api.SecondaryColor3fvEXT ((GLfloat *) &(to->secondaryColor));
			FILLDIRTY(c->secondaryColor);
			FILLDIRTY(c->dirty);
		}
		INVERTDIRTY(c->secondaryColor, nbitID);
	}
#endif

	if (CHECKDIRTY(c->index, bitID)) {
		if (to->index != from->index) {
			diff_api.Indexf (to->index);
			FILLDIRTY(c->index);
			FILLDIRTY(c->dirty);
		}
		INVERTDIRTY(c->index, nbitID);
	}

	if (CHECKDIRTY(c->normal, bitID)) {
		if (COMPARE_VECTOR (from->normal, to->normal)) {
			diff_api.Normal3fv ((GLfloat *) &(to->normal.x));
			FILLDIRTY(c->normal);
			FILLDIRTY(c->dirty);
		}
		INVERTDIRTY(c->normal, nbitID);
	}

	for (i = 0; i < maxTextureUnits; i++)
	{
		if (CHECKDIRTY(c->texCoord[i], bitID)) {
			if (COMPARE_TEXCOORD (from->texCoord[i], to->texCoordPre[i])) {
				diff_api.MultiTexCoord4fvARB (i+GL_TEXTURE0_ARB, (GLfloat *) &(to->texCoord[i].s));
				FILLDIRTY(c->normal);
				FILLDIRTY(c->dirty);
			}
			INVERTDIRTY(c->texCoord[i], nbitID);
		}
	}

	INVERTDIRTY(c->dirty, nbitID);
}

void crStateCurrentDiff (CRCurrentBits *c, GLbitvalue *bitID,
					 CRCurrentState *from, CRCurrentState *to)
{
	int i,j;
	GLbitvalue nbitID[CR_MAX_BITARRAY];

	for (j=0;j<CR_MAX_BITARRAY;j++)
		nbitID[j] = ~bitID[j];

	if (CHECKDIRTY(c->enable, bitID)) {
		if (from->normalize != to->normalize) {
			if (to->normalize == GL_TRUE)
				diff_api.Enable(GL_NORMALIZE);
			else
				diff_api.Disable(GL_NORMALIZE);
			from->normalize = to->normalize;
		}
		INVERTDIRTY(c->enable, nbitID);
	}

	if (CHECKDIRTY(c->raster, bitID)) {
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
		INVERTDIRTY(c->raster, nbitID);
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
	if (CHECKDIRTY(c->color, bitID)) {
		if (COMPARE_COLOR(from->color,to->colorPre)) {
			diff_api.Color4fv ((GLfloat *) &(to->colorPre.r));
		}
		from->color = to->color;
		INVERTDIRTY(c->color, nbitID);
	}

#ifdef CR_EXT_secondary_color
	if (CHECKDIRTY(c->secondaryColor, bitID)) {
		if (COMPARE_COLOR(from->secondaryColor,to->secondaryColorPre)) {
			diff_api.SecondaryColor3fvEXT ((GLfloat *) &(to->secondaryColorPre.r));
		}
		from->secondaryColor = to->secondaryColor;
		INVERTDIRTY(c->secondaryColor, nbitID);
	}
#endif

	if (CHECKDIRTY(c->index, bitID)) {
		if (from->index != to->indexPre) {
			diff_api.Indexf (to->index);
		}
		from->index = to->index;
		INVERTDIRTY(c->index, nbitID);
	}

	if (CHECKDIRTY(c->normal, bitID)) {
		if (COMPARE_VECTOR (from->normal, to->normalPre)) {
			diff_api.Normal3fv ((GLfloat *) &(to->normalPre.x));
		}
		from->normal = to->normal;
		INVERTDIRTY(c->normal, nbitID);
	}

	for ( i = 0 ; i < CR_MAX_TEXTURE_UNITS ; i++)
	{
		if (CHECKDIRTY(c->texCoord[i], bitID)) {
			if (COMPARE_TEXCOORD (from->texCoord[i], to->texCoordPre[i])) {
				diff_api.MultiTexCoord4fvARB (GL_TEXTURE0_ARB + i, (GLfloat *) &(to->texCoordPre[i].s));
			}
			from->texCoord[i] = to->texCoord[i];
			INVERTDIRTY(c->texCoord[i], nbitID);
		}
	}

	if (CHECKDIRTY(c->edgeFlag, bitID)) {
		if (from->edgeFlag != to->edgeFlagPre) {
			diff_api.EdgeFlag (to->edgeFlagPre);
		}
		from->edgeFlag = to->edgeFlag;
		INVERTDIRTY(c->edgeFlag, nbitID);
	}

	INVERTDIRTY(c->dirty, nbitID);
}
