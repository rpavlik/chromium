/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/* 
 * The majority of this file is pulled from Mesa 4.0.x with the
 * permission of Brian Paul
 */

#include "tilesortspu.h"
#include "tilesortspu_proto.h"
#include "cr_packfunctions.h"
#include "cr_mem.h"
#include <math.h>

static GLfloat inv_tab[MAX_EVAL_ORDER];

#define LEN_SQUARED_3FV( V ) (V[0]*V[0]+V[1]*V[1]+V[2]*V[2])

#define NORMALIZE_3FV( V )			\
do {						\
   GLdouble len = LEN_SQUARED_3FV(V);		\
   if (len > 1e-50) {				\
      len = 1.0 / sqrt(len);			\
      V[0] = (GLfloat) (V[0] * len);		\
      V[1] = (GLfloat) (V[1] * len);		\
      V[2] = (GLfloat) (V[2] * len);		\
   }						\
} while(0)

/*
 * eval.c was written by
 * Bernd Barsuhn (bdbarsuh@cip.informatik.uni-erlangen.de) and
 * Volker Weiss (vrweiss@cip.informatik.uni-erlangen.de).
 *
 * My original implementation of evaluators was simplistic and didn't
 * compute surface normal vectors properly.  Bernd and Volker applied
 * used more sophisticated methods to get better results.
 *
 * Thanks guys!
 */

/*
 * Horner scheme for Bezier curves
 *
 * Bezier curves can be computed via a Horner scheme.
 * Horner is numerically less stable than the de Casteljau
 * algorithm, but it is faster. For curves of degree n
 * the complexity of Horner is O(n) and de Casteljau is O(n^2).
 * Since stability is not important for displaying curve
 * points I decided to use the Horner scheme.
 *
 * A cubic Bezier curve with control points b0, b1, b2, b3 can be
 * written as
 *
 *        (([3]        [3]     )     [3]       )     [3]
 * c(t) = (([0]*s*b0 + [1]*t*b1)*s + [2]*t^2*b2)*s + [3]*t^2*b3
 *
 *                                           [n]
 * where s=1-t and the binomial coefficients [i]. These can
 * be computed iteratively using the identity:
 *
 * [n]               [n  ]             [n]
 * [i] = (n-i+1)/i * [i-1]     and     [0] = 1
 */


static void
_math_horner_bezier_curve(const GLfloat * cp, GLfloat * out, GLfloat t,
			  GLuint dim, GLuint order)
{
   GLfloat s, powert, bincoeff;
   GLuint i, k;

   if (order >= 2) {
      bincoeff = (GLfloat) (order - 1);
      s = 1.0F - t;

      for (k = 0; k < dim; k++)
	 out[k] = s * cp[k] + bincoeff * t * cp[dim + k];

      for (i = 2, cp += 2 * dim, powert = t * t; i < order;
	   i++, powert *= t, cp += dim) {
	 bincoeff *= (GLfloat) (order - i);
	 bincoeff *= inv_tab[i];

	 for (k = 0; k < dim; k++)
	    out[k] = s * out[k] + bincoeff * powert * cp[k];
      }
   }
   else {			/* order=1 -> constant curve */

      for (k = 0; k < dim; k++)
	 out[k] = cp[k];
   }
}

/*
 * Tensor product Bezier surfaces
 *
 * Again the Horner scheme is used to compute a point on a
 * TP Bezier surface. First a control polygon for a curve
 * on the surface in one parameter direction is computed,
 * then the point on the curve for the other parameter
 * direction is evaluated.
 *
 * To store the curve control polygon additional storage
 * for max(uorder,vorder) points is needed in the
 * control net cn.
 */

static void
_math_horner_bezier_surf(GLfloat * cn, GLfloat * out, GLfloat u, GLfloat v,
			 GLuint dim, GLuint uorder, GLuint vorder)
{
   GLfloat *cp = cn + uorder * vorder * dim;
   GLuint i, uinc = vorder * dim;

   if (vorder > uorder) {
      if (uorder >= 2) {
	 GLfloat s, poweru, bincoeff;
	 GLuint j, k;

	 /* Compute the control polygon for the surface-curve in u-direction */
	 for (j = 0; j < vorder; j++) {
	    GLfloat *ucp = &cn[j * dim];

	    /* Each control point is the point for parameter u on a */
	    /* curve defined by the control polygons in u-direction */
	    bincoeff = (GLfloat) (uorder - 1);
	    s = 1.0F - u;

	    for (k = 0; k < dim; k++)
	       cp[j * dim + k] = s * ucp[k] + bincoeff * u * ucp[uinc + k];

	    for (i = 2, ucp += 2 * uinc, poweru = u * u; i < uorder;
		 i++, poweru *= u, ucp += uinc) {
	       bincoeff *= (GLfloat) (uorder - i);
	       bincoeff *= inv_tab[i];

	       for (k = 0; k < dim; k++)
		  cp[j * dim + k] =
		     s * cp[j * dim + k] + bincoeff * poweru * ucp[k];
	    }
	 }

	 /* Evaluate curve point in v */
	 _math_horner_bezier_curve(cp, out, v, dim, vorder);
      }
      else			/* uorder=1 -> cn defines a curve in v */
	 _math_horner_bezier_curve(cn, out, v, dim, vorder);
   }
   else {			/* vorder <= uorder */

      if (vorder > 1) {
	 GLuint i;

	 /* Compute the control polygon for the surface-curve in u-direction */
	 for (i = 0; i < uorder; i++, cn += uinc) {
	    /* For constant i all cn[i][j] (j=0..vorder) are located */
	    /* on consecutive memory locations, so we can use        */
	    /* horner_bezier_curve to compute the control points     */

	    _math_horner_bezier_curve(cn, &cp[i * dim], v, dim, vorder);
	 }

	 /* Evaluate curve point in u */
	 _math_horner_bezier_curve(cp, out, u, dim, uorder);
      }
      else			/* vorder=1 -> cn defines a curve in u */
	 _math_horner_bezier_curve(cn, out, u, dim, uorder);
   }
}

