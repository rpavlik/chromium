/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <float.h>
#include "tilesortspu.h"
#include "tilesortspu_gen.h"
#include "tilesortspu_dlstate_gen.h"
#include "cr_packfunctions.h"
#include "cr_dlm.h"
#include "cr_error.h"
#include "state/cr_stateerror.h"

void TILESORTSPU_APIENTRY
tilesortspu_NewList(GLuint list, GLuint mode) 
{
	CRPackContext *c;
    
	GET_THREAD(thread);

	/* No matter how we're configured (i.e. compiling all functions locally,
	 * compiling state functions locally, or sending all display lists
	 * immediately to servers), these will have to be saved.
	 */
	thread->currentContext->displayListMode = mode;
	thread->currentContext->displayListIdentifier = list;

	/* the state tracker will do error checking and the flush for us. */
	crStateNewList( list, mode );

	if (tilesort_spu.lazySendDLists || tilesort_spu.listTrack) {
		/* In either case, we're locally compiling display lists
		 * for later reference, so tell the DLM that we're starting
		 * a compile.  Note that we always do a strict compile
		 * (and not compile-and-execute), in order to simplify
		 * logic; compile-and-execute is simulated later.
		 */
		crDLMNewList(list, GL_COMPILE);

		/* If we're doing lazy display lists, we don't send the
		 * NewList to any server until needed.  Return for now.
		 */
		if (tilesort_spu.lazySendDLists)
			return;
	}

	/* If we get here, we're not doing lazy display lists; rather, we're
	 * sending display lists to servers as they are created.  (Note that
	 * we don't come here if we're replaying a display list; we hit
	 * a crPack function instead.)
	 */

	/* Normally, the current packing buffer is for geometry only.
	 * State-change data is packed into a separate buffer after doing
	 * state differencing.  However, when we build display lists state
	 * change commands get put into the main pack buffer.  So, turn off
	 * this debug/sanity-check flag.  We can turn it back on in EndList().
	 */
	c = crPackGetContext();

	c->buffer.geometry_only = GL_FALSE;

	if (tilesort_spu.swap)
		crPackNewListSWAP(list, mode);
	else
		crPackNewList(list, mode);

	if (tilesort_spu.autoDListBBoxes && !tilesort_spu.listTrack) {
		crPackResetBBOX(thread->packer);
	}
}


void TILESORTSPU_APIENTRY
tilesortspu_EndList(void) 
{
	GET_CONTEXT(ctx);
	CRPackContext *c = crPackGetContext();
	const GLuint list = ctx->lists.currentIndex;
	GLenum oldMode = thread->currentContext->displayListMode;
	GLuint oldList = thread->currentContext->displayListIdentifier;

	/* In all cases, shut off display list creation or compilation */
	thread->currentContext->displayListMode = GL_FALSE;
	thread->currentContext->displayListIdentifier = 0;
	crStateEndList();

	/* If we're compiling locally, initialize the list of servers that
	 * have received this display list to empty.
	 */
	if (tilesort_spu.lazySendDLists) {
		GLboolean *listSent;

		/* Finish compiling */
		crDLMEndList();

		/* Create a list of which servers have received
		 * the display list so far.  The list should be,
		 * of course, empty.  Note that we replace any
		 * existing hash table entry, just in case we're
		 * redefining an existing display list.
		 */
		listSent = crCalloc(tilesort_spu.num_servers * sizeof(GLboolean));
		crHashtableReplace(tilesort_spu.listTable, list, listSent, crFree);
		    
		/* If the display list was originally created with
		 * GL_COMPILE_AND_EXECUTE, we changed the mode to
		 * GL_COMPILE and only compiled elements.  Now we
		 * simulate GL_COMPILE_AND_EXECUTE by making
		 a reference to the list.
		*/
		if (oldMode == GL_COMPILE_AND_EXECUTE) {
			tilesortspu_CallList(oldList);
		}

		/* All done. */
		return;
	}

	if (tilesort_spu.listTrack) {
		/* If we're here, we're compiling state functions, but also sending
		 * the display lists to servers as they are created.  We have to
		 * finish our local compile, and then let the code fall through
		 * to pack the EndList instruction.
		 */
		crDLMEndList();
	}

	if (tilesort_spu.swap)
		crPackEndListSWAP();
	else
		crPackEndList();

	/* Turn the geometry_only flag back on.
	 * See the longer comment above in tilesortspu_NewList().
	 */
	c->buffer.geometry_only = GL_TRUE;

	tilesortspuBroadcastGeom(0);

}


