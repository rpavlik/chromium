/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */
	
#include "tilesortspu.h"
#include "cr_packfunctions.h"
#include "cr_mem.h"


/* OpenGL doesn't have this token, be we want it here */
#ifndef GL_BOOL
#define GL_BOOL 0x1
#endif


/*
 * If the given <pname> is an OpenGL limit query, query all downstream
 * servers for their limits, find the minimum, return it in <results>
 * and return GL_TRUE.
 * If pname is not a limit, return GL_FALSE.
 */
static GLboolean
GetLimit(GLenum pname, GLenum type, void *results)
{
	ThreadInfo *thread0 = &(tilesort_spu.thread[0]);
	GLint numValues = 1;
	GLboolean minMax = GL_FALSE;
	GLfloat params[16];
	GLint i, j;

	switch (pname) {
	case GL_MAX_ATTRIB_STACK_DEPTH:
		params[0] = (GLfloat) CR_MAX_ATTRIB_STACK_DEPTH;
		break;
	case GL_MAX_CLIENT_ATTRIB_STACK_DEPTH:
		params[0] = (GLfloat) CR_MAX_ATTRIB_STACK_DEPTH;
		break;
	case GL_MAX_CLIP_PLANES:
		params[0] = (GLfloat) CR_MAX_CLIP_PLANES;
		break;
	case GL_MAX_ELEMENTS_VERTICES:  /* 1.2 */
		params[0] = (GLfloat) CR_MAX_ELEMENTS_VERTICES;
		break;
	case GL_MAX_ELEMENTS_INDICES:   /* 1.2 */
		params[0] = (GLfloat) CR_MAX_ELEMENTS_INDICES;
		break;
	case GL_MAX_EVAL_ORDER:
		params[0] = (GLfloat) CR_MAX_EVAL_ORDER;
		break;
	case GL_MAX_LIGHTS:
		params[0] = (GLfloat) CR_MAX_LIGHTS;
		break;
	case GL_MAX_LIST_NESTING:
		params[0] = (GLfloat) CR_MAX_LIST_NESTING;
		break;
	case GL_MAX_MODELVIEW_STACK_DEPTH:
		params[0] = (GLfloat) CR_MAX_MODELVIEW_STACK_DEPTH;
		break;
	case GL_MAX_NAME_STACK_DEPTH:
		params[0] = (GLfloat) CR_MAX_NAME_STACK_DEPTH;
		break;
	case GL_MAX_PIXEL_MAP_TABLE:
		params[0] = (GLfloat) CR_MAX_PIXEL_MAP_TABLE;
		break;
	case GL_MAX_PROJECTION_STACK_DEPTH:
		params[0] = (GLfloat) CR_MAX_PROJECTION_STACK_DEPTH;
		break;
	case GL_MAX_TEXTURE_SIZE:
		params[0] = (GLfloat) CR_MAX_TEXTURE_SIZE;
		break;
	case GL_MAX_3D_TEXTURE_SIZE:
		params[0] = (GLfloat) CR_MAX_3D_TEXTURE_SIZE;
		break;
	case GL_MAX_TEXTURE_STACK_DEPTH:
		params[0] = (GLfloat) CR_MAX_TEXTURE_STACK_DEPTH;
		break;
	case GL_MAX_VIEWPORT_DIMS:
		params[0] = (GLfloat) CR_MAX_VIEWPORT_DIM;
		params[1] = (GLfloat) CR_MAX_VIEWPORT_DIM;
		numValues = 2;
		break;
	case GL_SUBPIXEL_BITS:
		params[0] = (GLfloat) CR_SUBPIXEL_BITS;
		break;
	case GL_ALIASED_POINT_SIZE_RANGE:
		params[0] = (GLfloat) CR_ALIASED_POINT_SIZE_MIN;
		params[1] = (GLfloat) CR_ALIASED_POINT_SIZE_MAX;
		numValues = 2;
		minMax = GL_TRUE;
		break;
	case GL_SMOOTH_POINT_SIZE_RANGE:
		params[0] = (GLfloat) CR_SMOOTH_POINT_SIZE_MIN;
		params[1] = (GLfloat) CR_SMOOTH_POINT_SIZE_MAX;
		numValues = 2;
		minMax = GL_TRUE;
		break;
	case GL_SMOOTH_POINT_SIZE_GRANULARITY:
		params[0] = (GLfloat) CR_POINT_SIZE_GRANULARITY;
		break;
	case GL_ALIASED_LINE_WIDTH_RANGE:
		params[0] = (GLfloat) CR_ALIASED_LINE_WIDTH_MIN;
		params[1] = (GLfloat) CR_ALIASED_LINE_WIDTH_MAX;
		numValues = 2;
		minMax = GL_TRUE;
		break;
	case GL_SMOOTH_LINE_WIDTH_RANGE:
		params[0] = (GLfloat) CR_SMOOTH_LINE_WIDTH_MIN;
		params[1] = (GLfloat) CR_SMOOTH_LINE_WIDTH_MAX;
		numValues = 2;
		minMax = GL_TRUE;
		break;
	case GL_SMOOTH_LINE_WIDTH_GRANULARITY:
		params[0] = (GLfloat) CR_LINE_WIDTH_GRANULARITY;
		break;
	/* GL_ARB_multitexture */
	case GL_MAX_TEXTURE_UNITS_ARB:
		params[0] = (GLfloat) CR_MAX_TEXTURE_UNITS;
		break;
	/* GL_NV_register_combiners */
	case GL_MAX_GENERAL_COMBINERS_NV:
		params[0] = (GLfloat) CR_MAX_GENERAL_COMBINERS;
		break;
	default:
		return GL_FALSE; /* not a GL limit */
	}

	CRASSERT(numValues < 16);

	/* Save the default pack buffer */
	crPackGetBuffer( thread0->packer, &(thread0->geometry_pack) );

	/*
	 * loop over servers, issuing the glGet.
	 * We send it via the zero-th thread's server connections.
	 */
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		int writeback = 1;
		GLfloat values[16];

		crPackSetBuffer( thread0->packer, &(thread0->pack[i]) );

		if (tilesort_spu.swap)
			crPackGetFloatvSWAP( pname, values, &writeback );
		else
			crPackGetFloatv( pname, values, &writeback );

		crPackGetBuffer( thread0->packer, &(thread0->pack[i]) );

		/* Flush buffer (send to server) */
		tilesortspuSendServerBuffer( i );

		/* Get return value */
		while (writeback) {
			crNetRecv();
		}

		/* update current minimum(s) */
		if (minMax) {
			CRASSERT(numValues == 2);
			/* element 0 is a minimum, element 1 is a maximum */
			if (values[0] > params[0])
				params[0] = values[0];
			if (values[1] < params[1])
				params[1] = values[1];
		}
		else {
			for (j = 0; j < numValues; j++)
				if (values[j] < params[j])
					params[j] = values[j];
		}
	}

	/* Restore the default pack buffer */
	crPackSetBuffer( thread0->packer, &(thread0->geometry_pack) );

	/* return the results */
	if (type == GL_BOOL)
	{
		GLboolean *bResult = (GLboolean *) results;
		for (j = 0; j < numValues; j++)
			bResult[j] = (GLboolean) (params[j] ? GL_TRUE : GL_FALSE);
	}
	else if (type == GL_INT)
	{
		GLint *iResult = (GLint *) results;
		for (j = 0; j < numValues; j++)
			iResult[j] = (GLint) params[j];
	}
	else if (type == GL_FLOAT)
	{
		GLfloat *fResult = (GLfloat *) results;
		for (j = 0; j < numValues; j++)
			fResult[j] = (GLfloat) params[j];
	}
	else if (type == GL_DOUBLE)
	{
		GLdouble *dResult = (GLdouble *) results;
		for (j = 0; j < numValues; j++)
			dResult[j] = (GLdouble) params[j];
	}
	return GL_TRUE;
}



