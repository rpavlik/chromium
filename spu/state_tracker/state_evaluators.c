/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "cr_mem.h"
#include "cr_glstate.h"
#include "state/cr_statetypes.h"
#include "state_internals.h"

/*  Here are the order of the enums */
/*      GL_MAP1_COLOR_4 */
/*      GL_MAP1_INDEX */
/*      GL_MAP1_NORMAL */
/*      GL_MAP1_TEXTURE_COORD_1 */
/*      GL_MAP1_TEXTURE_COORD_2 */
/*      GL_MAP1_TEXTURE_COORD_3 */
/*      GL_MAP1_TEXTURE_COORD_4 */
/*      GL_MAP1_VERTEX_3 */
/*      GL_MAP1_VERTEX_4 */

/*      GL_MAP2_COLOR_4 */
/*      GL_MAP2_INDEX */
/*      GL_MAP2_NORMAL */
/*      GL_MAP2_TEXTURE_COORD_1 */
/*      GL_MAP2_TEXTURE_COORD_2 */
/*      GL_MAP2_TEXTURE_COORD_3 */
/*      GL_MAP2_TEXTURE_COORD_4 */
/*      GL_MAP2_VERTEX_3 */
/*      GL_MAP2_VERTEX_4 */

const int gleval_sizes[] = {4, 1, 3, 1, 2, 3, 4, 3, 4};


/* Initialize a 1-D evaluator map */
static void
init_1d_map( CREvaluatorState *e, GLenum map, int n, const float *initial )
{
	GLint i;
	const GLint k = map - GL_MAP1_COLOR_4;
	CRASSERT(k >= 0);
	CRASSERT(k < GLEVAL_TOT);
	e->eval1D[k].u1 = 0.0;
	e->eval1D[k].u2 = 1.0;
	e->eval1D[k].order = 1;
	e->eval1D[k].coeff = (GLdouble *) crAlloc(n * sizeof(GLdouble));
	for (i = 0; i < n; i++)
		e->eval1D[k].coeff[i] = initial[i];
}


/* Initialize a 2-D evaluator map */
static void
init_2d_map( CREvaluatorState *e, GLenum map, int n, const float *initial )
{
	GLint i;
	const GLint k = map - GL_MAP2_COLOR_4;
	CRASSERT(k >= 0);
	CRASSERT(k < GLEVAL_TOT);
	e->eval2D[k].u1 = 0.0;
	e->eval2D[k].u2 = 1.0;
	e->eval2D[k].v1 = 0.0;
	e->eval2D[k].v2 = 1.0;
	e->eval2D[k].uorder = 1;
	e->eval2D[k].vorder = 1;
	e->eval2D[k].coeff = (GLdouble *) crAlloc(n * sizeof(GLdouble));
	for (i = 0; i < n; i++)
		e->eval2D[k].coeff[i] = initial[i];
}


void crStateEvaluatorInit(CREvaluatorState *e) 
{
	static GLfloat vertex[4] = { 0.0, 0.0, 0.0, 1.0 };
	static GLfloat normal[3] = { 0.0, 0.0, 1.0 };
	static GLfloat index[1] = { 1.0 };
	static GLfloat color[4] = { 1.0, 1.0, 1.0, 1.0 };
	static GLfloat texcoord[4] = { 0.0, 0.0, 0.0, 1.0 };

	e->autoNormal = GL_FALSE;

	init_1d_map(e, GL_MAP1_VERTEX_3, 3, vertex);
	init_1d_map(e, GL_MAP1_VERTEX_4, 4, vertex);
	init_1d_map(e, GL_MAP1_INDEX, 1, index);
	init_1d_map(e, GL_MAP1_COLOR_4, 4, color);
	init_1d_map(e, GL_MAP1_NORMAL, 3, normal);
	init_1d_map(e, GL_MAP1_TEXTURE_COORD_1, 1, texcoord);
	init_1d_map(e, GL_MAP1_TEXTURE_COORD_2, 2, texcoord);
	init_1d_map(e, GL_MAP1_TEXTURE_COORD_3, 3, texcoord);
	init_1d_map(e, GL_MAP1_TEXTURE_COORD_4, 4, texcoord);

	init_2d_map(e, GL_MAP2_VERTEX_3, 3, vertex);
	init_2d_map(e, GL_MAP2_VERTEX_4, 4, vertex);
	init_2d_map(e, GL_MAP2_INDEX, 1, index);
	init_2d_map(e, GL_MAP2_COLOR_4, 4, color);
	init_2d_map(e, GL_MAP2_NORMAL, 3, normal);
	init_2d_map(e, GL_MAP2_TEXTURE_COORD_1, 1, texcoord);
	init_2d_map(e, GL_MAP2_TEXTURE_COORD_2, 2, texcoord);
	init_2d_map(e, GL_MAP2_TEXTURE_COORD_3, 3, texcoord);
	init_2d_map(e, GL_MAP2_TEXTURE_COORD_4, 4, texcoord);

	e->un1D = 1;
	e->u11D = 0.0; 
	e->u21D = 1.0;

	e->un2D = 1;
	e->vn2D = 1;
	e->u12D = 0.0;
	e->u22D = 1.0;
	e->v12D = 0.0;
	e->v22D = 1.0;
}