/*
 * The direct de Casteljau algorithm is used when a point on the
 * surface and the tangent directions spanning the tangent plane
 * should be computed (this is needed to compute normals to the
 * surface). In this case the de Casteljau algorithm approach is
 * nicer because a point and the partial derivatives can be computed
 * at the same time. To get the correct tangent length du and dv
 * must be multiplied with the (u2-u1)/uorder-1 and (v2-v1)/vorder-1.
 * Since only the directions are needed, this scaling step is omitted.
 *
 * De Casteljau needs additional storage for uorder*vorder
 * values in the control net cn.
 */

static void
_math_de_casteljau_surf(GLfloat * cn, GLfloat * out, GLfloat * du,
			GLfloat * dv, GLfloat u, GLfloat v, GLuint dim,
			GLuint uorder, GLuint vorder)
{
   GLfloat *dcn = cn + uorder * vorder * dim;
   GLfloat us = 1.0F - u, vs = 1.0F - v;
   GLuint h, i, j, k;
   GLuint minorder = uorder < vorder ? uorder : vorder;
   GLuint uinc = vorder * dim;
   GLuint dcuinc = vorder;

   /* Each component is evaluated separately to save buffer space  */
   /* This does not drasticaly decrease the performance of the     */
   /* algorithm. If additional storage for (uorder-1)*(vorder-1)   */
   /* points would be available, the components could be accessed  */
   /* in the innermost loop which could lead to less cache misses. */

#define CN(I,J,K) cn[(I)*uinc+(J)*dim+(K)]
#define DCN(I, J) dcn[(I)*dcuinc+(J)]
   if (minorder < 3) {
      if (uorder == vorder) {
	 for (k = 0; k < dim; k++) {
	    /* Derivative direction in u */
	    du[k] = vs * (CN(1, 0, k) - CN(0, 0, k)) +
	       v * (CN(1, 1, k) - CN(0, 1, k));

	    /* Derivative direction in v */
	    dv[k] = us * (CN(0, 1, k) - CN(0, 0, k)) +
	       u * (CN(1, 1, k) - CN(1, 0, k));

	    /* bilinear de Casteljau step */
	    out[k] = us * (vs * CN(0, 0, k) + v * CN(0, 1, k)) +
	       u * (vs * CN(1, 0, k) + v * CN(1, 1, k));
	 }
      }
      else if (minorder == uorder) {
	 for (k = 0; k < dim; k++) {
	    /* bilinear de Casteljau step */
	    DCN(1, 0) = CN(1, 0, k) - CN(0, 0, k);
	    DCN(0, 0) = us * CN(0, 0, k) + u * CN(1, 0, k);

	    for (j = 0; j < vorder - 1; j++) {
	       /* for the derivative in u */
	       DCN(1, j + 1) = CN(1, j + 1, k) - CN(0, j + 1, k);
	       DCN(1, j) = vs * DCN(1, j) + v * DCN(1, j + 1);

	       /* for the `point' */
	       DCN(0, j + 1) = us * CN(0, j + 1, k) + u * CN(1, j + 1, k);
	       DCN(0, j) = vs * DCN(0, j) + v * DCN(0, j + 1);
	    }

	    /* remaining linear de Casteljau steps until the second last step */
	    for (h = minorder; h < vorder - 1; h++)
	       for (j = 0; j < vorder - h; j++) {
		  /* for the derivative in u */
		  DCN(1, j) = vs * DCN(1, j) + v * DCN(1, j + 1);

		  /* for the `point' */
		  DCN(0, j) = vs * DCN(0, j) + v * DCN(0, j + 1);
	       }

	    /* derivative direction in v */
	    dv[k] = DCN(0, 1) - DCN(0, 0);

	    /* derivative direction in u */
	    du[k] = vs * DCN(1, 0) + v * DCN(1, 1);

	    /* last linear de Casteljau step */
	    out[k] = vs * DCN(0, 0) + v * DCN(0, 1);
	 }
      }
      else {			/* minorder == vorder */

	 for (k = 0; k < dim; k++) {
	    /* bilinear de Casteljau step */
	    DCN(0, 1) = CN(0, 1, k) - CN(0, 0, k);
	    DCN(0, 0) = vs * CN(0, 0, k) + v * CN(0, 1, k);
	    for (i = 0; i < uorder - 1; i++) {
	       /* for the derivative in v */
	       DCN(i + 1, 1) = CN(i + 1, 1, k) - CN(i + 1, 0, k);
	       DCN(i, 1) = us * DCN(i, 1) + u * DCN(i + 1, 1);

	       /* for the `point' */
	       DCN(i + 1, 0) = vs * CN(i + 1, 0, k) + v * CN(i + 1, 1, k);
	       DCN(i, 0) = us * DCN(i, 0) + u * DCN(i + 1, 0);
	    }

	    /* remaining linear de Casteljau steps until the second last step */
	    for (h = minorder; h < uorder - 1; h++)
	       for (i = 0; i < uorder - h; i++) {
		  /* for the derivative in v */
		  DCN(i, 1) = us * DCN(i, 1) + u * DCN(i + 1, 1);

		  /* for the `point' */
		  DCN(i, 0) = us * DCN(i, 0) + u * DCN(i + 1, 0);
	       }

	    /* derivative direction in u */
	    du[k] = DCN(1, 0) - DCN(0, 0);

	    /* derivative direction in v */
	    dv[k] = us * DCN(0, 1) + u * DCN(1, 1);

	    /* last linear de Casteljau step */
	    out[k] = us * DCN(0, 0) + u * DCN(1, 0);
	 }
      }
   }
   else if (uorder == vorder) {
      for (k = 0; k < dim; k++) {
	 /* first bilinear de Casteljau step */
	 for (i = 0; i < uorder - 1; i++) {
	    DCN(i, 0) = us * CN(i, 0, k) + u * CN(i + 1, 0, k);
	    for (j = 0; j < vorder - 1; j++) {
	       DCN(i, j + 1) = us * CN(i, j + 1, k) + u * CN(i + 1, j + 1, k);
	       DCN(i, j) = vs * DCN(i, j) + v * DCN(i, j + 1);
	    }
	 }

	 /* remaining bilinear de Casteljau steps until the second last step */
	 for (h = 2; h < minorder - 1; h++)
	    for (i = 0; i < uorder - h; i++) {
	       DCN(i, 0) = us * DCN(i, 0) + u * DCN(i + 1, 0);
	       for (j = 0; j < vorder - h; j++) {
		  DCN(i, j + 1) = us * DCN(i, j + 1) + u * DCN(i + 1, j + 1);
		  DCN(i, j) = vs * DCN(i, j) + v * DCN(i, j + 1);
	       }
	    }

	 /* derivative direction in u */
	 du[k] = vs * (DCN(1, 0) - DCN(0, 0)) + v * (DCN(1, 1) - DCN(0, 1));

	 /* derivative direction in v */
	 dv[k] = us * (DCN(0, 1) - DCN(0, 0)) + u * (DCN(1, 1) - DCN(1, 0));

	 /* last bilinear de Casteljau step */
	 out[k] = us * (vs * DCN(0, 0) + v * DCN(0, 1)) +
	    u * (vs * DCN(1, 0) + v * DCN(1, 1));
      }
   }
   else if (minorder == uorder) {
      for (k = 0; k < dim; k++) {
	 /* first bilinear de Casteljau step */
	 for (i = 0; i < uorder - 1; i++) {
	    DCN(i, 0) = us * CN(i, 0, k) + u * CN(i + 1, 0, k);
	    for (j = 0; j < vorder - 1; j++) {
	       DCN(i, j + 1) = us * CN(i, j + 1, k) + u * CN(i + 1, j + 1, k);
	       DCN(i, j) = vs * DCN(i, j) + v * DCN(i, j + 1);
	    }
	 }

	 /* remaining bilinear de Casteljau steps until the second last step */
	 for (h = 2; h < minorder - 1; h++)
	    for (i = 0; i < uorder - h; i++) {
	       DCN(i, 0) = us * DCN(i, 0) + u * DCN(i + 1, 0);
	       for (j = 0; j < vorder - h; j++) {
		  DCN(i, j + 1) = us * DCN(i, j + 1) + u * DCN(i + 1, j + 1);
		  DCN(i, j) = vs * DCN(i, j) + v * DCN(i, j + 1);
	       }
	    }

	 /* last bilinear de Casteljau step */
	 DCN(2, 0) = DCN(1, 0) - DCN(0, 0);
	 DCN(0, 0) = us * DCN(0, 0) + u * DCN(1, 0);
	 for (j = 0; j < vorder - 1; j++) {
	    /* for the derivative in u */
	    DCN(2, j + 1) = DCN(1, j + 1) - DCN(0, j + 1);
	    DCN(2, j) = vs * DCN(2, j) + v * DCN(2, j + 1);

	    /* for the `point' */
	    DCN(0, j + 1) = us * DCN(0, j + 1) + u * DCN(1, j + 1);
	    DCN(0, j) = vs * DCN(0, j) + v * DCN(0, j + 1);
	 }

	 /* remaining linear de Casteljau steps until the second last step */
	 for (h = minorder; h < vorder - 1; h++)
	    for (j = 0; j < vorder - h; j++) {
	       /* for the derivative in u */
	       DCN(2, j) = vs * DCN(2, j) + v * DCN(2, j + 1);

	       /* for the `point' */
	       DCN(0, j) = vs * DCN(0, j) + v * DCN(0, j + 1);
	    }

	 /* derivative direction in v */
	 dv[k] = DCN(0, 1) - DCN(0, 0);

	 /* derivative direction in u */
	 du[k] = vs * DCN(2, 0) + v * DCN(2, 1);

	 /* last linear de Casteljau step */
	 out[k] = vs * DCN(0, 0) + v * DCN(0, 1);
      }
   }
   else {			/* minorder == vorder */

      for (k = 0; k < dim; k++) {
	 /* first bilinear de Casteljau step */
	 for (i = 0; i < uorder - 1; i++) {
	    DCN(i, 0) = us * CN(i, 0, k) + u * CN(i + 1, 0, k);
	    for (j = 0; j < vorder - 1; j++) {
	       DCN(i, j + 1) = us * CN(i, j + 1, k) + u * CN(i + 1, j + 1, k);
	       DCN(i, j) = vs * DCN(i, j) + v * DCN(i, j + 1);
	    }
	 }

	 /* remaining bilinear de Casteljau steps until the second last step */
	 for (h = 2; h < minorder - 1; h++)
	    for (i = 0; i < uorder - h; i++) {
	       DCN(i, 0) = us * DCN(i, 0) + u * DCN(i + 1, 0);
	       for (j = 0; j < vorder - h; j++) {
		  DCN(i, j + 1) = us * DCN(i, j + 1) + u * DCN(i + 1, j + 1);
		  DCN(i, j) = vs * DCN(i, j) + v * DCN(i, j + 1);
	       }
	    }

	 /* last bilinear de Casteljau step */
	 DCN(0, 2) = DCN(0, 1) - DCN(0, 0);
	 DCN(0, 0) = vs * DCN(0, 0) + v * DCN(0, 1);
	 for (i = 0; i < uorder - 1; i++) {
	    /* for the derivative in v */
	    DCN(i + 1, 2) = DCN(i + 1, 1) - DCN(i + 1, 0);
	    DCN(i, 2) = us * DCN(i, 2) + u * DCN(i + 1, 2);

	    /* for the `point' */
	    DCN(i + 1, 0) = vs * DCN(i + 1, 0) + v * DCN(i + 1, 1);
	    DCN(i, 0) = us * DCN(i, 0) + u * DCN(i + 1, 0);
	 }

	 /* remaining linear de Casteljau steps until the second last step */
	 for (h = minorder; h < uorder - 1; h++)
	    for (i = 0; i < uorder - h; i++) {
	       /* for the derivative in v */
	       DCN(i, 2) = us * DCN(i, 2) + u * DCN(i + 1, 2);

	       /* for the `point' */
	       DCN(i, 0) = us * DCN(i, 0) + u * DCN(i + 1, 0);
	    }

	 /* derivative direction in u */
	 du[k] = DCN(1, 0) - DCN(0, 0);

	 /* derivative direction in v */
	 dv[k] = us * DCN(0, 2) + u * DCN(1, 2);

	 /* last linear de Casteljau step */
	 out[k] = us * DCN(0, 0) + u * DCN(1, 0);
      }
   }
#undef DCN
#undef CN
}

