/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */


/*
 * This file manages OpenGL rendering contexts in the faker library.
 * The big issue is switching between Chromium and native GL context
 * management.  This is where we support multiple client OpenGL
 * windows.  Typically, one window is handled by Chromium while any
 * other windows are handled by the native OpenGL library.
 */

#include "chromium.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_dispatch.h"
#include "cr_threads.h"
#include "cr_spu.h"


/* Ta-da!  This is the master dispatch table.  We maintain it here
 * with the other dispatch table manipulation functions.
 * The "glimPtr" variable contains a pointer to the last dispatch
 * table we copied into "glim".  This allows us to make comparisons
 * based on the source of the dispatch tables we're working with.
 * Note that "glimPtr" is only used if we're single-threaded, because
 * in a multi-threaded environment, the thread-specific data associated
 * with crdispatchTSD is used instead.
 */

SPUDispatchTable glim;
SPUDispatchTable crThreadsafeDispatch;


/* This points at the dispatch table installed when no other
 * dispatch table is available.  It's usually the dispatch
 * table from the SPU chain.
 */
static SPUDispatchTable *defaultDispatchTable = NULL;

/* The crDispatchTableTSD is thread-specific data that
 * points to the dispatch table for a thread.  The dispatchInfo
 * pointer (either the global, or the thread-specific data)
 * points at the dispatch table information structure for
 * the current thread/context.
 */
#ifdef CHROMIUM_THREADSAFE
static CRtsd crDispatchTableTSD;
static CRtsd dispatchInfoTSD;
#define DISPATCH_SET_CURRENT_INFO(info) crSetTSD(&dispatchInfoTSD, info)
#define DISPATCH_GET_CURRENT_INFO() ((crCurrentDispatchInfo *)crGetTSD(&dispatchInfoTSD))
#else
static SPUDispatchTable *glimPtr = NULL;
static crCurrentDispatchInfo *dispatchInfo;
#define DISPATCH_SET_CURRENT_INFO(info) dispatchInfo = (info)
#define DISPATCH_GET_CURRENT_INFO() (dispatchInfo)
#endif


/* Check to see if we're being called from a different thread. */
#ifdef CHROMIUM_THREADSAFE
static int multipleThreads = 0;
static void check_threaded(void)
{
    static int initialized = 0;
    static unsigned long threadId;

    if (multipleThreads) {
	/* Already noticed we've got multiple threads;
	 * more don't make no difference.
	 */
    }
    else if (!initialized) {
	/* First time through for real (we'll fall through since the
	 * global crDispatchMultipleThreads is initialized to 0).
	 * Since it's our first time, there's only one known graphics
	 * thread, and we don't have to change the global.
	 *
	 * Note we have something of a race condition here; if two threads
	 * simultaneously hit this section, they'll each decide they're 
	 * single-threaded, and each set the threadId to their own ID.
	 * Sooner or later, though, whichever one didn't "win" the
	 * first race will notice the multiple threads.
	 */
	threadId = crThreadID();
	initialized = 1;
    }
    else if (threadId != crThreadID()) {
	/* Oops, multiple threads.  Install the thread-safe dispatch
	 * table.  The pointer to the currently installed dispatch
	 * table should already be in the correct thread-specific data.
	 */
	multipleThreads = 1;
	crSPUCopyDispatchTable(&glim, &crThreadsafeDispatch);
    }
}
#endif


static void update_dispatch_table(SPUDispatchTable *newTable)
{
    /* If we have no real table to install, install the default
     * table, if we have one.  Otherwise, there's not a lot we
     * can do.
     */
    if (newTable == NULL) newTable = defaultDispatchTable;
    if (newTable == NULL) return;

#ifdef CHROMIUM_THREADSAFE
    /* Check our threadedness */
    check_threaded();

    /* Must set the dispatch table pointer, even if we've not found
     * any evidence that multiple threads are running graphics; if
     * we later find that we are running multiple graphics threads,
     * all we'll have to do is change to the threadsafe dispatch
     * table (since the thread-specific dispatch table pointer will
     * already be set).
     */
    crSetTSD(&crDispatchTableTSD, (void *) newTable);

    /* Go ahead and copy the table over (as an optimization)
     * if we haven't discovered ourselves to be multi-threaded
     * yet.
     */
    if (!multipleThreads) {
	crSPUCopyDispatchTable(&glim, newTable);
    }
#else
    /* If we're not thread-safe, we can check the glimPtr to see
     * if we're doing a redundant copy.
     */
    if (glimPtr != newTable) 
    {
	crSPUCopyDispatchTable(&glim, newTable);
	glimPtr = newTable;
    }
#endif
}

/* Called to initialize the dispatch table module.  The
 * dispatch table passed in is to be used as the default
 * dispatch table if there are no dispatch table layers.
 */
void crDispatchInit(SPUDispatchTable *defaultTable)
{
    defaultDispatchTable = defaultTable;

    /* We're likely called before anything is set up. */
    crSPUInitDispatchTable(&glim);

    update_dispatch_table(defaultTable);
}

void crInitDispatchInfo(crCurrentDispatchInfo *info)
{
    if (info) {
	info->topLayer = NULL;
	info->bottomLayer = NULL;
    }
}

/* This is the generic function to arrange for a
 * new overlay layer for the dispatch table.  With the structure
 * returned, the caller can modify the dispatch table dynamically
 * (via crChangeDispatchLayer) without stomping on other modules
 * that may have tried to override the dispatch table for their
 * own purposes. 
 *
 * Modules that only need to install a static dispatch table should use 
 * the simpler utility function crPushDispatchTable(). XXX nonexistant???
 */
