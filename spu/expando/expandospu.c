/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_spu.h"
#include "cr_dlm.h"
#include "expandospu.h"

static GLint EXPANDOSPU_APIENTRY
CreateContext(const char *displayName, GLint visBits)
{
	CRDLM *dlm;
	CRDLMContextState *dlmContext;
	GLint contextId;

	/* Our configuration for the DLM we're going to create.
	 * It says that we want the DLM to handle everything
	 * itself, making display lists transparent to 
	 * the host SPU.
	 */
	CRDLMConfig dlmConfig = {
		CRDLM_DEFAULT_BUFFERSIZE,	/* bufferSize */
		CRDLM_HANDLE_DLM,		/* handleCreation */
		CRDLM_HANDLE_DLM,		/* handleReference */
	};

	/* Get an official context ID from our super */
	contextId = expando_spu.super.CreateContext(displayName, visBits);

	/* Supplement that with our DLM.  In a more correct situation, we should
	 * see if we've been called through glXCreateContext, which has a parameter
	 * for sharing DLMs.  We don't currently get that information, so for now
	 * give each context its own DLM.
	 */
	dlm = crDLMNewDLM(sizeof(dlmConfig), &dlmConfig);
	if (!dlm) {
		crDebug("expando: couldn't get DLM!");
	}

	dlmContext = crDLMNewContext(dlm, NULL);
	if (!dlmContext) {
		crDebug("expando: couldn't get dlmContext");
	}

	/* We're not going to hold onto the dlm ourselves, so we can
	 * free it.  It won't be actually freed until all the structures
	 * that refer to it (like our dlmContext) are freed.
	 */
	crDLMFreeDLM(dlm);
    
	/* The DLM context should be associated with the user context */
	crHashtableAdd(expando_spu.contextTable, contextId,
								 (void *)dlmContext);

	return contextId;
}

static void EXPANDOSPU_APIENTRY
DestroyContext(GLint contextId)
{
	/* Destroy our context information */
	crHashtableDelete(expando_spu.contextTable, contextId, 
										expando_free_dlm_context);

	/* Pass along the destruction to our super. */
	expando_spu.super.DestroyContext(contextId);
}

static void EXPANDOSPU_APIENTRY
MakeCurrent(GLint crWindow, GLint nativeWindow, GLint contextId)
{
	CRDLMContextState *dlmContext;

	expando_spu.super.MakeCurrent(crWindow, nativeWindow, contextId);

	dlmContext = crHashtableSearch(expando_spu.contextTable, contextId);
	crDLMSetCurrentState(dlmContext);
}


/*
 * NOTE: we don't really need these wrapper functions (could plug the
 * crdlm_* functions directly into _cr_expando_table but this makes
 * things a big easier to debug for now.
 */

static void EXPANDOSPU_APIENTRY
NewList(GLuint list, GLenum mode)
{
	crdlm_NewList(list, mode, &(expando_spu.self));
}

static void EXPANDOSPU_APIENTRY
EndList(void)
{
	const GLenum list = crDLMGetCurrentList();
	const GLenum mode = crDLMGetCurrentMode();
	crdlm_EndList();
	crdlm_RestoreDispatch();  /* companion to crdlm_EndList() */
	if (mode == GL_COMPILE_AND_EXECUTE)
		crdlm_CallList(list, &expando_spu.self);
}

static void EXPANDOSPU_APIENTRY
CallList(GLuint list)
{
	crdlm_CallList(list, &expando_spu.self);
}

static void EXPANDOSPU_APIENTRY
CallLists(GLsizei n, GLenum type, const GLvoid *lists)
{
	crdlm_CallLists(n, type, lists, &expando_spu.self);
}

static void EXPANDOSPU_APIENTRY
DeleteLists(GLuint first, GLsizei range)
{
	crdlm_DeleteLists(first, range);
}

static GLuint EXPANDOSPU_APIENTRY
GenLists(GLsizei range)
{
	 return crdlm_GenLists(range);
}

static GLboolean EXPANDOSPU_APIENTRY
IsList(GLuint list)
{
	 return crdlm_IsList(list);
}

static void EXPANDOSPU_APIENTRY
ListBase(GLuint base)
{
	 crdlm_ListBase(base);
}


SPUNamedFunctionTable _cr_expando_table[] = {
	{ "CreateContext", (SPUGenericFunction) CreateContext },
	{ "DestroyContext", (SPUGenericFunction) DestroyContext },
	{ "MakeCurrent", (SPUGenericFunction) MakeCurrent },
	{ "NewList", (SPUGenericFunction) NewList },
	{ "EndList", (SPUGenericFunction) EndList },
	{ "CallList", (SPUGenericFunction) CallList },
	{ "CallLists", (SPUGenericFunction) CallLists },
	{ "DeleteLists", (SPUGenericFunction) DeleteLists },
	{ "GenLists", (SPUGenericFunction) GenLists },
	{ "ListBase", (SPUGenericFunction) ListBase },
	{ "IsList", (SPUGenericFunction) IsList },
	{ NULL, NULL }
};
