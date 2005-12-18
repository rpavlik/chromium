/* Copyright (c) 2001-2003, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */

#include "server_dispatch.h"
#include "server.h"
#include "cr_mem.h"


/*
 * Notes on ID translation:
 *
 * The server, in serializing multiple remote streams into a single
 * context, only has one set of context-specific resources (simple state,
 * display lists, textures, etc.).
 *
 * Simple state is maintained in software.  Display lists and textures
 * are maintained differently.  The server context keeps *all* the display
 * lists and textures stored by any context & client, and uses a translator
 * to keep track of the mapping between each context's set of IDs and the
 * server's master set of IDs.
 */

static GLuint TranslateTextureID( GLuint id )
{
	if (!cr_server.sharedTextureObjects && id) {
		int client = cr_server.curClient->number;
		return id + client * 100000;
	}
	return id;
}

/* XXXX Note: shared/separate Program ID numbers aren't totally implemented! */
static GLuint TranslateProgramID( GLuint id )
{
	if (!cr_server.sharedPrograms && id) {
		int client = cr_server.curClient->number;
		return id + client * 100000;
	}
	return id;
}


void SERVER_DISPATCH_APIENTRY
crServerDispatchNewList( GLuint list, GLenum mode )
{
	GLuint translatedList;
	if (mode == GL_COMPILE_AND_EXECUTE)
		crWarning("using glNewList(GL_COMPILE_AND_EXECUTE) can confuse the crserver");

	/* If this is the ID of a list that already exists, re-use the earlier ID */
	translatedList = crTranslateListId(cr_server.curClient->currentTranslator, list);
	if (translatedList == 0) {
	    /* Here, the list ID isn't already in use.  Find a new ID for it to
	     * use, and install it in the translator.
	     */
	    translatedList = cr_server.head_spu->dispatch_table.GenLists(1);
	    crTranslateAddListId(cr_server.curClient->currentTranslator, list, translatedList);
	}
	    
	/* Continue with the device's list ID */
	crStateNewList( list, mode );
	cr_server.head_spu->dispatch_table.NewList( translatedList, mode );
}


void SERVER_DISPATCH_APIENTRY
crServerDispatchCallList( GLuint list )
{
	GLuint translatedList = crTranslateListId(cr_server.curClient->currentTranslator, list);

	if (cr_server.curClient->currentCtx->lists.mode == 0) {
		/* we're not compiling, so execute the list now */
		CRMuralInfo *mural = cr_server.curClient->currentMural;
		int i;

		if (!mural->viewportValidated) {
			crServerComputeViewportBounds(&(cr_server.curClient->currentCtx->viewport), mural);
		}

		if (mural->numExtents == 0) {
			/* Issue the list as-is */
			cr_server.head_spu->dispatch_table.CallList( translatedList );
		}
		else {
			/* Loop over the extents (tiles) calling glCallList() */
			for ( i = 0; i < mural->numExtents; i++ )	{
				if (cr_server.run_queue->client->currentCtx)
					crServerSetOutputBounds( mural, i );
				cr_server.head_spu->dispatch_table.CallList( translatedList );
			}
		}
	}
	else {
		/* we're compiling glCallList into another list - just pass it through */
		cr_server.head_spu->dispatch_table.CallList( translatedList );
	}
}


/**
 * Translate an array of display list IDs from various datatypes to GLuint
 * IDs while adding the per-client offset.
 */
