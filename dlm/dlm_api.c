#include <float.h>
#include "cr_dlm.h"
#include "cr_mem.h"
#include "cr_unpack.h"
#include "cr_packfunctions.h"
#include "dlm.h"



/* This file defines the display list functions such as NewList, EndList,
 * CallList, CallLists, IsList, DeleteLists, etc.
 * Generally, SPUs will call these as needed to implement dislay lists.
 * See the expando and tilesort SPUs for examples.
 *
 * The functions which compile GL functions into our display lists are named:
 *     void DLM_APIENTRY crdlm_<function name>
 * where <function_name> is the Chromium function name (which in 
 * turn is the GL function name with the "gl" prefix removed).
 *
 * All these entry points require that a CRDLMContextState structure
 * be created (with crDLMNewContext()) and assigned to the current
 * thread (with crDLMSetCurrentState()).
 */


/*
 * Begin compiling a list.
 * table - points to an SPUDispatchTable (typically the SPU's self table)
 *         which will get loaded with display list compilation functions
 *         (as seen in dlm_dispatch.c)
 */
void
crdlm_NewList(GLuint listIdentifier, GLenum mode, SPUDispatchTable * table)
{
	DLMListInfo *listInfo;
	CRDLMContextState *listState = CURRENT_STATE();

	CRASSERT(table);

	/* crDebug("crdlm_NewList(%d,%d)", listIdentifier, mode); */

	/* Error checks: 0 is not a valid identifier, and
	 * we can't call NewList if NewList has been called
	 * more recently than EndList.
	 *
	 * The caller is expected to check for an improper
	 * mode parameter (GL_INVALID_ENUM), or for a NewList
	 * within a glBegin/glEnd (GL_INVALID_OPERATION).
	 */
	if (listState == NULL)
	{
		crWarning
			("DLM error: NewList(%d,%d) called with no current state (%s line %d)\n",
			 listIdentifier, mode, __FILE__, __LINE__);
		return;
	}

	if (listState->dlm->handleCreation & CRDLM_LOCALFUNC)
	{
		if (listIdentifier == 0)
		{
			crdlm_error(__LINE__, __FILE__, GL_INVALID_VALUE,
									 "NewList called with a list identifier of 0");
			return;
		}
		if (listState->currentListInfo != NULL)
		{
			char msg[1000];
			sprintf(msg, "NewList called with display list %d while display list %d was already open",
							(int) listIdentifier, (int) listState->currentListIdentifier);
			crdlm_error(__LINE__, __FILE__, GL_INVALID_OPERATION, msg);
			return;
		}

		if (!(listInfo = (DLMListInfo *) crAlloc(sizeof(DLMListInfo))))
		{
			char msg[1000];
			sprintf(msg, "could not allocate %d bytes of memory in NewList",
									 sizeof(DLMListInfo));
			crdlm_error(__LINE__, __FILE__, GL_OUT_OF_MEMORY, msg);									 
			return;
		}

		listInfo->first = listInfo->last = NULL;
		if (!(listInfo->references = crAllocHashtable()))
		{
			crFree(listInfo);
			crdlm_error(__LINE__, __FILE__, GL_OUT_OF_MEMORY,
									 "could not allocate memory in NewList");
			return;
		}
		listInfo->bbox.xmin = FLT_MAX;
		listInfo->bbox.xmax = -FLT_MAX;
		listInfo->bbox.ymin = FLT_MAX;
		listInfo->bbox.ymax = -FLT_MAX;
		listInfo->bbox.zmin = FLT_MAX;
		listInfo->bbox.zmax = -FLT_MAX;

		listState->currentListInfo = listInfo;
		listState->currentListIdentifier = listIdentifier;
		listState->currentListMode = mode;

		/* Don't use crSPUCopyDispatch() since we don't want to get hooked
		 * into the dispatch "copy_of" list.
		 */
		crMemcpy(&listState->savedDispatchTable, table, sizeof(SPUDispatchTable));
		listState->savedDispatchTablePtr = table;
	}

	crdlm_setup_compile_dispatcher(table);
}


/* This small utility routine is used to traverse a buffer
 * list, freeing each buffer.  It is used to free the buffer
 * list in the DLMListInfo structure, both when freeing the
 * entire structure and when freeing just the retained content.
 */
static void
free_instance_list(DLMInstanceList * instance)
{
	while (instance)
	{
		DLMInstanceList *nextInstance = instance->next;
		crFree(instance);
		instance = nextInstance;
	}
}

/* This utility routine frees a DLMListInfo structure and all
 * of its components.  It is used directly, when deleting a
 * single list; it is also used as a callback function for
 * hash tree operations (Free and Replace).
 *
 * The parameter is listed as a (void *) instead of a (DLMListInfo *)
 * in order that the function can be used as a callback routine for
 * the hash table functions.  The (void *) designation causes no
 * other harm, save disabling type-checking on the pointer argument
 * of the function.
 */
