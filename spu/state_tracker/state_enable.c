#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "cr_glstate.h"
#include "state/cr_statetypes.h"
#include "state_internals.h"

static void __enableSet (CRContext *g, CRStateBits *sb, GLbitvalue neg_bitid,
				GLenum cap, GLboolean val) 
{
#if 0

	int i;
	i = cap - GL_CLIP_PLANE0;
	if (i >= 0 && i < g->trans.maxclipplanes) {
		g->trans.clip[i] = val;
		sb->trans.enable = neg_bitid;
		sb->trans.dirty = neg_bitid;
		return;
	}
	i = cap - GL_LIGHT0;
	if (i >= 0 && i < g->lighting.maxlights) {
		g->lighting.light[i].enable = val;
		sb->lighting.light[i].dirty = neg_bitid;
		sb->lighting.light[i].enable = neg_bitid;
		sb->lighting.dirty = neg_bitid;
		return;
	}
#endif

	switch (cap) {
#if 0
		case GL_AUTO_NORMAL:
			g->eval.autonormal = val;
			sb->eval.enable = neg_bitid;
			sb->eval.dirty = neg_bitid;
			break;
#endif
#if 1
		case GL_ALPHA_TEST:
			g->buffer.alphaTest = val;
			sb->buffer.enable = neg_bitid;
			sb->buffer.dirty = neg_bitid;
			break;
#endif
#if 1
		case GL_BLEND:
			g->buffer.blend = val;
			sb->buffer.enable = neg_bitid;
			sb->buffer.dirty = neg_bitid;
			break;
#endif
#if 0
		case GL_COLOR_MATERIAL :
			g->lighting.colormaterial = val;
			sb->lighting.enable = neg_bitid;
			sb->lighting.dirty = neg_bitid;
			break;
#endif
#if 0
		case GL_CULL_FACE :
			g->polygon.cullface = val;
			sb->polygon.enable = neg_bitid;
			sb->polygon.dirty = neg_bitid;
			break;
#endif
#if 1
		case GL_DEPTH_TEST :
			g->buffer.depthTest = val;
			sb->buffer.enable = neg_bitid;
			sb->buffer.dirty = neg_bitid;
			break;
#endif
#if 1
		case GL_DITHER :
			g->buffer.dither = val;
			sb->buffer.enable = neg_bitid;
			sb->buffer.dirty = neg_bitid;
			break;
#endif
#if 0
		case GL_FOG :
			g->fog.enable = val;
			sb->fog.enable = neg_bitid;
			sb->fog.dirty = neg_bitid;
			break;
#endif
#if 0
		case GL_LIGHTING :
			g->lighting.lighting = val;
			sb->lighting.enable = neg_bitid;
			sb->lighting.dirty = neg_bitid;
			break;
#endif
#if 0
		case GL_LINE_SMOOTH :
			g->line.linesmooth = val;
			sb->line.enable = neg_bitid;
			sb->line.dirty = neg_bitid;
			break;
#endif
#if 0
		case GL_LINE_STIPPLE :
			g->line.linestipple = val;
			sb->line.enable = neg_bitid;
			sb->line.dirty = neg_bitid;
			break;
#endif
#if 1
		case GL_LOGIC_OP :
			g->buffer.logicOp = val;
			sb->buffer.enable = neg_bitid;
			sb->buffer.dirty = neg_bitid;
			break;
#endif
#if 1
		case GL_NORMALIZE :
			g->current.normalize = val;
			sb->current.enable = neg_bitid;
			sb->current.dirty = neg_bitid;
			break;
#endif
#if 0
		case GL_POINT_SMOOTH :
			g->line.pointsmooth = val;
			sb->line.enable = neg_bitid;
			sb->line.dirty = neg_bitid;
			break;
#endif
#if 0
		case GL_POLYGON_OFFSET_FILL:
			g->polygon.polygonoffset = val;
			sb->polygon.enable = neg_bitid;
			sb->polygon.dirty = neg_bitid;
			break;
#endif
#if 0
		case GL_POLYGON_SMOOTH :
			g->polygon.polygonsmooth = val;
			sb->polygon.enable = neg_bitid;
			sb->polygon.dirty = neg_bitid;
			break;
#endif
#if 0
		case GL_POLYGON_STIPPLE :
			g->polygon.polygonstipple = val;
			sb->polygon.enable = neg_bitid;
			sb->polygon.dirty = neg_bitid;
			break;
#endif
#if 0
		case GL_SCISSOR_TEST :
			g->viewport.scissortest = val;
			sb->viewport.enable = neg_bitid;
			sb->viewport.dirty = neg_bitid;
			break;
#endif
#if 0
		case GL_STENCIL_TEST :
			g->stencil.stenciltest= val;
			sb->stencil.enable = neg_bitid;
			sb->stencil.dirty = neg_bitid;
			break;
#endif
#if 0
		case GL_TEXTURE_1D :
			g->texture.enabled1d = val;
			sb->texture.enable = neg_bitid;
			sb->texture.dirty = neg_bitid;
			break;
#endif
#if 0
		case GL_TEXTURE_2D :
			g->texture.enabled2d = val;
			sb->texture.enable = neg_bitid;
			sb->texture.dirty = neg_bitid;
			break;
#endif
#if 0
		case GL_TEXTURE_3D :
			g->texture.enabled3d = val;
			sb->texture.enable = neg_bitid;
			sb->texture.dirty = neg_bitid;
			break;
#endif
#if 0
		case GL_TEXTURE_GEN_Q :
			g->texture.texturegen.q = val;
			sb->texture.enable = neg_bitid;
			sb->texture.dirty = neg_bitid;
			break;
#endif
#if 0
		case GL_TEXTURE_GEN_R :
			g->texture.texturegen.p = val;
			sb->texture.enable = neg_bitid;
			sb->texture.dirty = neg_bitid;
			break;
#endif
#if 0
		case GL_TEXTURE_GEN_S :
			g->texture.texturegen.s = val;
			sb->texture.enable = neg_bitid;
			sb->texture.dirty = neg_bitid;
			break;
#endif
#if 0
		case GL_TEXTURE_GEN_T :
			g->texture.texturegen.t = val;
			sb->texture.enable = neg_bitid;
			sb->texture.dirty = neg_bitid;
			break;
#endif

#if 0
		case GL_MAP1_COLOR_4 :
		case GL_MAP1_INDEX :
		case GL_MAP1_NORMAL :
		case GL_MAP1_TEXTURE_COORD_1 :
		case GL_MAP1_TEXTURE_COORD_2 :
		case GL_MAP1_TEXTURE_COORD_3 :
		case GL_MAP1_TEXTURE_COORD_4 :
		case GL_MAP1_VERTEX_3 :
		case GL_MAP1_VERTEX_4 :
			g->eval.enable1d[cap - GL_MAP1_COLOR_4] = val;
			sb->eval.enable1d[cap - GL_MAP1_COLOR_4] = neg_bitid;
			sb->eval.dirty = neg_bitid;
			break;
#endif

#if 0
		case GL_MAP2_COLOR_4 :
		case GL_MAP2_INDEX :
		case GL_MAP2_NORMAL :
		case GL_MAP2_TEXTURE_COORD_1 :
		case GL_MAP2_TEXTURE_COORD_2 :
		case GL_MAP2_TEXTURE_COORD_3 :
		case GL_MAP2_TEXTURE_COORD_4 :
		case GL_MAP2_VERTEX_3 :
		case GL_MAP2_VERTEX_4 :
			g->eval.enable2d[cap - GL_MAP2_COLOR_4] = val;
			sb->eval.enable2d[cap - GL_MAP2_COLOR_4] = neg_bitid;
			sb->eval.dirty = neg_bitid;
			break;
#endif

		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glEnable/glDisable called with bogus cap: %d", cap);
			return;
	}
}


void STATE_APIENTRY crStateEnable (GLenum cap) {
	CRContext *g = GetCurrentContext();
	CRStateBits *sb = GetCurrentBits();

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glEnable called in begin/end");
		return;
	}

	FLUSH();

	__enableSet(g, sb, g->neg_bitid, cap, GL_TRUE);

}


void STATE_APIENTRY crStateDisable (GLenum cap) {
	CRContext *g = GetCurrentContext();
	CRStateBits *sb = GetCurrentBits();

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glDisable called in begin/end");
		return;
	}

	FLUSH();

	__enableSet(g, sb, g->neg_bitid, cap, GL_FALSE);
}