static void
TranslateListIDs(GLsizei n, GLenum type, const GLvoid *lists, GLuint *newLists)
{
	GLsizei i;
	GLuint listBase = cr_server.curClient->currentCtx->lists.base;

	switch (type) {
	case GL_UNSIGNED_BYTE:
		{
			const GLubyte *src = (const GLubyte *) lists;
			for (i = 0; i < n; i++) {
				newLists[i] = crTranslateListId(cr_server.curClient->currentTranslator, src[i] + listBase);
			}
		}
		break;
	case GL_BYTE:
		{
			const GLbyte *src = (const GLbyte *) lists;
			for (i = 0; i < n; i++) {
				newLists[i] = crTranslateListId(cr_server.curClient->currentTranslator, src[i] + listBase);
			}
		}
		break;
	case GL_UNSIGNED_SHORT:
		{
			const GLushort *src = (const GLushort *) lists;
			for (i = 0; i < n; i++) {
				newLists[i] = crTranslateListId(cr_server.curClient->currentTranslator, src[i] + listBase);
			}
		}
		break;
	case GL_SHORT:
		{
			const GLshort *src = (const GLshort *) lists;
			for (i = 0; i < n; i++) {
				newLists[i] = crTranslateListId(cr_server.curClient->currentTranslator, src[i] + listBase);
			}
		}
		break;
	case GL_UNSIGNED_INT:
		{
			const GLuint *src = (const GLuint *) lists;
			for (i = 0; i < n; i++) {
				newLists[i] = crTranslateListId(cr_server.curClient->currentTranslator, src[i] + listBase);
			}
		}
		break;
	case GL_INT:
		{
			const GLint *src = (const GLint *) lists;
			for (i = 0; i < n; i++) {
				newLists[i] = crTranslateListId(cr_server.curClient->currentTranslator, src[i] + listBase);
			}
		}
		break;
	case GL_FLOAT:
		{
			const GLfloat *src = (const GLfloat *) lists;
			for (i = 0; i < n; i++) {
				newLists[i] = crTranslateListId(cr_server.curClient->currentTranslator, (GLuint) src[i] + listBase);
			}
		}
		break;
	case GL_2_BYTES:
		{
			const GLubyte *src = (const GLubyte *) lists;
			for (i = 0; i < n; i++) {
				newLists[i] = crTranslateListId(cr_server.curClient->currentTranslator, (src[i*2+0]*256 + src[i*2+1] + listBase));
			}
		}
		break;
	case GL_3_BYTES:
		{
			const GLubyte *src = (const GLubyte *) lists;
			for (i = 0; i < n; i++) {
				newLists[i] = crTranslateListId(cr_server.curClient->currentTranslator, (src[i*3+0]*256*256 + src[i*3+1]*256 + src[i*3+2] + listBase));
			}
		}
		break;
	case GL_4_BYTES:
		{
			const GLubyte *src = (const GLubyte *) lists;
			for (i = 0; i < n; i++) {
				newLists[i] = crTranslateListId(cr_server.curClient->currentTranslator, (src[i*4+0]*256*256*256 + src[i*4+1]*256*256 + src[i*4+2]*256 + src[i*4+3] + listBase));
			}
		}
		break;
	default:
		crWarning("CRServer: invalid display list datatype 0x%x", type);
	}
}


void SERVER_DISPATCH_APIENTRY
crServerDispatchCallLists( GLsizei n, GLenum type, const GLvoid *lists )
{
	/* always need to translate IDs */
	GLuint *translatedLists = (GLuint *) crAlloc(n * sizeof(GLuint));
	if (translatedLists == NULL) {
		crWarning("CRServer: out of memory dispatching CallLists())");
		return;
	}
	/* Translate into GL_UNSIGNED_INT */
	TranslateListIDs(n, type, lists, translatedLists);

	if (cr_server.curClient->currentCtx->lists.mode == 0) {
		/* we're not compiling, so execute the list now */
		CRMuralInfo *mural = cr_server.curClient->currentMural;
		int i;

		if (!mural->viewportValidated) {
			crServerComputeViewportBounds(&(cr_server.curClient->currentCtx->viewport), mural);
		}

		if (mural->numExtents == 0) {
			/* Issue the list as-is */
			cr_server.head_spu->dispatch_table.CallLists( n, GL_UNSIGNED_INT, translatedLists );
		}
		else {
			/* Loop over the extents (tiles) calling glCallList() */
			for ( i = 0; i < mural->numExtents; i++ ) {
				if (cr_server.run_queue->client->currentCtx)
					crServerSetOutputBounds( mural, i );
				cr_server.head_spu->dispatch_table.CallLists( n, GL_UNSIGNED_INT, translatedLists );
			}
		}
	}
	else {
		/* we're compiling glCallList into another list - just pass it through */
		cr_server.head_spu->dispatch_table.CallLists( n, GL_UNSIGNED_INT, translatedLists );
	}

	crFree((void *) translatedLists);  /* malloc'd above */
}