/*
 * Do one-time initialization for evaluators.
 */
void
_math_init_eval(void)
{
   GLuint i;

   /* KW: precompute 1/x for useful x.
    */
   for (i = 1; i < MAX_EVAL_ORDER; i++)
      inv_tab[i] = 1.0F / i;
}

/* EVALUATORS */

static void do_EvalCoord1f(GLfloat u)
{
   GET_CONTEXT(ctx);
   CREvaluatorState *e = &(ctx->eval);


   /** Color Index **/
   if (e->enable1D[GL_MAP1_INDEX - GL_MAP1_COLOR_4])
   {
      GLint i = GL_MAP1_INDEX - GL_MAP1_COLOR_4;
      GLfloat findex;
      GLfloat uu = (u - e->eval1D[i].u1) * e->eval1D[i].du;
      _math_horner_bezier_curve(e->eval1D[i].coeff, 
		      			&findex, uu, 1, e->eval1D[i].order);
      if (tilesort_spu.swap) {
	      crPackIndexiSWAP( (GLint) findex );
      } else {
              crPackIndexi( (GLint) findex );
      }
   }

   /** Color **/
   if (e->enable1D[GL_MAP1_COLOR_4 - GL_MAP1_COLOR_4])
   {
      GLint i = GL_MAP1_COLOR_4 - GL_MAP1_COLOR_4;
      GLfloat fcolor[4];
      GLfloat uu = (u - e->eval1D[i].u1) * e->eval1D[i].du;
      _math_horner_bezier_curve(e->eval1D[i].coeff,
		      			fcolor, uu, 4, e->eval1D[i].order);
      if (tilesort_spu.swap) {
	      crPackColor4fvSWAP( fcolor );
      } else {
              crPackColor4fv( fcolor );
      }
   }

   /** Normal Vector **/
   if (e->enable1D[GL_MAP1_NORMAL - GL_MAP1_COLOR_4])
   {
      GLint i = GL_MAP1_NORMAL - GL_MAP1_COLOR_4;
      GLfloat normal[3];
      GLfloat uu = (u - e->eval1D[i].u1) * e->eval1D[i].du;
      _math_horner_bezier_curve(e->eval1D[i].coeff,
		      			normal, uu, 3, e->eval1D[i].order);
      if (tilesort_spu.swap) {
              crPackNormal3fvSWAP( normal );
      } else {
	      crPackNormal3fv( normal );
      }
   }

   /** Texture Coordinates **/
   if (e->enable1D[GL_MAP1_TEXTURE_COORD_4 - GL_MAP1_COLOR_4])
   {
      GLint i = GL_MAP1_TEXTURE_COORD_4 - GL_MAP1_COLOR_4;
      GLfloat texcoord[4];
      GLfloat uu = (u - e->eval1D[i].u1) * e->eval1D[i].du;
      _math_horner_bezier_curve(e->eval1D[i].coeff, 
		      			texcoord, uu, 4, e->eval1D[i].order);
      if (tilesort_spu.swap) {
              crPackTexCoord4fvSWAP( texcoord );
      } else {
              crPackTexCoord4fv( texcoord );
      }
   }
   else if (e->enable1D[GL_MAP1_TEXTURE_COORD_3 - GL_MAP1_COLOR_4])
   {
      GLint i = GL_MAP1_TEXTURE_COORD_3 - GL_MAP1_COLOR_4;
      GLfloat texcoord[4];
      GLfloat uu = (u - e->eval1D[i].u1) * e->eval1D[i].du;
      _math_horner_bezier_curve(e->eval1D[i].coeff, 
		      			texcoord, uu, 3, e->eval1D[i].order);
      if (tilesort_spu.swap) {
              crPackTexCoord3fvSWAP( texcoord );
      } else {
              crPackTexCoord3fv( texcoord );
      }
   }
   else if (e->enable1D[GL_MAP1_TEXTURE_COORD_2 - GL_MAP1_COLOR_4])
   {
      GLint i = GL_MAP1_TEXTURE_COORD_2 - GL_MAP1_COLOR_4;
      GLfloat texcoord[4];
      GLfloat uu = (u - e->eval1D[i].u1) * e->eval1D[i].du;
      _math_horner_bezier_curve(e->eval1D[i].coeff,
		      			texcoord, uu, 2, e->eval1D[i].order);
      if (tilesort_spu.swap) {
              crPackTexCoord2fvSWAP( texcoord );
      } else {
              crPackTexCoord2fv( texcoord );
      }
   }
   else if (e->enable1D[GL_MAP1_TEXTURE_COORD_1 - GL_MAP1_COLOR_4])
   {
      GLint i = GL_MAP1_TEXTURE_COORD_1 - GL_MAP1_COLOR_4;
      GLfloat texcoord[4];
      GLfloat uu = (u - e->eval1D[i].u1) * e->eval1D[i].du;
      _math_horner_bezier_curve(e->eval1D[i].coeff,
		      			texcoord, uu, 1, e->eval1D[i].order);
      if (tilesort_spu.swap) {
              crPackTexCoord1fvSWAP( texcoord );
      } else {
              crPackTexCoord1fv( texcoord );
      }
   }
  
   /** Vertex **/
   if (e->enable1D[GL_MAP1_VERTEX_4 - GL_MAP1_COLOR_4])
   {
      GLint i = GL_MAP1_VERTEX_4 - GL_MAP1_COLOR_4;
      GLfloat vertex[4];
      GLfloat uu = (u - e->eval1D[i].u1) * e->eval1D[i].du;
      _math_horner_bezier_curve(e->eval1D[i].coeff,
		      			vertex, uu, 4, e->eval1D[i].order);
      if (tilesort_spu.swap) {
              crPackVertex4fvBBOX_COUNTSWAP( vertex );
      } else {
              crPackVertex4fvBBOX_COUNT( vertex );
      }
   }
   else if (e->enable1D[GL_MAP1_VERTEX_3 - GL_MAP1_COLOR_4])
   {
      GLint i = GL_MAP1_VERTEX_3 - GL_MAP1_COLOR_4;
      GLfloat vertex[4];
      GLfloat uu = (u - e->eval1D[i].u1) * e->eval1D[i].du;
      _math_horner_bezier_curve(e->eval1D[i].coeff,
		      			vertex, uu, 3, e->eval1D[i].order);
      if (tilesort_spu.swap) {
              crPackVertex3fvBBOX_COUNTSWAP( vertex );
      } else {
              crPackVertex3fvBBOX_COUNT( vertex );
      }
   }
}

