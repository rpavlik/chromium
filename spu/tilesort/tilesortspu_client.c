/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_packfunctions.h"
#include "cr_error.h"
#include "cr_net.h"
#include "tilesortspu.h"
#include "tilesortspu_gen.h"


void TILESORTSPU_APIENTRY
tilesortspu_ArrayElement( GLint index )
{
	GET_THREAD(thread);
	CRClientState *clientState = &(thread->currentContext->State->client);
	GLenum dlMode = thread->currentContext->displayListMode;
	if (dlMode != GL_FALSE && tilesort_spu.lazySendDLists) {
		crDLMCompileArrayElement(index, clientState);
		return;
	}
	if (tilesort_spu.swap)
	{
		crPackExpandArrayElementSWAP( index, clientState );
	}
	else
	{
		crPackExpandArrayElement( index, clientState );
	}
}


void TILESORTSPU_APIENTRY
tilesortspu_DrawArrays( GLenum mode, GLint first, GLsizei count )
{
	GET_CONTEXT(ctx);
	CRClientState *clientState = &(thread->currentContext->State->client);
	GLenum dlMode = thread->currentContext->displayListMode;
	if (dlMode != GL_FALSE && tilesort_spu.lazySendDLists) {
		crDLMCompileDrawArrays(mode, first, count, clientState);
		return;
	}

	if (count < 0)
	{
		crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, "tilesortspu_DrawArrays(count < 0)");
		return;
	}

	if (mode > GL_POLYGON)
	{
		crStateError( __LINE__, __FILE__, GL_INVALID_ENUM, "tilesortspu_DrawArrays(mode=%d)", mode);
		return;
	}

	if (ctx->current.inBeginEnd)
	{
		crStateError( __LINE__, __FILE__, GL_INVALID_OPERATION,  "tilesortspu_DrawArrays called in a Begin/End" );
		return;
	}

	if (crStateUseServerArrays()) {
		/* This is like a glBegin, have to flush all preceeding state changes */
		tilesortspuFlush( thread );
		crPackDrawArrays( mode, first, count );
		/* then broadcast the drawing command */
		tilesortspuBroadcastGeom(1);
	}
	else {
		/* client-side arrays, expand into simpler commands */
		int i;
		tilesort_spu.self.Begin(mode);
		if (tilesort_spu.swap)
		{
			for (i=0; i<count; i++) 
			{
				crPackExpandArrayElementSWAP(first++, clientState);
			}
		}
		else
		{
			for (i=0; i<count; i++) 
			{
				crPackExpandArrayElement(first++, clientState);
			}
		}
		tilesort_spu.self.End();
	}
}


void TILESORTSPU_APIENTRY
tilesortspu_DrawElements(GLenum mode, GLsizei count, GLenum type,
												 const GLvoid *indices)
{
	GET_CONTEXT(ctx);
	CRClientState *clientState = &(thread->currentContext->State->client);
	GLenum dlMode = thread->currentContext->displayListMode;

	if (dlMode != GL_FALSE && tilesort_spu.lazySendDLists) {
		crDLMCompileDrawElements(mode, count, type, indices, clientState);
		return;
	}

	if (count < 0) {
		crStateError( __LINE__, __FILE__, GL_INVALID_VALUE,
									"tilesortspu_DrawElements(count=%d)", count);
		return;
	}

	if (mode > GL_POLYGON) {
		crStateError( __LINE__, __FILE__, GL_INVALID_ENUM,
									"tilesortspu_DrawElements(mode=%d)", mode);
		return;
	}

	if (type != GL_UNSIGNED_BYTE &&
			type != GL_UNSIGNED_SHORT &&
			type != GL_UNSIGNED_INT) {
		crStateError( __LINE__, __FILE__, GL_INVALID_ENUM,
									"tilesortspu_DrawElements(type=%d)", type);
		return;
	}

	if (ctx->current.inBeginEnd) {
		crStateError( __LINE__, __FILE__, GL_INVALID_OPERATION,
									"tilesortspu_DrawElements called in a Begin/End" );
		return;
	}

	if (crStateUseServerArrays() &&
			crStateUseServerArrayElements()) {
		/* Both array data and element indices are on server */
		/* This is like a glBegin, have to flush all preceeding state changes */
		tilesortspuFlush( thread );
		if (tilesort_spu.swap)
			crPackDrawElementsSWAP( mode, count, type, indices );
		else
			crPackDrawElements( mode, count, type, indices );
		/* then broadcast the drawing command */
		tilesortspuBroadcastGeom(1);
	}
	else if (crStateUseServerArrays()) {
		/* array vertex data is on server, but element indices are local */
		tilesort_spu.self.Begin(mode);
		if (tilesort_spu.swap)
			crPackUnrollDrawElementsSWAP(count, type, indices);
		else
			crPackUnrollDrawElements(count, type, indices);
		tilesort_spu.self.End();
	}
	else {
		/* client-side arrays, expand into simpler commands */
		tilesort_spu.self.Begin(mode);
		/* Note: 999 indicates that glBegin/End should not be called */
		if (tilesort_spu.swap)
			crPackExpandDrawElementsSWAP(999, count, type, indices, clientState);
		else
			crPackExpandDrawElements(999, count, type, indices, clientState);
		tilesort_spu.self.End();
	}
}


