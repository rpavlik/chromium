/* Copyright (c) 2001-2003, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */

#include "server_dispatch.h"
#include "server.h"
#include "cr_mem.h"


/*
 * Notes on ID translation:
 *
 * If a server has multiple clients (in the case of parallel applications)
 * and N of the clients all create a display list with ID K, does K name
 * one display list or N different display lists?
 *
 * By default, there is one display list named K.  If the clients put
 * identical commands into list K, then this is fine.  But if the clients
 * each put something different into list K when they created it, then this
 * is a serious problem.
 *
 * By zeroing the 'shared_display_lists' configuration option, we can tell
 * the server to make list K be unique for all N clients.  We do this by
 * translating K into a new, unique ID dependant on which client we're
 * talking to (curClient->number).
 *
 * Same story for texture objects, vertex programs, etc.
 *
 * The application can also dynamically switch between shared and private
 * display lists with:
 *   glChromiumParameteri(GL_SHARED_DISPLAY_LISTS_CR, GL_TRUE)
 * and
 *   glChromiumParameteri(GL_SHARED_DISPLAY_LISTS_CR, GL_FALSE)
 *
 */



static GLuint TranslateListID( GLuint id )
{
	if (!cr_server.sharedDisplayLists) {
		int client = cr_server.curClient->number;
		return id + client * 100000;
	}
	return id;
}


static GLuint TranslateTextureID( GLuint id )
{
	if (!cr_server.sharedTextureObjects && id) {
		int client = cr_server.curClient->number;
		return id + client * 100000;
	}
	return id;
}


void SERVER_DISPATCH_APIENTRY crServerDispatchNewList( GLuint list, GLenum mode )
{
	if (mode == GL_COMPILE_AND_EXECUTE)
		crWarning("using glNewList(GL_COMPILE_AND_EXECUTE) can confuse the crserver");

	list = TranslateListID( list );
	crStateNewList( list, mode );
	cr_server.head_spu->dispatch_table.NewList( list, mode );
}

void SERVER_DISPATCH_APIENTRY crServerDispatchCallList( GLuint list )
{
	list = TranslateListID( list );

	if (cr_server.curClient->currentCtx->lists.mode == 0) {
		/* we're not compiling, so execute the list now */
		CRMuralInfo *mural = cr_server.curClient->currentMural;
		int i;

		if (!mural->viewportValidated) {
			crServerComputeViewportBounds(&(cr_server.curClient->currentCtx->viewport), mural);
		}

		if (mural->numExtents == 0) {
			/* Issue the list as-is */
			cr_server.head_spu->dispatch_table.CallList( list );
		}
		else {
			/* Loop over the extents (tiles) calling glCallList() */
			for ( i = 0; i < mural->numExtents; i++ )	{
				if (cr_server.run_queue->client->currentCtx)
					crServerSetOutputBounds( mural, i );
				cr_server.head_spu->dispatch_table.CallList( list );
			}
		}
	}
	else {
		/* we're compiling glCallList into another list - just pass it through */
		cr_server.head_spu->dispatch_table.CallList( list );
	}
}

void SERVER_DISPATCH_APIENTRY crServerDispatchCallLists( GLsizei n, GLenum type, const GLvoid *lists )
{
	/* XXX not sure what to do here, in terms of ID translation */

	if (cr_server.curClient->currentCtx->lists.mode == 0) {
		/* we're not compiling, so execute the list now */
		CRMuralInfo *mural = cr_server.curClient->currentMural;
		int i;

		if (!mural->viewportValidated) {
			crServerComputeViewportBounds(&(cr_server.curClient->currentCtx->viewport), mural);
		}

		if (mural->numExtents == 0) {
			/* Issue the list as-is */
			cr_server.head_spu->dispatch_table.CallLists( n, type, lists );
		}
		else {
			/* Loop over the extents (tiles) calling glCallList() */
			for ( i = 0; i < mural->numExtents; i++ ) {
				if (cr_server.run_queue->client->currentCtx)
					crServerSetOutputBounds( mural, i );
				cr_server.head_spu->dispatch_table.CallLists( n, type, lists );
			}
		}
	}
	else {
		/* we're compiling glCallList into another list - just pass it through */
		cr_server.head_spu->dispatch_table.CallLists( n, type, lists );
	}
}

GLboolean SERVER_DISPATCH_APIENTRY crServerDispatchIsList( GLuint list )
{
	GLboolean retval;
	list = TranslateListID( list );
	crServerReturnValue( &retval, sizeof(retval) );
	retval = cr_server.head_spu->dispatch_table.IsList( list );
	return retval;
}


void SERVER_DISPATCH_APIENTRY crServerDispatchDeleteLists( GLuint list, GLsizei range )
{
	list = TranslateListID( list );
	crStateDeleteLists( list, range );
	cr_server.head_spu->dispatch_table.DeleteLists( list, range );
}


void SERVER_DISPATCH_APIENTRY crServerDispatchBindTexture( GLenum target, GLuint texture )
{
	texture = TranslateTextureID( texture );
	crStateBindTexture( target, texture );
	cr_server.head_spu->dispatch_table.BindTexture( target, texture );
}


void SERVER_DISPATCH_APIENTRY crServerDispatchDeleteTextures( GLsizei n, const GLuint *textures)
{
	if (!cr_server.sharedTextureObjects) {
		GLuint *newTextures = (GLuint *) crAlloc(n * sizeof(GLuint));
		GLint i;
		if (!newTextures) {
			/* XXX out of memory error */
			return;
		}
		for (i = 0; i < n; i++) {
			newTextures[i] = TranslateTextureID( textures[i] );
		}
		crStateDeleteTextures( n, newTextures );
		cr_server.head_spu->dispatch_table.DeleteTextures( n, newTextures );
		crFree(newTextures);
	}
	else {
		crStateDeleteTextures( n, textures );
		cr_server.head_spu->dispatch_table.DeleteTextures( n, textures );
	}
}


GLboolean SERVER_DISPATCH_APIENTRY crServerDispatchIsTexture( GLuint texture )
{
	GLboolean retval;
	texture = TranslateTextureID( texture );
	retval = cr_server.head_spu->dispatch_table.IsTexture( texture );
	crServerReturnValue( &retval, sizeof(retval) );
	return retval; /* WILL PROBABLY BE IGNORED */
}


/* XXX todo: PrioritizeTextures and AreTextureResident */

/* XXX todo: vertex program IDs */

