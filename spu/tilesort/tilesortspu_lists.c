/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <float.h>
#include "tilesortspu.h"
#include "tilesortspu_proto.h"
#include "cr_packfunctions.h"
#include "cr_error.h"
#include "state/cr_stateerror.h"



void TILESORTSPU_APIENTRY
tilesortspu_NewList(GLuint list, GLuint mode) 
{
	/* Normally, the current packing buffer is for geometry only.
	 * State-change data is packed into a separate buffer after doing
	 * state differencing.  However, when we build display lists state
	 * change commands get put into the main pack buffer.  So, turn off
	 * this debug/sanity-check flag.  We can turn it back on in EndList().
	 */
	CRPackContext *c = crPackGetContext();
	c->buffer.geometry_only = GL_FALSE;

	/* the state tracker will do error checking and the flush for us. */
	crStateNewList( list, mode );

	/* GL_COMPILE_AND_EXECUTE confuses the server! */
	mode = GL_COMPILE;

	/*
	 * XXX TODO: Implement lazy display list upload.
	 * Don't send display list to server X, until we actually need to
	 * execute it there.
	 */
	if (tilesort_spu.swap)
		crPackNewListSWAP(list, mode);
	else
		crPackNewList(list, mode);

	crdlm_NewList(list, mode, &(tilesort_spu.self));
}



void TILESORTSPU_APIENTRY
tilesortspu_EndList(void) 
{
	GET_CONTEXT(ctx);
	CRPackContext *c = crPackGetContext();
	const GLuint mode = ctx->lists.mode;
	const GLuint list = ctx->lists.currentIndex;

	crdlm_EndList();
	crStateEndList();

	/* now play the list through the packer to send it to the servers */
	crdlm_CallList(list, &tilesort_spu.packerDispatch);

	/* the state tracker will do error checking and the flush for us. */
	if (tilesort_spu.swap)
		crPackEndListSWAP();
	else
		crPackEndList();

	/* restore normal/default dispatch table */
	crdlm_RestoreDispatch();  /* companion to crdlm_EndList() */

	tilesortspuBroadcastGeom(0);

	/* Turn the geometry_only flag back on.
	 * See the longer comment above in tilesortspu_NewList().
	 */
	c->buffer.geometry_only = GL_TRUE;

	if (mode == GL_COMPILE_AND_EXECUTE) {
		/* we really used GL_COMPILE mode.  Now, replay/execute the list */
		tilesortspu_CallList(list);
	}
}


/*
 * This gets called after glCallList.  It replays the display lists' state
 * change commands in order to update the servers' CRContext records.
 * Called via tilesortspu_CallList() and via the state table built by
 * tilesortspuLoadStateTable().
 * This function will get called recursively for nested display lists.
 */
void TILESORTSPU_APIENTRY
tilesortspuStateCallList(GLuint list)
{
	GET_THREAD(thread);
	ContextInfo *context = thread->currentContext;
	CRContext *savedContext = crStateGetCurrent();
	int i;

	/* NOTE: this function may get called recursively when calling a nested
	 * display list.  Therefore, we have to save the current context when
	 * we're called and restore it when we leave.
	 */

	/* update servers' state records */
	for (i = 0; i < tilesort_spu.num_servers; i++) {
		crStateSetCurrent(context->server[i].State);
		crdlm_CallList(list, &tilesort_spu.stateDispatch);
	}

	/* Update our local, application context state */
	crStateSetCurrent(context->State);
	crdlm_CallList(list, &tilesort_spu.stateDispatch);

	/* restore previously current context */
	crStateSetCurrent(savedContext);
}


/*
 * As above, but for glCallLists.
 */
void TILESORTSPU_APIENTRY
tilesortspuStateCallLists(GLsizei n, GLenum type, const GLvoid *lists)
{
	GET_THREAD(thread);
	ContextInfo *context = thread->currentContext;
	CRContext *savedContext = crStateGetCurrent();
	int i;

	/* NOTE: this function may get called recursively when calling a nested
	 * display list.  Therefore, we have to save the current context when
	 * we're called and restore it when we leave.
	 */

	/* update servers' state records */
	for (i = 0; i < tilesort_spu.num_servers; i++) {
		crStateSetCurrent(context->server[i].State);
		crdlm_ListBase(context->State->lists.base); /* not server[i] state */
		crdlm_CallLists(n, type, lists, &tilesort_spu.stateDispatch);
	}

	/* Update our local, application context state */
	crStateSetCurrent(context->State);
	crdlm_ListBase(context->State->lists.base);
	crdlm_CallLists(n, type, lists, &tilesort_spu.stateDispatch);

	/* restore previously current context */
	crStateSetCurrent(savedContext);
}


