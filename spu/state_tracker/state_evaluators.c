#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "cr_mem.h"
#include "cr_glstate.h"
#include "state/cr_statetypes.h"

/*  Here are the order of the enums */
/*      GL_MAP1_VERTEX_3 */
/*      GL_MAP1_VERTEX_4 */
/*      GL_MAP1_COLOR_4 */
/*      GL_MAP1_INDEX */
/*      GL_MAP1_NORMAL */
/*      GL_MAP1_TEXTURE_COORD_1 */
/*      GL_MAP1_TEXTURE_COORD_2 */
/*      GL_MAP1_TEXTURE_COORD_3 */
/*      GL_MAP1_TEXTURE_COORD_4 */
/*      GL_MAP2_VERTEX_3 */
/*      GL_MAP2_VERTEX_4 */
/*      GL_MAP2_COLOR_4 */
/*      GL_MAP2_INDEX */
/*      GL_MAP2_NORMAL */
/*      GL_MAP2_TEXTURE_COORD_1 */
/*      GL_MAP2_TEXTURE_COORD_2 */
/*      GL_MAP2_TEXTURE_COORD_3 */
/*      GL_MAP2_TEXTURE_COORD_4 */

const int gleval_sizes[] = {3, 4, 4, 1, 3, 1, 2, 3, 4};

void crStateEvaluatorInit(CREvaluatorState *e) 
{
	int i;

#if 0
	e->maxEvalOrder = c->maxEvalOrder;
#else
	e->maxEvalOrder = 4; // -=-
#endif

	e->autoNormal = GL_FALSE;

	/* What are the correct defaults?!? */
	for (i=0; i < GLEVAL_TOT; i++) 
	{
		e->enable1d[i] = GL_FALSE;
		e->eval1d[i].coeff = (GLdouble *) crAlloc (sizeof(GLdouble) * e->maxEvalOrder*4);
		e->eval1d[i].order = 0;
		e->eval1d[i].u1 = 0.0;
		e->eval1d[i].u2 = 1.0;

		e->enable2d[i] = GL_FALSE;
		e->eval2d[i].coeff= (GLdouble *) crAlloc (sizeof(GLdouble) * e->maxEvalOrder*4*2);
		e->eval2d[i].uorder=0;
		e->eval2d[i].vorder=0;
		e->eval2d[i].u1=0.0;
		e->eval2d[i].u2=1.0;
		e->eval2d[i].v1=0.0;
		e->eval2d[i].v2=1.0;
	}

	e->un1d = 1;
	e->u11d = 0.0; 
	e->u21d = 1.0;

	e->un2d = 1;
	e->vn2d = 1;
	e->u12d = 0.0;
	e->u22d = 1.0;
	e->v12d = 0.0;
	e->v22d = 1.0;
}

void STATE_APIENTRY crStateMap1d (GLenum target, GLdouble u1, GLdouble u2, 
		GLint stride, GLint order, const GLdouble *points) 
{
	CRContext *g = GetCurrentContext();
	CREvaluatorState *e = &(g->eval);
	CREvaluator1d *eval1d;
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

	if (u1 == u2)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Map1d: u1 == u2: %lf", u1);
		return;
	}

	if (order < 1 || order > e->maxEvalOrder)
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

	eval1d = e->eval1d+i;
	eval1d->order = order;
	eval1d->u1 = u1;
	eval1d->u2 = u2;

	p = points;
	size = stride;
	for (j=0; j<order; j++) {
		for (k=0; k<size; k++)
			eval1d->coeff[j] = p[k];
		p+=size;
	}

	eb->dirty = g->neg_bitid;
	eb->eval1d[i] = g->neg_bitid;
}

