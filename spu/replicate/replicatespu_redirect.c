/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "replicatespu.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_string.h"
#include "cr_packfunctions.h"

#include <arpa/inet.h>
#include <X11/Xmd.h>
#include <X11/extensions/vnc.h>

#if 0
static void replicatespuDebugOpcodes( CRPackBuffer *pack )
{
	unsigned char *tmp;
	for (tmp = pack->opcode_start; tmp > pack->opcode_current; tmp--)
	{
		crDebug( "  %d (0x%p, 0x%p)", *tmp, tmp, pack->opcode_current );
	}
	crDebug( "\n" );
}
#endif

static int caught_x11_error = 0;

static int (*old_xerror_handler)( Display *dpy, XErrorEvent *ev);

static int x11_error_handler( Display *dpy, XErrorEvent *ev )
{
	if (ev->error_code == BadWindow ||
	    ev->error_code == BadGC ||
	    ev->error_code == BadPixmap ||
 	    ev->error_code == BadDrawable) {
		caught_x11_error = 1;
	}
	else {
		if (old_xerror_handler) {
			(*old_xerror_handler)( dpy, ev );
		}
		caught_x11_error = 0;
	}
	return 0;
}


/*
 * Called from crHashtableWalk().  Basically, setup args and call
 * crStateTextureObjectDiff().
 */
static void TextureObjDiffCallback( unsigned long key, void *data1, void *data2 )
{
	CRContext *ctx = (CRContext *) data2;
	CRTextureObj *tobj = (CRTextureObj *) data1;
	unsigned int j = 0;
	CRbitvalue *bitID = ctx->bitid;
	CRbitvalue nbitID[CR_MAX_BITARRAY];

	if (!tobj)
		return;

	for (j=0;j<CR_MAX_BITARRAY;j++)
		nbitID[j] = ~bitID[j];

	crStateTextureObjectDiff(ctx, bitID, nbitID, tobj,
													 GL_TRUE /* always dirty */);
}

void replicatespuRePositionWindow(WindowInfo *winInfo)
{
	Window root;
	int x, y;
	unsigned int width, height, bw, d;
	XWindowAttributes winAtt;

	old_xerror_handler = XSetErrorHandler( x11_error_handler );

	XSync(replicate_spu.glx_display, 0);
	XGetGeometry(replicate_spu.glx_display, (Window)winInfo->nativeWindow, 
		      &root, &x, &y, &width, &height, &bw, &d);
	XGetWindowAttributes(replicate_spu.glx_display, (Window)winInfo->nativeWindow, &winAtt);
#if 0
	XMoveWindow(replicate_spu.glx_display, winInfo->nativeWindow, x, y);
#else
	XResizeWindow(replicate_spu.glx_display, winInfo->nativeWindow, width, height-1);
	XResizeWindow(replicate_spu.glx_display, winInfo->nativeWindow, width, height);
#endif
	/* Check if the window is mapped */
	if (winAtt.map_state == IsViewable) 
		winInfo->viewable = GL_TRUE;
  	else
		winInfo->viewable = GL_FALSE;

	XSync(replicate_spu.glx_display, 0);
	if (caught_x11_error) {
		caught_x11_error = 0;
	}
	XSetErrorHandler( old_xerror_handler );
}