crDispatchLayer *crNewDispatchLayer(SPUDispatchTable *newTable, 
        void (*changedCallback)(SPUDispatchTable *changedTable, void *callbackParm),
	void *callbackParm)
{
    crCurrentDispatchInfo *currentInfo = DISPATCH_GET_CURRENT_INFO();
    crDispatchLayer *newLayer;

    /* If there's no current info yet, we have no layers; either this
     * is the first time we've been called for a thread of execution,
     * or we've freed all the layers.  Either way, we need to get a
     * new information structure.
     */
    if (!currentInfo) {
	crWarning("crNewDispatchLayer: no current dispatch info\n");
	return NULL;
#if 0
	currentInfo = crAlloc(sizeof(crCurrentDispatchInfo));
	if (!currentInfo) {
	    return NULL;
	}
	crInitDispatchInfo(currentInfo);
	DISPATCH_SET_CURRENT_INFO(currentInfo);
#endif
    }

    /* Now go about allocating and initializing a layer */
    newLayer = (crDispatchLayer *)crAlloc(sizeof(crDispatchLayer));
    if (!newLayer) return NULL;
    newLayer->dispatchTable = newTable;
    newLayer->changedCallback = changedCallback;
    newLayer->callbackParm = callbackParm;

    /* Add it to the linked list.  We need to keep track
     * of the last layer in the node.  Since every thread has
     * its own set of dispatch layers, we don't need a mutex
     * or any such around this.
     */
    newLayer->prev = currentInfo->topLayer;
    newLayer->next = NULL;
    if (currentInfo->topLayer) {
        currentInfo->topLayer->next = newLayer;
    }
    currentInfo->topLayer = newLayer;

    /* Actually change the dispatch table.  If we're
     * running single-threaded, that means we copy the
     * new dispatch table to glim.  If we're running
     * multi-threaded, it means we set the thread-specific
     * dispatch table pointer, and leave the thread-safe
     * dispatch functions in glim.
     */
    update_dispatch_table(newTable);

    /* Return the layer allocated. */
    return newLayer;
}

void crChangeDispatchLayer(crDispatchLayer *layer,
        SPUDispatchTable *changedTable)
{
    /* We only have to do any work if the table actually
     * changed.  If it's the same (as might often be the
     * case for stubMakeCurrent), this has no effect.
     */
    if (layer->dispatchTable != changedTable) {
        layer->dispatchTable = changedTable;

        /* If there is no successor (i.e. we're the last table
         * in the list), then we have to install the new dispatch
         * table.  If there is a successor, then we have to tell
         * the successor that the dispatch table pointer he was
         * given (in order to be able to call back up the chain)
         * is now out-of-date.
         */
        if (layer->next) {
            /* There is a successor.  Tell the successor about
             * the new table.
             */
            if (layer->next->changedCallback) {
                (*layer->next->changedCallback)(changedTable, layer->next->callbackParm);
            }
        }
        else {
            /* we're updating the top layer, which is currently
             * active.  Change the dispatch table.
             */
            update_dispatch_table(changedTable);
        }
    }
}


void crFreeDispatchLayer(crDispatchLayer *layer)
{
    if (layer->next) {
        /* There is a successor.  By pulling ourselves
         * from the list, our successor's predecessor
         * table changes.
         */
        if (layer->prev) {
            /* A predecessor layer exists.  Let its successor
	     * (the same as our current successor) know that
	     * its parent dispatch table has changed.
	     */
            if (layer->next->changedCallback) {
                (*layer->next->changedCallback)(layer->prev->dispatchTable, layer->next->callbackParm);
            }
            layer->prev->next = layer->next;
            layer->next->prev = layer->prev;
        }
        else {
            /* No predecessor layer */
            if (layer->next->changedCallback) {
                (*layer->next->changedCallback)(NULL, layer->next->callbackParm);
            }
            layer->next->prev = NULL;
        }
    }
    else {
        /* No successor, we're on top.  We don't have to notify
         * anyone of changes, but we do have to change the top
         * layer and update dispatch tables (that is, if we have
	 * a current context & top layer - if the last thing we did
	 * was destroy a context or set the current context to NULL,
	 * there's no current dispatch layer...)
         */
	crCurrentDispatchInfo *currentInfo = DISPATCH_GET_CURRENT_INFO();
	if (currentInfo) {
	    currentInfo->topLayer = layer->prev;
	    if (layer->prev) {
		/* We have a predecessor to install */
		update_dispatch_table(layer->prev->dispatchTable);
		layer->prev->next = NULL;
	    }
	    else {
		/* No predecessor - the stack will be empty.  This really
		 * shouldn't happen until a context is freed.  In any
		 * case, free up all the stuff we allocated.
		 */
		update_dispatch_table(NULL);
		crFree(currentInfo);
		DISPATCH_SET_CURRENT_INFO(NULL);
	    }
	}
    }

    /* Free the memory for the layer */
    crFree(layer);
}

/* This is called whenever the context changes, to set up the
 * dispatch tables for the executing thread to match those
 * needed by the context.
 */
void crSetCurrentDispatchInfo(crCurrentDispatchInfo *info)
{
    DISPATCH_SET_CURRENT_INFO(info);
    if (info && info->topLayer) {
	update_dispatch_table(info->topLayer->dispatchTable);
    }
    else {
	update_dispatch_table(NULL);
    }
}
