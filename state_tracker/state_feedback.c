/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "state.h"
#include "cr_glwrapper.h"
#include "state_internals.h"
#include "state/cr_feedback.h"

#define FB_3D		0x01
#define FB_4D		0x02
#define FB_INDEX	0x04
#define FB_COLOR	0x08
#define FB_TEXTURE	0X10

#define FEEDBACK_TOKEN( T )				\
	if (f->count < f->bufferSize) {	\
	   f->buffer[f->count] = (GLfloat) (T); \
	}							\
	f->count++;

/*
 * Transform a point (column vector) by a matrix:   Q = M * P
 */
#define TRANSFORM_POINT( Q, M, P )					\
   Q.x = M.m00 * P.x + M.m10 * P.y + M.m20 * P.z + M.m30 * P.w;\
   Q.y = M.m01 * P.x + M.m11 * P.y + M.m21 * P.z + M.m31 * P.w;\
   Q.z = M.m02 * P.x + M.m12 * P.y + M.m22 * P.z + M.m32 * P.w;\
   Q.w = M.m03 * P.x + M.m13 * P.y + M.m23 * P.z + M.m33 * P.w;

/*
 * Put a vertex into the feedback buffer.
 */
void _feedback_vertex( CRFeedbackState *f,
                         const GLvectorf vertex,
			 const GLcolorf color,
			 GLuint index,
			 const GLvectorf texcoord )
{
   FEEDBACK_TOKEN( vertex.x );
   FEEDBACK_TOKEN( vertex.y );

   if (f->mask & FB_3D) {
      FEEDBACK_TOKEN( vertex.z );
   }

   if (f->mask & FB_4D) {
      FEEDBACK_TOKEN( vertex.w );
   }

   /* We don't deal with color index in Chromium */
   if (f->mask & FB_INDEX) {
      FEEDBACK_TOKEN( (GLfloat) index );
   }

   if (f->mask & FB_COLOR) {
      FEEDBACK_TOKEN( color.r );
      FEEDBACK_TOKEN( color.g );
      FEEDBACK_TOKEN( color.b );
      FEEDBACK_TOKEN( color.a );
   }

   if (f->mask & FB_TEXTURE) {
      FEEDBACK_TOKEN( texcoord.x );
      FEEDBACK_TOKEN( texcoord.y );
      FEEDBACK_TOKEN( texcoord.z );
      FEEDBACK_TOKEN( texcoord.w );
   }
}

void transform_and_feedback_vertex( GLvectorf vertex, GLvectorf texcoord )
{
	CRContext *g = GetCurrentContext();
	CRFeedbackState *f = &(g->feedback);
	CRViewportState *v = &(g->viewport);
	CRTransformState *t = &(g->transform);
	CRCurrentState *c = &(g->current);
	GLvectorf eye;
	GLvectorf clip;

	TRANSFORM_POINT( eye, t->modelView[t->modelViewDepth], vertex );
	TRANSFORM_POINT( clip, t->projection[t->projectionDepth], eye );

   	/*
    	 * Combined clip testing with clip-to-window coordinate mapping.
    	 */
   	{
		/* XXX FIXME, tx, ty - rasteroffsetX,Y */
      		GLfloat sx = (GLfloat)v->viewportW / 2, tx = sx;
      		GLfloat sy = (GLfloat)v->viewportH / 2, ty = sy;
      		GLfloat sz = (GLfloat)(v->farClip - v->nearClip) / 2.0f, tz = sz + (GLfloat)v->nearClip;
      
		if (clip.x > clip.w || clip.x < -clip.w ||
             	    clip.y > clip.w || clip.y < -clip.w ||
             	    clip.z > clip.w || clip.z < -clip.w ) 
		{
            		/* vertex is clipped */
#if 0
			printf("VERTEX CLIPPED\n");
#endif
      		}
      		else 
		{
            		/* vertex not clipped */
            		GLfloat d = 1.0F / clip.w;
            		vertex.x = clip.x * d * sx + tx;
            		vertex.y = clip.y * d * sy + ty;
            		vertex.z = clip.z * d * sz + tz;
	
			_feedback_vertex( f, vertex, c->color, 0, texcoord );
      		}
   	}
}

void STATE_APIENTRY
crStateFeedbackBuffer( GLsizei size, GLenum type, GLfloat *buffer )
{
	CRContext *g = GetCurrentContext();
	CRFeedbackState *f = &(g->feedback);

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "FeedbackBuffer called in begin/end");
		return;
	}

   	if (g->renderMode==GL_FEEDBACK) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					 "Invalid Operation GL_FEEDBACK");
      		return;
   	}
 	if (size<0) {
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
					 "Invalid Value size < 0");
      		return;
	}
   	if (!buffer) {
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE,
					 "Invalid Value buffer = NULL");
      		f->bufferSize = 0;
      		return;
	}

	FLUSH();

   	switch (type) {
      	case GL_2D:
	 	f->mask = 0;
	 	break;
      	case GL_3D:
	 	f->mask = FB_3D;
	 	break;
      	case GL_3D_COLOR:
	 	f->mask = (FB_3D | FB_COLOR);			/* FB_INDEX ?? */
	 	break;
      	case GL_3D_COLOR_TEXTURE:
	 	f->mask = (FB_3D | FB_COLOR | FB_TEXTURE); 	/* FB_INDEX ?? */
	 	break;
      	case GL_4D_COLOR_TEXTURE:
	 	f->mask = (FB_3D | FB_4D | FB_COLOR | FB_TEXTURE); /* FB_INDEX ?? */
	 	break;
	default:
		crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					 "invalid type");
	 	return;
   	}

   	f->type = type;
   	f->bufferSize = size;
   	f->buffer = buffer;
   	f->count = 0;	         
}


