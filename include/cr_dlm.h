/* Display List Manager definitions
 */

#ifndef CR_DLM_H
#define CR_DLM_H

#if defined(WINDOWS)
#define DLM_APIENTRY __stdcall
#else
#define DLM_APIENTRY
#endif

#include "chromium.h"
#include "state/cr_client.h"
#include "cr_spu.h"
#include "cr_hash.h"
#include "cr_threads.h"
#include "cr_pack.h"
#ifdef CHROMIUM_THREADSAFE
#include "cr_threads.h"
#endif

/* 3D bounding box */
typedef struct {
	double xmin, xmax, ymin, ymax, zmin, zmax;
} CRDLMBounds;

/* This is enough information to hold an instance of a single function call */
typedef struct DLMInstanceList {
	struct DLMInstanceList *next;
	void (*execute)(struct DLMInstanceList *instance, SPUDispatchTable *dispatchTable);
} DLMInstanceList;

typedef struct {
	DLMInstanceList *first, *last;
	CRHashTable *references; /* display lists that this display list calls */
	CRDLMBounds bbox;
} DLMListInfo;

typedef struct {
	/* This holds all the display list information, hashed by list identifier. */
	CRHashTable *displayLists;

	/* This is a count of the number of contexts/users that are using
	 * this DLM.
	 */
	unsigned int userCount;

#ifdef CHROMIUM_THREADSAFE
	/* This mutex protects the displayLists hash table from simultaneous
	 * updates by multiple contexts.
	 */
	CRmutex dlMutex;
	CRtsd tsdKey;
#endif

	/* Configuration information - see the CRDLMConfig structure below
	 * for details.
	 */
	unsigned int bufferSize;
	int handleCreation;
	int handleReference;
} CRDLM;

/* This structure holds thread-specific state.  Each thread can be
 * associated with one (and only one) context; and each context can
 * be associated with one (and only one) DLM.  Making things interesting,
 * though, is that each DLM can be associated with multiple contexts.
 *
 * So the thread-specific data key is associated with each context, not
 * with each DLM.  Two different threads can, through two different
 * contexts that share a single DLM, each have independent state and
 * conditions.
 */

typedef struct {
	CRDLM *dlm;			/* the DLM associated with this state */
	unsigned long currentListIdentifier;	/* open display list */
	DLMListInfo *currentListInfo;	/* open display list data */
	GLenum currentListMode;		/* GL_COMPILE or GL_COMPILE_AND_EXECUTE */
	GLuint listBase;

	/* The dispatch table we should call for passthrough functions.  We need
	 * this because we override the default dispatch table.  This is set
	 * when we become current.
	 */
	SPUDispatchTable savedDispatchTable;
	SPUDispatchTable* savedDispatchTablePtr;

	/* Client state we need to be aware of */
	CRClientState *clientState;
} CRDLMContextState;

/* These constants define the functional behavior of the CRDLM
 * override functions.  HANDLE_DLM means that the DLM handler will
 * be used; HANDLE_SPU means that the SPU handler will be used.
 * HANDLE_ALL means that both will be called in succession (the
 * DLM will be called first, then the SPU).
 */
#define CRDLM_LOCALFUNC	0x01
#define CRDLM_SPUFUNC	0x02

#define CRDLM_HANDLE_DLM	(CRDLM_LOCALFUNC)
#define CRDLM_HANDLE_SPU	(CRDLM_SPUFUNC)
#define CRDLM_HANDLE_ALL	(CRDLM_LOCALFUNC|CRDLM_SPUFUNC)


/* These additional structures are for passing information to and from the 
 * CRDLM interface routines.
 */