void STATE_APIENTRY crStateMap1f (GLenum target, GLfloat u1, GLfloat u2, 
		GLint stride, GLint order, const GLfloat *points) 
{
	CRContext *g = GetCurrentContext();
	CREvaluatorState *e = &(g->eval);
	CREvaluator1d *eval1d;
	CRStateBits *sb = GetCurrentBits();
	CREvaluatorBits *eb = &(sb->eval);

	int i, j, k;
	const GLfloat *p;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "Map1f called in begin/end");
		return;
	}

	if (u1 == u2)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Map1f: u1 == u2: %f", u1);
		return;
	}

	if (order < 1 || order > e->maxEvalOrder)
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

	eval1d = e->eval1d+i;
	eval1d->order = order;
	eval1d->u1 = (GLdouble) u1;
	eval1d->u2 = (GLdouble) u2;

	p = points;
	for (j=0; j<order; j++) {
		for (k=0; k<gleval_sizes[i]; k++)
			eval1d->coeff[j] = (GLdouble) p[k];
		p+=stride;
	}

	eb->dirty = g->neg_bitid;
	eb->eval1d[i] = g->neg_bitid;
}


void STATE_APIENTRY crStateMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder,
		GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points) 
{
	CRContext *g = GetCurrentContext();
	CREvaluatorState *e = &(g->eval);
	CREvaluator2d *eval2d;
	CRStateBits *sb = GetCurrentBits();
	CREvaluatorBits *eb = &(sb->eval);

	int i, j, k, m;
	const GLfloat *p;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "Map2f called in begin/end");
		return;
	}

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

	if (uorder < 1 || uorder > e->maxEvalOrder)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Map2f: order oob: %d", uorder);
		return;
	}

	if (vorder < 1 || vorder > e->maxEvalOrder)
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


	eval2d = e->eval2d+i;
	eval2d->uorder = uorder;
	eval2d->vorder = vorder;
	eval2d->u1 = (GLdouble) u1;
	eval2d->u2 = (GLdouble) u2;
	eval2d->v1 = (GLdouble) v1;
	eval2d->v2 = (GLdouble) v2;

	p = points;
	for (j=0; j<vorder; j++) {
		for (k=0; k<uorder; k++) {
			for (m=0; m<gleval_sizes[i]; m++)
				eval2d->coeff[k+j*uorder] = (GLdouble) p[m];
			p+=ustride;
		}
		p-=ustride;
		p+=vstride;
	}

	eb->dirty = g->neg_bitid;
	eb->eval2d[i] = g->neg_bitid;
}

