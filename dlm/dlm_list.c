#include <stdio.h>
#include "cr_spu.h"
#include "cr_dlm.h"
#include "cr_mem.h"
#include "cr_error.h"
#include "dlm.h"
#include "dlm_dispatch.h"
#include "dlm_client.h"
#include "dlm_list.h"

static void DLM_APIENTRY executeNewList(DLMInstanceList *x, SPUDispatchTable *dispatchTable)
{
	struct instanceNewList *instance = (struct instanceNewList *)x;
	dispatchTable->NewList(instance->list, instance->mode);
}

void DLM_APIENTRY crdlm_compile_NewList( GLuint list, GLuint mode )
{
	CRDLMContextState *state = CURRENT_STATE();
	struct instanceNewList *instance;
	instance = crCalloc(sizeof(struct instanceNewList));
	if (!instance) {
		crdlm_error(__LINE__, __FILE__, GL_OUT_OF_MEMORY,
			"out of memory adding NewList to display list");
		return;
	}
	instance->next = NULL;
	instance->execute = executeNewList;
	instance->list = list;
	instance->mode = mode;
	if (!state->currentListInfo->first) {
		state->currentListInfo->first = (DLMInstanceList *)instance;
	}
	else {
		state->currentListInfo->last->next = (DLMInstanceList *)instance;
	}
	state->currentListInfo->last = (DLMInstanceList *)instance;
}

static void DLM_APIENTRY executeEndList(DLMInstanceList *x, SPUDispatchTable *dispatchTable)
{
	dispatchTable->EndList();
}

void DLM_APIENTRY crdlm_compile_EndList( void )
{
	CRDLMContextState *state = CURRENT_STATE();
	struct instanceEndList *instance;
	instance = crCalloc(sizeof(struct instanceEndList));
	if (!instance) {
		crdlm_error(__LINE__, __FILE__, GL_OUT_OF_MEMORY,
			"out of memory adding EndList to display list");
		return;
	}
	instance->next = NULL;
	instance->execute = executeEndList;
	if (!state->currentListInfo->first) {
		state->currentListInfo->first = (DLMInstanceList *)instance;
	}
	else {
		state->currentListInfo->last->next = (DLMInstanceList *)instance;
	}
	state->currentListInfo->last = (DLMInstanceList *)instance;
}