#define CROSS_PROD(n, u, v) \
  (n)[0] = (u)[1]*(v)[2] - (u)[2]*(v)[1]; \
  (n)[1] = (u)[2]*(v)[0] - (u)[0]*(v)[2]; \
  (n)[2] = (u)[0]*(v)[1] - (u)[1]*(v)[0]


static void do_EvalCoord2f( GLfloat u, GLfloat v )
{   
   GET_CONTEXT(ctx);
   CREvaluatorState *e = &(ctx->eval);

   /** Color Index **/
   if (e->enable2D[GL_MAP2_INDEX - GL_MAP2_COLOR_4])
   {
      GLint i = GL_MAP2_INDEX - GL_MAP2_COLOR_4;
      GLfloat findex;
      GLfloat uu = (u - e->eval2D[i].u1) * e->eval2D[i].du;
      GLfloat vv = (v - e->eval2D[i].v1) * e->eval2D[i].dv;
      _math_horner_bezier_surf(e->eval2D[i].coeff, &findex, uu, vv, 1,
                         e->eval2D[i].uorder, e->eval2D[i].vorder);
      if (tilesort_spu.swap) {
              crPackIndexiSWAP( (GLuint) (GLint) findex );
      } else {
              crPackIndexi( (GLuint) (GLint) findex );
      }
   }

   /** Color **/
   if (e->enable2D[GL_MAP2_COLOR_4 - GL_MAP2_COLOR_4])
   {
      GLint i = GL_MAP2_COLOR_4 - GL_MAP2_COLOR_4;
      GLfloat fcolor[4];
      GLfloat uu = (u - e->eval2D[i].u1) * e->eval2D[i].du;
      GLfloat vv = (v - e->eval2D[i].v1) * e->eval2D[i].dv;
      _math_horner_bezier_surf(e->eval2D[i].coeff, fcolor, uu, vv, 4,
                         e->eval2D[i].uorder, e->eval2D[i].vorder);
      if (tilesort_spu.swap) {
              crPackColor4fvSWAP( fcolor );
      } else {
              crPackColor4fv( fcolor );
      }
   }

   /** Normal **/
   if (e->enable2D[GL_MAP2_NORMAL - GL_MAP2_COLOR_4] &&
 	(!e->autoNormal || (!e->enable2D[GL_MAP2_VERTEX_3 - GL_MAP2_COLOR_4] &&
			    !e->enable2D[GL_MAP2_VERTEX_4 - GL_MAP2_COLOR_4])))
   {
      GLint i = GL_MAP2_NORMAL - GL_MAP2_COLOR_4;
      GLfloat normal[3];
      GLfloat uu = (u - e->eval2D[i].u1) * e->eval2D[i].du;
      GLfloat vv = (v - e->eval2D[i].v1) * e->eval2D[i].dv;
      _math_horner_bezier_surf(e->eval2D[i].coeff, normal, uu, vv, 3,
                         e->eval2D[i].uorder, e->eval2D[i].vorder);
      if (tilesort_spu.swap) {
              crPackNormal3fvSWAP( normal );
      } else {
              crPackNormal3fv( normal );
      }
   }

   /** Texture Coordinates **/
   if (e->enable2D[GL_MAP2_TEXTURE_COORD_4 - GL_MAP2_COLOR_4])
   {
      GLint i = GL_MAP2_TEXTURE_COORD_4 - GL_MAP2_COLOR_4;
      GLfloat texcoord[4];
      GLfloat uu = (u - e->eval2D[i].u1) * e->eval2D[i].du;
      GLfloat vv = (v - e->eval2D[i].v1) * e->eval2D[i].dv;
      _math_horner_bezier_surf(e->eval2D[i].coeff, texcoord, uu, vv, 4,
                         e->eval2D[i].uorder, e->eval2D[i].vorder);
      if (tilesort_spu.swap) {
              crPackTexCoord4fvSWAP( texcoord );
      } else {
              crPackTexCoord4fv( texcoord );
      }
   }
   else if (e->enable2D[GL_MAP2_TEXTURE_COORD_3 - GL_MAP2_COLOR_4])
   {
      GLint i = GL_MAP2_TEXTURE_COORD_3 - GL_MAP2_COLOR_4;
      GLfloat texcoord[4];
      GLfloat uu = (u - e->eval2D[i].u1) * e->eval2D[i].du;
      GLfloat vv = (v - e->eval2D[i].v1) * e->eval2D[i].dv;
      _math_horner_bezier_surf(e->eval2D[i].coeff, texcoord, uu, vv, 3,
                         e->eval2D[i].uorder, e->eval2D[i].vorder);
      if (tilesort_spu.swap) {
              crPackTexCoord3fvSWAP( texcoord );
      } else {
              crPackTexCoord3fv( texcoord );
      }
   }
   else if (e->enable2D[GL_MAP2_TEXTURE_COORD_2 - GL_MAP2_COLOR_4])
   {
      GLint i = GL_MAP2_TEXTURE_COORD_2 - GL_MAP2_COLOR_4;
      GLfloat texcoord[4];
      GLfloat uu = (u - e->eval2D[i].u1) * e->eval2D[i].du;
      GLfloat vv = (v - e->eval2D[i].v1) * e->eval2D[i].dv;
      _math_horner_bezier_surf(e->eval2D[i].coeff, texcoord, uu, vv, 2,
                         e->eval2D[i].uorder, e->eval2D[i].vorder);
      if (tilesort_spu.swap) {
              crPackTexCoord2fvSWAP( texcoord );
      } else {
              crPackTexCoord2fv( texcoord );
      }
   }
   else if (e->enable2D[GL_MAP2_TEXTURE_COORD_1 - GL_MAP2_COLOR_4])
   {
      GLint i = GL_MAP2_TEXTURE_COORD_1 - GL_MAP2_COLOR_4;
      GLfloat texcoord[4];
      GLfloat uu = (u - e->eval2D[i].u1) * e->eval2D[i].du;
      GLfloat vv = (v - e->eval2D[i].v1) * e->eval2D[i].dv;
      _math_horner_bezier_surf(e->eval2D[i].coeff, texcoord, uu, vv, 1,
                         e->eval2D[i].uorder, e->eval2D[i].vorder);
      if (tilesort_spu.swap) {
              crPackTexCoord1fvSWAP( texcoord );
      } else {
              crPackTexCoord1fv( texcoord );
      }
   }

   /** Vertex **/
   if (e->enable2D[GL_MAP2_VERTEX_4 - GL_MAP2_COLOR_4])
   {
      GLint i = GL_MAP2_VERTEX_4 - GL_MAP2_COLOR_4;
      GLfloat vertex[4];
      GLfloat normal[3];
      GLfloat uu = (u - e->eval2D[i].u1) * e->eval2D[i].du;
      GLfloat vv = (v - e->eval2D[i].v1) * e->eval2D[i].dv;

      if (e->autoNormal) {
         GLfloat idu[4], idv[4];

         _math_de_casteljau_surf(e->eval2D[i].coeff, vertex, 
			 	 idu, idv, uu, vv, 4,
                         	 e->eval2D[i].uorder, e->eval2D[i].vorder);

         CROSS_PROD(normal, idu, idv);
         NORMALIZE_3FV(normal);
	 if (tilesort_spu.swap) {
	         crPackNormal3fvSWAP(normal);
	         crPackVertex4fvBBOX_COUNTSWAP(vertex);
	 } else {
	         crPackNormal3fv(normal);
	         crPackVertex4fvBBOX_COUNT(vertex);
	 }
      }
      else {
         _math_horner_bezier_surf(e->eval2D[i].coeff, vertex, 
			 	 uu, vv, 4,
                         	 e->eval2D[i].uorder, e->eval2D[i].vorder);
	 if (tilesort_spu.swap) {
	         crPackVertex4fvBBOX_COUNTSWAP(vertex);
	 } else {
	         crPackVertex4fvBBOX_COUNT(vertex);
	 }
      }
   }
   else if (e->enable2D[GL_MAP2_VERTEX_3 - GL_MAP2_COLOR_4])
   {
      GLint i = GL_MAP2_VERTEX_3 - GL_MAP2_COLOR_4;
      GLfloat vertex[4];
      GLfloat uu = (u - e->eval2D[i].u1) * e->eval2D[i].du;
      GLfloat vv = (v - e->eval2D[i].v1) * e->eval2D[i].dv;
      if (e->autoNormal) {
         GLfloat idu[3], idv[3];
	 GLfloat normal[3];
         _math_de_casteljau_surf(e->eval2D[i].coeff, vertex, 
			  	 idu, idv, uu, vv, 3,
                         	 e->eval2D[i].uorder, e->eval2D[i].vorder);
         CROSS_PROD(normal, idu, idv);
         NORMALIZE_3FV(normal);
	 if (tilesort_spu.swap) {
	         crPackNormal3fvSWAP( normal );
	         crPackVertex3fvBBOX_COUNTSWAP(vertex);
	 } else {
	         crPackNormal3fv( normal );
	         crPackVertex3fvBBOX_COUNT(vertex);
	 }
      }
      else {
         _math_horner_bezier_surf(e->eval2D[i].coeff, vertex, 
			 	 uu, vv, 3,
                         	 e->eval2D[i].uorder, e->eval2D[i].vorder);
	 if (tilesort_spu.swap) {
	         crPackVertex3fvBBOX_COUNTSWAP(vertex);
	 } else {
	         crPackVertex3fvBBOX_COUNT(vertex);
	 }
      }
   }
}