void STATE_APIENTRY crStateMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder,
		GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points) 
{
	CRContext *g = GetCurrentContext();
	CREvaluatorState *e = &(g->eval);
	CREvaluator2d *eval2d;
	CRStateBits *sb = GetCurrentBits();
	CREvaluatorBits *eb = &(sb->eval);

	int i, j, k, m;
	const GLdouble *p;

	if (g->current.inBeginEnd)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, "Map2d called in begin/end");
		return;
	}

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

	if (uorder < 1 || uorder > e->maxEvalOrder)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "Map2d: order oob: %d", uorder);
		return;
	}

	if (vorder < 1 || vorder > e->maxEvalOrder)
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

	eval2d = e->eval2d+i;
	eval2d->uorder = uorder;
	eval2d->vorder = vorder;
	eval2d->u1 = u1;
	eval2d->u2 = u2;
	eval2d->v1 = v1;
	eval2d->v2 = v2;

	p = points;
	for (j=0; j<vorder; j++) {
		for (k=0; k<uorder; k++) {
			for (m=0; m<gleval_sizes[i]; m++)
				eval2d->coeff[k+j*uorder] = (GLfloat) p[m];
			p+=ustride;
		}
		p-=ustride;
		p+=vstride;
	}

	eb->dirty = g->neg_bitid;
	eb->eval2d[i] = g->neg_bitid;
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

	if (un < 0)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "MapGrid1d: un < 0: %d", un);
		return;
	}

	e->un1d = un;
	e->u11d = u1;
	e->u21d = u2;

	eb->dirty = g->neg_bitid;
	eb->grid1d = g->neg_bitid;
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

	if (un < 0)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "MapGrid1d: un < 0: %d", un);
		return;
	}

	e->un1d = un;
	e->u11d = (GLdouble) u1;
	e->u21d = (GLdouble) u2;

	eb->dirty = g->neg_bitid;
	eb->grid1d = g->neg_bitid;
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

	e->un2d = un;
	e->vn2d = un;
	e->u12d = u1;
	e->u22d = u2;
	e->v12d = v1;
	e->v22d = v2;

	eb->dirty = g->neg_bitid;
	eb->grid2d = g->neg_bitid;
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

	e->un2d = un;
	e->vn2d = un;
	e->u12d = (GLdouble) u1;
	e->u22d = (GLdouble) u2;
	e->v12d = (GLdouble) v1;
	e->v22d = (GLdouble) v2;

	eb->dirty = g->neg_bitid;
	eb->grid2d = g->neg_bitid;
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
				size = gleval_sizes[i] * e->eval2d[i].uorder * e->eval2d[i].vorder;
				for (j=0; j<size; j++)
				{
					v[j] = e->eval2d[i].coeff[j];
				}
				break;
			case GL_ORDER:
				v[0] = (GLdouble) e->eval2d[i].uorder;
				v[1] = (GLdouble) e->eval2d[i].vorder;
				break;
			case GL_DOMAIN:
				v[0] = e->eval2d[i].u1;
				v[1] = e->eval2d[i].u2;
				v[2] = e->eval2d[i].v1;
				v[3] = e->eval2d[i].v2;
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
				size = gleval_sizes[i] * e->eval1d[i].order;
				for (j=0; j<size; j++)
				{
					v[j] = e->eval1d[i].coeff[j];
				}
				break;
			case GL_ORDER:
				*v = (GLdouble) e->eval1d[i].order;
				break;
			case GL_DOMAIN:
				v[0] = e->eval1d[i].u1;
				v[1] = e->eval1d[i].u2;
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
				size = gleval_sizes[i] * e->eval2d[i].uorder * e->eval2d[i].vorder;
				for (j=0; j<size; j++)
				{
					v[j] = (GLfloat) e->eval2d[i].coeff[j];
				}
				break;
			case GL_ORDER:
				v[0] = (GLfloat) e->eval2d[i].uorder;
				v[1] = (GLfloat) e->eval2d[i].vorder;
				break;
			case GL_DOMAIN:
				v[0] = (GLfloat) e->eval2d[i].u1;
				v[1] = (GLfloat) e->eval2d[i].u2;
				v[2] = (GLfloat) e->eval2d[i].v1;
				v[3] = (GLfloat) e->eval2d[i].v2;
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
				size = gleval_sizes[i] * e->eval1d[i].order;
				for (j=0; j<size; j++)
				{
					v[j] = (GLfloat) e->eval1d[i].coeff[j];
				}
				break;
			case GL_ORDER:
				*v = (GLfloat) e->eval1d[i].order;
				break;
			case GL_DOMAIN:
				v[0] = (GLfloat) e->eval1d[i].u1;
				v[1] = (GLfloat) e->eval1d[i].u2;
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
				size = gleval_sizes[i] * e->eval2d[i].uorder * e->eval2d[i].vorder;
				for (j=0; j<size; j++)
				{
					v[j] = (GLint) e->eval2d[i].coeff[j];
				}
				break;
			case GL_ORDER:
				v[0] = e->eval2d[i].uorder;
				v[1] = e->eval2d[i].vorder;
				break;
			case GL_DOMAIN:
				v[0] = (GLint) e->eval2d[i].u1;
				v[1] = (GLint) e->eval2d[i].u2;
				v[2] = (GLint) e->eval2d[i].v1;
				v[3] = (GLint) e->eval2d[i].v2;
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
				size = gleval_sizes[i] * e->eval1d[i].order;
				for (j=0; j<size; j++)
				{
					v[j] = (GLint) e->eval1d[i].coeff[j];
				}
				break;
			case GL_ORDER:
				*v = e->eval1d[i].order;
				break;
			case GL_DOMAIN:
				v[0] = (GLint) e->eval1d[i].u1;
				v[1] = (GLint) e->eval1d[i].u2;
				break;
			default:
				crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
						"GetMapiv: invalid target: %d", target);
				return;
		}
	}
}