/**
 * This gets called after glCallList.  It replays the display lists' state
 * change commands in order to update the servers' CRContext records.
 * Called via tilesortspu_CallList() and via the state table built by
 * tilesortspuLoadStateTable().
 * This function will get called recursively for nested display lists.
 */
void TILESORTSPU_APIENTRY
tilesortspu_StateCallList(GLuint list)
{
	/* This will be struck only if we find a compiled CallList function
	 * in a display list, and we need to reproduce its effects on the
	 * current state record.
	 *
	 * Fortunately, that's really easy to do.
	 */
	crDLMReplayListState(list, &tilesort_spu.stateDispatch);
}


/*
 * As above, but for glCallLists.
 */
void TILESORTSPU_APIENTRY
tilesortspu_StateCallLists(GLsizei n, GLenum type, const GLvoid *lists)
{
	crDLMReplayListsState(n, type, lists, &tilesort_spu.stateDispatch);
}


void TILESORTSPU_APIENTRY
tilesortspu_CallList(GLuint list)
{
	GET_THREAD(thread);
	GLenum dlMode = thread->currentContext->displayListMode;
	WindowInfo *winInfo = thread->currentContext->currentWindow;
	GLboolean resetBBox = GL_FALSE;
	TileSortBucketInfo bucket_info;

	/* If we're supposed to compile display lists, and a display list is being
	 * compiled, then compile this function.
	 */
	if (dlMode != GL_FALSE) {
		if (tilesort_spu.lazySendDLists || tilesort_spu.listTrack)
			crDLMCompileCallList(list);
		else if (tilesort_spu.swap)
			crPackCallList(list);
		else
			crPackCallList(list);
		return;
	}

	/* We're executing a glCallList. */

#if 0
	/* This is experimental code to fix some issues with glBindTexture
	 * inside display lists.  Basically, if a BindTexture is called in
	 * a display list, we need to have flushed any changes to the texture
	 * object before calling the list.
	 */
	{
		int i;
		crPackReleaseBuffer( thread->packer );
		for ( i = 0 ; i < tilesort_spu.num_servers; i++ ) {
			CRContext *serverState = thread->currentContext->server[i].State;
			thread->state_server_index = i;
			crPackSetBuffer( thread->packer, &(thread->buffer[i]) );
			crStateDiffAllTextureObjects(thread->currentContext->State,
																	 serverState->bitid);
			crPackReleaseBuffer( thread->packer );
		}
		crPackSetBuffer( thread->packer, &(thread->geometry_buffer) );
		thread->state_server_index = -1;
	}
#endif

	/*
	 * If there's a bounding box, all servers with domains that
	 * intersect the bounding box will need to call this display
	 * list.  Otherwise, if there's no bounding box, play it safe
	 * and send the display list to all servers.
	 */
	if (crDLMListHasBounds(list)) {
		CRDLMBounds bounds;
		GLfloat bbox[6];

		crDLMGetBounds(list, &bounds);

		/* Run the bucket sort routine to determine which servers will
		 * receive this display list.
		 */
		bucket_info.objectMin.x = (GLfloat) bounds.xmin;
		bucket_info.objectMin.y = (GLfloat) bounds.ymin;
		bucket_info.objectMin.z = (GLfloat) bounds.zmin;
		bucket_info.objectMax.x = (GLfloat) bounds.xmax;
		bucket_info.objectMax.y = (GLfloat) bounds.ymax;
		bucket_info.objectMax.z = (GLfloat) bounds.zmax;
		tilesortspuBucketGeometry(winInfo, &bucket_info);

		/*
		 * Set the bbox bounds here so when the crPackCallList() call (below)
		 * gets bucketed, it gets sent to the right servers.
		 */
		bbox[0] = (GLfloat) bounds.xmin;
		bbox[1] = (GLfloat) bounds.ymin;
		bbox[2] = (GLfloat) bounds.zmin;
		bbox[3] = (GLfloat) bounds.xmax;
		bbox[4] = (GLfloat) bounds.ymax;
		bbox[5] = (GLfloat) bounds.zmax;
		tilesortspu_ChromiumParametervCR(GL_OBJECT_BBOX_CR, GL_FLOAT, 6, bbox);
		resetBBox = GL_TRUE;
	}
	else {
		/* broadcast - set all bucket_info hit flags */
		int i;
		for (i = 0 ; i < (tilesort_spu.num_servers + 31) / 32; i++) {
			bucket_info.hits[i] = ~0;
		}
	}

	/* If we've been sending display lists lazily (i.e. only sending
	 * them to servers when they're needed), check to see whether
	 * any of the servers that need this display list hasn't seen
	 * the display list itself yet.  Note that the "listSent" array
	 * is created when the list is defined, and contains indicators
	 * for every server regarding whether or not they've received
	 * this display list.  If and when the "listSent" array is
	 * deleted for a given display list, this indicates that the
	 * list has been sent to all servers, and doesn't need to be
	 * tracked further.
	 *
	 * Note that if we enter this block, we know we're not in the middle
	 * of compiling a display list, so it's safe to pack and send all those
	 * instructions to the server.  (If we were compiling a display list, 
	 * and lazySendDLists were configured, then we'd have returned a few
	 * blocks ago.)
	 */
	if (tilesort_spu.lazySendDLists) {
		GLboolean *listSent
			= (GLboolean *) crHashtableSearch(tilesort_spu.listTable, list);
		GLboolean needAnySend = GL_FALSE;
		TileSortBucketInfo send_info;

		/* The listSent array will be NULL if the list has been
		 * sent to all servers already.
		 */
		if (listSent != NULL) {
		    int numSent = 0, i;

		    /* determine which servers need the display list, but don't yet have it */
		    crMemZero(&send_info, sizeof(send_info));
		    for (i = 0 ; i < tilesort_spu.num_servers; i++) {
			    const int node32 = i >> 5, node = i & 0x1f;
			    if (bucket_info.hits[node32] & (1 << node)) {
				    /* server i needs the list */
				    if (!listSent[i]) {
					    /* server i doesn't have the list yet */
					    needAnySend = GL_TRUE;
					    listSent[i] = GL_TRUE;
					    send_info.hits[node32] |= (1 << node);
				    }
			    }
			    if (listSent[i])
				    numSent++;
		    }

		    if (numSent == tilesort_spu.num_servers) {
			/* record the fact that the list has been now sent to _all_ servers */
			crHashtableDelete(tilesort_spu.listTable, list, crFree);
		    }
		}

		if (needAnySend) {
			/* The bucketing info has already been set.  Just pack up
			 * all the compiled display list functions and they'll get sent
			 * to the servers that need them.
			 */
			 static const CRrecti fullscreen = {-CR_MAXINT, CR_MAXINT, -CR_MAXINT, CR_MAXINT};
			tilesortspuFlush( thread );
			crDLMSendList(list, &tilesort_spu.packerDispatch);
			send_info.pixelBounds = fullscreen;
			tilesortspuFlushToServers( thread, &send_info );
		}
	}

	if (thread->currentContext->providedBBOX == GL_DEFAULT_BBOX_CR) 
	{
		tilesortspuFlush( thread );
	}

	if (tilesort_spu.swap)
		crPackCallListSWAP( list );
	else
		crPackCallList( list );

	/* Executing glCallList */

	if (!crDLMListHasBounds(list)) {
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

	/* Only update state on servers if we actually sent the display list! */
	if (tilesort_spu.listTrack) {
		ContextInfo *context = thread->currentContext;
		CRContext *savedContext = crStateGetCurrent();
		int i;

		/* update servers' state records */
		for (i = 0; i < tilesort_spu.num_servers; i++) {
			const int node32 = i >> 5, node = i & 0x1f;
			if ((bucket_info.hits[node32] & (1 << node))) {
				crStateSetCurrent(context->server[i].State);
				crDLMReplayListState(list, &tilesort_spu.stateDispatch);
			}
		}

		/* Update our local, application context state */
		crStateSetCurrent(context->State);
		crDLMReplayListState(list, &tilesort_spu.stateDispatch);

		/* restore previously current context */
		crStateSetCurrent(savedContext);
	}

	/* If we used an auto bounding box, turn it off now */
	if (resetBBox)
		tilesortspu_ChromiumParametervCR(GL_DEFAULT_BBOX_CR, GL_FLOAT, 0, NULL);
}


void TILESORTSPU_APIENTRY
tilesortspu_CallLists(GLsizei n, GLenum type, const GLvoid *lists)
{
	GLint i;
	GET_THREAD(thread);
	GLenum dlMode = thread->currentContext->displayListMode;
	ContextInfo *context = thread->currentContext;
	const GLuint base = context->State->lists.base;

	if (dlMode != GL_FALSE) {
		if (tilesort_spu.lazySendDLists || tilesort_spu.listTrack)
			crDLMCompileCallLists(n, type, lists);
		else if (tilesort_spu.swap)
			crPackCallListsSWAP(n, type, lists);
		else
			crPackCallLists(n, type, lists);
		return;
	}


#define EXPAND(typeEnum, typeCast)                       \
    case typeEnum:                                       \
      {                                                  \
        const typeCast *data = (const typeCast *) lists; \
        for (i = 0; i < n; i++) {                        \
          tilesortspu_CallList(base + (GLuint) data[i]); \
        }                                                \
      }                                                  \
      break

	switch (type) {
		EXPAND(GL_BYTE, GLbyte);
		EXPAND(GL_UNSIGNED_BYTE, GLubyte);
		EXPAND(GL_SHORT, GLshort);
		EXPAND(GL_UNSIGNED_SHORT, GLushort);
		EXPAND(GL_INT, GLint);
		EXPAND(GL_UNSIGNED_INT, GLuint);
		EXPAND(GL_FLOAT, GLfloat);

		case GL_2_BYTES:
			{
				const GLubyte *data = (const GLubyte *) lists;
				for (i = 0; i < n; i++, data += 2)
					tilesortspu_CallList(base + 256 * data[0] + data[1]);
			}
			break;

		case GL_3_BYTES:
			{
				const GLubyte *data = (const GLubyte *) lists;
				for (i = 0; i < n; i++, data += 3)
					tilesortspu_CallList(base
															 + 256 * (256 * data[0] + data[1]) + data[2]);
			}
			break;

		case GL_4_BYTES:
			{
				const GLubyte *data = (const GLubyte *) lists;
				for (i = 0; i < n; i++, data += 4)
					tilesortspu_CallList(base
							 + 256 * (256 * (256 * data[0] + data[1]) + data[2]) + data[3]);
			}
			break;

		default:
			crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, "CallLists(type)");
			return;
	}
#undef EXPAND
}

