/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "cr_glstate.h"
#include "state/cr_statetypes.h"
#include "state_internals.h"
#include "state_extensionfuncs.h"
#include "cr_error.h"
#include "cr_mem.h"

void crStateAttribInit (CRAttribState *a) 
{
	int i;
	a->attribStackDepth = 0;
	a->accumBufferStackDepth = 0;
	a->colorBufferStackDepth = 0;
	a->currentStackDepth = 0;
	a->depthBufferStackDepth = 0;
	a->enableStackDepth = 0;

	for ( i = 0 ; i < CR_MAX_ATTRIB_STACK_DEPTH ; i++)
	{
		a->enableStack[i].clip = NULL;
		a->enableStack[i].light = NULL;
	}
	a->evalStackDepth = 0;
	a->fogStackDepth = 0;
	a->lightingStackDepth = 0;
	for ( i = 0 ; i < CR_MAX_ATTRIB_STACK_DEPTH ; i++)
	{
		a->lightingStack[i].light = NULL;
	}
	a->lineStackDepth = 0;
	a->listStackDepth = 0;
	a->pixelModeStackDepth = 0;
	a->pointStackDepth = 0;
	a->polygonStackDepth = 0;
	a->polygonStippleStackDepth = 0;
	a->scissorStackDepth = 0;
	a->stencilBufferStackDepth = 0;
	a->textureStackDepth = 0;
	a->transformStackDepth = 0;
	a->viewportStackDepth = 0;

	a->maxAttribStackDepth = CR_MAX_ATTRIB_STACK_DEPTH; // XXX
}

