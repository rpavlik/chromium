/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "tilesortspu.h"
#include "cr_error.h"
#include "cr_packfunctions.h"
#include "cr_glstate.h"
#include "cr_mem.h"

#define vmin(a,b) ((a)<(b)?(a):(b))

/* Ok.  So these might be major hacks 
** but the alternative is a huge switch statement
** and our protocol generator sorts the opcodes,
** so why not just do greater than less than?
*/
#define IS_VERTEX(a)	((a) >= CR_VERTEX2D_OPCODE && (a) <= CR_VERTEX4S_OPCODE)
#define IS_COLOR(a)		((a) >= CR_COLOR3B_OPCODE && (a) <= CR_COLOR4US_OPCODE)
#define IS_NORMAL(a)	((a) >= CR_NORMAL3B_OPCODE && (a) <= CR_NORMAL3S_OPCODE)
#define IS_INDEX(a)		((a) >= CR_INDEXD_OPCODE && (a) <= CR_INDEXS_OPCODE)
#define IS_TEXCOORD(a)	(((a) >= CR_TEXCOORD1D_OPCODE && (a) <= CR_TEXCOORD4S_OPCODE) || (((a) >= CR_MULTITEXCOORD1DARB_OPCODE) && ((a) <= CR_MULTITEXCOORD4SARB_OPCODE)))
#define IS_EDGEFLAG(a)	((a) == CR_EDGEFLAG_OPCODE)

#define ASSERT_BOUNDS(op, data) \
				CRASSERT(op <= thread->packer->buffer.opcode_start); \
				CRASSERT(op > thread->packer->buffer.opcode_end); \
				CRASSERT(data >= thread->packer->buffer.data_start); \
				CRASSERT(data < thread->packer->buffer.data_end)

static const GLvectorf vdefault = {0.0f, 0.0f, 0.0f, 1.0f};