/* GenLists is generated to do a pack/writeback to the server,
 * since the server has to know about all the distributed
 * display list IDs (in case there are multiple clients going
 * to one server, and they're all sharing a single display list
 * namespace)
 */

/**
 * Delete list info both from our state tracker and the DLM context.
 * NOTE: we're using the crStateGenLists and crStateIsList functions
 * otherwise.
 */
void TILESORTSPU_APIENTRY
tilesortspu_DeleteLists(GLuint list, GLsizei range)
{
	/*crDebug("DeleteLists(%d, %d)",list, range);*/
	/* always pack DeleteLists to send it to servers */
	if (tilesort_spu.swap) 
	{
		crPackDeleteListsSWAP( list, range );
	}
	else 
	{
		crPackDeleteLists( list, range );
	}
	crStateDeleteLists(list, range);

	if (tilesort_spu.listTrack || tilesort_spu.lazySendDLists)
		crDLMDeleteLists(list, range);

	if (tilesort_spu.lazySendDLists)
		crHashtableDeleteBlock(tilesort_spu.listTable, list, range, crFree);
}


/* This is custom because the DLM needs to know about any changes to the
 * list base, or it cannot manage CallLists elements correctly.
 */
void TILESORTSPU_APIENTRY
tilesortspu_ListBase(GLuint base)
{
	GET_THREAD(thread);
	if (thread->currentContext->displayListMode != GL_FALSE) {
		if (tilesort_spu.lazySendDLists || tilesort_spu.listTrack) {
			crDLMCompileListBase(base);
			if (tilesort_spu.lazySendDLists) return;
		}
		if (tilesort_spu.swap) {
			crPackListBaseSWAP(base);
		}
		else {
			crPackListBase(base);
		}
		return;
	}
	crStateListBase(base);
	crDLMListBase(base);
}