void TILESORTSPU_APIENTRY tilesortspu_EvalCoord1f( GLfloat u )
{
   GET_CONTEXT(ctx);
   CRCurrentState *c = &(ctx->current);
   GLfloat normal[3], texcoord[CR_MAX_TEXTURE_UNITS][4], color[4];
   GLfloat index;
   GLint i;

   CRASSERT(CR_MAX_TEXTURE_UNITS >= ctx->limits.maxTextureUnits);

   for (i = 0; i < CR_MAX_TEXTURE_UNITS; i++)
	   crMemcpy (&texcoord[i], &c->texCoord[i], 4 * sizeof(GLfloat));

   crMemcpy(&normal, &c->normal, 3 * sizeof(GLfloat));
   crMemcpy(&color, &c->color, 4 * sizeof(GLfloat));
   index = c->index;

   do_EvalCoord1f( u );

   for (i = 0; i < CR_MAX_TEXTURE_UNITS; i++)
	   crMemcpy (&c->texCoord[i], &texcoord[i], 4 * sizeof(GLfloat));

   crMemcpy(&c->normal, &normal, 3 * sizeof(GLfloat));
   crMemcpy(&c->color, &color, 4 * sizeof(GLfloat));
   c->index = index;
}

void TILESORTSPU_APIENTRY tilesortspu_EvalCoord2f( GLfloat u, GLfloat v )
{
   GET_CONTEXT(ctx);
   CRCurrentState *c = &(ctx->current);
   GLfloat normal[3], texcoord[CR_MAX_TEXTURE_UNITS][4], color[4];
   GLfloat index;
   GLint i;

   CRASSERT(CR_MAX_TEXTURE_UNITS >= ctx->limits.maxTextureUnits);

   for (i = 0; i < CR_MAX_TEXTURE_UNITS; i++)
	   crMemcpy (&texcoord[i], &c->texCoord[i], 4 * sizeof(GLfloat));

   crMemcpy(&normal, &c->normal, 3 * sizeof(GLfloat));
   crMemcpy(&color, &c->color, 4 * sizeof(GLfloat));
   index = c->index;

   do_EvalCoord2f( u, v );

   for (i = 0; i < CR_MAX_TEXTURE_UNITS; i++)
	   crMemcpy (&c->texCoord[i], &texcoord[i], 4 * sizeof(GLfloat));

   crMemcpy(&c->normal, &normal, 3 * sizeof(GLfloat));
   crMemcpy(&c->color, &color, 4 * sizeof(GLfloat));
   c->index = index;
}