void tilesortspuPinch (void) 
{
	GET_CONTEXT(ctx);
	CRCurrentState *c = &(ctx->current);
	int vtx_count = c->current->vtx_count - c->current->vtx_count_begin;
	int numRestore;
	int loop = 0;
	int wind = 0;
	unsigned int i, j;
	unsigned char * op;
	unsigned char * data;
	unsigned char * vtx_op;
	unsigned char * vtx_data;

	unsigned char *color_ptr = ctx->current.current->color.ptr;
	unsigned char *normal_ptr = ctx->current.current->normal.ptr;
	unsigned char *texCoord_ptr[CR_MAX_TEXTURE_UNITS];
	unsigned char *edgeFlag_ptr = ctx->current.current->edgeFlag.ptr;
	/* unsigned char *index_ptr = ctx->current.current->index.ptr; */
	CRVertex v_current;

	v_current.pos = vdefault;
	v_current.color = c->color;
	v_current.normal = c->normal;
	for (i = 0 ; i < ctx->limits.maxTextureUnits ; i++)
	{
		texCoord_ptr[i] = ctx->current.current->texCoord.ptr[i];
		v_current.texCoord[i] = c->texCoord[i];
	}
	v_current.edgeFlag = c->edgeFlag;
	v_current.index = c->index;

	/* First lets figure out how many vertices
	 * we need to recover.  Note to self --
	 * "vertexes" is NOT A WORD.
	 */
	if (!c->inBeginEnd || vtx_count == 0) 
	{
		thread->pinchState.numRestore = 0;
		return;
	}
	switch (c->mode) 
	{
		case GL_POINTS:
			numRestore = 0;
			break;
		case GL_LINES:
			numRestore = vtx_count % 2;
			break;
		case GL_LINE_STRIP:
			/* A seperate flag is used to inidcate
			 ** that this is really a line loop that 
			 ** we're sending down as a line strip.
			 ** If so, we need to recover the first 
			 ** vertex and the last one.
			 */
			if (thread->pinchState.isLoop) 
			{
				numRestore = 2;
				loop = 1;
				break;
			}
			numRestore = 1;
			break;
		case GL_LINE_LOOP:
			numRestore = vmin(vtx_count,2);
			/* Only convert this into a 
			 ** line strip if we've issued 
			 ** more than two vertices.
			 */
			loop = vtx_count>2?1:0;
			break;
		case GL_TRIANGLES:
			numRestore = vtx_count % 3;
			break;
		case GL_TRIANGLE_STRIP:
			numRestore = vmin(vtx_count,2);
			/* Check the winding. If we're issuing 
			 ** the odd vertex, the winding of the 
			 ** triangle changes. We need to record
			 ** this for restoring the strip.
			 */
			wind = (vtx_count > 2)?vtx_count%2:0;
			break;
			/* Note to self: GL_TRIANGLE_FAN <=> GL_POLYGON 
			 ** if you ignore the vertex params
			 */
		case GL_TRIANGLE_FAN:
		case GL_POLYGON:
			numRestore = vmin(vtx_count,2);
			loop = 1;
			break;
		case GL_QUADS:
			numRestore = vtx_count % 4;
			break;
		case GL_QUAD_STRIP:
			/* Don't care about winding here 
			 ** since quads consist of even 
			 ** num of triangles.
			 */
			if (vtx_count < 4)
				numRestore = vtx_count;
			else 
				numRestore = 2 + (vtx_count%2);
			break;
		default:
			crError( "Unkown mode: %d", c->mode );
			return;
	}

	vtx_op = thread->packer->buffer.opcode_current;
	vtx_data = thread->packer->buffer.data_current;

	for (i = numRestore; i != 0 ; i--) 
	{
		CRVertex *vtx = thread->pinchState.vtx + i - 1;

		op = vtx_op;
		data = vtx_data;

		/* If we're restoring a line strip
		 ** that is really a line loop, the 
		 ** first vertex is still sitting in
		 ** the first vtx.  Lets quit early
		 ** and preserve that value.  It will
		 ** be issued at glEnd.
		 */
		if (i==1 && thread->pinchState.isLoop) 
		{
			break;
		}

		/* Search for a vertex command */

		/* If we're dealing with a loop (polygon, fan, lineloop),
		 ** the first vertex comes is found by searching forward
		 ** from the glBegin.  The other vertices come from the end
		 ** moving backward.  Note we fill the restored vtx list
		 ** starting from the end.
		 */
		if ( i==1 && loop )
		{
			op = thread->pinchState.beginOp;
			data = thread->pinchState.beginData;

			/* Perform the line loop to line strip 
			 ** conversion. Set the isloop flag so 
			 ** we don't forget 
			 */
			if (c->mode == GL_LINE_LOOP) 
			{
				thread->pinchState.isLoop = GL_TRUE;
				c->mode = GL_LINE_STRIP;
				*((GLenum *)data) = GL_LINE_STRIP;
			}

			do 
			{
				data += __cr_packet_length_table[*op]; /* generated */
				op--;
				ASSERT_BOUNDS(op, data);
			} while (!IS_VERTEX(*op));
		} 
		else 
		{ 
			do 
			{
				op++;
				data -= __cr_packet_length_table[*op];
				ASSERT_BOUNDS(op, data);
			} while (!IS_VERTEX(*op));
		}


		/* Found a vertex */
		vtx_op = op;
		vtx_data = data;

		/* Lets search for the parameters */
		/* The following code should be auto-generated but oh well... */
		if (vtx_op < thread->packer->buffer.opcode_start) 
		{
			/* Is the color pointer after my vertex? */
			if (color_ptr > vtx_data) 
			{

				/* Perform the search */
				op = vtx_op+1;
				data = vtx_data - __cr_packet_length_table[*(vtx_op+1)];
				while (op <= thread->packer->buffer.opcode_start && !IS_COLOR(*op)) 
				{
					op++;
					data -= __cr_packet_length_table[*op];
				}

				/* Did I hit the begining of the buffer? */
				if (op > thread->packer->buffer.opcode_start) 
				{
					v_current.color = c->colorPre;
					color_ptr = NULL;
				} 
				else 
				{
					ASSERT_BOUNDS (op, data);
					VPINCH_CONVERT_COLOR (*op, data, v_current.color);
					color_ptr = data;
				}
			}

			/* Is the normal pointer after my vertex? */
			if (normal_ptr > vtx_data) 
			{
				/* Perform the search */
				op = vtx_op+1;
				data = vtx_data - __cr_packet_length_table[*(vtx_op+1)];
				while (op <= thread->packer->buffer.opcode_start && !IS_NORMAL(*op)) 
				{
					op++;
					data -= __cr_packet_length_table[*op];
				}

				/* Did I hit the begining of the buffer? */
				if (op > thread->packer->buffer.opcode_start) 
				{
					v_current.normal = c->normalPre;
					normal_ptr = NULL;
				} 
				else 
				{
					ASSERT_BOUNDS (op, data);
					VPINCH_CONVERT_NORMAL (*op, data, v_current.normal)
						normal_ptr = data;
				}
			}

			/* Is the texture pointer after my vertex? */
			for (j = 0 ; j < ctx->limits.maxTextureUnits ; j++)
			{
				if (texCoord_ptr[j] > vtx_data) 
				{

					/* Perform the search */
					op = vtx_op+1;
					data = vtx_data - __cr_packet_length_table[*(vtx_op+1)];
					while (op <= thread->packer->buffer.opcode_start && !IS_TEXCOORD(*op)) 
					{
						op++;
						data -= __cr_packet_length_table[*op];
					}

					/* Did I hit the begining of the buffer? */
					if (op > thread->packer->buffer.opcode_start) 
					{
						v_current.texCoord[j] = c->texCoordPre[j];
						texCoord_ptr[j] = NULL;
					} 
					else 
					{
						ASSERT_BOUNDS (op, data);
						VPINCH_CONVERT_TEXCOORD (*op, data, v_current.texCoord[j])
							texCoord_ptr[j] = data;
					}
				}
			}

			/* Is the stupid edgeFlag pointer after my vertex? */
			if (edgeFlag_ptr > vtx_data) 
			{

				/* Perform the search */
				op = vtx_op+1;
				data = vtx_data - __cr_packet_length_table[*(vtx_op+1)];
				while (op <= thread->packer->buffer.opcode_start && !IS_EDGEFLAG(*op)) 
				{
					op++;
					data -= __cr_packet_length_table[*op];
				}

				/* Did I hit the begining of the buffer? */
				if (op > thread->packer->buffer.opcode_start) 
				{
					v_current.edgeFlag = c->edgeFlagPre;
					edgeFlag_ptr = NULL;
				} 
				else
				{
					ASSERT_BOUNDS (op, data);
					VPINCH_CONVERT_EDGEFLAG (*op, data, v_current.edgeFlag)
						edgeFlag_ptr = data;
				}
			}
		} 
		else 
		{
			v_current.color = c->colorPre;
			v_current.normal = c->normalPre;
			for (j = 0 ; j < ctx->limits.maxTextureUnits; j++)
			{
				v_current.texCoord[j] = c->texCoordPre[j];
			}
			v_current.edgeFlag = c->edgeFlagPre;
		}

		/* Copy current values */
		*vtx = v_current;

		/* Extract the position */
		switch (*vtx_op) 
		{
			case CR_VERTEX2D_OPCODE:
				__convert_d2(&(vtx->pos.x), (GLdouble *) vtx_data);
				break;
			case CR_VERTEX2F_OPCODE:
				__convert_f2(&(vtx->pos.x), (GLfloat *) vtx_data);
				break;
			case CR_VERTEX2I_OPCODE:
				__convert_i2(&(vtx->pos.x), (GLint *) vtx_data);
				break;
			case CR_VERTEX2S_OPCODE:
				__convert_s2(&(vtx->pos.x), (GLshort *) vtx_data);
				break;
			case CR_VERTEX3D_OPCODE:
				__convert_d3(&(vtx->pos.x), (GLdouble *) vtx_data);
				break;
			case CR_VERTEX3F_OPCODE:
				__convert_f3(&(vtx->pos.x), (GLfloat *) vtx_data);
				break;
			case CR_VERTEX3I_OPCODE:
				__convert_i3(&(vtx->pos.x), (GLint *) vtx_data);
				break;
			case CR_VERTEX3S_OPCODE:
				__convert_s3(&(vtx->pos.x), (GLshort *) vtx_data);
				break;
			case CR_VERTEX4D_OPCODE:
				__convert_d4(&(vtx->pos.x), (GLdouble *) vtx_data);
				break;
			case CR_VERTEX4F_OPCODE:
				__convert_f4(&(vtx->pos.x), (GLfloat *) vtx_data);
				break;
			case CR_VERTEX4I_OPCODE:
				__convert_i4(&(vtx->pos.x), (GLint *) vtx_data);
				break;
			case CR_VERTEX4S_OPCODE:
				__convert_s4(&(vtx->pos.x), (GLshort *) vtx_data);
				break;
			default:
				crError( "Bad pinch opcode: %d", *vtx_op );
				break;
		}
	}

	/* record the number of vtx to restore
	 ** and the winding info before we quit 
	 */
	thread->pinchState.numRestore = numRestore;
	thread->pinchState.wind = wind;
}