void STATE_APIENTRY crStateMap1d (GLenum target, GLdouble u1, GLdouble u2, 
		GLint stride, GLint order, const GLdouble *points) 
{
	CRContext *g = GetCurrentContext();
	CREvaluatorState *e = &(g->eval);
	CREvaluator1D *eval1D;
	CRStateBits *sb = GetCurrentBits();
	CREvaluatorBits *eb = &(sb->eval);

	int i, j, k;
	int size;
	const GLdouble *p;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "Map1d called in begin/end");
		return;
	}

	FLUSH();

	if (u1 == u2)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Map1d: u1 == u2: %lf", u1);
		return;
	}

	if (order < 1 || order > (int) g->limits.maxEvalOrder)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Map1d: order oob: %d", order);
		return;
	}

	i = target-GL_MAP1_COLOR_4;

	if (i < 0 || i >= GLEVAL_TOT)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "Map1d: invalid target specified: %d", target);
		return;
	}

	if (stride < gleval_sizes[i])
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Map1d: stride less than size of coord: %d", stride);
		return;
	}

	eval1D = e->eval1D+i;
	eval1D->order = order;
	eval1D->u1 = u1;
	eval1D->u2 = u2;

	p = points;
	size = stride;
	for (j=0; j<order; j++) {
		for (k=0; k<size; k++)
			eval1D->coeff[j] = p[k];
		p+=size;
	}

	eb->dirty = g->neg_bitid;
	eb->eval1D[i] = g->neg_bitid;
}

void STATE_APIENTRY crStateMap1f (GLenum target, GLfloat u1, GLfloat u2, 
		GLint stride, GLint order, const GLfloat *points) 
{
	CRContext *g = GetCurrentContext();
	CREvaluatorState *e = &(g->eval);
	CREvaluator1D *eval1D;
	CRStateBits *sb = GetCurrentBits();
	CREvaluatorBits *eb = &(sb->eval);

	int i, j, k;
	const GLfloat *p;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "Map1f called in begin/end");
		return;
	}

	FLUSH();

	if (u1 == u2)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Map1f: u1 == u2: %f", u1);
		return;
	}

	if (order < 1 || order > (int) g->limits.maxEvalOrder)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Map1f: order oob: %d", order);
		return;
	}

	i = target-GL_MAP1_COLOR_4;

	if (i < 0 || i >= GLEVAL_TOT)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "Map1f: invalid target specified: %d", target);
		return;
	}

	if (stride < gleval_sizes[i])
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Map1f: stride less than size of coord: %d", stride);
		return;
	}

	eval1D = e->eval1D+i;
	eval1D->order = order;
	eval1D->u1 = (GLdouble) u1;
	eval1D->u2 = (GLdouble) u2;

	p = points;
	for (j=0; j<order; j++) {
		for (k=0; k<gleval_sizes[i]; k++)
			eval1D->coeff[j] = (GLdouble) p[k];
		p+=stride;
	}

	eb->dirty = g->neg_bitid;
	eb->eval1D[i] = g->neg_bitid;
}


