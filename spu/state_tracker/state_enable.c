#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "cr_glstate.h"
#include "state/cr_statetypes.h"
#include "state_internals.h"

static void __enableSet (CRContext *g, CRStateBits *sb, GLbitvalue neg_bitid,
				GLenum cap, GLboolean val) 
{
	int i;
	i = cap - GL_CLIP_PLANE0;
	if (i >= 0 && i < g->transform.maxClipPlanes) {
		g->transform.clip[i] = val;
		sb->transform.enable = neg_bitid;
		sb->transform.dirty = neg_bitid;
		return;
	}
	i = cap - GL_LIGHT0;
	if (i >= 0 && i < g->lighting.maxLights) {
		g->lighting.light[i].enable = val;
		sb->lighting.light[i].dirty = neg_bitid;
		sb->lighting.light[i].enable = neg_bitid;
		sb->lighting.dirty = neg_bitid;
		return;
	}

	switch (cap) {
		case GL_AUTO_NORMAL:
			g->eval.autoNormal = val;
			sb->eval.enable = neg_bitid;
			sb->eval.dirty = neg_bitid;
			break;
		case GL_ALPHA_TEST:
			g->buffer.alphaTest = val;
			sb->buffer.enable = neg_bitid;
			sb->buffer.dirty = neg_bitid;
			break;
		case GL_BLEND:
			g->buffer.blend = val;
			sb->buffer.enable = neg_bitid;
			sb->buffer.dirty = neg_bitid;
			break;
		case GL_COLOR_MATERIAL :
			g->lighting.colorMaterial = val;
			sb->lighting.enable = neg_bitid;
			sb->lighting.dirty = neg_bitid;
			break;
		case GL_CULL_FACE :
			g->polygon.cullFace = val;
			sb->polygon.enable = neg_bitid;
			sb->polygon.dirty = neg_bitid;
			break;
		case GL_DEPTH_TEST :
			g->buffer.depthTest = val;
			sb->buffer.enable = neg_bitid;
			sb->buffer.dirty = neg_bitid;
			break;
		case GL_DITHER :
			g->buffer.dither = val;
			sb->buffer.enable = neg_bitid;
			sb->buffer.dirty = neg_bitid;
			break;
		case GL_FOG :
			g->fog.enable = val;
			sb->fog.enable = neg_bitid;
			sb->fog.dirty = neg_bitid;
			break;
		case GL_LIGHTING :
			g->lighting.lighting = val;
			sb->lighting.enable = neg_bitid;
			sb->lighting.dirty = neg_bitid;
			break;
		case GL_LINE_SMOOTH :
			g->line.lineSmooth = val;
			sb->line.enable = neg_bitid;
			sb->line.dirty = neg_bitid;
			break;
		case GL_LINE_STIPPLE :
			g->line.lineStipple = val;
			sb->line.enable = neg_bitid;
			sb->line.dirty = neg_bitid;
			break;
		case GL_LOGIC_OP :
			g->buffer.logicOp = val;
			sb->buffer.enable = neg_bitid;
			sb->buffer.dirty = neg_bitid;
			break;
		case GL_NORMALIZE :
			g->current.normalize = val;
			sb->current.enable = neg_bitid;
			sb->current.dirty = neg_bitid;
			break;
		case GL_POINT_SMOOTH :
			g->line.pointSmooth = val;
			sb->line.enable = neg_bitid;
			sb->line.dirty = neg_bitid;
			break;
		case GL_POLYGON_OFFSET_FILL:
			g->polygon.polygonOffset = val;
			sb->polygon.enable = neg_bitid;
			sb->polygon.dirty = neg_bitid;
			break;
		case GL_POLYGON_SMOOTH :
			g->polygon.polygonSmooth = val;
			sb->polygon.enable = neg_bitid;
			sb->polygon.dirty = neg_bitid;
			break;
		case GL_POLYGON_STIPPLE :
			g->polygon.polygonStipple = val;
			sb->polygon.enable = neg_bitid;
			sb->polygon.dirty = neg_bitid;
			break;
		case GL_SCISSOR_TEST :
			g->viewport.scissorTest = val;
			sb->viewport.enable = neg_bitid;
			sb->viewport.dirty = neg_bitid;
			break;
		case GL_STENCIL_TEST :
			g->stencil.stencilTest= val;
			sb->stencil.enable = neg_bitid;
			sb->stencil.dirty = neg_bitid;
			break;
		case GL_TEXTURE_1D :
			g->texture.enabled1D = val;
			sb->texture.enable = neg_bitid;
			sb->texture.dirty = neg_bitid;
			break;
		case GL_TEXTURE_2D :
			g->texture.enabled2D = val;
			sb->texture.enable = neg_bitid;
			sb->texture.dirty = neg_bitid;
			break;
#if 0
		case GL_TEXTURE_3D :
			g->texture.enabled3D = val;
			sb->texture.enable = neg_bitid;
			sb->texture.dirty = neg_bitid;
			break;
#endif
		case GL_TEXTURE_GEN_Q :
			g->texture.textureGen.q = val;
			sb->texture.enable = neg_bitid;
			sb->texture.dirty = neg_bitid;
			break;
		case GL_TEXTURE_GEN_R :
			g->texture.textureGen.p = val;
			sb->texture.enable = neg_bitid;
			sb->texture.dirty = neg_bitid;
			break;
		case GL_TEXTURE_GEN_S :
			g->texture.textureGen.s = val;
			sb->texture.enable = neg_bitid;
			sb->texture.dirty = neg_bitid;
			break;
		case GL_TEXTURE_GEN_T :
			g->texture.textureGen.t = val;
			sb->texture.enable = neg_bitid;
			sb->texture.dirty = neg_bitid;
			break;
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
