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
tilesortspuStateNewList(GLuint list, GLuint mode) 
{
	/* the state tracker will do error checking and the flush for us. */
	if (tilesort_spu.replay)
	{
		if (tilesort_spu.swap)
			crPackNewListSWAP(list, mode);
		else
			crPackNewList(list, mode);
		crStateNewList( list, mode );
	}
}

void TILESORTSPU_APIENTRY
tilesortspuStateEndList(void) 
{
	if (tilesort_spu.replay) 
	{
		if (tilesort_spu.swap)
			crPackEndListSWAP();
		else
			crPackEndList();
		crStateEndList();
	}
}

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
	GLboolean *listSent = NULL;

	c->buffer.geometry_only = GL_FALSE;
	/*crDebug("tilesortspu_NewList(%d,%d)",list,mode); */

	/* the state tracker will do error checking and the flush for us. */
	crStateNewList( list, mode );

	/* GL_COMPILE_AND_EXECUTE confuses the server! */
	mode = GL_COMPILE;

	if (tilesort_spu.lazySendDLists)
	{
		/*
		 * Implement lazy display list upload.
		 * Don't send display list to server X, until we actually need to
		 * execute it there.
		 * Cache display list
		 * Swap to packerDispatch and cache until EndList
		 */
		/* the state tracker will do error checking and the flush for us. */
		if (tilesort_spu.replay)
		{
			if (tilesort_spu.swap)
				crPackNewListSWAP(list, mode);
			else
				crPackNewList(list, mode);
		} else {
			listSent = crCalloc(tilesort_spu.num_servers * sizeof(GLboolean));
			crHashtableAdd(tilesort_spu.listTable, list, listSent);

			crdlm_NewList(list, mode, &(tilesort_spu.self));
			crdlm_compile_NewList( list, mode );
		}
	} 
	else 
	{
		/* pack/send the display list now so all servers receive it */
		if (tilesort_spu.swap)
			crPackNewListSWAP(list, mode);
		else
			crPackNewList(list, mode);

		if (tilesort_spu.listTrack)
			crdlm_NewList(list, mode, &(tilesort_spu.self));
	}

	if (!tilesort_spu.listTrack && !tilesort_spu.lazySendDLists) {
		tilesortspuLoadListTable();
	}

	if (tilesort_spu.autoDListBBoxes && !tilesort_spu.listTrack) {
		GET_THREAD(thread);
		crPackResetBBOX(thread->packer);
	}
}



