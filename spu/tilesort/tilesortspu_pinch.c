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
#define IS_VERTEX(a)	(((a) >= CR_VERTEX2D_OPCODE && (a) <= CR_VERTEX4S_OPCODE) \
											 || ((a) >= CR_VERTEXATTRIB1DARB_OPCODE && (a) <= CR_VERTEXATTRIB4USVARB_OPCODE))
#define IS_COLOR(a)		((a) >= CR_COLOR3B_OPCODE && (a) <= CR_COLOR4US_OPCODE)
#define IS_NORMAL(a)	((a) >= CR_NORMAL3B_OPCODE && (a) <= CR_NORMAL3S_OPCODE)
#define IS_INDEX(a)		((a) >= CR_INDEXD_OPCODE && (a) <= CR_INDEXS_OPCODE)
#define IS_TEXCOORD(a)	(((a) >= CR_TEXCOORD1D_OPCODE && (a) <= CR_TEXCOORD4S_OPCODE) || (((a) >= CR_MULTITEXCOORD1DARB_OPCODE) && ((a) <= CR_MULTITEXCOORD4SARB_OPCODE)))
#define IS_EDGEFLAG(a)	((a) == CR_EDGEFLAG_OPCODE)
#define IS_VERTEX_ATTRIB(a)	((a) >= CR_VERTEXATTRIB1DARB_OPCODE && (a) <= CR_VERTEXATTRIB4USVARB_OPCODE)

#define ASSERT_BOUNDS(op, data) \
				CRASSERT(op <= thread->packer->buffer.opcode_start); \
				CRASSERT(op > thread->packer->buffer.opcode_end); \
				CRASSERT(data >= thread->packer->buffer.data_start); \
				CRASSERT(data < thread->packer->buffer.data_end); \
				CRASSERT(((data - op + 0x3) & ~0x3) + sizeof(CRMessageOpcodes) <= thread->packer->buffer.mtu)

static const GLvectorf vdefault = {0.0f, 0.0f, 0.0f, 1.0f};

