#ifndef _DLM_H
#define _DLM_H

#include "cr_dlm.h"
#include "cr_spu.h"

#ifdef CHROMIUM_THREADSAFE
#define DLM_LOCK(dlm) crLockMutex(&(dlm->dlMutex))
#define DLM_UNLOCK(dlm) crUnlockMutex(&(dlm->dlMutex))
extern CRtsd CRDLMTSDKey;
#define SET_CURRENT_STATE(state) crSetTSD(&CRDLMTSDKey, (void *)state)
#define CURRENT_STATE() ((CRDLMContextState *)crGetTSD(&CRDLMTSDKey))
#else
#define DLM_LOCK(dlm)
#define DLM_UNLOCK(dlm)
extern CRDLMContextState *CRDLMCurrentState;
#define SET_CURRENT_STATE(state) CRDLMCurrentState = (state)
#define CURRENT_STATE() (CRDLMCurrentState)
#endif

/* These are the dispatch tables built using dlm_dispatch.py. */
#if 000
extern SPUDispatchTable dlm_dispatch_pass,
    dlm_dispatch_compile,
    dlm_dispatch_compileAndExecute;
#endif

/* These are API functions that we'll use */
/* Declarations for API-level functions, installed with the dispatch table */
/***void DLM_APIENTRY crdlm_ListBase(GLuint listBase);***/
void DLM_APIENTRY crdlm_EnableClientState(GLenum array);
void DLM_APIENTRY crdlm_DisableClientState(GLenum array);

/* These routines are intended to be used within the DLM library, across
 * the modules therein, but not as an API into the DLM library from
 * outside.
 */
extern void crdlmWarning( int line, char *file, GLenum error, char *format, ... );
extern void crdlm_free_list(/* DLMListInfo * */ void *listInfo);
extern void crdlm_add_to_dl(CRDLMContextState *state, void *x, 
	void (*execute)(DLMInstanceList *instance, SPUDispatchTable *dispatchTable));

extern void crdlm_setup_compile_dispatcher(SPUDispatchTable *t);
extern void crdlm_restore_dispatcher(SPUDispatchTable *t, const SPUDispatchTable *original);

extern void crdlm_error(int line, const char *file, GLenum error, const char *info);

#endif