void
crdlm_free_list(void *parm)
{
	DLMListInfo *listInfo = (DLMListInfo *) parm;

	free_instance_list(listInfo->first);
	listInfo->first = listInfo->last = NULL;

	/* The references list has no allocated information; it's
	 * just a set of entries.  So we don't need to free any
	 * information as each entry is deleted.
	 */
	crFreeHashtable(listInfo->references, NULL);

	crFree(listInfo);
}


void
crdlm_EndList(void)
{
	CRDLMContextState *listState = CURRENT_STATE();

	/* Error check: cannot call EndList without a (successful)
	 * preceding NewList.
	 *
	 * The caller is expected to check for glNewList within
	 * a glBegin/glEnd sequence.
	 */
	if (listState == NULL)
	{
		crWarning
			("DLM error: EndList called with no current state (%s line %d)\n",
			 __FILE__, __LINE__);
		return;
	}
	if (listState->currentListInfo == NULL)
	{
		crdlm_error(__LINE__, __FILE__, GL_INVALID_OPERATION,
								 "EndList called while no display list was open");
		return;
	}

	DLM_LOCK(listState->dlm);

	/* This function will either replace the list information that's
	 * already present with our new list information, freeing the
	 * former list information; or will add the new information
	 * to the set of display lists, depending on whether the
	 * list already exists or not.
	 */
	crHashtableReplace(listState->dlm->displayLists,
										 listState->currentListIdentifier,
										 listState->currentListInfo, crdlm_free_list);
	DLM_UNLOCK(listState->dlm);

	/*
	printf("%s bbox:\n", __FUNCTION__);
	printf(" X: %f .. %f\n", listState->currentListInfo->bbox.xmin, listState->currentListInfo->bbox.xmax);
	printf(" Y: %f .. %f\n", listState->currentListInfo->bbox.ymin, listState->currentListInfo->bbox.ymax);
	printf(" Z: %f .. %f\n", listState->currentListInfo->bbox.zmin, listState->currentListInfo->bbox.zmax);
	*/

	/* reset the current state to show the list had been ended */
	listState->currentListIdentifier = 0;
	listState->currentListInfo = NULL;
}


/*
 * This should be called in conjunction with crdlm_EndList().  It restores
 * the dispatch table that was passed into crdlm_NewList() to its original
 * state.  Ideally, this function would be part of crdlm_EndList() but we
 * have to split it out to make things work in the tilesort SPU.
 */
void
crdlm_RestoreDispatch(void)
{
	CRDLMContextState *listState = CURRENT_STATE();
	CRASSERT(listState->savedDispatchTablePtr);
	/* restore original dispatch table */
	crdlm_restore_dispatcher(listState->savedDispatchTablePtr,
													 &(listState->savedDispatchTable));
	listState->savedDispatchTablePtr = NULL;
}


void
crdlm_DeleteLists(GLuint firstListIdentifier, GLsizei range)
{
	CRDLMContextState *listState = CURRENT_STATE();
	register unsigned int i;

	if (listState == NULL)
	{
		crWarning
			("DLM error: DeleteLists(%d,%d) called with no current state (%s line %d)\n",
			 firstListIdentifier, range, __FILE__, __LINE__);
		return;
	}
	if (range < 0)
	{
		char msg[1000];
		sprintf(msg, "DeleteLists called with range (%d) less than zero", range);
		crdlm_error(__LINE__, __FILE__, GL_INVALID_VALUE, msg);								 
		return;
	}

	/* Interestingly, there doesn't seem to be an error for deleting
	 * display list 0, which cannot exist.
	 *
	 * We could delete the desired lists by walking the entire hash of
	 * display lists and looking for and deleting any in our range; or we
	 * could delete lists one by one.  The former is faster if the hashing
	 * algorithm is inefficient or if we're deleting all or most of our
	 * lists; the latter is faster if we're deleting a relatively small
	 * number of lists.
	 *
	 * For now, we'll go with the latter; it's also easier to implement
	 * given the current functions available.
	 */
	DLM_LOCK(listState->dlm);
	for (i = 0; i < range; i++)
	{
		crHashtableDelete(listState->dlm->displayLists,
											firstListIdentifier + i, crdlm_free_list);
	}
	DLM_UNLOCK(listState->dlm);
}