void STATE_APIENTRY crStatePushAttrib(GLbitfield mask)
{
	CRContext *g = GetCurrentContext();
	CRAttribState *a = &(g->attrib);
	CRStateBits *sb = GetCurrentBits();
	CRAttribBits *ab = &(sb->attrib);
	int i;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glPushAttrib called in Begin/End");
		return;
	}

	if (a->attribStackDepth == CR_MAX_ATTRIB_STACK_DEPTH - 1)
	{
		crStateError(__LINE__, __FILE__, GL_STACK_OVERFLOW, "glPushAttrib called with a full stack!" );
	}

	FLUSH();

	a->pushMaskStack[a->attribStackDepth++] = mask;

	if (mask & GL_ACCUM_BUFFER_BIT)
	{
		a->accumBufferStack[a->accumBufferStackDepth++].accumClearValue = g->buffer.accumClearValue;
	}
	if (mask & GL_COLOR_BUFFER_BIT)
	{
		a->colorBufferStack[a->colorBufferStackDepth].alphaTest = g->buffer.alphaTest;
		a->colorBufferStack[a->colorBufferStackDepth].alphaTestFunc = g->buffer.alphaTestFunc;
		a->colorBufferStack[a->colorBufferStackDepth].alphaTestRef = g->buffer.alphaTestRef;
		a->colorBufferStack[a->colorBufferStackDepth].blend = g->buffer.blend;
		a->colorBufferStack[a->colorBufferStackDepth].blendSrc = g->buffer.blendSrc;
		a->colorBufferStack[a->colorBufferStackDepth].blendDst = g->buffer.blendDst;
		a->colorBufferStack[a->colorBufferStackDepth].blendColor = g->buffer.extensions.blendColor;
		a->colorBufferStack[a->colorBufferStackDepth].blendEquation = g->buffer.extensions.blendEquation;
		a->colorBufferStack[a->colorBufferStackDepth].dither = g->buffer.dither;
		a->colorBufferStack[a->colorBufferStackDepth].drawBuffer = g->buffer.drawBuffer;
		a->colorBufferStack[a->colorBufferStackDepth].logicOp = g->buffer.logicOp;
		// a->colorBufferStack[a->colorBufferStackDepth].indexLogicOp = g->buffer.indexLogicOp;
		a->colorBufferStack[a->colorBufferStackDepth].logicOpMode = g->buffer.logicOpMode;
		a->colorBufferStack[a->colorBufferStackDepth].colorClearValue = g->buffer.colorClearValue;
		a->colorBufferStack[a->colorBufferStackDepth].indexClearValue = g->buffer.indexClearValue;
		a->colorBufferStack[a->colorBufferStackDepth].colorWriteMask = g->buffer.colorWriteMask;
		a->colorBufferStack[a->colorBufferStackDepth].indexWriteMask = g->buffer.indexWriteMask;
		a->colorBufferStackDepth++;
	}
	if (mask & GL_CURRENT_BIT)
	{
		a->currentStack[a->currentStackDepth].color = g->current.color;
		a->currentStack[a->currentStackDepth].index = g->current.index;
		a->currentStack[a->currentStackDepth].normal = g->current.normal;
		for (i = 0 ; i < CR_MAX_TEXTURE_UNITS ; i++)
		{
			a->currentStack[a->currentStackDepth].texCoord[i] = g->current.texCoord[i];
		}
		a->currentStack[a->currentStackDepth].rasterPos = g->current.rasterPos;
		a->currentStack[a->currentStackDepth].rasterValid = g->current.rasterValid;
		a->currentStack[a->currentStackDepth].rasterColor = g->current.rasterColor;
		a->currentStack[a->currentStackDepth].rasterIndex = g->current.rasterIndex;
		a->currentStack[a->currentStackDepth].rasterTexture = g->current.rasterTexture;
		a->currentStack[a->currentStackDepth].edgeFlag = g->current.edgeFlag;
		a->currentStackDepth++;
	}
	if (mask & GL_DEPTH_BUFFER_BIT)
	{
		a->depthBufferStack[a->depthBufferStackDepth].depthTest = g->buffer.depthTest;
		a->depthBufferStack[a->depthBufferStackDepth].depthFunc = g->buffer.depthFunc;
		a->depthBufferStack[a->depthBufferStackDepth].depthClearValue = g->buffer.depthClearValue;
		a->depthBufferStack[a->depthBufferStackDepth].depthMask = g->buffer.depthMask;
		a->depthBufferStackDepth++;
	}
	if (mask & GL_ENABLE_BIT)
	{
		if (a->enableStack[a->enableStackDepth].clip == NULL)
		{
			a->enableStack[a->enableStackDepth].clip = (GLboolean *) crAlloc( g->transform.maxClipPlanes * sizeof( GLboolean ));
		}
		if (a->enableStack[a->enableStackDepth].light == NULL)
		{
			a->enableStack[a->enableStackDepth].light = (GLboolean *) crAlloc( g->lighting.maxLights * sizeof( GLboolean ));
		}
		a->enableStack[a->enableStackDepth].alphaTest = g->buffer.alphaTest;
		a->enableStack[a->enableStackDepth].autoNormal = g->eval.autoNormal;
		a->enableStack[a->enableStackDepth].blend = g->buffer.blend;
		for (i = 0 ; i < g->transform.maxClipPlanes ; i++)
		{
			a->enableStack[a->enableStackDepth].clip[i] = g->transform.clip[i];
		}
		a->enableStack[a->enableStackDepth].colorMaterial = g->lighting.colorMaterial;
		a->enableStack[a->enableStackDepth].cullFace = g->polygon.cullFace;
		a->enableStack[a->enableStackDepth].depthTest = g->buffer.depthTest;
		a->enableStack[a->enableStackDepth].dither = g->buffer.dither;
		a->enableStack[a->enableStackDepth].fog = g->fog.enable;
		for (i = 0 ; i < g->lighting.maxLights ; i++)
		{
			a->enableStack[a->enableStackDepth].light[i] = g->lighting.light[i].enable;
		}
		a->enableStack[a->enableStackDepth].lighting = g->lighting.lighting;
		a->enableStack[a->enableStackDepth].lineSmooth = g->line.lineSmooth;
		a->enableStack[a->enableStackDepth].lineStipple = g->line.lineStipple;
		a->enableStack[a->enableStackDepth].logicOp = g->buffer.logicOp;
		// a->enableStack[a->enableStackDepth].indexLogicOp = g->buffer.indexLogicOp;
		for (i = 0 ; i < GLEVAL_TOT ; i++)
		{
			a->enableStack[a->enableStackDepth].map1[i] = g->eval.enable1D[i];
			a->enableStack[a->enableStackDepth].map2[i] = g->eval.enable2D[i];
		}
		a->enableStack[a->enableStackDepth].normalize = g->current.normalize;
		a->enableStack[a->enableStackDepth].pointSmooth = g->line.pointSmooth;
		a->enableStack[a->enableStackDepth].polygonOffsetLine = g->polygon.polygonOffsetLine;
		a->enableStack[a->enableStackDepth].polygonOffsetFill = g->polygon.polygonOffsetFill;
		a->enableStack[a->enableStackDepth].polygonOffsetPoint = g->polygon.polygonOffsetPoint;
		a->enableStack[a->enableStackDepth].polygonSmooth = g->polygon.polygonSmooth;
		a->enableStack[a->enableStackDepth].polygonStipple = g->polygon.polygonStipple;
		a->enableStack[a->enableStackDepth].scissorTest = g->viewport.scissorTest;
		a->enableStack[a->enableStackDepth].stencilTest = g->stencil.stencilTest;
		for (i = 0 ; i < CR_MAX_TEXTURE_UNITS; i++)
		{
			a->enableStack[a->enableStackDepth].texture1D[i] = g->texture.enabled1D[i];
			a->enableStack[a->enableStackDepth].texture2D[i] = g->texture.enabled2D[i];
			a->enableStack[a->enableStackDepth].texture3D[i] = g->texture.enabled3D[i];
			a->enableStack[a->enableStackDepth].textureGenS[i] = g->texture.textureGen[i].s;
			a->enableStack[a->enableStackDepth].textureGenT[i] = g->texture.textureGen[i].t;
			a->enableStack[a->enableStackDepth].textureGenR[i] = g->texture.textureGen[i].r;
			a->enableStack[a->enableStackDepth].textureGenQ[i] = g->texture.textureGen[i].q;
		}
		a->enableStackDepth++;
	}
	if (mask & GL_EVAL_BIT)
	{
		for (i = 0 ; i < GLEVAL_TOT ; i++)
		{
			a->evalStack[a->evalStackDepth].map1[i] = g->eval.enable1D[i];
			a->evalStack[a->evalStackDepth].map2[i] = g->eval.enable2D[i];
		}
		a->evalStack[a->evalStackDepth].autoNormal = g->eval.autoNormal;
		a->evalStackDepth++;
		crError( "Pushing evaluators is not quite implemented yet." );
	}
	if (mask & GL_FOG_BIT)
	{
		a->fogStack[a->fogStackDepth].enable = g->fog.enable;
		a->fogStack[a->fogStackDepth].color = g->fog.color;
		a->fogStack[a->fogStackDepth].density = g->fog.density;
		a->fogStack[a->fogStackDepth].start = g->fog.start;
		a->fogStack[a->fogStackDepth].end = g->fog.end;
		a->fogStack[a->fogStackDepth].index = g->fog.index;
		a->fogStack[a->fogStackDepth].mode = g->fog.mode;
		a->fogStackDepth++;
	}
	if (mask & GL_HINT_BIT)
	{
		crError( "Pushing hints is not quite implemented yet" );
	}
	if (mask & GL_LIGHTING_BIT)
	{
		if (a->lightingStack[a->lightingStackDepth].light == NULL)
		{
			a->lightingStack[a->lightingStackDepth].light = (CRLight *) crAlloc( g->lighting.maxLights * sizeof( CRLight ));
		}
		a->lightingStack[a->lightingStackDepth].lightModelAmbient = g->lighting.lightModelAmbient;
		a->lightingStack[a->lightingStackDepth].lightModelLocalViewer = g->lighting.lightModelLocalViewer;
		a->lightingStack[a->lightingStackDepth].lightModelTwoSide = g->lighting.lightModelTwoSide;
		a->lightingStack[a->lightingStackDepth].lighting = g->lighting.lighting;
		for (i = 0 ; i < g->lighting.maxLights; i++)
		{
			a->lightingStack[a->lightingStackDepth].light[i].enable = g->lighting.light[i].enable;
			a->lightingStack[a->lightingStackDepth].light[i].ambient = g->lighting.light[i].ambient;
			a->lightingStack[a->lightingStackDepth].light[i].diffuse = g->lighting.light[i].diffuse;
			a->lightingStack[a->lightingStackDepth].light[i].specular = g->lighting.light[i].specular;
			a->lightingStack[a->lightingStackDepth].light[i].spotDirection = g->lighting.light[i].spotDirection;
			a->lightingStack[a->lightingStackDepth].light[i].position = g->lighting.light[i].position;
			a->lightingStack[a->lightingStackDepth].light[i].spotExponent = g->lighting.light[i].spotExponent;
			a->lightingStack[a->lightingStackDepth].light[i].spotCutoff = g->lighting.light[i].spotCutoff;
			a->lightingStack[a->lightingStackDepth].light[i].constantAttenuation = g->lighting.light[i].constantAttenuation;
			a->lightingStack[a->lightingStackDepth].light[i].linearAttenuation = g->lighting.light[i].linearAttenuation;
			a->lightingStack[a->lightingStackDepth].light[i].quadraticAttenuation = g->lighting.light[i].quadraticAttenuation;
		}
		for (i = 0 ; i < 2 ; i++)
		{
			a->lightingStack[a->lightingStackDepth].ambient[i] = g->lighting.ambient[i];
			a->lightingStack[a->lightingStackDepth].diffuse[i] = g->lighting.diffuse[i];
			a->lightingStack[a->lightingStackDepth].specular[i] = g->lighting.specular[i];
			a->lightingStack[a->lightingStackDepth].emission[i] = g->lighting.emission[i];
			a->lightingStack[a->lightingStackDepth].shininess[i] = g->lighting.shininess[i];
		}
		a->lightingStack[a->lightingStackDepth].shadeModel = g->lighting.shadeModel;
		a->lightingStackDepth++;
	}
	if (mask & GL_LINE_BIT)
	{
		a->lineStack[a->lineStackDepth].lineSmooth = g->line.lineSmooth;
		a->lineStack[a->lineStackDepth].lineStipple = g->line.lineStipple;
		a->lineStack[a->lineStackDepth].pattern = g->line.pattern;
		a->lineStack[a->lineStackDepth].repeat = g->line.repeat;
		a->lineStack[a->lineStackDepth].width = g->line.width;
		a->lineStackDepth++;
	}
	if (mask & GL_LIST_BIT)
	{
		a->listStack[a->listStackDepth].base = g->lists.base;
		a->listStackDepth++;
	}
	if (mask & GL_PIXEL_MODE_BIT)
	{
		a->pixelModeStack[a->pixelModeStackDepth].bias = g->pixel.bias;
		a->pixelModeStack[a->pixelModeStackDepth].scale = g->pixel.scale;
		a->pixelModeStack[a->pixelModeStackDepth].indexOffset = g->pixel.indexOffset;
		a->pixelModeStack[a->pixelModeStackDepth].indexShift = g->pixel.indexShift;
		a->pixelModeStack[a->pixelModeStackDepth].mapColor = g->pixel.mapColor;
		a->pixelModeStack[a->pixelModeStackDepth].mapStencil = g->pixel.mapStencil;
		a->pixelModeStack[a->pixelModeStackDepth].xZoom = g->pixel.xZoom;
		a->pixelModeStack[a->pixelModeStackDepth].yZoom = g->pixel.yZoom;
		a->pixelModeStack[a->pixelModeStackDepth].readBuffer = g->buffer.readBuffer;
		a->pixelModeStackDepth++;
	}
	if (mask & GL_POINT_BIT)
	{
		a->pointStack[a->pointStackDepth].pointSmooth = g->line.pointSmooth;
		a->pointStack[a->pointStackDepth].pointSize = g->line.pointSize;
		a->pointStackDepth++;
	}
	if (mask & GL_POLYGON_BIT)
	{
		a->polygonStack[a->polygonStackDepth].cullFace = g->polygon.cullFace;
		a->polygonStack[a->polygonStackDepth].cullFaceMode = g->polygon.cullFaceMode;
		a->polygonStack[a->polygonStackDepth].frontFace = g->polygon.frontFace;
		a->polygonStack[a->polygonStackDepth].frontMode = g->polygon.frontMode;
		a->polygonStack[a->polygonStackDepth].backMode = g->polygon.backMode;
		a->polygonStack[a->polygonStackDepth].polygonSmooth = g->polygon.polygonSmooth;
		a->polygonStack[a->polygonStackDepth].polygonStipple = g->polygon.polygonStipple;
		a->polygonStack[a->polygonStackDepth].polygonOffsetFill = g->polygon.polygonOffsetFill;
		a->polygonStack[a->polygonStackDepth].polygonOffsetLine = g->polygon.polygonOffsetLine;
		a->polygonStack[a->polygonStackDepth].polygonOffsetPoint = g->polygon.polygonOffsetPoint;
		a->polygonStack[a->polygonStackDepth].offsetFactor = g->polygon.offsetFactor;
		a->polygonStack[a->polygonStackDepth].offsetUnits = g->polygon.offsetUnits;
		a->polygonStackDepth++;
	}
	if (mask & GL_POLYGON_STIPPLE_BIT)
	{
		memcpy( a->polygonStippleStack[a->polygonStippleStackDepth].pattern, g->polygon.stipple, 32*sizeof(GLint) );
		a->polygonStippleStackDepth++;
	}
	if (mask & GL_SCISSOR_BIT)
	{
		a->scissorStack[a->scissorStackDepth].scissorTest = g->viewport.scissorTest;
		a->scissorStack[a->scissorStackDepth].scissorX = g->viewport.scissorX;
		a->scissorStack[a->scissorStackDepth].scissorY = g->viewport.scissorY;
		a->scissorStack[a->scissorStackDepth].scissorW = g->viewport.scissorW;
		a->scissorStack[a->scissorStackDepth].scissorH = g->viewport.scissorH;
		a->scissorStackDepth++;
	}
	if (mask & GL_STENCIL_BUFFER_BIT)
	{
		a->stencilBufferStack[a->stencilBufferStackDepth].stencilTest = g->stencil.stencilTest;
		a->stencilBufferStack[a->stencilBufferStackDepth].func = g->stencil.func;
		a->stencilBufferStack[a->stencilBufferStackDepth].mask = g->stencil.mask;
		a->stencilBufferStack[a->stencilBufferStackDepth].ref = g->stencil.ref;
		a->stencilBufferStack[a->stencilBufferStackDepth].fail = g->stencil.fail;
		a->stencilBufferStack[a->stencilBufferStackDepth].passDepthFail = g->stencil.passDepthFail;
		a->stencilBufferStack[a->stencilBufferStackDepth].passDepthPass = g->stencil.passDepthPass;
		a->stencilBufferStack[a->stencilBufferStackDepth].clearValue = g->stencil.clearValue;
		a->stencilBufferStack[a->stencilBufferStackDepth].writeMask = g->stencil.writeMask;
		a->stencilBufferStackDepth++;
	}
	if (mask & GL_TEXTURE_BIT)
	{
		for (i = 0 ; i < CR_MAX_TEXTURE_UNITS ; i++)
		{
			a->textureStack[a->textureStackDepth].enabled1D[i] = g->texture.enabled1D[i];
			a->textureStack[a->textureStackDepth].enabled2D[i] = g->texture.enabled2D[i];
			a->textureStack[a->textureStackDepth].enabled3D[i] = g->texture.enabled3D[i];
			a->textureStack[a->textureStackDepth].textureGen[i] = g->texture.textureGen[i];
			a->textureStack[a->textureStackDepth].objSCoeff[i] = g->texture.objSCoeff[i];
			a->textureStack[a->textureStackDepth].objTCoeff[i] = g->texture.objTCoeff[i];
			a->textureStack[a->textureStackDepth].objRCoeff[i] = g->texture.objRCoeff[i];
			a->textureStack[a->textureStackDepth].objQCoeff[i] = g->texture.objQCoeff[i];
			a->textureStack[a->textureStackDepth].eyeSCoeff[i] = g->texture.eyeSCoeff[i];
			a->textureStack[a->textureStackDepth].eyeTCoeff[i] = g->texture.eyeTCoeff[i];
			a->textureStack[a->textureStackDepth].eyeRCoeff[i] = g->texture.eyeRCoeff[i];
			a->textureStack[a->textureStackDepth].eyeQCoeff[i] = g->texture.eyeQCoeff[i];
			a->textureStack[a->textureStackDepth].gen[i] = g->texture.gen[i];
		}
		// Is this right?  It sure doesn't seem right.
		a->textureStack[a->textureStackDepth].borderColor[0] = g->texture.currentTexture1D->borderColor;
		a->textureStack[a->textureStackDepth].borderColor[1] = g->texture.currentTexture2D->borderColor;
		a->textureStack[a->textureStackDepth].borderColor[2] = g->texture.currentTexture3D->borderColor;
		a->textureStack[a->textureStackDepth].minFilter[0] = g->texture.currentTexture1D->minFilter;
		a->textureStack[a->textureStackDepth].minFilter[1] = g->texture.currentTexture2D->minFilter;
		a->textureStack[a->textureStackDepth].minFilter[2] = g->texture.currentTexture3D->minFilter;
		a->textureStack[a->textureStackDepth].magFilter[0] = g->texture.currentTexture1D->magFilter;
		a->textureStack[a->textureStackDepth].magFilter[1] = g->texture.currentTexture2D->magFilter;
		a->textureStack[a->textureStackDepth].magFilter[2] = g->texture.currentTexture3D->magFilter;
		a->textureStack[a->textureStackDepth].wrapS[0] = g->texture.currentTexture1D->wrapS;
		a->textureStack[a->textureStackDepth].wrapS[1] = g->texture.currentTexture2D->wrapS;
		a->textureStack[a->textureStackDepth].wrapS[2] = g->texture.currentTexture3D->wrapS;
		a->textureStack[a->textureStackDepth].wrapT[0] = g->texture.currentTexture1D->wrapT;
		a->textureStack[a->textureStackDepth].wrapT[1] = g->texture.currentTexture2D->wrapT;
		a->textureStack[a->textureStackDepth].wrapT[2] = g->texture.currentTexture3D->wrapT;
		a->textureStackDepth++;
	}
	if (mask & GL_TRANSFORM_BIT)
	{
		if (a->transformStack[a->transformStackDepth].clip == NULL)
		{
			a->transformStack[a->transformStackDepth].clip = (GLboolean *) crAlloc( g->transform.maxClipPlanes * sizeof( GLboolean ));
		}
		if (a->transformStack[a->transformStackDepth].clipPlane == NULL)
		{
			a->transformStack[a->transformStackDepth].clipPlane = (GLvectord *) crAlloc( g->transform.maxClipPlanes * sizeof( GLvectord ));
		}
		a->transformStack[a->transformStackDepth].mode = g->transform.mode;
		for (i = 0 ; i < g->transform.maxClipPlanes ; i++)
		{
			a->transformStack[a->transformStackDepth].clip[i] = g->transform.clip[i];
			a->transformStack[a->transformStackDepth].clipPlane[i] = g->transform.clipPlane[i];
		}
		a->transformStack[a->transformStackDepth].normalize = g->current.normalize;
		a->transformStackDepth++;
	}
	if (mask & GL_VIEWPORT_BIT)
	{
		a->viewportStack[a->viewportStackDepth].viewportX = g->viewport.viewportX;
		a->viewportStack[a->viewportStackDepth].viewportY = g->viewport.viewportY;
		a->viewportStack[a->viewportStackDepth].viewportW = g->viewport.viewportW;
		a->viewportStack[a->viewportStackDepth].viewportH = g->viewport.viewportH;
		a->viewportStack[a->viewportStackDepth].nearClip = g->viewport.nearClip;
		a->viewportStack[a->viewportStackDepth].farClip = g->viewport.farClip;
		a->viewportStackDepth++;
	}

	ab->dirty = g->neg_bitid;
}