void STATE_APIENTRY crStateMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder,
		GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points) 
{
	CRContext *g = GetCurrentContext();
	CREvaluatorState *e = &(g->eval);
	CREvaluator2D *eval2D;
	CRStateBits *sb = GetCurrentBits();
	CREvaluatorBits *eb = &(sb->eval);

	int i, j, k, m;
	const GLfloat *p;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "Map2f called in begin/end");
		return;
	}

	FLUSH();

	if (u1 == u2)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Map2f: u1 == u2: %f", u1);
		return;
	}

	if (v1 == v2)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Map2f: v1 == v2: %f", v1);
		return;
	}

	if (uorder < 1 || uorder > (int) g->limits.maxEvalOrder)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Map2f: order oob: %d", uorder);
		return;
	}

	if (vorder < 1 || vorder > (int) g->limits.maxEvalOrder)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Map2f: vorder oob: %d", vorder);
		return;
	}

	i = target-GL_MAP2_COLOR_4;

	if (i < 0 || i >= GLEVAL_TOT)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "Map2f: invalid target specified: %d", target);
		return;
	}

	if (ustride < gleval_sizes[i])
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Map2f: stride less than size of coord: %d", ustride);
		return;
	}

	if (vstride < gleval_sizes[i])
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Map2f: stride less than size of coord: %d", vstride);
		return;
	}


	eval2D = e->eval2D+i;
	eval2D->uorder = uorder;
	eval2D->vorder = vorder;
	eval2D->u1 = (GLdouble) u1;
	eval2D->u2 = (GLdouble) u2;
	eval2D->v1 = (GLdouble) v1;
	eval2D->v2 = (GLdouble) v2;

	p = points;
	for (j=0; j<vorder; j++) {
		for (k=0; k<uorder; k++) {
			for (m=0; m<gleval_sizes[i]; m++)
				eval2D->coeff[k+j*uorder] = (GLdouble) p[m];
			p+=ustride;
		}
		p-=ustride;
		p+=vstride;
	}

	eb->dirty = g->neg_bitid;
	eb->eval2D[i] = g->neg_bitid;
}

void STATE_APIENTRY crStateMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder,
		GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points) 
{
	CRContext *g = GetCurrentContext();
	CREvaluatorState *e = &(g->eval);
	CREvaluator2D *eval2D;
	CRStateBits *sb = GetCurrentBits();
	CREvaluatorBits *eb = &(sb->eval);

	int i, j, k, m;
	const GLdouble *p;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "Map2d called in begin/end");
		return;
	}

	FLUSH();

	if (u1 == u2)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Map2d: u1 == u2: %lf", u1);
		return;
	}

	if (v1 == v2)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Map2d: v1 == v2: %lf", v1);
		return;
	}

	if (uorder < 1 || uorder > (int) g->limits.maxEvalOrder)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Map2d: order oob: %d", uorder);
		return;
	}

	if (vorder < 1 || vorder > (int) g->limits.maxEvalOrder)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Map2d: vorder oob: %d", vorder);
		return;
	}

	i = target-GL_MAP2_COLOR_4;

	if (i < 0 || i >= GLEVAL_TOT)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "Map2d: invalid target specified: %d", target);
		return;
	}

	if (ustride < gleval_sizes[i])
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Map2d: stride less than size of coord: %d", ustride);
		return;
	}

	if (vstride < gleval_sizes[i])
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Map2d: stride less than size of coord: %d", vstride);
		return;
	}

	eval2D = e->eval2D+i;
	eval2D->uorder = uorder;
	eval2D->vorder = vorder;
	eval2D->u1 = u1;
	eval2D->u2 = u2;
	eval2D->v1 = v1;
	eval2D->v2 = v2;

	p = points;
	for (j=0; j<vorder; j++) {
		for (k=0; k<uorder; k++) {
			for (m=0; m<gleval_sizes[i]; m++)
				eval2D->coeff[k+j*uorder] = (GLfloat) p[m];
			p+=ustride;
		}
		p-=ustride;
		p+=vstride;
	}

	eb->dirty = g->neg_bitid;
	eb->eval2D[i] = g->neg_bitid;
}


void STATE_APIENTRY crStateMapGrid1d (GLint un, GLdouble u1, GLdouble u2) 
{
	CRContext *g = GetCurrentContext();
	CREvaluatorState *e = &(g->eval);
	CRStateBits *sb = GetCurrentBits();
	CREvaluatorBits *eb = &(sb->eval);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "MapGrid1d called in begin/end");
		return;
	}

	FLUSH();

	if (un < 0)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "MapGrid1d: un < 0: %d", un);
		return;
	}

	e->un1D = un;
	e->u11D = u1;
	e->u21D = u2;

	eb->dirty = g->neg_bitid;
	eb->grid1D = g->neg_bitid;
}