void TILESORTSPU_APIENTRY
tilesortspu_EndList(void) 
{
	GET_CONTEXT(ctx);
	CRPackContext *c = crPackGetContext();
	const GLuint mode = ctx->lists.mode;
	const GLuint list = ctx->lists.currentIndex;

	/*crDebug("tilesortspu_EndList()"); */

	if (!tilesort_spu.replay) 
	{
		if (tilesort_spu.lazySendDLists) {
			crdlm_compile_EndList();
		}
		if (tilesort_spu.listTrack || tilesort_spu.lazySendDLists)
			crdlm_EndList();
	}
	crStateEndList();

	if (tilesort_spu.lazySendDLists)
	{
		if (tilesort_spu.replay) 
		{
			if (tilesort_spu.swap)
				crPackEndListSWAP();
			else
				crPackEndList();
		} else {
			/* restore normal/default dispatch table */
			crdlm_RestoreDispatch();  /* companion to crdlm_EndList() */

			/* Turn the geometry_only flag back on.
			 * See the longer comment above in tilesortspu_NewList().
			 */
			c->buffer.geometry_only = GL_TRUE;

			if (mode == GL_COMPILE_AND_EXECUTE) 
			{
				if (tilesort_spu.swap)
					crPackEndListSWAP();
				else
					crPackEndList();

				tilesortspuBroadcastGeom(0);

				/* we really used GL_COMPILE mode.  Now, replay/execute the list */
				tilesortspu_CallList(list);
			}
		}
	} 
	else {
		/* now play the list through the packer to send it to the servers */
		if (tilesort_spu.listTrack)
			crdlm_CallList(list, &tilesort_spu.packerDispatch);

		/* the state tracker will do error checking and the flush for us. */
		if (tilesort_spu.swap)
			crPackEndListSWAP();
		else
			crPackEndList();

		/* restore normal/default dispatch table */
		if (tilesort_spu.listTrack)
			crdlm_RestoreDispatch();  /* companion to crdlm_EndList() */
		else {
			tilesortspuLoadSortTable();
		}

		tilesortspuBroadcastGeom(0);

		/* Turn the geometry_only flag back on.
	 	 * See the longer comment above in tilesortspu_NewList().
	 	 */
		c->buffer.geometry_only = GL_TRUE;

		if (mode == GL_COMPILE_AND_EXECUTE) 
		{
			/* we really used GL_COMPILE mode.  Now, replay/execute the list */
			tilesortspu_CallList(list);
		}
	}

	/* Set the list's bounding box */
	if (thread->currentContext->providedBBOX == GL_OBJECT_BBOX_CR) {
		/*CRASSERT(thread->packer->updateBBOX == 0);*/
		/* use the user-provided bounding box */
		crDLMSetBounds(tilesort_spu.dlm, list,
									 thread->packer->bounds_min.x,
									 thread->packer->bounds_min.y,
									 thread->packer->bounds_min.z,
									 thread->packer->bounds_max.x,
									 thread->packer->bounds_max.y,
									 thread->packer->bounds_max.z);
	}
	else if (tilesort_spu.autoDListBBoxes) {
		if (tilesort_spu.listTrack) {
			/* The DLM will have already recorded the list's bounding box */
		}
		else {
			/* use the packer's bounding box */
			crDLMSetBounds(tilesort_spu.dlm, list,
										 thread->packer->bounds_min.x,
										 thread->packer->bounds_min.y,
										 thread->packer->bounds_min.z,
										 thread->packer->bounds_max.x,
										 thread->packer->bounds_max.y,
										 thread->packer->bounds_max.z);
		}
	}
	else {
		/* set NULL bounding box - lists will get broadcast */
		crDLMSetBounds(tilesort_spu.dlm, list,
									 FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
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

	/*crDebug("tilesortspuStateCallList( %u )", list); */

	/* NOTE: this function may get called recursively when calling a nested
	 * display list.  Therefore, we have to save the current context when
	 * we're called and restore it when we leave.
	 */

	/* update servers' state records */
	for (i = 0; i < tilesort_spu.num_servers; i++) 
	{
		/*crDebug("tilesortspuStateCallList( server = %d )", i ); */
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

	/*crDebug("tilesortspuStateCallLists( %d, %d, *lists)", n, type ); */

	/* NOTE: this function may get called recursively when calling a nested
	 * display list.  Therefore, we have to save the current context when
	 * we're called and restore it when we leave.
	 */

	/* update servers' state records */
	for (i = 0; i < tilesort_spu.num_servers; i++) 
	{
		/*crDebug("tilesortspuStateCallLists( server = %d )", i ); */
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
	GET_THREAD(thread);
	WindowInfo *winInfo = thread->currentContext->currentWindow;
	GLboolean resetBBox = GL_FALSE;
	TileSortBucketInfo bucket_info;

	/*
	 * check if compiling this call, or executing it
	 */
	if (thread->currentContext->State->lists.mode != 0) {
		/* we're compiling a glCallList into another display list */
		if (tilesort_spu.listTrack && tilesort_spu.lazySendDLists)
			crdlm_compile_CallList( list );
	}
	else {
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
				crPackSetBuffer( thread->packer, &(thread->buffer[i]) );
				crStateDiffAllTextureObjects(thread->currentContext->State,
																		 serverState->bitid);
				crPackReleaseBuffer( thread->packer );
			}
		crPackSetBuffer( thread->packer, &(thread->geometry_buffer) );
		}
#endif

		/*
		 * Determine which servers will get this display list.
		 */
		if (crDLMListHasBounds(tilesort_spu.dlm, list)) {
			CRDLMBounds bounds;
			GLfloat bbox[6];

			crDLMGetBounds(tilesort_spu.dlm, list, &bounds);

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

		/* 
		 * If we're sending display lists lazily, check if we need to send this
		 * list to any new servers.
		 */
		if (tilesort_spu.lazySendDLists && !crDLMIsListSent(tilesort_spu.dlm, list)) {
			GLboolean *listSent
				= (GLboolean *) crHashtableSearch(tilesort_spu.listTable, list);
			GLboolean needAnySend = GL_FALSE;
			TileSortBucketInfo send_info;
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
				crDLMListSent(tilesort_spu.dlm, list);
			}

			if (needAnySend) {
				 static const CRrecti fullscreen = {-CR_MAXINT, CR_MAXINT, -CR_MAXINT, CR_MAXINT};
				tilesortspuFlush( thread );
				tilesort_spu.replay = 1;
				crdlm_CallList(list, &tilesort_spu.packerDispatch);
				tilesort_spu.replay = 0;
				send_info.pixelBounds = fullscreen;
				tilesortspuFlushToServers( thread, &send_info );
			}
		}

		if (thread->currentContext->providedBBOX == GL_DEFAULT_BBOX_CR) 
		{
			tilesortspuFlush( thread );
		}
	}


	/* Always pack CallList.  It'll get sent to servers based on the usual
	 * bucketing routine.
	 */
	if (tilesort_spu.swap)
	   crPackCallListSWAP( list );
	else
	   crPackCallList( list );

	if (thread->currentContext->State->lists.mode == 0) 
	{
		/* Executing glCallList */

		if (!crDLMListHasBounds(tilesort_spu.dlm, list)) {
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
					crdlm_CallList(list, &tilesort_spu.stateDispatch);
				}
			}

			/* Update our local, application context state */
			crStateSetCurrent(context->State);
			crdlm_CallList(list, &tilesort_spu.stateDispatch);

			/* restore previously current context */
			crStateSetCurrent(savedContext);
		}
	}

	/* If we used an auto bounding box, turn it off now */
	if (resetBBox)
		tilesortspu_ChromiumParametervCR(GL_DEFAULT_BBOX_CR, GL_FLOAT, 0, NULL);
}