typedef struct {
	/* The size, in bytes, that the packer will initially allocate for
	 * each new buffer.
	 */
#define CRDLM_DEFAULT_BUFFERSIZE (1024*1024)
	unsigned int bufferSize;	/* this will be allocated for each buffer */

	/* How to handle display list creation.  If handled only by the DLM,
	 * the NewList() and all functions up to the EndList() will not
	 * be passed back to the host SPU.  If handled by the SPU only,
	 * the DLM will not interfere.  If handled by both, the DLM will
	 * maintain a copy, but all the instructions will also be passed
	 * to the host SPU.
	 */
#define CRDLM_DEFAULT_HANDLECREATION CRDLM_HANDLE_DLM
	int handleCreation;

	/* How to handle a display list reference.  If handled by the DLM,
	 * the contents of the referenced display list will be sent out;
	 * the host SPU will never see a reference.  If handled by the 
	 * SPU, the reference will be passed directly to the SPU.
	 */
#define CRDLM_DEFAULT_HANDLEREFERENCE CRDLM_HANDLE_DLM
	int handleReference;
} CRDLMConfig;

/* Positive values match GL error values.
 * 0 (GL_NO_ERROR) is returned for success
 * Negative values are internal errors.
 * Possible positive values (from GL/gl.h) are:
 * GL_NO_ERROR (0x0)
 * GL_INVALID_ENUM (0x0500)
 * GL_INVALID_VALUE (0x0501)
 * GL_INVALID_OPERATION (0x0502)
 * GL_STACK_OVERFLOW (0x0503)
 * GL_STACK_UNDERFLOW (0x0504)
 * GL_OUT_OF_MEMORY (0x0505)
 */
typedef int CRDLMError;

/* This error reported if there's no current state. The caller is responsible
 * for appropriately allocating context state with crDLMNewContext(), and
 * for making it current with crDLMMakeCurrent().
 */
#define CRDLM_ERROR_STATE	(-1)


typedef void (*CRDLMErrorCallback)(int line, const char *file, GLenum error, const char *info);


#ifdef __cplusplus
extern "C" {
#endif

/* Declarations for DLM-level functions */
CRDLM *crDLMNewDLM(int configSize, const CRDLMConfig *config);
CRDLMContextState *crDLMNewContext(CRDLM *dlm, CRClientState *clientState);
void crDLMFreeContext(CRDLMContextState *state);
void crDLMUseDLM(CRDLM *dlm);
void crDLMFreeDLM(CRDLM *dlm);
void crDLMSetCurrentState(CRDLMContextState *state);
CRDLMContextState *crDLMGetCurrentState(void);
void crDLMSendAllLists(CRDLM *dlm, CRClientState *restoreClientState, 
	SPUDispatchTable *dispatchTable);
void crDLMSendList(CRDLM *dlm, unsigned long listIdentifier,
	CRClientState *restoreClientState, SPUDispatchTable *dispatchTable);
void crDLMReplayList(CRDLM *dlm, unsigned long listIdentifier, SPUDispatchTable *dispatchTable);

CRDLMError crDLMDeleteListContent(CRDLM *dlm, unsigned long listIdentifier);
int crDLMGetReferences(CRDLM *dlm, unsigned long listIdentifier,
    int firstIndex, int sizeofBuffer, unsigned int *buffer);
CRDLMError crDLMGetBounds(CRDLM *dlm, unsigned long listIdentifier, CRDLMBounds *bounds);

GLuint crDLMGetCurrentList(void);
GLenum crDLMGetCurrentMode(void);

void crDLMErrorFunction(CRDLMErrorCallback callback);

/* Public functions which basically correspond to OpenGL functions */
void crdlm_NewList(GLuint listIdentifier, GLenum mode, SPUDispatchTable * table);
void crdlm_EndList(void);
void crdlm_RestoreDispatch(void);
void crdlm_CallList(GLuint listIdentifier, SPUDispatchTable * table);
void crdlm_CallLists(GLsizei n, GLenum type, const GLvoid *lists, SPUDispatchTable *table);
void crdlm_DeleteLists(GLuint firstListIdentifier, GLsizei range);
void crdlm_ListBase(GLuint listBase);
GLuint crdlm_GenLists(GLsizei range);
GLboolean crdlm_IsList(GLuint list);

/* special case export */
void crdlm_compile_CallList(GLuint list);
void crdlm_compile_CallLists(GLsizei n, GLenum type, const GLvoid *lists);

#ifdef __cplusplus
}
#endif

#endif /* CR_DLM_H */