void TILESORTSPU_APIENTRY tilesortspu_GetDoublev( GLenum pname, GLdouble *params )
{
	if (!GetLimit(pname, GL_DOUBLE, params))
		crStateGetDoublev( pname, params );
}

void TILESORTSPU_APIENTRY tilesortspu_GetFloatv( GLenum pname, GLfloat *params )
{
	if (!GetLimit(pname, GL_FLOAT, params))
		crStateGetFloatv( pname, params );
}

void TILESORTSPU_APIENTRY tilesortspu_GetIntegerv( GLenum pname, GLint *params )
{
	if (!GetLimit(pname, GL_INT, params))
		crStateGetIntegerv( pname, params );
}

void TILESORTSPU_APIENTRY tilesortspu_GetBooleanv( GLenum pname, GLboolean *params )
{
	if (!GetLimit(pname, GL_BOOL, params))
		crStateGetBooleanv( pname, params );
}


#if 000
/* NOT FINISHED YET (BP) */

static const GLubyte *
GetExtensionsString(void)
{
	ThreadInfo *thread0 = &(tilesort_spu.thread[0]);
	GLubyte **extensions;
	GLint i;
	GLubyte *ext;

	extensions = (GLubyte **) crCalloc(tilesort_spu.num_servers * sizeof(GLubyte *));
	if (!extensions)
	{
		crWarning("Out of memory in tilesortspu::GetExtensionsString");
		return NULL;
	}

	/* Save the default pack buffer */
	crPackGetBuffer( thread0->packer, &(thread0->geometry_pack) );

	/*
	 * loop over servers, issuing the glGet.
	 * We send it via the zero-th thread's server connections.
	 */
	for (i = 0; i < tilesort_spu.num_servers; i++)
	{
		int writeback = 1;

		extensions[i] = crCalloc(50 * 1000);
		CRASSERT(extensions[i]);
		extensions[i][50*1000-1] = 123;

		crPackSetBuffer( thread0->packer, &(thread0->pack[i]) );

		if (tilesort_spu.swap)
			crPackGetStringSWAP( GL_EXTENSIONS, extensions[i], &writeback );
		else
			crPackGetString( GL_EXTENSIONS, extensions[i], &writeback );

		crPackGetBuffer( thread0->packer, &(thread0->pack[i]) );

		/* Flush buffer (send to server) */
		tilesortspuSendServerBuffer( i );

		/* Get return value */
		while (writeback) {
			crNetRecv();
		}

		/* make sure we didn't overflow the buffer */
		CRASSERT(extensions[i][50*1000-1] == 123);

	}

	/* Restore the default pack buffer */
	crPackSetBuffer( thread0->packer, &(thread0->geometry_pack) );

	ext = crSPUMergeExtensions(tilesort_spu.num_servers, extensions);

	for (i = 0; i < tilesort_spu.num_servers; i++)
		crFree(extensions[i]);
	crFree(extensions);

	return ext;
}

const GLubyte * TILESORTSPU_APIENTRY tilesortspu_GetString( GLenum pname )
{
	if (pname == GL_EXTENSIONS+1000)
	{
		/* Query all servers for their extensions, return the intersection */
		return GetExtensionsString();
	}
	else
	{
		return crStateGetString(pname);
	}
}
#endif