void TILESORTSPU_APIENTRY
tilesortspu_CallLists(GLsizei n, GLenum type, const GLvoid *lists)
{
#if 1111
	/* Not really efficient, but reliable */
	GET_THREAD(thread);
	ContextInfo *context = thread->currentContext;
	const GLuint base = context->State->lists.base;
	GLint i;

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
#undef EXPAN
#else

	GET_CONTEXT(ctx);
#if 0
	ContextInfo *context = thread->currentContext;
	CRContext *savedContext = crStateGetCurrent();
#endif

	/*crDebug("tilesortspu_CallLists(%d, %d, *lists)",n, type ); */

	/*
	 * XXX we have no support for auto bounding boxes here -- someday?
	 */

	if (thread->currentContext->State->lists.mode == 0) 
	{
		/* 
		 * If lazy then replay display list and send it out now
		 */
		if (tilesort_spu.lazySendDLists)
		{
			/*crDebug("REPLAY LAZY Lists n = %u",n); */
			/* now play the list through the packer to send it to the servers 
			 * cache testing is done in crdlm_LazyCallLists
			 */

			tilesort_spu.replay = 1;
			crdlm_LazyCallLists(n, type, lists, &tilesort_spu.packerDispatch);
			tilesort_spu.replay = 0;
		
		}

		/* Execute glCallList */
		if (thread->currentContext->providedBBOX == GL_DEFAULT_BBOX_CR) 
		{
			tilesortspuFlush( thread );
		}
	}
	else 
	{
		/* we're compiling glCallLists into another display list */
		if (tilesort_spu.listTrack && tilesort_spu.lazySendDLists)
			crdlm_compile_CallLists(n, type, lists);
	}


	/* always pack CallLists to send it to servers */
	if (tilesort_spu.swap) 
	{
		crPackListBaseSWAP( ctx->lists.base );
		crPackCallListsSWAP( n, type, lists );
	}
	else 
	{
		crPackListBase( ctx->lists.base );
		crPackCallLists( n, type, lists );
	}

	if (thread->currentContext->State->lists.mode == 0) 
	{
		/* Execute glCallLists */

		if (thread->currentContext->providedBBOX == GL_DEFAULT_BBOX_CR) 
	{
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


		/*  only update state on servers if we actually sent the display list!
		 */
		if (tilesort_spu.listTrack && tilesort_spu.lazySendDLists) {
			crdlm_ListBase(ctx->lists.base);
			tilesortspuStateCallLists(n, type, lists);
		}
	}
#endif
}


/*
 * 
 */
GLuint TILESORTSPU_APIENTRY
tilesortspu_GenLists(GLsizei range)
{
	GET_THREAD(thread);
	GLuint i;
	GLuint return_value = 0;

	/*crDebug("GenLists(%d)", range);*/
	/* always pack GenLists to send it to servers */
	crPackReleaseBuffer( thread->packer );
	for ( i = 0; i < (unsigned int)tilesort_spu.num_servers ; i++ )
	{
		int writeback = 1;

		crPackSetBuffer ( thread->packer, &(thread->buffer[i]) );

		if (tilesort_spu.swap) 
		{
			crPackGenListsSWAP( range , &return_value, &writeback );
		}
		else 
		{
			crPackGenLists( range, &return_value, &writeback  );
		}

		crStateGenLists(range);
		crPackReleaseBuffer ( thread->packer );

	        /* Flush buffer (send to server) */
                tilesortspuSendServerBuffer( i );

		/* Get return value */
		while (writeback) {
			crNetRecv();
		}

		if (tilesort_spu.swap)
			return_value = (GLint) SWAP32(return_value);

		if (!return_value)
			return -1;  /* something went wrong on the server */

	}

	/* The default geometry pack buffer */
	crPackSetBuffer( thread->packer, &(thread->geometry_buffer) );

	if (tilesort_spu.listTrack && tilesort_spu.lazySendDLists)
		crdlm_GenLists(range);

	if (tilesort_spu.lazySendDLists)
		crHashtableAllocKeys(tilesort_spu.listTable, range);
	return return_value;
}

/*
 * Delete list info both from our state tracker and the DLM context.
 * NOTE: we're using the crStateGenLists and crStateIsList functions
 * otherwise.
 */
void TILESORTSPU_APIENTRY
tilesortspu_DeleteLists(GLuint list, GLsizei range)
{
	GLuint i;

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

	if (tilesort_spu.listTrack && tilesort_spu.lazySendDLists)
		crdlm_DeleteLists(list, range);

	if (tilesort_spu.lazySendDLists)
		for (i = list; i <= list + range -1; i++)
			crHashtableDelete(tilesort_spu.listTable, i, crFree);
}


/* XXX fix and plug into dispatcher */
void TILESORTSPU_APIENTRY
tilesortspu_ListBase(GLuint base)
{
	if (tilesort_spu.swap) 
	{
		crPackListBaseSWAP( base );
	}
	else 
	{
		crPackListBase( base );
	}
	crStateListBase(base);
	if (tilesort_spu.listTrack && tilesort_spu.lazySendDLists)
		crdlm_ListBase(base);
}