void TILESORTSPU_APIENTRY tilesortspu_EvalCoord1fv( const GLfloat *u )
{
   tilesortspu_EvalCoord1f( u[0] );
}

void TILESORTSPU_APIENTRY tilesortspu_EvalCoord2fv( const GLfloat *u )
{
   tilesortspu_EvalCoord2f( u[0], u[1] );
}

void TILESORTSPU_APIENTRY tilesortspu_EvalCoord2dv( const GLdouble *u )
{
   tilesortspu_EvalCoord2f( (GLfloat) u[0], (GLfloat) u[1] );
}

void TILESORTSPU_APIENTRY tilesortspu_EvalCoord2d( GLdouble u, GLdouble v )
{
   tilesortspu_EvalCoord2f( (GLfloat) u, (GLfloat) v );
}

void TILESORTSPU_APIENTRY tilesortspu_EvalCoord1dv( const GLdouble *u )
{
   tilesortspu_EvalCoord1f( (GLfloat) u[0] );
}

void TILESORTSPU_APIENTRY tilesortspu_EvalCoord1d( GLdouble u )
{
   tilesortspu_EvalCoord1f( (GLfloat) u );
}

void TILESORTSPU_APIENTRY tilesortspu_EvalPoint1( GLint i )
{
   GET_CONTEXT(ctx);
   CREvaluatorState *e = &(ctx->eval);
   GLfloat du = ((e->u21D - e->u11D) /
		 (GLfloat) e->un1D);
   GLfloat u = i * du + e->u11D;

   tilesortspu_EvalCoord1f( u );
}