void STATE_APIENTRY
crStatePassThrough( GLfloat token )
{
	CRContext *g = GetCurrentContext();
	CRFeedbackState *f = &(g->feedback);

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					 "PassThrough called in begin/end");
		return;
	}

	FLUSH();

   	if (g->renderMode==GL_FEEDBACK) {
      		FEEDBACK_TOKEN( (GLfloat) (GLint) GL_PASS_THROUGH_TOKEN );
      		FEEDBACK_TOKEN( token );
   	}
}

void STATE_APIENTRY
crStateSelectBuffer( GLsizei size, GLuint *buffer )
{
	CRContext *g = GetCurrentContext();
	CRSelectionState *se = &(g->selection);

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					 "SelectBuffer called in begin/end");
		return;
	}

   	if (g->renderMode==GL_SELECT) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
			"SelectBuffer called with RenderMode = GL_SELECT");
      		return;
   	}

   	FLUSH();

   	se->buffer = buffer;
   	se->bufferSize = size;
   	se->bufferCount = 0;
   	se->hitFlag = GL_FALSE;
   	se->hitMinZ = 1.0;
   	se->hitMaxZ = 0.0;
}


#define WRITE_RECORD( V )					\
	if (se->bufferCount < se->bufferSize) {	\
	   se->buffer[se->bufferCount] = (V);	\
	}							\
	se->bufferCount++;



void _update_hitflag( GLfloat z )
{
	CRContext *g = GetCurrentContext();
	CRSelectionState *se = &(g->selection);

   	se->hitFlag = GL_TRUE;

   	if (z < se->hitMinZ)
      		se->hitMinZ = z;

   	if (z > se->hitMaxZ)
      		se->hitMaxZ = z;
}


static void write_hit_record( CRSelectionState *se )
{
   	GLuint i;
   	GLuint zmin, zmax, zscale = (~0u);

   	/* hitMinZ and hitMaxZ are in [0,1].  Multiply these values by */
   	/* 2^32-1 and round to nearest unsigned integer. */

   	zmin = (GLuint) ((GLfloat) zscale * se->hitMinZ);
   	zmax = (GLuint) ((GLfloat) zscale * se->hitMaxZ);

   	WRITE_RECORD( se->nameStackDepth );
   	WRITE_RECORD( zmin );
   	WRITE_RECORD( zmax );
   	for (i = 0; i < se->nameStackDepth; i++) {
      		WRITE_RECORD( se->nameStack[i] );
   	}

   	se->hits++;
   	se->hitFlag = GL_FALSE;
   	se->hitMinZ = 1.0;
   	se->hitMaxZ = -1.0;
}



void STATE_APIENTRY
crStateInitNames( void )
{
	CRContext *g = GetCurrentContext();
	CRSelectionState *se = &(g->selection);

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					 "InitNames called in begin/end");
		return;
	}

	FLUSH();

   	/* Record the hit before the hitFlag is wiped out again. */
   	if (g->renderMode == GL_SELECT) {
      		if (se->hitFlag) {
         		write_hit_record( se );
      		}
   	}
   
	se->nameStackDepth = 0;
   	se->hitFlag = GL_FALSE;
   	se->hitMinZ = 1.0;
   	se->hitMaxZ = 0.0;
}


void STATE_APIENTRY
crStateLoadName( GLuint name )
{
	CRContext *g = GetCurrentContext();
	CRSelectionState *se = &(g->selection);

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					 "LoadName called in begin/end");
		return;
	}


   	if (g->renderMode != GL_SELECT) {
     		 return;
   	}

   	if (se->nameStackDepth == 0) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					 "nameStackDepth = 0");
      	return;
   	}

   	FLUSH();

   	if (se->hitFlag) {
      		write_hit_record( se );
   	}
   	if (se->nameStackDepth < MAX_NAME_STACK_DEPTH) {
      		se->nameStack[se->nameStackDepth-1] = name;
   	}
   	else {
      		se->nameStack[MAX_NAME_STACK_DEPTH-1] = name;
   	}
}


void STATE_APIENTRY
crStatePushName( GLuint name )
{
	CRContext *g = GetCurrentContext();
	CRSelectionState *se = &(g->selection);

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					 "PushName called in begin/end");
		return;
	}


   	if (g->renderMode != GL_SELECT) {
      		return;
   	}

   	FLUSH();

   	if (se->hitFlag) {
      		write_hit_record( se );
   	}
   	if (se->nameStackDepth >= MAX_NAME_STACK_DEPTH) {
		crStateError(__LINE__, __FILE__, GL_STACK_OVERFLOW,
					 "nameStackDepth overflow");
   	}
   	else
      		se->nameStack[se->nameStackDepth++] = name;
}

void STATE_APIENTRY
crStatePopName( void )
{
	CRContext *g = GetCurrentContext();
	CRSelectionState *se = &(g->selection);

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					 "PopName called in begin/end");
		return;
	}

   	if (g->renderMode != GL_SELECT) {
      		return;
   	}

   	FLUSH();

   	if (se->hitFlag) {
      		write_hit_record( se );
   	}

   	if (se->nameStackDepth == 0) {
		crStateError(__LINE__, __FILE__, GL_STACK_UNDERFLOW,
					 "nameStackDepth underflow");
   	}
   	else
      		se->nameStackDepth--;
}