void STATE_APIENTRY crStatePopAttrib(void) 
{
	CRContext *g = GetCurrentContext();
	CRAttribState *a = &(g->attrib);
	CRStateBits *sb = GetCurrentBits();
	CRAttribBits *ab = &(sb->attrib);

	GLbitvalue mask;
	int i;


	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "glPopAttrib called in Begin/End");
		return;
	}

	if (a->attribStackDepth == 0)
	{
		crStateError(__LINE__, __FILE__, GL_STACK_UNDERFLOW, "glPopAttrib called with an empty stack!" );
	}

	FLUSH();

	mask = a->pushMaskStack[--a->attribStackDepth];

	if (mask & GL_ACCUM_BUFFER_BIT)
	{
		if (a->accumBufferStackDepth == 0)
		{
			crStateError(__LINE__, __FILE__, GL_STACK_UNDERFLOW, "glPopAttrib called with an empty accum buffer stack!" );
		}
		a->accumBufferStackDepth--;
		g->buffer.accumClearValue = a->accumBufferStack[a->accumBufferStackDepth].accumClearValue;
		sb->buffer.dirty = g->neg_bitid;
		sb->buffer.clearAccum = g->neg_bitid;
	}
	if (mask & GL_COLOR_BUFFER_BIT)
	{
		if (a->colorBufferStackDepth == 0)
		{
			crStateError(__LINE__, __FILE__, GL_STACK_UNDERFLOW, "glPopAttrib called with an empty color buffer stack!" );
		}
		a->colorBufferStackDepth--;
		g->buffer.alphaTest = a->colorBufferStack[a->colorBufferStackDepth].alphaTest;
		g->buffer.alphaTestFunc = a->colorBufferStack[a->colorBufferStackDepth].alphaTestFunc;
		g->buffer.alphaTestRef = a->colorBufferStack[a->colorBufferStackDepth].alphaTestRef;
		g->buffer.blend = a->colorBufferStack[a->colorBufferStackDepth].blend;
		g->buffer.blendSrc = a->colorBufferStack[a->colorBufferStackDepth].blendSrc;
		g->buffer.blendDst = a->colorBufferStack[a->colorBufferStackDepth].blendDst;
		g->buffer.extensions.blendColor = a->colorBufferStack[a->colorBufferStackDepth].blendColor;
		g->buffer.extensions.blendEquation = a->colorBufferStack[a->colorBufferStackDepth].blendEquation;
		g->buffer.dither = a->colorBufferStack[a->colorBufferStackDepth].dither;
		g->buffer.drawBuffer = a->colorBufferStack[a->colorBufferStackDepth].drawBuffer;
		g->buffer.logicOp = a->colorBufferStack[a->colorBufferStackDepth].logicOp;
	 // g->buffer.indexLogicOp = a->colorBufferStack[a->colorBufferStackDepth].indexLogicOp;
		g->buffer.logicOpMode = a->colorBufferStack[a->colorBufferStackDepth].logicOpMode;
		g->buffer.colorClearValue = a->colorBufferStack[a->colorBufferStackDepth].colorClearValue;
		g->buffer.indexClearValue = a->colorBufferStack[a->colorBufferStackDepth].indexClearValue;
		g->buffer.colorWriteMask = a->colorBufferStack[a->colorBufferStackDepth].colorWriteMask;
		g->buffer.indexWriteMask = a->colorBufferStack[a->colorBufferStackDepth].indexWriteMask;
		sb->buffer.dirty = g->neg_bitid;
		sb->buffer.enable = g->neg_bitid;
		sb->buffer.alphaFunc = g->neg_bitid;
		sb->buffer.blendFunc = g->neg_bitid;
		sb->buffer.extensions = g->neg_bitid;
		sb->buffer.drawBuffer = g->neg_bitid;
		sb->buffer.logicOp = g->neg_bitid;
		sb->buffer.clearColor = g->neg_bitid;
		sb->buffer.clearIndex = g->neg_bitid;
		sb->buffer.colorWriteMask = g->neg_bitid;
		sb->buffer.indexMask = g->neg_bitid;
	}
	if (mask & GL_CURRENT_BIT)
	{
		if (a->currentStackDepth == 0)
		{
			crStateError(__LINE__, __FILE__, GL_STACK_UNDERFLOW, "glPopAttrib called with an empty current stack!" );
		}
		a->currentStackDepth--;
		g->current.color = a->currentStack[a->currentStackDepth].color;
		g->current.index = a->currentStack[a->currentStackDepth].index;
		g->current.normal = a->currentStack[a->currentStackDepth].normal;
		for (i = 0 ; i < CR_MAX_TEXTURE_UNITS ; i++)
		{
			g->current.texCoord[i] = a->currentStack[a->currentStackDepth].texCoord[i];
		}
		g->current.rasterPos = a->currentStack[a->currentStackDepth].rasterPos;
		g->current.rasterValid = a->currentStack[a->currentStackDepth].rasterValid;
		g->current.rasterColor = a->currentStack[a->currentStackDepth].rasterColor;
		g->current.rasterIndex = a->currentStack[a->currentStackDepth].rasterIndex;
		g->current.rasterTexture = a->currentStack[a->currentStackDepth].rasterTexture;
		g->current.edgeFlag = a->currentStack[a->currentStackDepth].edgeFlag;
		sb->current.dirty = g->neg_bitid;
		sb->current.color = g->neg_bitid;
		sb->current.index = g->neg_bitid;
		for (i = 0 ; i < CR_MAX_TEXTURE_UNITS ; i++)
		{
			sb->current.texCoord[i] = g->neg_bitid;
		}
		sb->current.normal = g->neg_bitid;
		sb->current.raster = g->neg_bitid;
		sb->current.edgeFlag = g->neg_bitid;
	}
	if (mask & GL_DEPTH_BUFFER_BIT)
	{
		if (a->depthBufferStackDepth == 0)
		{
			crStateError(__LINE__, __FILE__, GL_STACK_UNDERFLOW, "glPopAttrib called with an empty depth buffer stack!" );
		}
		a->depthBufferStackDepth--;
		g->buffer.depthTest = a->depthBufferStack[a->depthBufferStackDepth].depthTest;
		g->buffer.depthFunc = a->depthBufferStack[a->depthBufferStackDepth].depthFunc;
		g->buffer.depthClearValue = a->depthBufferStack[a->depthBufferStackDepth].depthClearValue;
		g->buffer.depthMask = a->depthBufferStack[a->depthBufferStackDepth].depthMask;
		sb->buffer.dirty = g->neg_bitid;
		sb->buffer.enable = g->neg_bitid;
		sb->buffer.depthFunc = g->neg_bitid;
		sb->buffer.clearDepth = g->neg_bitid;
		sb->buffer.depthMask = g->neg_bitid;
	}
	if (mask & GL_ENABLE_BIT)
	{
		if (a->enableStackDepth == 0)
		{
			crStateError(__LINE__, __FILE__, GL_STACK_UNDERFLOW, "glPopAttrib called with an empty enable stack!" );
		}
		a->enableStackDepth--;
		g->buffer.alphaTest = a->enableStack[a->enableStackDepth].alphaTest;
		g->eval.autoNormal = a->enableStack[a->enableStackDepth].autoNormal;
		g->buffer.blend = a->enableStack[a->enableStackDepth].blend;
		for (i = 0 ; i < g->transform.maxClipPlanes ; i++)
		{
			g->transform.clip[i] = a->enableStack[a->enableStackDepth].clip[i];
		}
		g->lighting.colorMaterial = a->enableStack[a->enableStackDepth].colorMaterial;
		g->polygon.cullFace = a->enableStack[a->enableStackDepth].cullFace;
		g->buffer.depthTest = a->enableStack[a->enableStackDepth].depthTest;
		g->buffer.dither = a->enableStack[a->enableStackDepth].dither;
		g->fog.enable = a->enableStack[a->enableStackDepth].fog;
		for (i = 0 ; i < g->lighting.maxLights ; i++)
		{
			g->lighting.light[i].enable = a->enableStack[a->enableStackDepth].light[i];
		}
		g->lighting.lighting = a->enableStack[a->enableStackDepth].lighting;
		g->line.lineSmooth = a->enableStack[a->enableStackDepth].lineSmooth;
		g->line.lineStipple = a->enableStack[a->enableStackDepth].lineStipple;
		g->buffer.logicOp = a->enableStack[a->enableStackDepth].logicOp;
		//g->buffer.indexLogicOp = a->enableStack[a->enableStackDepth].indexLogicOp;
		for (i = 0 ; i < GLEVAL_TOT ; i++)
		{
			g->eval.enable1D[i] = a->enableStack[a->enableStackDepth].map1[i];
			g->eval.enable2D[i] = a->enableStack[a->enableStackDepth].map2[i];
		}
		g->current.normalize = a->enableStack[a->enableStackDepth].normalize;
		g->line.pointSmooth = a->enableStack[a->enableStackDepth].pointSmooth;
		g->polygon.polygonOffsetLine = a->enableStack[a->enableStackDepth].polygonOffsetLine;
		g->polygon.polygonOffsetFill = a->enableStack[a->enableStackDepth].polygonOffsetFill;
		g->polygon.polygonOffsetPoint = a->enableStack[a->enableStackDepth].polygonOffsetPoint;
		g->polygon.polygonSmooth = a->enableStack[a->enableStackDepth].polygonSmooth;
		g->polygon.polygonStipple = a->enableStack[a->enableStackDepth].polygonStipple;
		g->viewport.scissorTest = a->enableStack[a->enableStackDepth].scissorTest;
		g->stencil.stencilTest = a->enableStack[a->enableStackDepth].stencilTest;
		for (i = 0 ; i < CR_MAX_TEXTURE_UNITS; i++)
		{
			g->texture.enabled1D[i] = a->enableStack[a->enableStackDepth].texture1D[i];
			g->texture.enabled2D[i] = a->enableStack[a->enableStackDepth].texture2D[i];
			g->texture.enabled3D[i] = a->enableStack[a->enableStackDepth].texture3D[i];
			g->texture.textureGen[i].s = a->enableStack[a->enableStackDepth].textureGenS[i];
			g->texture.textureGen[i].t = a->enableStack[a->enableStackDepth].textureGenT[i];
			g->texture.textureGen[i].r = a->enableStack[a->enableStackDepth].textureGenR[i];
			g->texture.textureGen[i].q = a->enableStack[a->enableStackDepth].textureGenQ[i];
		}
		sb->buffer.dirty = g->neg_bitid;
		sb->eval.dirty = g->neg_bitid;
		sb->transform.dirty = g->neg_bitid;
		sb->lighting.dirty = g->neg_bitid;
		sb->polygon.dirty = g->neg_bitid;
		sb->fog.dirty = g->neg_bitid;
		sb->line.dirty = g->neg_bitid;
		sb->polygon.dirty = g->neg_bitid;
		sb->viewport.dirty = g->neg_bitid;
		sb->stencil.dirty = g->neg_bitid;
		sb->texture.dirty = g->neg_bitid;

		sb->buffer.enable = g->neg_bitid;
		sb->eval.enable = g->neg_bitid;
		sb->transform.enable = g->neg_bitid;
		sb->lighting.enable = g->neg_bitid;
		sb->polygon.enable = g->neg_bitid;
		sb->fog.enable = g->neg_bitid;
		sb->line.enable = g->neg_bitid;
		sb->polygon.enable = g->neg_bitid;
		sb->viewport.enable = g->neg_bitid;
		sb->stencil.enable = g->neg_bitid;
		for (i = 0 ; i < CR_MAX_TEXTURE_UNITS ; i++)
		{
			sb->texture.enable[i] = g->neg_bitid;
		}
	}
	if (mask & GL_EVAL_BIT)
	{
		crError( "Popping evaluators is not quite implemented yet." );
	}
	if (mask & GL_FOG_BIT)
	{
		if (a->fogStackDepth == 0)
		{
			crStateError(__LINE__, __FILE__, GL_STACK_UNDERFLOW, "glPopAttrib called with an empty fog stack!" );
		}
		a->fogStackDepth--;
		g->fog.enable = a->fogStack[a->fogStackDepth].enable;
		g->fog.color = a->fogStack[a->fogStackDepth].color;
		g->fog.density = a->fogStack[a->fogStackDepth].density;
		g->fog.start = a->fogStack[a->fogStackDepth].start;
		g->fog.end = a->fogStack[a->fogStackDepth].end;
		g->fog.index = a->fogStack[a->fogStackDepth].index;
		g->fog.mode = a->fogStack[a->fogStackDepth].mode;
		sb->fog.dirty = g->neg_bitid;
		sb->fog.color = g->neg_bitid;
		sb->fog.index = g->neg_bitid;
		sb->fog.density = g->neg_bitid;
		sb->fog.start = g->neg_bitid;
		sb->fog.end = g->neg_bitid;
		sb->fog.mode = g->neg_bitid;
		sb->fog.enable = g->neg_bitid;
	}
	if (mask & GL_HINT_BIT)
	{
		crError( "Pushing hints is not quite implemented yet" );
	}
	if (mask & GL_LIGHTING_BIT)
	{
		if (a->lightingStackDepth == 0)
		{
			crStateError(__LINE__, __FILE__, GL_STACK_UNDERFLOW, "glPopAttrib called with an empty lighting stack!" );
		}
		a->lightingStackDepth--;
		g->lighting.lightModelAmbient = a->lightingStack[a->lightingStackDepth].lightModelAmbient;
		g->lighting.lightModelLocalViewer = a->lightingStack[a->lightingStackDepth].lightModelLocalViewer;
		g->lighting.lightModelTwoSide = a->lightingStack[a->lightingStackDepth].lightModelTwoSide;
		g->lighting.lighting = a->lightingStack[a->lightingStackDepth].lighting;
		for (i = 0 ; i < g->lighting.maxLights; i++)
		{
			g->lighting.light[i].enable = a->lightingStack[a->lightingStackDepth].light[i].enable;
			g->lighting.light[i].ambient = a->lightingStack[a->lightingStackDepth].light[i].ambient;
			g->lighting.light[i].diffuse = a->lightingStack[a->lightingStackDepth].light[i].diffuse;
			g->lighting.light[i].specular = a->lightingStack[a->lightingStackDepth].light[i].specular;
			g->lighting.light[i].spotDirection = a->lightingStack[a->lightingStackDepth].light[i].spotDirection;
			g->lighting.light[i].position = a->lightingStack[a->lightingStackDepth].light[i].position;
			g->lighting.light[i].spotExponent = a->lightingStack[a->lightingStackDepth].light[i].spotExponent;
			g->lighting.light[i].spotCutoff = a->lightingStack[a->lightingStackDepth].light[i].spotCutoff;
			g->lighting.light[i].constantAttenuation = a->lightingStack[a->lightingStackDepth].light[i].constantAttenuation;
			g->lighting.light[i].linearAttenuation = a->lightingStack[a->lightingStackDepth].light[i].linearAttenuation;
			g->lighting.light[i].quadraticAttenuation = a->lightingStack[a->lightingStackDepth].light[i].quadraticAttenuation;
		}
		for (i = 0 ; i < 2 ; i++)
		{
			g->lighting.ambient[i] = a->lightingStack[a->lightingStackDepth].ambient[i];
			g->lighting.diffuse[i] = a->lightingStack[a->lightingStackDepth].diffuse[i];
			g->lighting.specular[i] = a->lightingStack[a->lightingStackDepth].specular[i];
			g->lighting.emission[i] = a->lightingStack[a->lightingStackDepth].emission[i];
			g->lighting.shininess[i] = a->lightingStack[a->lightingStackDepth].shininess[i];
		}
		g->lighting.shadeModel = a->lightingStack[a->lightingStackDepth].shadeModel;
		sb->lighting.dirty = g->neg_bitid;
		sb->lighting.shadeModel = g->neg_bitid;
		sb->lighting.lightModel = g->neg_bitid;
		sb->lighting.material = g->neg_bitid;
		sb->lighting.enable = g->neg_bitid;
		for (i = 0 ; i < g->lighting.maxLights; i++)
		{
			sb->lighting.light[i].dirty = g->neg_bitid;
			sb->lighting.light[i].enable = g->neg_bitid;
			sb->lighting.light[i].ambient = g->neg_bitid;
			sb->lighting.light[i].diffuse = g->neg_bitid;
			sb->lighting.light[i].specular = g->neg_bitid;
			sb->lighting.light[i].position = g->neg_bitid;
			sb->lighting.light[i].attenuation = g->neg_bitid;
			sb->lighting.light[i].spot = g->neg_bitid;
		}
	}
	if (mask & GL_LINE_BIT)
	{
		if (a->lineStackDepth == 0)
		{
			crStateError(__LINE__, __FILE__, GL_STACK_UNDERFLOW, "glPopAttrib called with an empty line stack!" );
		}
		a->lineStackDepth--;
		g->line.lineSmooth = a->lineStack[a->lineStackDepth].lineSmooth;
		g->line.lineStipple = a->lineStack[a->lineStackDepth].lineStipple;
		g->line.pattern = a->lineStack[a->lineStackDepth].pattern;
		g->line.repeat = a->lineStack[a->lineStackDepth].repeat;
		g->line.width = a->lineStack[a->lineStackDepth].width;
		sb->line.dirty = g->neg_bitid;
		sb->line.enable = g->neg_bitid;
		sb->line.width = g->neg_bitid;
		sb->line.stipple = g->neg_bitid;
	}
	if (mask & GL_LIST_BIT)
	{
		if (a->listStackDepth == 0)
		{
			crStateError(__LINE__, __FILE__, GL_STACK_UNDERFLOW, "glPopAttrib called with an empty list stack!" );
		}
		a->listStackDepth--;
		g->lists.base = a->listStack[a->listStackDepth].base;
		sb->lists.dirty = g->neg_bitid;
	}
	if (mask & GL_PIXEL_MODE_BIT)
	{
		if (a->pixelModeStackDepth == 0)
		{
			crStateError(__LINE__, __FILE__, GL_STACK_UNDERFLOW, "glPopAttrib called with an empty pixel mode stack!" );
		}
		a->pixelModeStackDepth--;
		g->pixel.bias = a->pixelModeStack[a->pixelModeStackDepth].bias;
		g->pixel.scale = a->pixelModeStack[a->pixelModeStackDepth].scale;
		g->pixel.indexOffset = a->pixelModeStack[a->pixelModeStackDepth].indexOffset;
		g->pixel.indexShift = a->pixelModeStack[a->pixelModeStackDepth].indexShift;
		g->pixel.mapColor = a->pixelModeStack[a->pixelModeStackDepth].mapColor;
		g->pixel.mapStencil = a->pixelModeStack[a->pixelModeStackDepth].mapStencil;
		g->pixel.xZoom = a->pixelModeStack[a->pixelModeStackDepth].xZoom;
		g->pixel.yZoom = a->pixelModeStack[a->pixelModeStackDepth].yZoom;
		g->buffer.readBuffer = a->pixelModeStack[a->pixelModeStackDepth].readBuffer;
		sb->pixel.dirty = g->neg_bitid;
		sb->pixel.transfer = g->neg_bitid;
		sb->pixel.zoom = g->neg_bitid;
		sb->buffer.dirty = g->neg_bitid;
		sb->buffer.readBuffer = g->neg_bitid;
	}
	if (mask & GL_POINT_BIT)
	{
		if (a->pointStackDepth == 0)
		{
			crStateError(__LINE__, __FILE__, GL_STACK_UNDERFLOW, "glPopAttrib called with an empty point stack!" );
		}
		a->pointStackDepth--;
		g->line.pointSmooth = a->pointStack[a->pointStackDepth].pointSmooth;
		g->line.pointSize = a->pointStack[a->pointStackDepth].pointSize;
		sb->line.dirty = g->neg_bitid;
		sb->line.size = g->neg_bitid;
		sb->line.enable = g->neg_bitid;
	}
	if (mask & GL_POLYGON_BIT)
	{
		if (a->polygonStackDepth == 0)
		{
			crStateError(__LINE__, __FILE__, GL_STACK_UNDERFLOW, "glPopAttrib called with an empty polygon stack!" );
		}
		a->polygonStackDepth--;
		g->polygon.cullFace = a->polygonStack[a->polygonStackDepth].cullFace;
		g->polygon.cullFaceMode = a->polygonStack[a->polygonStackDepth].cullFaceMode;
		g->polygon.frontFace = a->polygonStack[a->polygonStackDepth].frontFace;
		g->polygon.frontMode = a->polygonStack[a->polygonStackDepth].frontMode;
		g->polygon.backMode = a->polygonStack[a->polygonStackDepth].backMode;
		g->polygon.polygonSmooth = a->polygonStack[a->polygonStackDepth].polygonSmooth;
		g->polygon.polygonStipple = a->polygonStack[a->polygonStackDepth].polygonStipple;
		g->polygon.polygonOffsetFill = a->polygonStack[a->polygonStackDepth].polygonOffsetFill;
		g->polygon.polygonOffsetLine = a->polygonStack[a->polygonStackDepth].polygonOffsetLine;
		g->polygon.polygonOffsetPoint = a->polygonStack[a->polygonStackDepth].polygonOffsetPoint;
		g->polygon.offsetFactor = a->polygonStack[a->polygonStackDepth].offsetFactor;
		g->polygon.offsetUnits = a->polygonStack[a->polygonStackDepth].offsetUnits;
		sb->polygon.dirty = g->neg_bitid;
		sb->polygon.enable = g->neg_bitid;
		sb->polygon.offset = g->neg_bitid;
		sb->polygon.mode = g->neg_bitid;
		sb->polygon.stipple = g->neg_bitid;
	}
	if (mask & GL_POLYGON_STIPPLE_BIT)
	{
		if (a->polygonStippleStackDepth == 0)
		{
			crStateError(__LINE__, __FILE__, GL_STACK_UNDERFLOW, "glPopAttrib called with an empty polygon stipple stack!" );
		}
		a->polygonStippleStackDepth--;
		memcpy( g->polygon.stipple, a->polygonStippleStack[a->polygonStippleStackDepth].pattern, 32*sizeof(GLint) );
		sb->polygon.dirty = g->neg_bitid;
		sb->polygon.stipple = g->neg_bitid;
	}
	if (mask & GL_SCISSOR_BIT)
	{
		if (a->scissorStackDepth == 0)
		{
			crStateError(__LINE__, __FILE__, GL_STACK_UNDERFLOW, "glPopAttrib called with an empty scissor stack!" );
		}
		a->scissorStackDepth--;
		g->viewport.scissorTest = a->scissorStack[a->scissorStackDepth].scissorTest;
		g->viewport.scissorX = a->scissorStack[a->scissorStackDepth].scissorX;
		g->viewport.scissorY = a->scissorStack[a->scissorStackDepth].scissorY;
		g->viewport.scissorW = a->scissorStack[a->scissorStackDepth].scissorW;
		g->viewport.scissorH = a->scissorStack[a->scissorStackDepth].scissorH;
		sb->viewport.dirty = g->neg_bitid;
		sb->viewport.enable = g->neg_bitid;
		sb->viewport.s_dims = g->neg_bitid;
	}
	if (mask & GL_STENCIL_BUFFER_BIT)
	{
		if (a->stencilBufferStackDepth == 0)
		{
			crStateError(__LINE__, __FILE__, GL_STACK_UNDERFLOW, "glPopAttrib called with an empty stencil stack!" );
		}
		a->stencilBufferStackDepth--;
		g->stencil.stencilTest = a->stencilBufferStack[a->stencilBufferStackDepth].stencilTest;
		g->stencil.func = a->stencilBufferStack[a->stencilBufferStackDepth].func;
		g->stencil.mask = a->stencilBufferStack[a->stencilBufferStackDepth].mask;
		g->stencil.ref = a->stencilBufferStack[a->stencilBufferStackDepth].ref;
		g->stencil.fail = a->stencilBufferStack[a->stencilBufferStackDepth].fail;
		g->stencil.passDepthFail = a->stencilBufferStack[a->stencilBufferStackDepth].passDepthFail;
		g->stencil.passDepthPass = a->stencilBufferStack[a->stencilBufferStackDepth].passDepthPass;
		g->stencil.clearValue = a->stencilBufferStack[a->stencilBufferStackDepth].clearValue;
		g->stencil.writeMask = a->stencilBufferStack[a->stencilBufferStackDepth].writeMask;
		sb->stencil.dirty = g->neg_bitid;
		sb->stencil.enable = g->neg_bitid;
		sb->stencil.func = g->neg_bitid;
		sb->stencil.op = g->neg_bitid;
		sb->stencil.clearValue = g->neg_bitid;
		sb->stencil.writeMask = g->neg_bitid;
	}
	if (mask & GL_TEXTURE_BIT)
	{
		if (a->textureStackDepth == 0)
		{
			crStateError(__LINE__, __FILE__, GL_STACK_UNDERFLOW, "glPopAttrib called with an empty texture stack!" );
		}
		a->textureStackDepth--;
		for (i = 0 ; i < CR_MAX_TEXTURE_UNITS ; i++)
		{
			g->texture.enabled1D[i] = a->textureStack[a->textureStackDepth].enabled1D[i];
			g->texture.enabled2D[i] = a->textureStack[a->textureStackDepth].enabled2D[i];
			g->texture.enabled3D[i] = a->textureStack[a->textureStackDepth].enabled3D[i];
			g->texture.textureGen[i] = a->textureStack[a->textureStackDepth].textureGen[i];
			g->texture.objSCoeff[i] = a->textureStack[a->textureStackDepth].objSCoeff[i];
			g->texture.objTCoeff[i] = a->textureStack[a->textureStackDepth].objTCoeff[i];
			g->texture.objRCoeff[i] = a->textureStack[a->textureStackDepth].objRCoeff[i];
			g->texture.objQCoeff[i] = a->textureStack[a->textureStackDepth].objQCoeff[i];
			g->texture.eyeSCoeff[i] = a->textureStack[a->textureStackDepth].eyeSCoeff[i];
			g->texture.eyeTCoeff[i] = a->textureStack[a->textureStackDepth].eyeTCoeff[i];
			g->texture.eyeRCoeff[i] = a->textureStack[a->textureStackDepth].eyeRCoeff[i];
			g->texture.eyeQCoeff[i] = a->textureStack[a->textureStackDepth].eyeQCoeff[i];
			g->texture.gen[i] = a->textureStack[a->textureStackDepth].gen[i];
		}
		// Is this right?  It sure doesn't seem right.
		g->texture.currentTexture1D->borderColor = a->textureStack[a->textureStackDepth].borderColor[0];
		g->texture.currentTexture2D->borderColor = a->textureStack[a->textureStackDepth].borderColor[1];
		g->texture.currentTexture3D->borderColor = a->textureStack[a->textureStackDepth].borderColor[2];
		g->texture.currentTexture1D->minFilter = a->textureStack[a->textureStackDepth].minFilter[0];
		g->texture.currentTexture2D->minFilter = a->textureStack[a->textureStackDepth].minFilter[1];
		g->texture.currentTexture3D->minFilter = a->textureStack[a->textureStackDepth].minFilter[2];
		g->texture.currentTexture1D->magFilter = a->textureStack[a->textureStackDepth].magFilter[0];
		g->texture.currentTexture2D->magFilter = a->textureStack[a->textureStackDepth].magFilter[1];
		g->texture.currentTexture3D->magFilter = a->textureStack[a->textureStackDepth].magFilter[2];
		g->texture.currentTexture1D->wrapS = a->textureStack[a->textureStackDepth].wrapS[0];
		g->texture.currentTexture2D->wrapS = a->textureStack[a->textureStackDepth].wrapS[1];
		g->texture.currentTexture3D->wrapS = a->textureStack[a->textureStackDepth].wrapS[2];
		g->texture.currentTexture1D->wrapT = a->textureStack[a->textureStackDepth].wrapT[0];
		g->texture.currentTexture2D->wrapT = a->textureStack[a->textureStackDepth].wrapT[1];
		g->texture.currentTexture3D->wrapT = a->textureStack[a->textureStackDepth].wrapT[2];
		sb->texture.dirty = g->neg_bitid;
		for (i = 0 ; i < CR_MAX_TEXTURE_UNITS ; i++)
		{
			sb->texture.enable[i] = g->neg_bitid;
			sb->texture.current[i] = g->neg_bitid;
			sb->texture.objGen[i] = g->neg_bitid;
			sb->texture.eyeGen[i] = g->neg_bitid;
			sb->texture.envBit[i] = g->neg_bitid;
			sb->texture.gen[i] = g->neg_bitid;
		}

		g->texture.currentTexture1D->dirty = g->neg_bitid;
		g->texture.currentTexture2D->dirty = g->neg_bitid;
		g->texture.currentTexture3D->dirty = g->neg_bitid;

		for (i = 0 ; i < CR_MAX_TEXTURE_UNITS ; i++)
		{
			g->texture.currentTexture1D->dirty = g->neg_bitid;
			g->texture.currentTexture2D->dirty = g->neg_bitid;
			g->texture.currentTexture3D->dirty = g->neg_bitid;

			g->texture.currentTexture1D->paramsBit[i] = g->neg_bitid;
			g->texture.currentTexture2D->paramsBit[i] = g->neg_bitid;
			g->texture.currentTexture3D->paramsBit[i] = g->neg_bitid;
		}
	}
	if (mask & GL_TRANSFORM_BIT)
	{
		if (a->transformStackDepth == 0)
		{
			crStateError(__LINE__, __FILE__, GL_STACK_UNDERFLOW, "glPopAttrib called with an empty transform stack!" );
		}
		a->transformStackDepth--;
		g->transform.mode = a->transformStack[a->transformStackDepth].mode;
		for (i = 0 ; i < g->transform.maxClipPlanes ; i++)
		{
			g->transform.clip[i] = a->transformStack[a->transformStackDepth].clip[i];
			g->transform.clipPlane[i] = a->transformStack[a->transformStackDepth].clipPlane[i];
		}
		g->current.normalize = a->transformStack[a->transformStackDepth].normalize;
		sb->transform.dirty = g->neg_bitid;
		sb->transform.mode = g->neg_bitid;
		sb->transform.clipPlane = g->neg_bitid;
		sb->current.dirty = g->neg_bitid;
		sb->current.enable = g->neg_bitid;
	}
	if (mask & GL_VIEWPORT_BIT)
	{
		if (a->viewportStackDepth == 0)
		{
			crStateError(__LINE__, __FILE__, GL_STACK_UNDERFLOW, "glPopAttrib called with an empty viewport stack!" );
		}
		a->viewportStackDepth--;
		g->viewport.viewportX = a->viewportStack[a->viewportStackDepth].viewportX;
		g->viewport.viewportY = a->viewportStack[a->viewportStackDepth].viewportY;
		g->viewport.viewportW = a->viewportStack[a->viewportStackDepth].viewportW;
		g->viewport.viewportH = a->viewportStack[a->viewportStackDepth].viewportH;
		g->viewport.nearClip = a->viewportStack[a->viewportStackDepth].nearClip;
		g->viewport.farClip = a->viewportStack[a->viewportStackDepth].farClip;
		sb->viewport.dirty = g->neg_bitid;
		sb->viewport.v_dims = g->neg_bitid;
		sb->viewport.depth = g->neg_bitid;
	}
	ab->dirty = g->neg_bitid;
}

void crStateAttribSwitch( CRAttribBits *bb, GLbitvalue bitID,
		CRAttribState *from, CRAttribState *to )
{
	if (to->attribStackDepth != 0 || from->attribStackDepth != 0)
	{
		crError( "Trying to switch contexts when the attribte stacks weren't zero.  Currently, this is not supported." );
	}
	(void) bb;
	(void) bitID;
}