void TILESORTSPU_APIENTRY tilesortspu_EvalPoint2( GLint i, GLint j )
{
   GET_CONTEXT(ctx);
   CREvaluatorState *e = &(ctx->eval);
   GLfloat du = ((e->u22D - e->u12D) / 
		 (GLfloat) e->un2D);
   GLfloat dv = ((e->v22D - e->v12D) / 
		 (GLfloat) e->vn2D);
   GLfloat u = i * du + e->u12D;
   GLfloat v = j * dv + e->v12D;

   tilesortspu_EvalCoord2f( u, v );
}

void TILESORTSPU_APIENTRY tilesortspu_EvalMesh1( GLenum mode, GLint i1, GLint i2)
{
   GET_CONTEXT(ctx);
   CREvaluatorState *e = &(ctx->eval);
   GLint i;
   GLfloat u, du;
   GLenum prim;

   switch (mode) {
      case GL_POINT:
         prim = GL_POINTS;
         break;
      case GL_LINE:
         prim = GL_LINE_STRIP;
         break;
      default:
	 crStateError( __LINE__, __FILE__, GL_INVALID_ENUM, "glEvalMesh1(bad mode)" );
         return;
   }

   /* No effect if vertex maps disabled.
    */
   if ( !e->enable1D[GL_MAP1_VERTEX_4 - GL_MAP1_COLOR_4] &&
 	!e->enable1D[GL_MAP1_VERTEX_3 - GL_MAP1_COLOR_4] )
      return;

   du = (e->u21D - e->u11D) / e->un1D;
   u = e->u11D + i1 * du;

   tilesortspu_Begin( prim );
   for (i=i1;i<=i2;i++,u+=du) {
      tilesortspu_EvalCoord1f( u );
   }
   tilesortspu_End( );
}