GLint STATE_APIENTRY
crStateRenderMode( GLenum mode )
{
	CRContext *g = GetCurrentContext();
	CRFeedbackState *f = &(g->feedback);
	CRSelectionState *se = &(g->selection);
   	GLint result;

	if (g->current.inBeginEnd) {
		crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "RenderMode called in begin/end");
		return 0;
	}

   	FLUSH();

   	switch (g->renderMode) {
      		case GL_RENDER:
	 		result = 0;
	 		break;
      		case GL_SELECT:
	 		if (se->hitFlag) {
	    			write_hit_record( se );
	 		}

	 		if (se->bufferCount > se->bufferSize) {
	  	  		/* overflow */
	    			result = -1;
	 		}
	 		else {
	    			result = se->hits;
	 		}
	 		se->bufferCount = 0;
	 		se->hits = 0;
	 		se->nameStackDepth = 0;
	 		break;
      		case GL_FEEDBACK:
	 		if (f->count > f->bufferSize) {
	    			/* overflow */
	    			result = -1;
	 		}
	 		else {
	    			result = f->count;
	 		}
	 		f->count = 0;
	 		break;
      		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					 "invalid rendermode");
	 		return 0;
   	}

   	switch (mode) {
      		case GL_RENDER:
         		break;
      		case GL_SELECT:
	 		if (se->bufferSize==0) {
	    			/* haven't called glSelectBuffer yet */
				crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					 "buffersize = 0");
	 		}
	 		break;
      		case GL_FEEDBACK:
	 		if (f->bufferSize==0) {
	    			/* haven't called glFeedbackBuffer yet */
				crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION,
					 "buffersize = 0");
	 		}
	 		break;
      		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM,
					 "invalid rendermode");
	 		return 0;
   	}

   	g->renderMode = mode;

   	return result;
}

/* 
 * Although these functions are used by the feedbackSPU alone, 
 * I've left them here as they interface to the other functions.....
 */

void STATE_APIENTRY
crStateFeedbackBegin( GLenum mode )
{
	CRContext *g = GetCurrentContext();
	CRFeedbackState *f = &(g->feedback);
	CRPolygonState *p = &(g->polygon);

	crStateBegin( mode );

	/* reset vertex counter */
	f->starti = 0;

	if (g->renderMode == GL_RENDER) return;

	switch (mode) {
	case GL_POINTS:
   		FEEDBACK_TOKEN( (GLfloat) (GLint) GL_POINT_TOKEN );
		break;
	case GL_LINES:
	case GL_LINE_LOOP:
	case GL_LINE_STRIP:
   		FEEDBACK_TOKEN( (GLfloat) (GLint) GL_LINE_TOKEN );
		break;
	case GL_TRIANGLES:
	case GL_TRIANGLE_STRIP:
	case GL_TRIANGLE_FAN:
	case GL_QUADS:
	case GL_QUAD_STRIP:
	case GL_POLYGON:
		switch (p->frontMode) {
			case GL_POINT:
				/* emit token in vertex */
				break;
			case GL_LINE:
   				FEEDBACK_TOKEN( (GLfloat) (GLint) GL_LINE_TOKEN );
				break;
			case GL_FILL:
   				FEEDBACK_TOKEN( (GLfloat) (GLint) GL_POLYGON_TOKEN );
				f->polygoni = f->count;
   				FEEDBACK_TOKEN( (GLfloat) 3 ); /* placeholder */
				break;
		}
		break;
	default:
		break;
	}
}

void STATE_APIENTRY
crStateFeedbackEnd( void )
{
	CRContext *g = GetCurrentContext();
	CRFeedbackState *f = &(g->feedback);
	CRPolygonState *p = &(g->polygon);
	CRCurrentState *c = &(g->current);

	crStateEnd( );

	if (c->mode == GL_LINE_LOOP) {
		/* replay the last/first vertex to feedback */
   		FEEDBACK_TOKEN( (GLfloat) (GLint) GL_LINE_TOKEN );
		transform_and_feedback_vertex( f->vertex, f->texcoord );
		transform_and_feedback_vertex( f->startv, f->startt );
	}
	if ((c->mode == GL_TRIANGLES ||
	     c->mode == GL_TRIANGLE_STRIP ||
	     c->mode == GL_TRIANGLE_FAN ||
	     c->mode == GL_QUADS ||
	     c->mode == GL_QUAD_STRIP ||
	     c->mode == GL_POLYGON) &&
	    p->frontMode == GL_LINE && (f->starti > 1) ) {
   		FEEDBACK_TOKEN( (GLfloat) (GLint) GL_LINE_TOKEN );
		transform_and_feedback_vertex( f->vertex, f->texcoord );
		transform_and_feedback_vertex( f->startv, f->startt );
	}

	if ((c->mode == GL_TRIANGLES ||
	     c->mode == GL_TRIANGLE_STRIP ||
	     c->mode == GL_TRIANGLE_FAN ||
	     c->mode == GL_QUADS ||
	     c->mode == GL_QUAD_STRIP ||
	     c->mode == GL_POLYGON) &&
	    p->frontMode == GL_FILL)
	   	f->buffer[f->polygoni] = (GLfloat) f->starti;
}

void STATE_APIENTRY
crStateFeedbackDrawPixels( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels )
{
	CRContext *g = GetCurrentContext();
	CRFeedbackState *f = &(g->feedback);

   	FEEDBACK_TOKEN( (GLfloat) (GLint) GL_DRAW_PIXEL_TOKEN );

	transform_and_feedback_vertex( f->rasterpos, f->texcoord );
}