void __pinchIssueParams (CRVertex *vtx) 
{
	GET_CONTEXT(ctx);
	GLfloat val[4];
	unsigned int i;

	for (i = 0 ; i < ctx->limits.maxTextureUnits ; i++)
	{
		val[0] = vtx->texCoord[i].s; val[1] = vtx->texCoord[i].t;
		val[2] = vtx->texCoord[i].r; val[3] = vtx->texCoord[i].q;
		if (i == 0)
		{
			if (tilesort_spu.swap)
			{
				crPackTexCoord4fvSWAP( (const GLfloat *) val);
			}
			else
			{
				crPackTexCoord4fv( (const GLfloat *) val);
			}
		}
		else
		{
			if (tilesort_spu.swap)
			{
				crPackMultiTexCoord4fvARBSWAP( i + GL_TEXTURE0_ARB, (const GLfloat *) val);
			}
			else
			{
				crPackMultiTexCoord4fvARB( i + GL_TEXTURE0_ARB, (const GLfloat *) val);
			}
		}
	}
	val[0] = vtx->normal.x; val[1] = vtx->normal.y;
	val[2] = vtx->normal.z;
	if (tilesort_spu.swap)
	{
		crPackNormal3fvSWAP((const GLfloat *) val);
		crPackEdgeFlagSWAP(vtx->edgeFlag);
	}
	else
	{
		crPackNormal3fv((const GLfloat *) val);
		crPackEdgeFlag(vtx->edgeFlag);
	}
	val[0] = vtx->color.r; val[1] = vtx->color.g;
	val[2] = vtx->color.b; val[3] = vtx->color.a;
	if (tilesort_spu.swap)
	{
		crPackColor4fvSWAP((const GLfloat *) val);
	}
	else
	{
		crPackColor4fv((const GLfloat *) val);
	}
}