/* The server must receive and store ListBase commands, but must
 * not execute them; the effects of the ListBase are already included
 * in the TranslateListIDs call, so passing the ListBase down to the
 * server would do bad things.
 */
void SERVER_DISPATCH_APIENTRY crServerDispatchListBase( GLuint base )

{
	crStateListBase( base );
	/* Do *not* send this through the head SPU dispatch table. */
	/* cr_server.head_spu->dispatch_table.ListBase( base ); */
}


GLboolean SERVER_DISPATCH_APIENTRY crServerDispatchIsList( GLuint list )
{
	GLboolean retval;
	list = crTranslateListId(cr_server.curClient->currentTranslator, list);
	retval = cr_server.head_spu->dispatch_table.IsList( list );
	crServerReturnValue( &retval, sizeof(retval) );
	return retval;
}


void SERVER_DISPATCH_APIENTRY crServerDispatchDeleteLists( GLuint list, GLsizei range )
{
	/* We have to delete these one by one on the server, because they
	 * may not be contiguous once mapped to the server's domain
	 */
	GLuint i;
	for (i = list; i < list + range; i++) {
		cr_server.head_spu->dispatch_table.DeleteLists( crTranslateListId(cr_server.curClient->currentTranslator, i), 1 );
	}
		
	/* The crState version is contiguous. */
	crStateDeleteLists( list, range );
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


GLboolean SERVER_DISPATCH_APIENTRY
crServerDispatchAreTexturesResident(GLsizei n, const GLuint *textures,
                                    GLboolean *residences)
{
	GLboolean retval;
	GLboolean *res = (GLboolean *) crAlloc(n * sizeof(GLboolean));
	GLsizei i;

	(void) residences;

	if (!cr_server.sharedTextureObjects) {
		GLuint *textures2 = (GLuint *) crAlloc(n * sizeof(GLuint));
		for (i = 0; i < n; i++)
			textures2[i] = TranslateTextureID(textures[i]);
		retval = cr_server.head_spu->dispatch_table.AreTexturesResident(n, textures2, res);
		crFree(textures2);
	}
	else {
		retval = cr_server.head_spu->dispatch_table.AreTexturesResident(n, textures, res);
	}
	crServerReturnValue(res, n * sizeof(GLboolean));

	crFree(res);

	return retval; /* WILL PROBABLY BE IGNORED */
}


GLboolean SERVER_DISPATCH_APIENTRY
crServerDispatchAreProgramsResidentNV(GLsizei n, const GLuint *programs,
																			GLboolean *residences)
{
	GLboolean retval;
	GLboolean *res = (GLboolean *) crAlloc(n * sizeof(GLboolean));
	GLsizei i;

	(void) residences;

	if (!cr_server.sharedTextureObjects) {
		GLuint *programs2 = (GLuint *) crAlloc(n * sizeof(GLuint));
		for (i = 0; i < n; i++)
			programs2[i] = TranslateProgramID(programs[i]);
		retval = cr_server.head_spu->dispatch_table.AreProgramsResidentNV(n, programs2, res);
		crFree(programs2);
	}
	else {
		retval = cr_server.head_spu->dispatch_table.AreProgramsResidentNV(n, programs, res);
	}

	crServerReturnValue(res, n * sizeof(GLboolean));
	crFree(res);

	return retval; /* WILL PROBABLY BE IGNORED */
}