void
crdlm_add_to_dl(CRDLMContextState * state, void *x,
								void (*execute) (DLMInstanceList * instance,
																 SPUDispatchTable * dispatchTable))
{
	DLMInstanceList *instance = (DLMInstanceList *) x;
	instance->next = NULL;
	instance->execute = execute;
	if (!state->currentListInfo->first)
	{
		state->currentListInfo->first = (DLMInstanceList *) instance;
	}
	else
	{
		state->currentListInfo->last->next = (DLMInstanceList *) instance;
	}
	state->currentListInfo->last = (DLMInstanceList *) instance;
}


void
crdlm_ListBase(GLuint listBase)
{
	CRDLMContextState *listState = CURRENT_STATE();

	if (listState == NULL)
	{
		crWarning
			("DLM error: ListBase(%d) called with no current state (%s line %d)\n",
			 listBase, __FILE__, __LINE__);
		return;
	}

	listState->listBase = listBase;
}


void
crdlm_CallList(GLuint listIdentifier, SPUDispatchTable * table)
{
	CRDLMContextState *listState = CURRENT_STATE();

	if (listState == NULL)
	{
		crWarning
			("DLM error: CallList(%d) called with no current state (%s line %d)\n",
			 listIdentifier, __FILE__, __LINE__);
		return;
	}

	crDLMReplayList(listState->dlm, listIdentifier, table);
}

void
crdlm_CallLists(GLsizei n, GLenum type, const GLvoid * lists,
								SPUDispatchTable *table)
{
	register int i;
	CRDLMContextState *listState = CURRENT_STATE();
	const GLuint base = listState->listBase;

	/* CallLists will simply expand into a sequence of CallList
	 * invocations.  The display list really doesn't care, after all...
	 */
	switch (type)
	{

#define EXPAND(typeIndicator, type)\
	case typeIndicator: { \
	    type *data = (type *)lists; \
      /*printf("Calling %d (b=%d)\n", (int)(base + *data), base);*/\
	    for (i = 0; i < n; i++, data++) crdlm_CallList(base + *data, table); \
	} \
	break

	EXPAND(GL_BYTE, GLbyte);
	EXPAND(GL_UNSIGNED_BYTE, GLubyte);
	EXPAND(GL_SHORT, GLshort);
	EXPAND(GL_UNSIGNED_SHORT, GLushort);
	EXPAND(GL_INT, GLint);
	EXPAND(GL_UNSIGNED_INT, GLuint);
	EXPAND(GL_FLOAT, GLfloat);

  case GL_2_BYTES:
		{
			GLubyte *data = (GLubyte *) lists;
			for (i = 0; i < n; i++, data += 2)
				crdlm_CallList(base + 256 * data[0] + data[1], table);
		}
		break;

	case GL_3_BYTES:
		{
			GLubyte *data = (GLubyte *) lists;
			for (i = 0; i < n; i++, data += 3)
				crdlm_CallList(base + 256 * (256 * data[0] + data[1]) + data[2], table);
		}
		break;

	case GL_4_BYTES:
		{
			GLubyte *data = (GLubyte *) lists;
			for (i = 0; i < n; i++, data += 4)
				crdlm_CallList(base +
											 256 * (256 * (256 * data[0] + data[1]) + data[2]) +
											 data[3], table);
		}
		break;

	default:
		crdlm_error(__LINE__, __FILE__, GL_INVALID_ENUM, "CallLists");
		return;
	}
}


/*
 * If the DLM client has a Cr state tracker, it should proably use
 * crStateGenLists.  But if the DLM client doesn't have a state tracker
 * it can use crdlm_GenLists.
 */
GLuint
crdlm_GenLists(GLsizei range)
{
	CRDLMContextState *listState = CURRENT_STATE();
	GLuint start;

	if (listState == NULL)
	{
		crWarning
			("DLM error: GenLists(%d) called with no current state (%s line %d)\n",
			 range, __FILE__, __LINE__);
		return 0;
	}

	/* We're supposed to return 0 if an empty range is requested */
	if (range == 0)
	{
		return 0;
	}

	DLM_LOCK(listState->dlm);
	start = crHashtableAllocKeys(listState->dlm->displayLists, range);
	DLM_UNLOCK(listState->dlm);
	if (!start)
	{
		crdlm_error(__LINE__, __FILE__, GL_OUT_OF_MEMORY, "glGenTextures");
	}
	return start;
}


/*
 * See comment for crdlm_GenLists.  Same story here.
 */
GLboolean
crdlm_IsList(GLuint list)
{
	CRDLMContextState *listState = CURRENT_STATE();

	if (listState == NULL)
	{
		crWarning
			("DLM error: IsLists(%d) called with no current state (%s line %d)\n",
			 list, __FILE__, __LINE__);
		return 0;
	}

	if (list == 0)
		return GL_FALSE;

	return crHashtableIsKeyUsed(listState->dlm->displayLists, list);
}
