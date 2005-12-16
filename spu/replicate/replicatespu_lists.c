/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */
	

#include <stdio.h>
#include "cr_server.h"
#include "cr_packfunctions.h"
#include "replicatespu.h"
#include "cr_dlm.h"
#include "replicatespu_proto.h"

GLuint REPLICATESPU_APIENTRY
replicatespu_GenLists( GLsizei range )
{
	/* Don't get IDs from the crservers since different servers will
	 * return different IDs.
	 */
	return crDLMGenLists(range);
}


void REPLICATESPU_APIENTRY replicatespu_NewList(GLuint listIdentifier, GLenum mode)
{
	GET_THREAD(thread);

	if (listIdentifier == 0) {
	    crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, 
		"replicate: NewList with zero list identifier");
	    return;
	}
	if (mode != GL_COMPILE && mode != GL_COMPILE_AND_EXECUTE) {
	    crStateError(__LINE__, __FILE__, GL_INVALID_ENUM, 
		"replicate: NewList with bad mode (0x%x)", mode);
	    return;
	}
	if (thread->currentContext->displayListMode != GL_FALSE) {
	    crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, 
		"replicate: NewList called while list %d was open",
		thread->currentContext->displayListIdentifier);
	    return;
	}

	/* All okay.  Tell the state tracker and the DLM that we've
	 * started a new display list.
	 */
	crStateNewList(listIdentifier, mode);
	crDLMNewList(listIdentifier, mode);

	/* Remember ourselves, too, so we don't have to waste a function
	 * call to figure out whether we're in a list or not.
	 */
	thread->currentContext->displayListMode = mode;
	thread->currentContext->displayListIdentifier = listIdentifier;

	/* Pack it up so we can send it to all the connected
	 * VNC clients.
	 */
	if (replicate_spu.swap)
	{
		crPackNewListSWAP(listIdentifier, mode);
	}
	else
	{
		crPackNewList(listIdentifier, mode);
	}
}

void REPLICATESPU_APIENTRY replicatespu_EndList(void)
{
	GET_THREAD(thread);

	if (thread->currentContext->displayListMode == GL_FALSE) {
	    crStateError(__LINE__, __FILE__, GL_INVALID_OPERATION, 
		"replicate: EndList called without a preceding NewList");
	    return;
	}

	/* All okay.  Tell the state tracker and the DLM that we've
	 * finished a new display list.
	 */
	crStateEndList();
	crDLMEndList();

	/* Remember ourselves, too, so we don't have to waste a function
	 * call to figure out whether we're in a list or not.
	 */
	thread->currentContext->displayListMode = GL_FALSE;
	thread->currentContext->displayListIdentifier = 0;

	/* Pack it up so we can send it to all the connected
	 * VNC clients.
	 */
	if (replicate_spu.swap)
	{
		crPackEndListSWAP();
	}
	else
	{
		crPackEndList();
	}
}

void REPLICATESPU_APIENTRY replicatespu_DeleteLists(GLuint listIdentifier, GLsizei range)
{
	if (range < 0) {
	    crStateError(__LINE__, __FILE__, GL_INVALID_VALUE, 
		"replicate: DeleteLists with negative range (%d)",
		range);
	    return;
	}

	crStateDeleteLists(listIdentifier, range);
	crDLMDeleteLists(listIdentifier, range);

	if (replicate_spu.swap)
	{
		crPackDeleteListsSWAP(listIdentifier, range);
	}
	else
	{
		crPackDeleteLists(listIdentifier, range);
	}
}

/* When we call a remote list, we'll duplicate any state effects
 * caused by the list in the local state records.
 */
void REPLICATESPU_APIENTRY replicatespu_CallList( GLuint list )
{
	GET_THREAD(thread);

	/* Compile if we're supposed to do so. */
	if (thread->currentContext->displayListMode != GL_FALSE) {
		crDLMCompileCallList(list);
	}
	if (replicate_spu.swap)
	{
		crPackCallListSWAP(list);
	}
	else
	{
		crPackCallList(list);
	}

	/* If we've just packed a call that is going to immediately be
	 * executed (i.e. one that isn't just being compiled), make
	 * the appropriate state change on our end, so we're aware of it.
	 */
	if (thread->currentContext->displayListMode != GL_COMPILE) {
	    crDLMReplayListState(list, &replicate_spu.state_dispatch);
	}
}

/* This function is called through the state update API when we hit a
 * compiled CallList function.  It executes the necessary state.
 */
void REPLICATESPU_APIENTRY replicatespu_StateCallList( GLuint list )
{
    crDLMReplayListState(list, &replicate_spu.state_dispatch);
}

void REPLICATESPU_APIENTRY replicatespu_CallLists( GLsizei n, GLenum type, const GLvoid * lists )
{
	GET_THREAD(thread);
	if (thread->currentContext->displayListMode != GL_FALSE) {
		crDLMCompileCallLists(n, type, lists);
	}
	if (replicate_spu.swap)
	{
		crPackCallListsSWAP(n, type, lists);
	}
	else
	{
		crPackCallLists(n, type, lists);
	}
	/* If we've just packed a call that is going to immediately be
	 * executed (i.e. one that isn't just being compiled), make
	 * the appropriate state change on our end, so we're aware of it.
	 */
	if (thread->currentContext->displayListMode != GL_COMPILE) {
	    crDLMReplayListsState(n, type, lists, &replicate_spu.state_dispatch);
	}
}

/* This function is called through the state update API when we hit a
 * compiled CallLists function.  It executes the necessary state.
 */
void REPLICATESPU_APIENTRY replicatespu_StateCallLists( GLsizei n, GLenum type, const GLvoid * lists )
{
    crDLMReplayListsState(n, type, lists, &replicate_spu.state_dispatch);
}

/* The DLM has to be told of the ListBase function, so it can correctly
 * manage CallLists invocations.
 */
void REPLICATESPU_APIENTRY replicatespu_ListBase( GLuint base )
{
	GET_THREAD(thread);
	if (thread->currentContext->displayListMode != GL_FALSE) {
		crDLMCompileListBase(base);
	}
	if (replicate_spu.swap)
	{
		crPackListBaseSWAP(base);
	}
	else
	{
		crPackListBase(base);
	}
	if (thread->currentContext->displayListMode != GL_COMPILE) {
		crStateListBase( base );
		crDLMListBase(base);
	}
}
