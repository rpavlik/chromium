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
	int *listSent = NULL;

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

			listSent = crCalloc(tilesort_spu.num_servers * sizeof(int));
			crHashtableAdd(tilesort_spu.listTable, list, listSent);

			crdlm_NewList(list, mode, &(tilesort_spu.self));
			crdlm_compile_NewList( list, mode );
		}

	} 
	else 
	{
		if (tilesort_spu.swap)
			crPackNewListSWAP(list, mode);
		else
			crPackNewList(list, mode);

		if (tilesort_spu.listTrack)
			crdlm_NewList(list, mode, &(tilesort_spu.self));

	}
	if (!tilesort_spu.listTrack)
		tilesortspuLoadListTable();
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
		if (tilesort_spu.lazySendDLists) 
			crdlm_compile_EndList();
		if (tilesort_spu.listTrack)
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
		else
			tilesortspuLoadSortTable();

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
	int i;
	int *listSent = NULL;

	/*crDebug("tilesortspu_CallList(%u)", list); */
	if (tilesort_spu.lazySendDLists)
		listSent = crHashtableSearch(tilesort_spu.listTable, list);

	/*if(listSent == NULL) */
	/*	crDebug("TILT"); */

	/* if (tilesort_spu.lazySendDLists) */
	/* 	for(i = 0;i < tilesort_spu.num_servers; i++)  */
	/* 		crDebug("list = %d listSent[%d] = %d",list, i, listSent[i]); */

	if (thread->currentContext->State->lists.mode == 0) 
	{
		/* 
		 * If lazy then replay display list and send it out now
		 */
		if (tilesort_spu.lazySendDLists && ! tilesort_spu.autoDListBBoxes && tilesort_spu.listTrack)
		{
			if(!crDLMIsListSent(tilesort_spu.dlm, list)) 
			{
				/*crDebug("REPLAY LAZY list = %u",list); */
				/* now play the list through the packer to send it to the servers */
	
				tilesortspuFlush( thread );

				tilesort_spu.replay = 1;
				crDLMListSent(tilesort_spu.dlm, list);
				crdlm_CallList(list, &tilesort_spu.packerDispatch);
				tilesort_spu.replay = 0;
				tilesortspuFlush( thread );
		
			}

		}

		/* Execute glCallList */

		/* Try to prefix this display list with a bounding box.
		 * But only if the user hasn't already specified his own bbox AND if
		 * the autoDListBBoxes option is turned on.
		 */
		if ((thread->currentContext->providedBBOX == GL_DEFAULT_BBOX_CR && tilesort_spu.autoDListBBoxes) ||
		     thread->currentContext->providedBBOX == GL_OBJECT_BBOX_CR)
		{
			CRDLMBounds bounds;


			if (tilesort_spu.listTrack
					&& crDLMGetBounds(tilesort_spu.dlm, list, &bounds) == GL_NO_ERROR
					&& bounds.xmin != FLT_MAX)
			{
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
				if ((thread->currentContext->providedBBOX == GL_DEFAULT_BBOX_CR && tilesort_spu.autoDListBBoxes)) 
				{
					tilesortspu_ChromiumParametervCR(GL_OBJECT_BBOX_CR, GL_FLOAT, 6, bbox);
					resetBBox = GL_TRUE;
				}

				if (tilesort_spu.lazySendDLists)
				{

					/* now play the list through the packer to send it to the servers */
		
					tilesortspuBucketGeometry(winInfo, &bucket_info);
					/* Now, see if we need to do things to each server */

					for ( i = 0 ; i < tilesort_spu.num_servers; i++ )
					{
						int node32 = i >> 5;
						int node = i & 0x1f;

						/* Check to see if this server needs geometry from us. */
						if (listSent[i] == 1)
						{
							/*crDebug( "listSent NOT sending list %d to server %d", list, i );*/
							continue;
						}
						if (!(bucket_info.hits[node32] & (1 << node)))
						{
							/*crDebug( "Bucket NOT sending list %d to server %d", list, i );*/
							continue;
						}
						/*crDebug( "Sending list %d to server %d", list, i ); */
						tilesortspuFlush( thread );

						tilesort_spu.replay = 1;
						listSent[i] = 1;
						crdlm_CallList(list, &tilesort_spu.packerDispatch);
						tilesort_spu.replay = 0;
					}

				}
			}
		}

		if (thread->currentContext->providedBBOX == GL_DEFAULT_BBOX_CR) 
		{
			tilesortspuFlush( thread );
		}
	}
	else 
	{
		/* we're compiling a glCallList into another display list */
		if (tilesort_spu.listTrack && tilesort_spu.lazySendDLists)
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

	if (thread->currentContext->State->lists.mode == 0) 
	{
		/* Execute glCallList */

		if (thread->currentContext->providedBBOX == GL_DEFAULT_BBOX_CR) 
		{
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

		/* only update state on servers if we actually sent the display list!
		 */

		/*crDebug("tilesortspu_CallList(%u) call tilesortspuStateCallList", list); */

		if (tilesort_spu.listTrack && tilesort_spu.lazySendDLists) 
		    for( i = 0; i < tilesort_spu.num_servers; i++)
		    {
			if(listSent && listSent[i] == 0) {
				/*crDebug( "list state sending list %d to server %d", list, i );*/
				tilesortspuStateCallList(list);
			} else
				tilesortspuStateCallList(list);
		    }
	}

	/* If we used an auto bounding box, turn it off now */
	if (resetBBox)
		tilesortspu_ChromiumParametervCR(GL_DEFAULT_BBOX_CR, GL_FLOAT, 0, NULL);
}


void TILESORTSPU_APIENTRY
tilesortspu_CallLists(GLsizei n, GLenum type, const GLvoid *lists)
{
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
		for( i = list; i <= list + range -1; i++)
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


/*
 * Need to delete local textures and textures on servers.
 * XXX this function should probably go elsewhere.
 */
void TILESORTSPU_APIENTRY
tilesortspu_DeleteTextures(GLsizei n, const GLuint *textures)
{
	crStateDeleteTextures(n, textures);
	crPackDeleteTextures(n, textures);
}


/*
 * Need to delete local programs and programs on servers.
 * XXX this function should probably go elsewhere.
 */
void TILESORTSPU_APIENTRY
tilesortspu_DeleteProgramsARB(GLsizei n, const GLuint *programs)
{
	crStateDeleteProgramsARB(n, programs);
	crPackDeleteProgramsARB(n, programs);
}