void __pinchIssueVertex (CRVertex *vtx) 
{
	GLfloat val[4];

	__pinchIssueParams (vtx);

	val[0] = vtx->pos.x; val[1] = vtx->pos.y;
	val[2] = vtx->pos.z; val[3] = vtx->pos.w;
	if (tilesort_spu.swap)
	{
		crPackVertex4fvBBOX_COUNTSWAP((const GLfloat *) val);
	}
	else
	{
		crPackVertex4fvBBOX_COUNT((const GLfloat *) val);
	}
}

/* This function is called at the end of Flush(), when it becomes necessary to 
 * restore the partial triangle that was clipped at the end. */

void tilesortspuPinchRestoreTriangle( void )
{
	GET_CONTEXT(ctx);
	CRCurrentState *c = &(ctx->current);
	int i;
	unsigned int j;
	CRVertex v;

	if (c->inBeginEnd) 
	{
		/*crDebug( "Restoring something..." ); */
		if (tilesort_spu.swap)
		{
			crPackBeginSWAP(c->mode);
		}
		else
		{
			crPackBegin(c->mode);
		}

		/* If the winding flag is set, it means
		 * that the strip was broken on an odd
		 * vertex number.  To fix, we issue the
		 * first vertex twice to resolve the ordering
		 */
		if (thread->pinchState.wind)
		{
			/*crDebug( "Winding..." );*/
			__pinchIssueVertex(thread->pinchState.vtx);
		}

		for (i = thread->pinchState.isLoop ? 1:0; 
				 i < thread->pinchState.numRestore; 
				 i++) 
		{
			/*crDebug( "issuing a vertex..." ); */
			__pinchIssueVertex(thread->pinchState.vtx + i);
		}

		/* Setup values for next vertex */
		for (j = 0 ; j < ctx->limits.maxTextureUnits; j++)
		{
			v.texCoord[j] = c->texCoord[j];
		}
		v.normal   = c->normal;
		v.edgeFlag = c->edgeFlag;
		v.color    = c->color;

		__pinchIssueParams (&v);
	}
}              