void STATE_APIENTRY
crStateFeedbackCopyPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum type )
{
	CRContext *g = GetCurrentContext();
	CRFeedbackState *f = &(g->feedback);

   	FEEDBACK_TOKEN( (GLfloat) (GLint) GL_COPY_PIXEL_TOKEN );

	transform_and_feedback_vertex( f->rasterpos, f->texcoord );
}

void STATE_APIENTRY
crStateFeedbackBitmap( GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap )
{
	CRContext *g = GetCurrentContext();
	CRFeedbackState *f = &(g->feedback);
	GLvectorf v;

   	FEEDBACK_TOKEN( (GLfloat) (GLint) GL_BITMAP_TOKEN );

	v.x = f->rasterpos.x - xorig;
	v.y = f->rasterpos.y - yorig;
	v.z = f->rasterpos.z;
	v.w = f->rasterpos.w;

	transform_and_feedback_vertex( v, f->texcoord );
}


void STATE_APIENTRY
crStateFeedbackTexCoord4f( GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 )
{
	CRContext *g = GetCurrentContext();
	CRFeedbackState *f = &(g->feedback);
	CRTransformState *t = &(g->transform);

	/* Save old texcoords for LINE_LOOP's */
	f->oldtexcoord.x = f->texcoord.x;
	f->oldtexcoord.y = f->texcoord.y;
	f->oldtexcoord.z = f->texcoord.z;
	f->oldtexcoord.w = f->texcoord.w;

	f->texcoord.x = v0;
	f->texcoord.y = v1;
	f->texcoord.z = v2;
	f->texcoord.w = v3;

	/* Feedback only operates on the first texture unit */
	TRANSFORM_POINT( f->texcoord, t->texture[0][t->textureDepth[0]], f->texcoord );

	/* Save first texcoord incase of LINE_LOOP */
	if (f->starti == 0) {
		f->startt.x = v0;
		f->startt.y = v1;
		f->startt.z = v2;
		f->startt.w = v3;
	}
}
 
void STATE_APIENTRY
crStateFeedbackTexCoord4fv( const GLfloat *v )
{
	crStateFeedbackTexCoord4f( v[0], v[1], v[2], v[3] );
}

void STATE_APIENTRY
crStateFeedbackTexCoord4s( GLshort v0, GLshort v1, GLshort v2, GLshort v3 )
{
	crStateFeedbackTexCoord4f( (GLfloat)v0, (GLfloat)v1, (GLfloat)v2, (GLfloat)v3 );
}

void STATE_APIENTRY
crStateFeedbackTexCoord4sv( const GLshort *v )
{
	crStateFeedbackTexCoord4f( (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3] );
}

void STATE_APIENTRY
crStateFeedbackTexCoord4i( GLint v0, GLint v1, GLint v2, GLint v3 )
{
	crStateFeedbackTexCoord4f( (GLfloat)v0, (GLfloat)v1, (GLfloat)v2, (GLfloat)v3 );
}

void STATE_APIENTRY
crStateFeedbackTexCoord4iv( const GLint *v )
{
	crStateFeedbackTexCoord4f( (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3] );
}

void STATE_APIENTRY
crStateFeedbackTexCoord4d( GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3 )
{
	crStateFeedbackTexCoord4f( (GLfloat)v0, (GLfloat)v1, (GLfloat)v2, (GLfloat)v3 );
}

void STATE_APIENTRY
crStateFeedbackTexCoord4dv( const GLdouble *v )
{
	crStateFeedbackTexCoord4f( (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3] );
}