void STATE_APIENTRY crStateMapGrid1f (GLint un, GLfloat u1, GLfloat u2) 
{
	CRContext *g = GetCurrentContext();
	CREvaluatorState *e = &(g->eval);
	CRStateBits *sb = GetCurrentBits();
	CREvaluatorBits *eb = &(sb->eval);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "MapGrid1d called in begin/end");
		return;
	}

	FLUSH();

	if (un < 0)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "MapGrid1d: un < 0: %d", un);
		return;
	}

	e->un1D = un;
	e->u11D = (GLdouble) u1;
	e->u21D = (GLdouble) u2;

	eb->dirty = g->neg_bitid;
	eb->grid1D = g->neg_bitid;
}

void STATE_APIENTRY crStateMapGrid2d (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2) 
{
	CRContext *g = GetCurrentContext();
	CREvaluatorState *e = &(g->eval);
	CRStateBits *sb = GetCurrentBits();
	CREvaluatorBits *eb = &(sb->eval);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "MapGrid2d called in begin/end");
		return;
	}

	FLUSH();

	if (un < 0)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "MapGrid2d: un < 0: %d", un);
		return;
	}

	if (vn < 0)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "MapGrid2d: vn < 0: %d", vn);
		return;
	}

	e->un2D = un;
	e->vn2D = un;
	e->u12D = u1;
	e->u22D = u2;
	e->v12D = v1;
	e->v22D = v2;

	eb->dirty = g->neg_bitid;
	eb->grid2D = g->neg_bitid;
}

void STATE_APIENTRY crStateMapGrid2f (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2) 
{
	CRContext *g = GetCurrentContext();
	CREvaluatorState *e = &(g->eval);
	CRStateBits *sb = GetCurrentBits();
	CREvaluatorBits *eb = &(sb->eval);

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "MapGrid2f called in begin/end");
		return;
	}

	FLUSH();

	if (un < 0)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "MapGrid2f: un < 0: %d", un);
		return;
	}

	if (vn < 0)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "MapGrid2f: vn < 0: %d", vn);
		return;
	}

	e->un2D = un;
	e->vn2D = un;
	e->u12D = (GLdouble) u1;
	e->u22D = (GLdouble) u2;
	e->v12D = (GLdouble) v1;
	e->v22D = (GLdouble) v2;

	eb->dirty = g->neg_bitid;
	eb->grid2D = g->neg_bitid;
}

void STATE_APIENTRY crStateGetMapdv(GLenum target, GLenum query, GLdouble * v)
{
	CRContext *g = GetCurrentContext();
	CREvaluatorState *e = &(g->eval);
	int i, j;
	int size;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
				"GetMapdv called in begin/end");
		return;
	}


	i = target-GL_MAP1_COLOR_4;

	if (i < 0 || i >= GLEVAL_TOT) 
	{
		i = target-GL_MAP2_COLOR_4;

		if (i < 0 || i >= GLEVAL_TOT) 
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					"GetMapdv: invalid target: %d", target);
			return;
		}

		switch (query) 
		{
			case GL_COEFF:
				size = gleval_sizes[i] * e->eval2D[i].uorder * e->eval2D[i].vorder;
				for (j=0; j<size; j++)
				{
					v[j] = e->eval2D[i].coeff[j];
				}
				break;
			case GL_ORDER:
				v[0] = (GLdouble) e->eval2D[i].uorder;
				v[1] = (GLdouble) e->eval2D[i].vorder;
				break;
			case GL_DOMAIN:
				v[0] = e->eval2D[i].u1;
				v[1] = e->eval2D[i].u2;
				v[2] = e->eval2D[i].v1;
				v[3] = e->eval2D[i].v2;
				break;
			default:
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"GetMapdv: invalid target: %d", target);
				return;
		}
	} 
	else 
	{
		switch (query) 
		{
			case GL_COEFF:
				size = gleval_sizes[i] * e->eval1D[i].order;
				for (j=0; j<size; j++)
				{
					v[j] = e->eval1D[i].coeff[j];
				}
				break;
			case GL_ORDER:
				*v = (GLdouble) e->eval1D[i].order;
				break;
			case GL_DOMAIN:
				v[0] = e->eval1D[i].u1;
				v[1] = e->eval1D[i].u2;
				break;
			default:
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"GetMapdv: invalid target: %d", target);
				return;
		}
	}
}

