/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdlib.h>
#include <stdio.h>
#include "state.h"
#include "state/cr_statetypes.h"
#include "state_internals.h"

void crStateHintInit (CRHintState *h) 
{
	h->perspectiveCorrection = GL_DONT_CARE;
	h->pointSmooth = GL_DONT_CARE;
	h->lineSmooth = GL_DONT_CARE;
	h->polygonSmooth = GL_DONT_CARE;
	h->fog = GL_DONT_CARE;
#ifdef CR_EXT_clip_volumne_hint
	h->clipVolumeClipping = GL_DONT_CARE;
#endif
#ifdef CR_ARB_texture_compression
	h->textureCompression = GL_DONT_CARE;
#endif
#ifdef CR_SGIS_generate_mipmap
	h->generateMipmap = GL_DONT_CARE;
#endif
}

void STATE_APIENTRY crStateHint(GLenum target, GLenum mode) 
{
	CRContext *g = GetCurrentContext();
	CRHintState *h = &(g->hint);
	CRStateBits *sb = GetCurrentBits();
	CRHintBits *hb = &(sb->hint);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					 "glHint called in Begin/End");
		return;
	}

	FLUSH();

	if (mode != GL_FASTEST && mode != GL_NICEST && mode != GL_DONT_CARE) {
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					 "glHint(mode)");
		return;
	}

	switch (target) {
	case GL_PERSPECTIVE_CORRECTION_HINT:
		h->perspectiveCorrection = mode;
		DIRTY(hb->perspectiveCorrection, g->neg_bitid);
		break;
	case GL_FOG_HINT:
		h->fog = mode;
		DIRTY(hb->fog, g->neg_bitid);
		break;
	case GL_LINE_SMOOTH_HINT:
		h->lineSmooth = mode;
		DIRTY(hb->lineSmooth, g->neg_bitid);
		break;
	case GL_POINT_SMOOTH_HINT:
		h->pointSmooth = mode;
		DIRTY(hb->pointSmooth, g->neg_bitid);
		break;
	case GL_POLYGON_SMOOTH_HINT:
		h->polygonSmooth = mode;
		DIRTY(hb->polygonSmooth, g->neg_bitid);
		break;
#ifdef CR_EXT_clip_volume_hint
	case GL_CLIP_VOLUME_CLIPPING_HINT_EXT:
		if (g->extensions.EXT_clip_volume_hint) {
			h->clipVolumeClipping = mode;
			DIRTY(hb->clipVolumeClipping, g->neg_bitid);
		}
		else {
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						 "glHint(target)");
			return;
		}
		break;
#endif
#ifdef CR_ARB_texture_compression
	case GL_TEXTURE_COMPRESSION_HINT_ARB:
		if (g->extensions.ARB_texture_compression) {
			h->textureCompression = mode;
			DIRTY(hb->textureCompression, g->neg_bitid);
		}
		else {
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						 "glHint(target)");
			return;
		}
		break;
#endif
#ifdef CR_SGIS_generate_mipmap
	case GL_GENERATE_MIPMAP_HINT_SGIS:
		if (g->extensions.SGIS_generate_mipmap) {
			h->generateMipmap = mode;
			DIRTY(hb->generateMipmap, g->neg_bitid);
		}
		else {
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						 "glHint(target)");
			return;
		}
		break;
#endif
	default:
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					 "glHint(target)");
		return;
	}

	DIRTY(hb->dirty, g->neg_bitid);
}
