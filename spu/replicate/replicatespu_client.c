/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "replicatespu.h"
#include "cr_packfunctions.h"
#include "cr_glstate.h"
#include "replicatespu_proto.h"

void REPLICATESPU_APIENTRY replicatespu_ArrayElement( GLint index )
{
	GET_THREAD(thread);
	ContextInfo *ctx = thread->currentContext;
	CRClientState *clientState = &(ctx->State->client);
	GLboolean serverArrays = GL_FALSE;

#if CR_ARB_vertex_buffer_object
	if (ctx->State->extensions.ARB_vertex_buffer_object)
		serverArrays = crStateUseServerArrays();
#endif

	/* Need this to operate correctly with the display list */
	if (thread->currentContext->displayListMode != GL_FALSE) {
	    crDLMCompileArrayElement(index, clientState);
	}

	if (serverArrays) {
		/* Send the DrawArrays command over the wire */
		if (replicate_spu.swap)
			crPackArrayElementSWAP( index );
		else
			crPackArrayElement( index );
	}
	else {
		/* evaluate locally */
		if (replicate_spu.swap)
			crPackExpandArrayElementSWAP( index, clientState );
		else
			crPackExpandArrayElement( index, clientState );
	}
}

void REPLICATESPU_APIENTRY replicatespu_DrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices )
{
	GET_THREAD(thread);
	ContextInfo *ctx = thread->currentContext;
	CRClientState *clientState = &(ctx->State->client);
	GLboolean serverArrays = GL_FALSE;

#if CR_ARB_vertex_buffer_object
	if (ctx->State->extensions.ARB_vertex_buffer_object)
		serverArrays = crStateUseServerArrays();
#endif

	/* Need this to operate correctly with the display list */
	if (thread->currentContext->displayListMode != GL_FALSE) {
	    crDLMCompileDrawElements(mode, count, type, indices, clientState);
	}

	if (serverArrays) {
		/* Send the DrawArrays command over the wire */
		if (replicate_spu.swap)
			crPackDrawElementsSWAP( mode, count, type, indices );
		else
			crPackDrawElements( mode, count, type, indices );
	}
	else {
		/* evaluate locally */
		if (replicate_spu.swap)
			crPackExpandDrawElementsSWAP( mode, count, type, indices, clientState );
		else
			crPackExpandDrawElements( mode, count, type, indices, clientState );
	}
}

void REPLICATESPU_APIENTRY replicatespu_DrawRangeElements( GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices )
{
	GET_THREAD(thread);
	ContextInfo *ctx = thread->currentContext;
	CRClientState *clientState = &(ctx->State->client);
	GLboolean serverArrays = GL_FALSE;

#if CR_ARB_vertex_buffer_object
	if (ctx->State->extensions.ARB_vertex_buffer_object)
		 serverArrays = crStateUseServerArrays();
#endif

	/* Need this to operate correctly with the display list */
	if (thread->currentContext->displayListMode != GL_FALSE) {
	    crDLMCompileDrawRangeElements(mode, start, end, count, type, indices, clientState);
	}

	if (serverArrays) {
		/* Send the DrawRangeElements command over the wire */
		if (replicate_spu.swap)
			crPackDrawRangeElementsSWAP( mode, start, end, count, type, indices );
		else
			crPackDrawRangeElements( mode, start, end, count, type, indices );
	}
	else {
		/* evaluate locally */
		if (replicate_spu.swap)
			crPackExpandDrawRangeElementsSWAP( mode, start, end, count, type, indices, clientState );
		else
			crPackExpandDrawRangeElements( mode, start, end, count, type, indices, clientState );
	}
}

void REPLICATESPU_APIENTRY replicatespu_DrawArrays( GLenum mode, GLint first, GLsizei count )
{
	GET_THREAD(thread);
	ContextInfo *ctx = thread->currentContext;
	CRClientState *clientState = &(ctx->State->client);
	GLboolean serverArrays = GL_FALSE;

#if CR_ARB_vertex_buffer_object
	if (ctx->State->extensions.ARB_vertex_buffer_object)
		 serverArrays = crStateUseServerArrays();
#endif

	/* Need this to operate correctly with the display list */
	if (thread->currentContext->displayListMode != GL_FALSE) {
	    crDLMCompileDrawArrays(mode, first, count, clientState);
	}

	if (serverArrays) {
		/* Send the DrawArrays command over the wire */
		if (replicate_spu.swap)
			crPackDrawArraysSWAP( mode, first, count );
		else
			crPackDrawArrays( mode, first, count );
	}
	else {
		/* evaluate locally */
		if (replicate_spu.swap)
			crPackExpandDrawArraysSWAP( mode, first, count, clientState );
		else
			crPackExpandDrawArrays( mode, first, count, clientState );
	}
}

#ifdef CR_EXT_multi_draw_arrays
void REPLICATESPU_APIENTRY replicatespu_MultiDrawArraysEXT( GLenum mode, GLint *first, GLsizei *count, GLsizei primcount )
{
	GLint i;
	for (i = 0; i < primcount; i++) {
		if (count[i] > 0) {
			replicatespu_DrawArrays(mode, first[i], count[i]);
		}
	}
}

void REPLICATESPU_APIENTRY replicatespu_MultiDrawElementsEXT( GLenum mode, const GLsizei *count, GLenum type, const GLvoid **indices, GLsizei primcount )
{
	GLint i;
	for (i = 0; i < primcount; i++) {
		if (count[i] > 0) {
			replicatespu_DrawElements(mode, count[i], type, indices[i]);
		}
	}
}
#endif