void TILESORTSPU_APIENTRY
tilesortspu_DrawRangeElements(GLenum mode, GLuint start, GLuint end,
															GLsizei count, GLenum type,
															const GLvoid *indices)
{
	GET_CONTEXT(ctx);
	CRClientState *clientState = &(thread->currentContext->State->client);
	GLenum dlMode = thread->currentContext->displayListMode;
	if (dlMode != GL_FALSE && tilesort_spu.lazySendDLists) {
		crDLMCompileDrawRangeElements(mode, start, end, count, type, indices, clientState);
		return;
	}

	if (count < 0)
	{
		crStateError( __LINE__, __FILE__, GL_INVALID_VALUE, "tilesortspu_DrawRangeElements(count=%d)", count);
		return;
	}

	if (mode > GL_POLYGON)
	{
		crStateError( __LINE__, __FILE__, GL_INVALID_ENUM, "tilesortspu_DrawRangeElements(mode=%d)", mode);
		return;
	}

	if (type != GL_UNSIGNED_BYTE && type != GL_UNSIGNED_SHORT && type != GL_UNSIGNED_INT)
	{
		crStateError( __LINE__, __FILE__, GL_INVALID_ENUM, "tilesortspu_DrawRangeElements(type=0x%x)", type);
		return;
	}
	
	if (ctx->current.inBeginEnd)
	{
		crStateError( __LINE__, __FILE__, GL_INVALID_OPERATION, "tilesortspu_DrawRangeElements called in a Begin/End" );
		return;
	}

	if (crStateUseServerArrays()) {
		/* This is like a glBegin, have to flush all preceeding state changes */
		tilesortspuFlush( thread );
		crPackDrawRangeElements( mode, start, end, count, type, indices );
		/* then broadcast the drawing command */
		tilesortspuBroadcastGeom(1);
	}
	else {
		/* client-side arrays, expand into simpler commands */
		GLubyte *p = (GLubyte *)indices;
		int i;
		tilesort_spu.self.Begin( mode );
		switch (type) 
		{
		case GL_UNSIGNED_BYTE:
			if (tilesort_spu.swap)
			{
				for (i=start; i<count; i++)
				{
					crPackExpandArrayElementSWAP((GLint) *p++, clientState);
				}
			}
			else
			{
				for (i=start; i<count; i++)
				{
					crPackExpandArrayElement((GLint) *p++, clientState);
				}
			}
			break;
		case GL_UNSIGNED_SHORT:
			if (tilesort_spu.swap)
			{
				for (i=start; i<count; i++) 
				{
					crPackExpandArrayElementSWAP((GLint) * (GLushort *) p, clientState);
					p+=sizeof (GLushort);
				}
			}
			else
			{
				for (i=start; i<count; i++) 
				{
					crPackExpandArrayElement((GLint) * (GLushort *) p, clientState);
					p+=sizeof (GLushort);
				}
			}
			break;
		case GL_UNSIGNED_INT:
			if (tilesort_spu.swap)
			{
				for (i=start; i<count; i++) 
				{
					crPackExpandArrayElementSWAP((GLint) * (GLuint *) p, clientState);
					p+=sizeof (GLuint);
				}
			}
			else
			{
				for (i=start; i<count; i++) 
				{
					crPackExpandArrayElement((GLint) * (GLuint *) p, clientState);
					p+=sizeof (GLuint);
				}
			}
			break;
		default:
			crStateError( __LINE__, __FILE__, GL_INVALID_ENUM,  "tilesortspu_DrawRangeElements(type=0x%s)", type);
			return;
		}
		tilesort_spu.self.End();
	}
}


#ifdef CR_EXT_multi_draw_arrays
void TILESORTSPU_APIENTRY
tilesortspu_MultiDrawArraysEXT(GLenum mode, GLint *first, GLsizei *count,
															 GLsizei primcount)
{
	GET_THREAD(thread);
	CRClientState *clientState = &(thread->currentContext->State->client);
	GLenum dlMode = thread->currentContext->displayListMode;
	if (dlMode != GL_FALSE && tilesort_spu.lazySendDLists) {
		crDLMCompileMultiDrawArraysEXT(mode, first, count, primcount, clientState);
		return;
	}
	if (tilesort_spu.swap)
	{
		crPackExpandMultiDrawArraysEXTSWAP( mode, first, count, primcount, clientState );
	}
	else
	{
		crPackExpandMultiDrawArraysEXT( mode, first, count, primcount, clientState );
	}
}


void TILESORTSPU_APIENTRY
tilesortspu_MultiDrawElementsEXT(GLenum mode, const GLsizei *count,
																 GLenum type, const GLvoid **indices,
																 GLsizei primcount )
{
	GET_THREAD(thread);
	CRClientState *clientState = &(thread->currentContext->State->client);
	GLenum dlMode = thread->currentContext->displayListMode;
	if (dlMode != GL_FALSE && tilesort_spu.lazySendDLists) {
		crDLMCompileMultiDrawElementsEXT(mode, count, type, indices, primcount, clientState);
		return;
	}
	if (tilesort_spu.swap)
	{
		crPackExpandMultiDrawElementsEXTSWAP( mode, count, type, indices, primcount, clientState );
	}
	else
	{
		crPackExpandMultiDrawElementsEXT( mode, count, type, indices, primcount, clientState );
	}
}
#endif