void STATE_APIENTRY
crStateFeedbackTexCoord1i( GLint v0 )
{
	crStateFeedbackTexCoord4f( (GLfloat)v0, 0.0f, 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackTexCoord1iv( const GLint *v )
{
	crStateFeedbackTexCoord4f( (GLfloat)v[0], 0.0f, 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackTexCoord1s( GLshort v0 )
{
	crStateFeedbackTexCoord4f( (GLfloat)v0, 0.0f, 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackTexCoord1sv( const GLshort *v )
{
	crStateFeedbackTexCoord4f( (GLfloat)v[0], 0.0f, 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackTexCoord1f( GLfloat v0 )
{
	crStateFeedbackTexCoord4f( v0, 0.0f, 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackTexCoord1fv( const GLfloat *v )
{
	crStateFeedbackTexCoord4f( v[0], 0.0f, 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackTexCoord1d( GLdouble v0 )
{
	crStateFeedbackTexCoord4f( (GLfloat)v0, 0.0f, 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackTexCoord1dv( const GLdouble *v )
{
	crStateFeedbackTexCoord4f( (GLfloat)v[0], 0.0f, 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackTexCoord2i( GLint v0, GLint v1 )
{
	crStateFeedbackTexCoord4f( (GLfloat)v0, (GLfloat)v1, 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackTexCoord2iv( const GLint *v )
{
	crStateFeedbackTexCoord4f( (GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackTexCoord2s( GLshort v0, GLshort v1 )
{
	crStateFeedbackTexCoord4f( (GLfloat)v0, (GLfloat)v1, 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackTexCoord2sv( const GLshort *v )
{
	crStateFeedbackTexCoord4f( (GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackTexCoord2f( GLfloat v0, GLfloat v1 )
{
	crStateFeedbackTexCoord4f( v0, v1, 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackTexCoord2fv( const GLfloat *v )
{
	crStateFeedbackTexCoord4f( v[0], v[1], 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackTexCoord2d( GLdouble v0, GLdouble v1 )
{
	crStateFeedbackTexCoord4f( (GLfloat)v0, (GLfloat)v1, 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackTexCoord2dv( const GLdouble *v )
{
	crStateFeedbackTexCoord4f( (GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackTexCoord3i( GLint v0, GLint v1, GLint v2 )
{
	crStateFeedbackTexCoord4f( (GLfloat)v0, (GLfloat)v1, (GLfloat)v2, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackTexCoord3iv( const GLint *v )
{
	crStateFeedbackTexCoord4f( (GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackTexCoord3s( GLshort v0, GLshort v1, GLshort v2 )
{
	crStateFeedbackTexCoord4f( (GLfloat)v0, (GLfloat)v1, (GLfloat)v2, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackTexCoord3sv( const GLshort *v )
{
	crStateFeedbackTexCoord4f( (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], 1.0f );
}

void STATE_APIENTRY
crStateFeedbackTexCoord3f( GLfloat v0, GLfloat v1, GLfloat v2 )
{
	crStateFeedbackTexCoord4f( v0, v1, v2, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackTexCoord3fv( const GLfloat *v )
{
	crStateFeedbackTexCoord4f( v[0], v[1], v[2], 1.0f );
}

void STATE_APIENTRY
crStateFeedbackTexCoord3d( GLdouble v0, GLdouble v1, GLdouble v2 )
{
	crStateFeedbackTexCoord4f( (GLfloat)v0, (GLfloat)v1, (GLfloat)v2, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackTexCoord3dv( const GLdouble *v )
{
	crStateFeedbackTexCoord4f( (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], 1.0f );
}


void STATE_APIENTRY
crStateFeedbackVertex4f( GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 )
{
	CRContext *g = GetCurrentContext();
	CRCurrentState *c = &(g->current);
	CRPolygonState *p = &(g->polygon);
	CRFeedbackState *f = &(g->feedback);
	
	if ((c->mode == GL_TRIANGLES ||
	     c->mode == GL_TRIANGLE_STRIP ||
	     c->mode == GL_TRIANGLE_FAN ||
	     c->mode == GL_QUADS ||
	     c->mode == GL_QUAD_STRIP ||
	     c->mode == GL_POLYGON) &&
	    p->frontMode == GL_POINT) {
   		FEEDBACK_TOKEN( (GLfloat) (GLint) GL_POINT_TOKEN );
	} else
	if ((c->mode == GL_TRIANGLES ||
	     c->mode == GL_TRIANGLE_STRIP ||
	     c->mode == GL_TRIANGLE_FAN ||
	     c->mode == GL_QUADS ||
	     c->mode == GL_QUAD_STRIP ||
	     c->mode == GL_POLYGON) &&
	    p->frontMode == GL_LINE && (f->starti > 1) ) {
   		FEEDBACK_TOKEN( (GLfloat) (GLint) GL_LINE_TOKEN );
		transform_and_feedback_vertex( f->vertex, f->oldtexcoord );
	}

	f->vertex.x = v0;
	f->vertex.y = v1;
	f->vertex.z = v2;
	f->vertex.w = v3;

	transform_and_feedback_vertex( f->vertex, f->texcoord );

	/* Save first vertex incase of LINE_LOOP */
	if (f->starti == 0) {
		f->startv.x = f->vertex.x;
		f->startv.y = f->vertex.y;
		f->startv.z = f->vertex.z;
		f->startv.w = f->vertex.w;
	}

	f->starti++;
}

void STATE_APIENTRY
crStateFeedbackVertex4fv( const GLfloat *v )
{
	crStateFeedbackVertex4f( v[0], v[1], v[2], v[3] );
}

void STATE_APIENTRY
crStateFeedbackVertex4s( GLshort v0, GLshort v1, GLshort v2, GLshort v3 )
{
	crStateFeedbackVertex4f( (GLfloat)v0, (GLfloat)v1, (GLfloat)v2, (GLfloat)v3 );
}

void STATE_APIENTRY
crStateFeedbackVertex4sv( const GLshort *v )
{
	crStateFeedbackVertex4f( (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3] );
}

void STATE_APIENTRY
crStateFeedbackVertex4i( GLint v0, GLint v1, GLint v2, GLint v3 )
{
	crStateFeedbackVertex4f( (GLfloat)v0, (GLfloat)v1, (GLfloat)v2, (GLfloat)v3 );
}

void STATE_APIENTRY
crStateFeedbackVertex4iv( const GLint *v )
{
	crStateFeedbackVertex4f( (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3] );
}

void STATE_APIENTRY
crStateFeedbackVertex4d( GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3 )
{
	crStateFeedbackVertex4f( (GLfloat)v0, (GLfloat)v1, (GLfloat)v2, (GLfloat)v3 );
}

void STATE_APIENTRY
crStateFeedbackVertex4dv( const GLdouble *v )
{
	crStateFeedbackVertex4f( (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3] );
}

void STATE_APIENTRY
crStateFeedbackVertex2i( GLint v0, GLint v1 )
{
	crStateFeedbackVertex4f( (GLfloat)v0, (GLfloat)v1, 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackVertex2iv( const GLint *v )
{
	crStateFeedbackVertex4f( (GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackVertex2s( GLshort v0, GLshort v1 )
{
	crStateFeedbackVertex4f( (GLfloat)v0, (GLfloat)v1, 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackVertex2sv( const GLshort *v )
{
	crStateFeedbackVertex4f( (GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackVertex2f( GLfloat v0, GLfloat v1 )
{
	crStateFeedbackVertex4f( v0, v1, 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackVertex2fv( const GLfloat *v )
{
	crStateFeedbackVertex4f( v[0], v[1], 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackVertex2d( GLdouble v0, GLdouble v1 )
{
	crStateFeedbackVertex4f( (GLfloat)v0, (GLfloat)v1, 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackVertex2dv( const GLdouble *v )
{
	crStateFeedbackVertex4f( (GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackVertex3i( GLint v0, GLint v1, GLint v2 )
{
	crStateFeedbackVertex4f( (GLfloat)v0, (GLfloat)v1, (GLfloat)v2, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackVertex3iv( const GLint *v )
{
	crStateFeedbackVertex4f( (GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackVertex3s( GLshort v0, GLshort v1, GLshort v2 )
{
	crStateFeedbackVertex4f( (GLfloat)v0, (GLfloat)v1, (GLfloat)v2, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackVertex3sv( const GLshort *v )
{
	crStateFeedbackVertex4f( (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], 1.0f );
}

void STATE_APIENTRY
crStateFeedbackVertex3f( GLfloat v0, GLfloat v1, GLfloat v2 )
{
	crStateFeedbackVertex4f( v0, v1, v2, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackVertex3fv( const GLfloat *v )
{
	crStateFeedbackVertex4f( v[0], v[1], v[2], 1.0f );
}

void STATE_APIENTRY
crStateFeedbackVertex3d( GLdouble v0, GLdouble v1, GLdouble v2 )
{
	crStateFeedbackVertex4f( (GLfloat)v0, (GLfloat)v1, (GLfloat)v2, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackVertex3dv( const GLdouble *v )
{
	crStateFeedbackVertex4f( (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], 1.0f );
}

void STATE_APIENTRY
crStateFeedbackRasterPos4f( GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 )
{
	CRContext *g = GetCurrentContext();
	CRFeedbackState *f = &(g->feedback);

	f->rasterpos.x = v0;
	f->rasterpos.y = v1;
	f->rasterpos.z = v2;
	f->rasterpos.w = v3;
}

void STATE_APIENTRY
crStateFeedbackRasterPos4fv( const GLfloat *v )
{
	crStateFeedbackRasterPos4f( v[0], v[1], v[2], v[3] );
}

void STATE_APIENTRY
crStateFeedbackRasterPos4s( GLshort v0, GLshort v1, GLshort v2, GLshort v3 )
{
	crStateFeedbackRasterPos4f( (GLfloat)v0, (GLfloat)v1, (GLfloat)v2, (GLfloat)v3 );
}

void STATE_APIENTRY
crStateFeedbackRasterPos4sv( const GLshort *v )
{
	crStateFeedbackRasterPos4f( (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3] );
}

void STATE_APIENTRY
crStateFeedbackRasterPos4i( GLint v0, GLint v1, GLint v2, GLint v3 )
{
	crStateFeedbackRasterPos4f( (GLfloat)v0, (GLfloat)v1, (GLfloat)v2, (GLfloat)v3 );
}

void STATE_APIENTRY
crStateFeedbackRasterPos4iv( const GLint *v )
{
	crStateFeedbackRasterPos4f( (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3] );
}

void STATE_APIENTRY
crStateFeedbackRasterPos4d( GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3 )
{
	crStateFeedbackRasterPos4f( (GLfloat)v0, (GLfloat)v1, (GLfloat)v2, (GLfloat)v3 );
}

void STATE_APIENTRY
crStateFeedbackRasterPos4dv( const GLdouble *v )
{
	crStateFeedbackRasterPos4f( (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], (GLfloat)v[3] );
}

void STATE_APIENTRY
crStateFeedbackRasterPos2i( GLint v0, GLint v1 )
{
	crStateFeedbackRasterPos4f( (GLfloat)v0, (GLfloat)v1, 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackRasterPos2iv( const GLint *v )
{
	crStateFeedbackRasterPos4f( (GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackRasterPos2s( GLshort v0, GLshort v1 )
{
	crStateFeedbackRasterPos4f( (GLfloat)v0, (GLfloat)v1, 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackRasterPos2sv( const GLshort *v )
{
	crStateFeedbackRasterPos4f( (GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackRasterPos2f( GLfloat v0, GLfloat v1 )
{
	crStateFeedbackRasterPos4f( v0, v1, 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackRasterPos2fv( const GLfloat *v )
{
	crStateFeedbackRasterPos4f( v[0], v[1], 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackRasterPos2d( GLdouble v0, GLdouble v1 )
{
	crStateFeedbackRasterPos4f( (GLfloat)v0, (GLfloat)v1, 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackRasterPos2dv( const GLdouble *v )
{
	crStateFeedbackRasterPos4f( (GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackRasterPos3i( GLint v0, GLint v1, GLint v2 )
{
	crStateFeedbackRasterPos4f( (GLfloat)v0, (GLfloat)v1, (GLfloat)v2, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackRasterPos3iv( const GLint *v )
{
	crStateFeedbackRasterPos4f( (GLfloat)v[0], (GLfloat)v[1], 0.0f, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackRasterPos3s( GLshort v0, GLshort v1, GLshort v2 )
{
	crStateFeedbackRasterPos4f( (GLfloat)v0, (GLfloat)v1, (GLfloat)v2, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackRasterPos3sv( const GLshort *v )
{
	crStateFeedbackRasterPos4f( (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], 1.0f );
}

void STATE_APIENTRY
crStateFeedbackRasterPos3f( GLfloat v0, GLfloat v1, GLfloat v2 )
{
	crStateFeedbackRasterPos4f( v0, v1, v2, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackRasterPos3fv( const GLfloat *v )
{
	crStateFeedbackRasterPos4f( v[0], v[1], v[2], 1.0f );
}

void STATE_APIENTRY
crStateFeedbackRasterPos3d( GLdouble v0, GLdouble v1, GLdouble v2 )
{
	crStateFeedbackRasterPos4f( (GLfloat)v0, (GLfloat)v1, (GLfloat)v2, 1.0f );
}

void STATE_APIENTRY
crStateFeedbackRasterPos3dv( const GLdouble *v )
{
	crStateFeedbackRasterPos4f( (GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2], 1.0f );
}



void transform_and_select_vertex( GLvectorf vertex )
{
	CRContext *g = GetCurrentContext();
	CRViewportState *v = &(g->viewport);
	CRTransformState *t = &(g->transform);
	GLvectorf eye;
	GLvectorf clip;

	TRANSFORM_POINT( eye, t->modelView[t->modelViewDepth], vertex );
	TRANSFORM_POINT( clip, t->projection[t->projectionDepth], eye );

   	/*
    	 * Combined clip testing with clip-to-window coordinate mapping.
    	 */
   	{
		/* XXX FIXME, tx, ty - rasteroffsetX,Y */
      		GLfloat sx = (GLfloat)v->viewportW / 2, tx = sx;
      		GLfloat sy = (GLfloat)v->viewportH / 2, ty = sy;
      		GLfloat sz = (GLfloat)(v->farClip - v->nearClip) / 2.0f, tz = sz + (GLfloat)v->nearClip;
      
		if (clip.x > clip.w || clip.x < -clip.w ||
             	    clip.y > clip.w || clip.y < -clip.w ||
             	    clip.z > clip.w || clip.z < -clip.w ) 
		{
            		/* vertex is clipped */
#if 0
			printf("VERTEX CLIPPED\n");
#endif
      		}
      		else 
		{
            		/* vertex not clipped */
            		GLfloat d = 1.0F / clip.w;
            		vertex.x = clip.x * d * sx + tx;
            		vertex.y = clip.y * d * sy + ty;
            		vertex.z = clip.z * d * sz + tz;
      		
			_update_hitflag( vertex.z );
      		}
   	}

}

void STATE_APIENTRY
crStateSelectRasterPos2d( GLdouble x, GLdouble y )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) x;
	vertex.y = (GLfloat) y;
	vertex.z = (GLfloat) 0.0f;
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectRasterPos2dv( const GLdouble *v )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) v[0];
	vertex.y = (GLfloat) v[1];
	vertex.z = (GLfloat) 0.0f;
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectRasterPos2f( GLfloat x, GLfloat y )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) x;
	vertex.y = (GLfloat) y;
	vertex.z = (GLfloat) 0.0f;
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectRasterPos2fv( const GLfloat *v )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) v[0];
	vertex.y = (GLfloat) v[1];
	vertex.z = (GLfloat) 0.0f;
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectRasterPos2i( GLint x, GLint y )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) x;
	vertex.y = (GLfloat) y;
	vertex.z = (GLfloat) 0.0f;
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectRasterPos2iv( const GLint *v )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) v[0];
	vertex.y = (GLfloat) v[1];
	vertex.z = (GLfloat) 0.0f;
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectRasterPos2s( GLshort x, GLshort y )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) x;
	vertex.y = (GLfloat) y;
	vertex.z = (GLfloat) 0.0f;
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectRasterPos2sv( const GLshort *v )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) v[0];
	vertex.y = (GLfloat) v[1];
	vertex.z = (GLfloat) 0.0f;
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectRasterPos3d( GLdouble x, GLdouble y, GLdouble z )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) x;
	vertex.y = (GLfloat) y;
	vertex.z = (GLfloat) z;
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectRasterPos3dv( const GLdouble *v )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) v[0];
	vertex.y = (GLfloat) v[1];
	vertex.z = (GLfloat) v[2];
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectRasterPos3f( GLfloat x, GLfloat y, GLfloat z )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) x;
	vertex.y = (GLfloat) y;
	vertex.z = (GLfloat) z;
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectRasterPos3fv( const GLfloat *v )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) v[0];
	vertex.y = (GLfloat) v[1];
	vertex.z = (GLfloat) v[2];
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectRasterPos3i( GLint x, GLint y, GLint z )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) x;
	vertex.y = (GLfloat) y;
	vertex.z = (GLfloat) z;
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectRasterPos3iv( const GLint *v )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) v[0];
	vertex.y = (GLfloat) v[1];
	vertex.z = (GLfloat) v[2];
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectRasterPos3s( GLshort x, GLshort y, GLshort z )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) x;
	vertex.y = (GLfloat) y;
	vertex.z = (GLfloat) z;
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectRasterPos3sv( const GLshort *v )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) v[0];
	vertex.y = (GLfloat) v[1];
	vertex.z = (GLfloat) v[2];
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectRasterPos4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) x;
	vertex.y = (GLfloat) y;
	vertex.z = (GLfloat) z;
	vertex.w = (GLfloat) w;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectRasterPos4dv( const GLdouble *v )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) v[0];
	vertex.y = (GLfloat) v[1];
	vertex.z = (GLfloat) v[2];
	vertex.w = (GLfloat) v[3];

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectRasterPos4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) x;
	vertex.y = (GLfloat) y;
	vertex.z = (GLfloat) z;
	vertex.w = (GLfloat) w;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectRasterPos4fv( const GLfloat *v )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) v[0];
	vertex.y = (GLfloat) v[1];
	vertex.z = (GLfloat) v[2];
	vertex.w = (GLfloat) v[3];

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectRasterPos4i( GLint x, GLint y, GLint z, GLint w )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) x;
	vertex.y = (GLfloat) y;
	vertex.z = (GLfloat) z;
	vertex.w = (GLfloat) w;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectRasterPos4iv( const GLint *v )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) v[0];
	vertex.y = (GLfloat) v[1];
	vertex.z = (GLfloat) v[2];
	vertex.w = (GLfloat) v[3];

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectRasterPos4s( GLshort x, GLshort y, GLshort z, GLshort w )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) x;
	vertex.y = (GLfloat) y;
	vertex.z = (GLfloat) z;
	vertex.w = (GLfloat) w;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectRasterPos4sv( const GLshort *v )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) v[0];
	vertex.y = (GLfloat) v[1];
	vertex.z = (GLfloat) v[2];
	vertex.w = (GLfloat) v[3];

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectVertex2d( GLdouble x, GLdouble y )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) x;
	vertex.y = (GLfloat) y;
	vertex.z = (GLfloat) 0.0f;
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectVertex2dv( const GLdouble *v )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) v[0];
	vertex.y = (GLfloat) v[1];
	vertex.z = (GLfloat) 0.0f;
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectVertex2f( GLfloat x, GLfloat y )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) x;
	vertex.y = (GLfloat) y;
	vertex.z = (GLfloat) 0.0f;
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectVertex2fv( const GLfloat *v )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) v[0];
	vertex.y = (GLfloat) v[1];
	vertex.z = (GLfloat) 0.0f;
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectVertex2i( GLint x, GLint y )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) x;
	vertex.y = (GLfloat) y;
	vertex.z = (GLfloat) 0.0f;
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectVertex2iv( const GLint *v )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) v[0];
	vertex.y = (GLfloat) v[1];
	vertex.z = (GLfloat) 0.0f;
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectVertex2s( GLshort x, GLshort y )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) x;
	vertex.y = (GLfloat) y;
	vertex.z = (GLfloat) 0.0f;
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectVertex2sv( const GLshort *v )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) v[0];
	vertex.y = (GLfloat) v[1];
	vertex.z = (GLfloat) 0.0f;
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectVertex3d( GLdouble x, GLdouble y, GLdouble z )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) x;
	vertex.y = (GLfloat) y;
	vertex.z = (GLfloat) z;
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectVertex3dv( const GLdouble *v )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) v[0];
	vertex.y = (GLfloat) v[1];
	vertex.z = (GLfloat) v[2];
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectVertex3f( GLfloat x, GLfloat y, GLfloat z )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) x;
	vertex.y = (GLfloat) y;
	vertex.z = (GLfloat) z;
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectVertex3fv( const GLfloat *v )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) v[0];
	vertex.y = (GLfloat) v[1];
	vertex.z = (GLfloat) v[2];
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectVertex3i( GLint x, GLint y, GLint z )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) x;
	vertex.y = (GLfloat) y;
	vertex.z = (GLfloat) z;
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectVertex3iv( const GLint *v )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) v[0];
	vertex.y = (GLfloat) v[1];
	vertex.z = (GLfloat) v[2];
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectVertex3s( GLshort x, GLshort y, GLshort z )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) x;
	vertex.y = (GLfloat) y;
	vertex.z = (GLfloat) z;
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectVertex3sv( const GLshort *v )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) v[0];
	vertex.y = (GLfloat) v[1];
	vertex.z = (GLfloat) v[2];
	vertex.w = (GLfloat) 1.0f;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectVertex4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) x;
	vertex.y = (GLfloat) y;
	vertex.z = (GLfloat) z;
	vertex.w = (GLfloat) w;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectVertex4dv( const GLdouble *v )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) v[0];
	vertex.y = (GLfloat) v[1];
	vertex.z = (GLfloat) v[2];
	vertex.w = (GLfloat) v[3];

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectVertex4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) x;
	vertex.y = (GLfloat) y;
	vertex.z = (GLfloat) z;
	vertex.w = (GLfloat) w;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectVertex4fv( const GLfloat *v )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) v[0];
	vertex.y = (GLfloat) v[1];
	vertex.z = (GLfloat) v[2];
	vertex.w = (GLfloat) v[3];

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectVertex4i( GLint x, GLint y, GLint z, GLint w )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) x;
	vertex.y = (GLfloat) y;
	vertex.z = (GLfloat) z;
	vertex.w = (GLfloat) w;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectVertex4iv( const GLint *v )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) v[0];
	vertex.y = (GLfloat) v[1];
	vertex.z = (GLfloat) v[2];
	vertex.w = (GLfloat) v[3];

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectVertex4s( GLshort x, GLshort y, GLshort z, GLshort w )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) x;
	vertex.y = (GLfloat) y;
	vertex.z = (GLfloat) z;
	vertex.w = (GLfloat) w;

	transform_and_select_vertex( vertex );
}

void STATE_APIENTRY
crStateSelectVertex4sv( const GLshort *v )
{
	GLvectorf vertex;

	vertex.x = (GLfloat) v[0];
	vertex.y = (GLfloat) v[1];
	vertex.z = (GLfloat) v[2];
	vertex.w = (GLfloat) v[3];

	transform_and_select_vertex( vertex );
}