void STATE_APIENTRY crStateGetMapfv(GLenum target, GLenum query, GLfloat* v)
{
	CRContext *g = GetCurrentContext();
	CREvaluatorState *e = &(g->eval);
	int i, j;
	int size;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
				"GetMapfv called in begin/end");
		return;
	}

	i = target-GL_MAP1_COLOR_4;
	if (i < 0 || i >= GLEVAL_TOT) 
	{
		i = target-GL_MAP2_COLOR_4;
		if (i < 0 || i >= GLEVAL_TOT) 
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					"GetMapfv: invalid target: %d", target);
			return;
		}
		switch (query) 
		{
			case GL_COEFF:
				size = gleval_sizes[i] * e->eval2D[i].uorder * e->eval2D[i].vorder;
				for (j=0; j<size; j++)
				{
					v[j] = (GLfloat) e->eval2D[i].coeff[j];
				}
				break;
			case GL_ORDER:
				v[0] = (GLfloat) e->eval2D[i].uorder;
				v[1] = (GLfloat) e->eval2D[i].vorder;
				break;
			case GL_DOMAIN:
				v[0] = (GLfloat) e->eval2D[i].u1;
				v[1] = (GLfloat) e->eval2D[i].u2;
				v[2] = (GLfloat) e->eval2D[i].v1;
				v[3] = (GLfloat) e->eval2D[i].v2;
				break;
			default:
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"GetMapfv: invalid target: %d", target);
				return;
		}
	} 
	else 
	{
		switch (query) 
		{
			case GL_COEFF:
				size = gleval_sizes[i] * e->eval1D[i].order;
				for (j=0; j<size; j++)
				{
					v[j] = (GLfloat) e->eval1D[i].coeff[j];
				}
				break;
			case GL_ORDER:
				*v = (GLfloat) e->eval1D[i].order;
				break;
			case GL_DOMAIN:
				v[0] = (GLfloat) e->eval1D[i].u1;
				v[1] = (GLfloat) e->eval1D[i].u2;
				break;
			default:
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"GetMapfv: invalid target: %d", target);
				return;
		}
	}
}

void STATE_APIENTRY crStateGetMapiv(GLenum target, GLenum query, GLint* v)
{
	CRContext *g = GetCurrentContext();
	CREvaluatorState *e = &(g->eval);
	int i, j;
	int size;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
				"GetMapiv called in begin/end");
		return;
	}

	i = target-GL_MAP1_COLOR_4;
	if (i < 0 || i >= GLEVAL_TOT) 
	{
		i = target-GL_MAP2_COLOR_4;
		if (i < 0 || i >= GLEVAL_TOT) 
		{
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					"GetMapiv: invalid target: %d", target);
			return;
		}
		switch (query) 
		{
			case GL_COEFF:
				size = gleval_sizes[i] * e->eval2D[i].uorder * e->eval2D[i].vorder;
				for (j=0; j<size; j++)
				{
					v[j] = (GLint) e->eval2D[i].coeff[j];
				}
				break;
			case GL_ORDER:
				v[0] = e->eval2D[i].uorder;
				v[1] = e->eval2D[i].vorder;
				break;
			case GL_DOMAIN:
				v[0] = (GLint) e->eval2D[i].u1;
				v[1] = (GLint) e->eval2D[i].u2;
				v[2] = (GLint) e->eval2D[i].v1;
				v[3] = (GLint) e->eval2D[i].v2;
				break;
			default:
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"GetMapiv: invalid target: %d", target);
				return;
		}
	} 
	else 
	{
		switch (query) 
		{
			case GL_COEFF:
				size = gleval_sizes[i] * e->eval1D[i].order;
				for (j=0; j<size; j++)
				{
					v[j] = (GLint) e->eval1D[i].coeff[j];
				}
				break;
			case GL_ORDER:
				*v = e->eval1D[i].order;
				break;
			case GL_DOMAIN:
				v[0] = (GLint) e->eval1D[i].u1;
				v[1] = (GLint) e->eval1D[i].u2;
				break;
			default:
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"GetMapiv: invalid target: %d", target);
				return;
		}
	}
}

