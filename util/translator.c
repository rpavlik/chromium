/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_hash.h"
#include "cr_mem.h"
#include "cr_error.h"
#include "cr_translator.h"

/*
 * This structure is used by the server to keep track
 * of various per-context objects.  The server keeps one
 * master set of (for example) display lists, and maps 
 * client-side IDs to the known low-level IDs.
 *
 * A reference count is necessary, because contexts that
 * are indicated to share display lists and other IDs
 * will just share the same translator.
 */
struct CRTranslator {
	GLuint referenceCount;
	CRHashTable *displayListIds;
};

CRTranslator *crTranslatorCreate(void)
{
	CRTranslator *newTranslator;
	newTranslator = crAlloc(sizeof(CRTranslator));
	if (newTranslator == NULL) {
		crWarning("could not allocate new translator");
		return NULL;
	}

	newTranslator->referenceCount = 1;
	newTranslator->displayListIds = crAllocHashtable();
	return newTranslator;
}

CRTranslator *crTranslatorShare(CRTranslator *translator)
{
	translator->referenceCount++;
	return translator;
}

void crTranslatorDestroy(void *t)
{
	CRTranslator *translator = (CRTranslator *)t;
	translator->referenceCount--;
	if (translator->referenceCount == 0) {
		crFreeHashtable(translator->displayListIds, NULL);
		crFree(translator);
	}
} 

GLuint crTranslateListId(const CRTranslator *translator, GLuint list)
{
	if (translator == NULL) return list;

	return (GLuint) crHashtableSearch(translator->displayListIds, list);
}

void crTranslateAddListId(CRTranslator *translator, GLuint highLevelList, GLuint lowLevelList)
{
	if (translator == NULL) return;

	crHashtableAdd(translator->displayListIds, highLevelList, (void *)lowLevelList);
}