static void replicatespuReCreateWindows(unsigned long key, void *data1, void *data2)
{
	ThreadInfo *thread = (ThreadInfo *) data2;
	WindowInfo *winInfo = (WindowInfo *) data1;
	GLint window = 0;
	GLint writeback = 1;
	Window root;
	int x, y;
	unsigned int width, height, bw, d;
	XWindowAttributes winAtt;

	old_xerror_handler = XSetErrorHandler( x11_error_handler );

	XSync(replicate_spu.glx_display, 0);
	XGetGeometry(replicate_spu.glx_display, (Window)winInfo->nativeWindow, &root, &x, &y, &width, &height, &bw, &d );

	XGetWindowAttributes(replicate_spu.glx_display, (Window)winInfo->nativeWindow, &winAtt);

	XSetErrorHandler( old_xerror_handler );
	
	if (caught_x11_error) {
		caught_x11_error = 0;
	}

	if (replicate_spu.swap)
		crPackWindowCreateSWAP( replicate_spu.dpyName, winInfo->visBits, &window, &writeback);
	else
		crPackWindowCreate( replicate_spu.dpyName, winInfo->visBits, &window, &writeback);

	replicatespuFlush( (void *)thread );

	/* Get return value */
	while (writeback) {
		crNetRecv();
	}

	if (replicate_spu.swap)
		window = (GLint) SWAP32(window);

	if (window < 0)
		crError("FAILED REPLICATION WINDOWCREATE\n");

	if (winAtt.map_state == IsUnviewable) 
	{
		if (replicate_spu.swap)
			crPackWindowShowSWAP( window, GL_FALSE );
		else
			crPackWindowShow( window, GL_FALSE );
		
		replicatespuFlush( (void *)thread );
	}
}

static void replicatespuRePositionWindows(unsigned long key, void *data1, void *data2)
{
	ThreadInfo *thread = (ThreadInfo *) data2;
	WindowInfo *winInfo = (WindowInfo *) data1;

	if (winInfo->width > 0) {
		if (replicate_spu.swap)
			crPackWindowSizeSWAP( winInfo->id, winInfo->width, winInfo->height );
		else
			crPackWindowSize( winInfo->id, winInfo->width, winInfo->height );

		replicatespuFlush( (void *)thread );
	}

#if 1
	if (winInfo->nativeWindow)
		replicatespuRePositionWindow(winInfo);
#endif
}