void TILESORTSPU_APIENTRY tilesortspu_EvalMesh2( GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)
{
   GET_CONTEXT(ctx);
   CREvaluatorState *e = &(ctx->eval);
   GLint i, j;
   GLfloat u, du, v, dv, v1, u1;

   /* No effect if vertex maps disabled.
    */
   if ( !e->enable2D[GL_MAP2_VERTEX_4 - GL_MAP2_COLOR_4] &&
 	!e->enable2D[GL_MAP2_VERTEX_3 - GL_MAP2_COLOR_4] )
      return;

   du = (e->u22D - e->u12D) / e->un2D;
   u1 = e->u12D + i1 * du;
   dv = (e->v22D - e->v12D) / e->vn2D;
   v1 = e->v12D + j1 * dv;

   {
      switch (mode) {
      case GL_POINT:
	 tilesortspu_Begin( GL_POINTS );
	 for (v=v1,j=j1;j<=j2;j++,v+=dv) {
	    for (u=u1,i=i1;i<=i2;i++,u+=du) {
	       tilesortspu_EvalCoord2f( u, v );
	    }
	 }
	 tilesortspu_End();
	 break;
      case GL_LINE:
	 for (v=v1,j=j1;j<=j2;j++,v+=dv) {
	    tilesortspu_Begin( GL_LINE_STRIP );
	    for (u=u1,i=i1;i<=i2;i++,u+=du) {
	       tilesortspu_EvalCoord2f( u, v );
	    }
	    tilesortspu_End();
	 }
	 for (u=u1,i=i1;i<=i2;i++,u+=du) {
	    tilesortspu_Begin( GL_LINE_STRIP );
	    for (v=v1,j=j1;j<=j2;j++,v+=dv) {
	       tilesortspu_EvalCoord2f( u, v );
	    }
	    tilesortspu_End();
	 }
	 break;
      case GL_FILL:
	 for (v=v1,j=j1;j<j2;j++,v+=dv) {
	    tilesortspu_Begin( GL_TRIANGLE_STRIP );
	    for (u=u1,i=i1;i<=i2;i++,u+=du) {
	       tilesortspu_EvalCoord2f( u, v );
	       tilesortspu_EvalCoord2f( u, v+dv );
	    }
	    tilesortspu_End();
	 }
	 break;
      default:
	 crStateError( __LINE__, __FILE__, GL_INVALID_ENUM, "glEvalMesh2(bad mode)" );
	 return;
      }
   }
}