void TILESORTSPU_APIENTRY
tilesortspu_CallList(GLuint list)
{
#if 0
	GET_CONTEXT(ctx);
#else
	GET_THREAD(thread);
#endif
	GLboolean resetBBox = GL_FALSE;

	if (thread->currentContext->State->lists.mode == 0) {
		/* Execute glCallList */

		/* Try to prefix this display list with a bounding box.
		 * But only if the user hasn't already specified his own bbox AND if
		 * the autoDListBBoxes option is turned on.
		 */
		if (thread->currentContext->providedBBOX == GL_DEFAULT_BBOX_CR &&
				tilesort_spu.autoDListBBoxes) {
			CRDLMBounds bounds;
			if (crDLMGetBounds(tilesort_spu.dlm, list, &bounds) == GL_NO_ERROR
					&& bounds.xmin != FLT_MAX) {
				GLfloat bbox[6];
				bbox[0] = (GLfloat) bounds.xmin;
				bbox[1] = (GLfloat) bounds.ymin;
				bbox[2] = (GLfloat) bounds.zmin;
				bbox[3] = (GLfloat) bounds.xmax;
				bbox[4] = (GLfloat) bounds.ymax;
				bbox[5] = (GLfloat) bounds.zmax;

				/*
				printf("Autobox for list %d:\n", list);
				printf(" %f .. %f\n", bounds.xmin, bounds.xmax);
			  printf(" %f .. %f\n", bounds.ymin, bounds.ymax);
				printf(" %f .. %f\n", bounds.zmin, bounds.zmax);
				*/

				/* set the bounding box */
				tilesortspu_ChromiumParametervCR(GL_OBJECT_BBOX_CR, GL_FLOAT, 6, bbox);
				resetBBox = GL_TRUE;
			}
		}

		if (thread->currentContext->providedBBOX == GL_DEFAULT_BBOX_CR) {
			tilesortspuFlush( thread );
		}
	}
	else {
		/* we're compiling a glCallList into another display list */
		crdlm_compile_CallList( list );
	}

	/* This is experimental code to fix some issues with glBindTexture
	 * inside display lists.  Basically, if a BindTexture is called in
	 * a display list, we need to have flushed any changes to the texture
	 * object before calling the list.
	 */
#if 0
	{
		 int i;
		 crPackReleaseBuffer( thread->packer );
		 for ( i = 0 ; i < tilesort_spu.num_servers; i++ ) {
				CRContext *serverState = thread->currentContext->server[i].State;
				crPackSetBuffer( thread->packer, &(thread->buffer[i]) );
				crStateDiffAllTextureObjects(ctx, serverState->bitid);
				crPackReleaseBuffer( thread->packer );
		 }
		 crPackSetBuffer( thread->packer, &(thread->geometry_buffer) );
	}
#endif


	/* Always pack CallList.  It'll get sent to servers based on the usual
	 * bucketing routine.
	 */
	if (tilesort_spu.swap)
	   crPackCallListSWAP( list );
	else
	   crPackCallList( list );

	if (thread->currentContext->State->lists.mode == 0) {
		/* Execute glCallList */

		if (thread->currentContext->providedBBOX == GL_DEFAULT_BBOX_CR) {
			/* we don't have a bounding box for the geometry in this
			 * display list, so need to broadcast.
			 */
			/*printf(" %s broadcast\n", __FUNCTION__);*/
			tilesortspuBroadcastGeom(1);
		}
		else {
			/* we relied upon the bucketing system to send the list to the
			 * appropriate servers.
			 */
			/*printf(" %s bucket\n", __FUNCTION__);*/
			tilesortspuFlush( thread );
		}

		/* XXX only update state on servers if we actually sent the display list!
		 */
		tilesortspuStateCallList(list);
	}

	/* If we used an auto bounding box, turn it off now */
	if (resetBBox)
		tilesortspu_ChromiumParametervCR(GL_DEFAULT_BBOX_CR, GL_FLOAT, 0, NULL);
}


void TILESORTSPU_APIENTRY
tilesortspu_CallLists(GLsizei n, GLenum type, const GLvoid *lists)
{
	GET_CONTEXT(ctx);

	/*
	 * XXX we have no support for auto bounding boxes here -- someday?
	 */

	if (thread->currentContext->State->lists.mode == 0) {
		/* Execute glCallList */
		if (thread->currentContext->providedBBOX == GL_DEFAULT_BBOX_CR) {
			tilesortspuFlush( thread );
		}
	}
	else {
		/* we're compiling glCallLists into another display list */
		crdlm_compile_CallLists(n, type, lists);
	}

	/* always pack CallLists to send it to servers */
	if (tilesort_spu.swap) {
		crPackListBaseSWAP( ctx->lists.base );
		crPackCallListsSWAP( n, type, lists );
	}
	else {
		crPackListBase( ctx->lists.base );
		crPackCallLists( n, type, lists );
	}

	if (thread->currentContext->State->lists.mode == 0) {
		/* Execute glCallLists */

		if (thread->currentContext->providedBBOX == GL_DEFAULT_BBOX_CR) {
			/* we don't have a bounding box for the geometry in this
			 * display list, so need to broadcast.
			 */
			tilesortspuBroadcastGeom(1);
		}
		else {
			/* we relied upon the bucketing system to send the list to the
			 * appropriate servers.
			 */
			tilesortspuFlush( thread );
		}

		crdlm_ListBase(ctx->lists.base);
		tilesortspuStateCallLists(n, type, lists);
	}
}


/*
 * Delete list info both from our state tracker and the DLM context.
 * NOTE: we're using the crStateGenLists and crStateIsList functions
 * otherwise.
 */
void TILESORTSPU_APIENTRY
tilesortspu_DeleteLists(GLuint list, GLsizei range)
{
	crStateDeleteLists(list, range);
	crdlm_DeleteLists(list, range);
}


/* XXX fix and plug into dispatcher */
void TILESORTSPU_APIENTRY
tilesortspu_ListBase(GLuint base)
{
	crStateListBase(base);
	crdlm_ListBase(base);
}