void crStateEvaluatorSwitch(CREvaluatorBits *e, GLbitvalue bitID,
				  CREvaluatorState *from, CREvaluatorState *to) 
{
	GLbitvalue nbitID = ~bitID;
	int i;

	if (e->enable & bitID) {
		if (from->autoNormal != to->autoNormal) {
			glAble able[2];
			able[0] = diff_api.Disable;
			able[1] = diff_api.Enable;
			able[to->autoNormal](GL_AUTO_NORMAL);
			e->enable = GLBITS_ONES;
			e->dirty = GLBITS_ONES;
		}
		e->enable &= nbitID;
	}
	for (i=0; i<GLEVAL_TOT; i++) {
		if (e->eval1D[i] & bitID) {
			int size = from->eval1D[i].order * gleval_sizes[i] * 
						sizeof (*from->eval1D[i].coeff);
			if (from->eval1D[i].order != to->eval1D[i].order ||
				from->eval1D[i].u1 != from->eval1D[i].u1 ||
				from->eval1D[i].u2 != from->eval1D[i].u2 ||
				memcmp ((const void *) from->eval1D[i].coeff,
						(const void *) to->eval1D[i].coeff, size)) {
				diff_api.Map1d(i+GL_MAP1_COLOR_4, to->eval1D[i].u1, to->eval1D[i].u2,
							 gleval_sizes[i], to->eval1D[i].order, to->eval1D[i].coeff);
				e->dirty = GLBITS_ONES;
				e->eval1D[i] = GLBITS_ONES;
			}
			e->eval1D[i] &= nbitID;
		}
	}

	for (i=0; i<GLEVAL_TOT; i++) {
		if (e->eval2D[i] & bitID) {
			int size = from->eval2D[i].uorder * from->eval2D[i].vorder *
						gleval_sizes[i] * sizeof (*from->eval2D[i].coeff);
			if (from->eval2D[i].uorder != to->eval2D[i].uorder ||
				from->eval2D[i].vorder != to->eval2D[i].vorder ||
				from->eval2D[i].u1 != from->eval2D[i].u1 ||
				from->eval2D[i].u2 != from->eval2D[i].u2 ||
				from->eval2D[i].v1 != from->eval2D[i].v1 ||
				from->eval2D[i].v2 != from->eval2D[i].v2 ||
				memcmp ((const void *) from->eval2D[i].coeff,
						(const void *) to->eval2D[i].coeff, size)) {
				diff_api.Map2d(i+GL_MAP2_COLOR_4,
							 to->eval2D[i].u1, to->eval2D[i].u2,
							 gleval_sizes[i], to->eval2D[i].uorder,
							 to->eval2D[i].v1, to->eval2D[i].v2,
							 gleval_sizes[i], to->eval2D[i].vorder,
							 to->eval2D[i].coeff);
				e->dirty = GLBITS_ONES;
				e->eval2D[i] = GLBITS_ONES;
			}
			e->eval2D[i] &= nbitID;
		}
	}
	if (e->grid1D & bitID) {
		if (from->u11D != to->u11D ||
			from->u21D != to->u21D ||
			from->un1D != to->un1D) {
			diff_api.MapGrid1d (to->un1D, to->u11D, to->u21D);
			e->dirty = GLBITS_ONES;
			e->grid1D = GLBITS_ONES;
		}
		e->grid1D &= nbitID;
	}		
	if (e->grid2D & bitID) {
		if (from->u12D != to->u12D ||
			from->u22D != to->u22D ||
			from->un2D != to->un2D ||
			from->v12D != to->v12D ||
			from->v22D != to->v22D ||
			from->vn2D != to->vn2D) {
			diff_api.MapGrid2d (to->un2D, to->u12D, to->u22D,
							  to->vn2D, to->v12D, to->v22D);
			e->dirty = GLBITS_ONES;
			e->grid1D = GLBITS_ONES;
		}
		e->grid1D &= nbitID;
	}
	e->dirty &= nbitID;
}