void replicatespuReplicateCreateContext(int ipaddress) 
{
	GET_THREAD(thread);
	struct in_addr addr;
	char *hosturl;
	char *ipstring;
	int r_slot;
	int slot;
	int writeback = 1;

	replicatespuFlush( (void *)thread );

#ifdef CHROMIUM_THREADSAFE_notyet
	crLockMutex(&_ReplicateMutex);
#endif

	for (r_slot = 1; r_slot < CR_MAX_REPLICANTS; r_slot++) {
		if (!replicate_spu.rserver[r_slot].conn || replicate_spu.rserver[r_slot].conn->type == CR_NO_CONNECTION)
			break;
	}
	if (r_slot == CR_MAX_REPLICANTS) {
		crWarning("UNABLE TO CONNECT CLIENT - NO MORE REPLICANT SLOTS\n");
		return;
	}

	if (replicate_spu.vncAvailable)
		XVncChromiumStart(replicate_spu.glx_display, ipaddress, CHROMIUM_START_PORT + r_slot);

	thread->broadcast = 0;

	addr.s_addr = ipaddress;
	ipstring = inet_ntoa(addr);
	hosturl = crAlloc(crStrlen(ipstring) + 9);
	sprintf(hosturl, "tcpip://%s", ipstring);

	crDebug("replicateSPU attaching to %s\n",hosturl);

	/* connect to the remote VNC server */
	replicate_spu.rserver[r_slot].name = crStrdup( replicate_spu.name );
	replicate_spu.rserver[r_slot].buffer_size = replicate_spu.buffer_size;
	replicate_spu.rserver[r_slot].conn = crNetConnectToServer( hosturl, CHROMIUM_START_PORT + r_slot, replicate_spu.rserver[r_slot].buffer_size, 1);

	/* hijack the current connection */
	thread->server.conn = replicate_spu.rserver[r_slot].conn;

/* WindowCreate */
	crHashtableWalk( replicate_spu.windowTable, replicatespuReCreateWindows, thread);

	for (slot = 0; slot < replicate_spu.numContexts; slot++) {
	    if (replicate_spu.context[slot].State != NULL) {
		CRContext *temp_c;
		GLint return_val = 0;

/* CreateContext */
		writeback = 1;

		if (replicate_spu.swap)
			crPackCreateContextSWAP( replicate_spu.dpyName, replicate_spu.context[slot].visBits, &return_val, &writeback);
		else
			crPackCreateContext( replicate_spu.dpyName, replicate_spu.context[slot].visBits, &return_val, &writeback);

		replicatespuFlush( (void *)thread );

		/* Get return value */
		while (writeback) {
			crNetRecv();
		}

		if (replicate_spu.swap)
			return_val = (GLint) SWAP32(return_val);

		if (!return_val)
			crError("FAILED REPLICATION CREATECONTEXT %d\n",return_val);
		/* Fill in the new context info */
		temp_c = crStateCreateContext(NULL, replicate_spu.context[slot].visBits);

		replicate_spu.context[slot].rserverCtx[r_slot] = return_val;

/* MakeCurrent */
		if (replicate_spu.swap)
			crPackMakeCurrentSWAP( replicate_spu.context[slot].currentWindow, 0, return_val );
		else
			crPackMakeCurrent( replicate_spu.context[slot].currentWindow, 0, return_val );

		replicatespuFlush( (void *)thread );

/* Diff Context */
#if 0
		crDebug("BEFORE CONTEXT DIFF\n");
		replicatespuDebugOpcodes( &thread->packer->buffer );
#endif
#if 1
		/* Recreate all the textures */
		{
			unsigned int u;
			CRTextureState *to = &(replicate_spu.context[slot].State->texture);
			crHashtableWalk(to->idHash, TextureObjDiffCallback, temp_c);

		/* Now troll the currentTexture bindings too.  
		 * Note that the callback function
	 	 * is set up to be called from a hashtable walk 
		 * (so it takes a key), so when
	 	 * we call it directly, we pass in a dummy key.
	 	 */
#define DUMMY_KEY 0
			for (u = 0; u < temp_c->limits.maxTextureUnits; u++) {
				TextureObjDiffCallback(DUMMY_KEY, (void *) to->unit[u].currentTexture1D, (void *)temp_c );
				TextureObjDiffCallback(DUMMY_KEY, (void *) to->unit[u].currentTexture2D, (void *)temp_c );
				TextureObjDiffCallback(DUMMY_KEY, (void *) to->unit[u].currentTexture3D, (void *)temp_c );
				TextureObjDiffCallback(DUMMY_KEY, (void *) to->unit[u].currentTextureCubeMap, (void *)temp_c );
			}
		}
#endif
		crStateDiffContext( temp_c, replicate_spu.context[slot].State );
#if 0
		crDebug("AFTER CONTEXT DIFF\n");
		replicatespuDebugOpcodes( &thread->packer->buffer );
#endif
		replicatespuFlush( (void *)thread );

		/* Send over all the display lists for this context. The temporary
		 * context should have all the client information needed, so that
		 * we can restore correct client state after we're done.
		 */
		crDLMSetupClientState(&replicate_spu.diff_dispatch);
		crDLMSendAllDLMLists(replicate_spu.context[slot].displayListManager, 
			&replicate_spu.diff_dispatch);
		crDLMRestoreClientState(&temp_c->client, &replicate_spu.diff_dispatch);

		replicatespuFlush( (void *)thread );

/* DestroyContext (only the temporary one) */
		crStateDestroyContext( temp_c );

	    }
	}


/* MakeCurrent, the current context */
	if (thread->currentContext) {

		if (replicate_spu.swap)
			crPackMakeCurrentSWAP( thread->currentContext->currentWindow, 0, thread->currentContext->rserverCtx[r_slot] );
	 	else
			crPackMakeCurrent( thread->currentContext->currentWindow, 0, thread->currentContext->rserverCtx[r_slot] );

		crStateMakeCurrent( thread->currentContext->State );
	}

	replicatespuFlush( (void *)thread );

	crHashtableWalk( replicate_spu.windowTable, replicatespuRePositionWindows, thread);

	replicatespuFlush( (void *)thread );

	/* Put back original server connection */
	thread->server.conn = replicate_spu.rserver[0].conn;
	thread->broadcast = 1;

	crDebug("Finished replicationSPU CreateContext\n");

#ifdef CHROMIUM_THREADSAFE_notyet
	crUnlockMutex(&_ReplicateMutex);
#endif
}