void tilesortspuPinch (void) 
{
	GET_CONTEXT(ctx);
	CRCurrentState *c = &(ctx->current);
	const int vtx_count = c->current->vtx_count - c->current->vtx_count_begin;
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

	/* make sure the table is the correct size. */
	CRASSERT( sizeof(__cr_packet_length_table) / sizeof(int) == CR_EXTEND_OPCODE + 1 );

	/* silence warnings */
	(void) __convert_b1;
	(void) __convert_b2;
	(void) __convert_b3;
	(void) __convert_b4;
	(void) __convert_ui1;
	(void) __convert_ui2;
	(void) __convert_ui3;
	(void) __convert_ui4;
	(void) __convert_l1;
	(void) __convert_l2;
	(void) __convert_l3;
	(void) __convert_l4;
	(void) __convert_us1;
	(void) __convert_us2;
	(void) __convert_us3;
	(void) __convert_us4;
	(void) __convert_ub1;
	(void) __convert_ub2;
	(void) __convert_ub3;
	(void) __convert_ub4;
	(void) __convert_rescale_s1;
	(void) __convert_rescale_s2;
	(void) __convert_rescale_b1;
	(void) __convert_rescale_b2;
	(void) __convert_rescale_ui1;
	(void) __convert_rescale_ui2;
	(void) __convert_rescale_i1;
	(void) __convert_rescale_i2;
	(void) __convert_rescale_us1;
	(void) __convert_rescale_us2;
	(void) __convert_rescale_ub1;
	(void) __convert_rescale_ub2;
	(void) __convert_boolean;
	(void) __convert_Ni1;
	(void) __convert_Ni2;
	(void) __convert_Ni3;
	(void) __convert_Ni4;
	(void) __convert_Nb1;
	(void) __convert_Nb2;
	(void) __convert_Nb3;
	(void) __convert_Nb4;
	(void) __convert_Nus1;
	(void) __convert_Nus2;
	(void) __convert_Nus3;
	(void) __convert_Nus4;
	(void) __convert_Nui1;
	(void) __convert_Nui2;
	(void) __convert_Nui3;
	(void) __convert_Nui4;
	(void) __convert_Ns1;
	(void) __convert_Ns2;
	(void) __convert_Ns3;
	(void) __convert_Ns4;
	(void) __convert_Nub1;
	(void) __convert_Nub2;
	(void) __convert_Nub3;
	(void) __convert_Nub4;
	(void) __convert_rescale_Ni1;
	(void) __convert_rescale_Ni2;
	(void) __convert_rescale_Ni3;
	(void) __convert_rescale_Ni4;
	(void) __convert_rescale_Nb1;
	(void) __convert_rescale_Nb2;
	(void) __convert_rescale_Nb3;
	(void) __convert_rescale_Nb4;
	(void) __convert_rescale_Nus1;
	(void) __convert_rescale_Nus2;
	(void) __convert_rescale_Nus3;
	(void) __convert_rescale_Nus4;
	(void) __convert_rescale_Nui1;
	(void) __convert_rescale_Nui2;
	(void) __convert_rescale_Nui3;
	(void) __convert_rescale_Nui4;
	(void) __convert_rescale_Ns1;
	(void) __convert_rescale_Ns2;
	(void) __convert_rescale_Ns3;
	(void) __convert_rescale_Ns4;
	(void) __convert_rescale_Nub1;
	(void) __convert_rescale_Nub2;
	(void) __convert_rescale_Nub3;
	(void) __convert_rescale_Nub4;

	for (i = 0; i < CR_MAX_VERTEX_ATTRIBS; i++) {
		 COPY_4V(v_current.attrib[i], c->vertexAttrib[i]);
	}
	for (i = 0 ; i < ctx->limits.maxTextureUnits ; i++)
	{
		texCoord_ptr[i] = ctx->current.current->texCoord.ptr[i];
	}
	v_current.edgeFlag = c->edgeFlag;
	v_current.colorIndex = c->colorIndex;

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
			/* A seperate flag is used to indicate
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
				CRASSERT(*op < sizeof(__cr_packet_length_table) / sizeof(int) - 1);
				CRASSERT(__cr_packet_length_table[*op] > 0);
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
				CRASSERT(*op < sizeof(__cr_packet_length_table) / sizeof(int) - 1);
				CRASSERT(__cr_packet_length_table[*op] > 0);
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
					CRASSERT(__cr_packet_length_table[*op] > 0);
					data -= __cr_packet_length_table[*op];
				}

				/* Did I hit the begining of the buffer? */
				if (op > thread->packer->buffer.opcode_start) 
				{
					COPY_4V(v_current.attrib[VERT_ATTRIB_COLOR0], c->vertexAttrib[VERT_ATTRIB_COLOR0]);
					color_ptr = NULL;
				} 
				else 
				{
					ASSERT_BOUNDS (op, data);
					VPINCH_CONVERT_COLOR (*op, data, v_current.attrib[VERT_ATTRIB_COLOR0]);
					color_ptr = data;
				}
			}

			/* Is the normal pointer after my vertex? */
			if (normal_ptr > vtx_data) 
			{
				/* Perform the search */
				op = vtx_op+1;
				CRASSERT(__cr_packet_length_table[*(vtx_op+1)] > 0);
				data = vtx_data - __cr_packet_length_table[*(vtx_op+1)];
				while (op <= thread->packer->buffer.opcode_start && !IS_NORMAL(*op)) 
				{
					op++;
					CRASSERT(__cr_packet_length_table[*op] > 0);
					data -= __cr_packet_length_table[*op];
				}

				/* Did I hit the begining of the buffer? */
				if (op > thread->packer->buffer.opcode_start) 
				{
					COPY_4V(v_current.attrib[VERT_ATTRIB_NORMAL], c->vertexAttrib[VERT_ATTRIB_NORMAL]);
					normal_ptr = NULL;
				} 
				else 
				{
					ASSERT_BOUNDS (op, data);
					VPINCH_CONVERT_NORMAL (*op, data, v_current.attrib[VERT_ATTRIB_NORMAL])
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
					CRASSERT(__cr_packet_length_table[*(vtx_op+1)] > 0);
					data = vtx_data - __cr_packet_length_table[*(vtx_op+1)];
					while (op <= thread->packer->buffer.opcode_start && !IS_TEXCOORD(*op)) 
					{
						op++;
						CRASSERT(__cr_packet_length_table[*op] > 0);
						data -= __cr_packet_length_table[*op];
					}

					/* Did I hit the begining of the buffer? */
					if (op > thread->packer->buffer.opcode_start) 
					{
						COPY_4V(v_current.attrib[VERT_ATTRIB_TEX0 + j], c->vertexAttribPre[VERT_ATTRIB_TEX0 + j]);
						texCoord_ptr[j] = NULL;
					} 
					else 
					{
						ASSERT_BOUNDS (op, data);
						VPINCH_CONVERT_TEXCOORD (*op, data, v_current.attrib[VERT_ATTRIB_TEX0 + j])
							texCoord_ptr[j] = data;
					}
				}
			}

			/* Is the stupid edgeFlag pointer after my vertex? */
			if (edgeFlag_ptr > vtx_data) 
			{

				/* Perform the search */
				op = vtx_op+1;
				CRASSERT(__cr_packet_length_table[*(vtx_op+1)] > 0);
				data = vtx_data - __cr_packet_length_table[*(vtx_op+1)];
				while (op <= thread->packer->buffer.opcode_start && !IS_EDGEFLAG(*op)) 
				{
					op++;
					CRASSERT(__cr_packet_length_table[*op] > 0);
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
					VPINCH_CONVERT_EDGEFLAG (*op, data, &v_current.edgeFlag)
						edgeFlag_ptr = data;
				}
			}

			/* XXX other vertex attribs like fog, secondary color, etc. */

		} 
		else 
		{
			int attr;
			for (attr = 1; attr < CR_MAX_VERTEX_ATTRIBS; attr++) {
				COPY_4V(v_current.attrib[attr], c->vertexAttribPre[attr]);
			}
			v_current.edgeFlag = c->edgeFlagPre;
		}

		/* Copy current values */
		*vtx = v_current;

		/* Extract the position */
		switch (*vtx_op) 
		{
			case CR_VERTEX2D_OPCODE:
				__convert_d2(vtx->attrib[VERT_ATTRIB_POS], (GLdouble *) vtx_data);
				break;
			case CR_VERTEX2F_OPCODE:
				__convert_f2(vtx->attrib[VERT_ATTRIB_POS], (GLfloat *) vtx_data);
				break;
			case CR_VERTEX2I_OPCODE:
				__convert_i2(vtx->attrib[VERT_ATTRIB_POS], (GLint *) vtx_data);
				break;
			case CR_VERTEX2S_OPCODE:
				__convert_s2(vtx->attrib[VERT_ATTRIB_POS], (GLshort *) vtx_data);
				break;
			case CR_VERTEX3D_OPCODE:
				__convert_d3(vtx->attrib[VERT_ATTRIB_POS], (GLdouble *) vtx_data);
				break;
			case CR_VERTEX3F_OPCODE:
				__convert_f3(vtx->attrib[VERT_ATTRIB_POS], (GLfloat *) vtx_data);
				break;
			case CR_VERTEX3I_OPCODE:
				__convert_i3(vtx->attrib[VERT_ATTRIB_POS], (GLint *) vtx_data);
				break;
			case CR_VERTEX3S_OPCODE:
				__convert_s3(vtx->attrib[VERT_ATTRIB_POS], (GLshort *) vtx_data);
				break;
			case CR_VERTEX4D_OPCODE:
				__convert_d4(vtx->attrib[VERT_ATTRIB_POS], (GLdouble *) vtx_data);
				break;
			case CR_VERTEX4F_OPCODE:
				__convert_f4(vtx->attrib[VERT_ATTRIB_POS], (GLfloat *) vtx_data);
				break;
			case CR_VERTEX4I_OPCODE:
				__convert_i4(vtx->attrib[VERT_ATTRIB_POS], (GLint *) vtx_data);
				break;
			case CR_VERTEX4S_OPCODE:
				__convert_s4(vtx->attrib[VERT_ATTRIB_POS], (GLshort *) vtx_data);
				break;
			case CR_VERTEXATTRIB1DARB_OPCODE:
				__convert_d1(vtx->attrib[VERT_ATTRIB_POS], (GLdouble *) vtx_data);
				break;
			case CR_VERTEXATTRIB1FARB_OPCODE:
				__convert_f1(vtx->attrib[VERT_ATTRIB_POS], (GLfloat *) vtx_data);
				break;
			case CR_VERTEXATTRIB1SARB_OPCODE:
				__convert_s1(vtx->attrib[VERT_ATTRIB_POS], (GLshort *) vtx_data);
				break;
			case CR_VERTEXATTRIB2DARB_OPCODE:
				__convert_d2(vtx->attrib[VERT_ATTRIB_POS], (GLdouble *) vtx_data);
				break;
			case CR_VERTEXATTRIB2FARB_OPCODE:
				__convert_f2(vtx->attrib[VERT_ATTRIB_POS], (GLfloat *) vtx_data);
				break;
			case CR_VERTEXATTRIB2SARB_OPCODE:
				__convert_s2(vtx->attrib[VERT_ATTRIB_POS], (GLshort *) vtx_data);
				break;
			case CR_VERTEXATTRIB3DARB_OPCODE:
				__convert_d3(vtx->attrib[VERT_ATTRIB_POS], (GLdouble *) vtx_data);
				break;
			case CR_VERTEXATTRIB3FARB_OPCODE:
				__convert_f3(vtx->attrib[VERT_ATTRIB_POS], (GLfloat *) vtx_data);
				break;
			case CR_VERTEXATTRIB3SARB_OPCODE:
				__convert_s3(vtx->attrib[VERT_ATTRIB_POS], (GLshort *) vtx_data);
				break;
			case CR_VERTEXATTRIB4NBVARB_OPCODE:
				__convert_Nb4(vtx->attrib[VERT_ATTRIB_POS], (GLbyte *) vtx_data);
				break;
			case CR_VERTEXATTRIB4NIVARB_OPCODE:
				__convert_Ni4(vtx->attrib[VERT_ATTRIB_POS], (GLint *) vtx_data);
				break;
			case CR_VERTEXATTRIB4NSVARB_OPCODE:
				__convert_Ns4(vtx->attrib[VERT_ATTRIB_POS], (GLshort *) vtx_data);
				break;
			case CR_VERTEXATTRIB4NUBARB_OPCODE:
				__convert_Nub4(vtx->attrib[VERT_ATTRIB_POS], (GLubyte *) vtx_data);
				break;
			case CR_VERTEXATTRIB4NUBVARB_OPCODE:
				__convert_Nub4(vtx->attrib[VERT_ATTRIB_POS], (GLubyte *) vtx_data);
				break;
			case CR_VERTEXATTRIB4NUIVARB_OPCODE:
				__convert_Nui4(vtx->attrib[VERT_ATTRIB_POS], (GLuint *) vtx_data);
				break;
			case CR_VERTEXATTRIB4NUSVARB_OPCODE:
				__convert_Nus4(vtx->attrib[VERT_ATTRIB_POS], (GLushort *) vtx_data);
				break;
			case CR_VERTEXATTRIB4BVARB_OPCODE:
				__convert_b4(vtx->attrib[VERT_ATTRIB_POS], (GLbyte *) vtx_data);
				break;
			case CR_VERTEXATTRIB4DARB_OPCODE:
				__convert_d4(vtx->attrib[VERT_ATTRIB_POS], (GLdouble *) vtx_data);
				break;
			case CR_VERTEXATTRIB4FARB_OPCODE:
				__convert_f4(vtx->attrib[VERT_ATTRIB_POS], (GLfloat *) vtx_data);
				break;
			case CR_VERTEXATTRIB4IVARB_OPCODE:
				__convert_Ni4(vtx->attrib[VERT_ATTRIB_POS], (GLint *) vtx_data);
				break;
			case CR_VERTEXATTRIB4SARB_OPCODE:
				__convert_s4(vtx->attrib[VERT_ATTRIB_POS], (GLshort *) vtx_data);
				break;
			case CR_VERTEXATTRIB4UBVARB_OPCODE:
				__convert_ub4(vtx->attrib[VERT_ATTRIB_POS], (GLubyte *) vtx_data);
				break;
			case CR_VERTEXATTRIB4UIVARB_OPCODE:
				__convert_ui4(vtx->attrib[VERT_ATTRIB_POS], (GLuint *) vtx_data);
				break;
			case CR_VERTEXATTRIB4USVARB_OPCODE:
				__convert_us4(vtx->attrib[VERT_ATTRIB_POS], (GLushort *) vtx_data);
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

static void __pinchIssueParams(const CRVertex *vtx) 
{
	GET_CONTEXT(ctx);
	GLfloat val[4];
	unsigned int i;

	for (i = 0 ; i < ctx->limits.maxTextureUnits ; i++)
	{
		COPY_4V(val, vtx->attrib[VERT_ATTRIB_TEX0 + i]);
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
	COPY_4V(val, vtx->attrib[VERT_ATTRIB_NORMAL]);
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
	COPY_4V(val, vtx->attrib[VERT_ATTRIB_COLOR0]);
	if (tilesort_spu.swap)
	{
		crPackColor4fvSWAP((const GLfloat *) val);
	}
	else
	{
		crPackColor4fv((const GLfloat *) val);
	}
	COPY_4V(val, vtx->attrib[VERT_ATTRIB_COLOR1]);
	if (tilesort_spu.swap)
	{
		crPackSecondaryColor3fvEXTSWAP((const GLfloat *) val);
	}
	else
	{
		crPackSecondaryColor3fvEXT((const GLfloat *) val);
	}
	/* XXX do other attribs */
}

static void __pinchIssueVertex(const CRVertex *vtx) 
{
	GLfloat val[4];

	__pinchIssueParams(vtx);

	COPY_4V(val, vtx->attrib[VERT_ATTRIB_POS]);
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
		for (j = 1 ; j < CR_MAX_VERTEX_ATTRIBS; j++)
		{
			COPY_4V(v.attrib[j], c->vertexAttrib[j]);
		}
		v.edgeFlag = c->edgeFlag;
		v.colorIndex = c->colorIndex;

		__pinchIssueParams (&v);
	}
}              