void crStateEvaluatorDiff(CREvaluatorBits *e, GLbitvalue bitID,
				 CREvaluatorState *from, CREvaluatorState *to) 
{
	GLbitvalue nbitID = ~bitID;
	glAble able[2];
	int i;
	able[0] = diff_api.Disable;
	able[1] = diff_api.Enable;

	if (e->enable & bitID) {
		if (from->autoNormal != to->autoNormal) {
			able[to->autoNormal](GL_AUTO_NORMAL);
			from->autoNormal = to->autoNormal;
		}
		e->enable &= nbitID;
	}
	for (i=0; i<GLEVAL_TOT; i++) {
		if (e->enable1D[i] & bitID) {
			if (from->enable1D[i] != to->enable1D[i]) {
				able[to->enable1D[i]](i+GL_MAP1_COLOR_4);
				from->enable1D[i] = to->enable1D[i];
			}
			e->enable1D[i] &= nbitID;
		}
		if (to->enable1D[i] && e->eval1D[i] & bitID) {
			int size = from->eval1D[i].order * gleval_sizes[i] * 
						sizeof (*from->eval1D[i].coeff);
			if (from->eval1D[i].order != to->eval1D[i].order ||
				from->eval1D[i].u1 != from->eval1D[i].u1 ||
				from->eval1D[i].u2 != from->eval1D[i].u2 ||
				memcmp ((const void *) from->eval1D[i].coeff,
						(const void *) to->eval1D[i].coeff, size)) {
				diff_api.Map1d(i+GL_MAP1_COLOR_4, to->eval1D[i].u1, to->eval1D[i].u2,
							 gleval_sizes[i], to->eval1D[i].order, to->eval1D[i].coeff);
				from->eval1D[i].order = to->eval1D[i].order;
				from->eval1D[i].u1 = from->eval1D[i].u1;
				from->eval1D[i].u2 = from->eval1D[i].u2;
				memcpy ((void *) from->eval1D[i].coeff,
						(const void *) to->eval1D[i].coeff, size);
			}
			e->eval1D[i] &= nbitID;
		}
	}

	for (i=0; i<GLEVAL_TOT; i++) {
		if (e->enable2D[i] & bitID) {
			if (from->enable2D[i] != to->enable2D[i]) {
				able[to->enable2D[i]](i+GL_MAP2_COLOR_4);
				from->enable2D[i] = to->enable2D[i];
			}
			e->enable2D[i] &= nbitID;
		}
		if (to->enable2D[i] && e->eval2D[i] & bitID) {
			int size = from->eval2D[i].uorder * from->eval2D[i].vorder *
						gleval_sizes[i] * sizeof (*from->eval2D[i].coeff);
			if (from->eval2D[i].uorder != to->eval2D[i].uorder ||
				from->eval2D[i].vorder != to->eval2D[i].vorder ||
				from->eval2D[i].u1 != from->eval2D[i].u1 ||
				from->eval2D[i].u2 != from->eval2D[i].u2 ||
				from->eval2D[i].v1 != from->eval2D[i].v1 ||
				from->eval2D[i].v2 != from->eval2D[i].v2 ||
				memcmp ((const void *) from->eval2D[i].coeff,
						(const void *) to->eval2D[i].coeff, size)) {
				diff_api.Map2d(i+GL_MAP2_COLOR_4,
							 to->eval2D[i].u1, to->eval2D[i].u2,
							 gleval_sizes[i], to->eval2D[i].uorder,
							 to->eval2D[i].v1, to->eval2D[i].v2,
							 gleval_sizes[i], to->eval2D[i].vorder,
							 to->eval2D[i].coeff);
				from->eval2D[i].uorder = to->eval2D[i].uorder;
				from->eval2D[i].vorder = to->eval2D[i].vorder;
				from->eval2D[i].u1 = from->eval2D[i].u1;
				from->eval2D[i].u2 = from->eval2D[i].u2;
				from->eval2D[i].v1 = from->eval2D[i].v1;
				from->eval2D[i].v2 = from->eval2D[i].v2;
				memcpy ((void *) from->eval2D[i].coeff,
						(const void *) to->eval2D[i].coeff, size);
			}
			e->eval2D[i] &= nbitID;
		}
	}
	if (e->grid1D & bitID) {
		if (from->u11D != to->u11D ||
			from->u21D != to->u21D ||
			from->un1D != to->un1D) {
			diff_api.MapGrid1d (to->un1D, to->u11D, to->u21D);
			from->u11D = to->u11D;
			from->u21D = to->u21D;
			from->un1D = to->un1D;
		}
		e->grid1D &= nbitID;
	}		
	if (e->grid2D & bitID) {
		if (from->u12D != to->u12D ||
			from->u22D != to->u22D ||
			from->un2D != to->un2D ||
			from->v12D != to->v12D ||
			from->v22D != to->v22D ||
			from->vn2D != to->vn2D) {
			diff_api.MapGrid2d (to->un2D, to->u12D, to->u22D,
							  to->vn2D, to->v12D, to->v22D);
			from->u12D = to->u12D;
			from->u22D = to->u22D;
			from->un2D = to->un2D;
			from->v12D = to->v12D;
			from->v22D = to->v22D;
			from->vn2D = to->vn2D;
		}
		e->grid1D &= nbitID;
	}
	e->dirty &= nbitID;
}
