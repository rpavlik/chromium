/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "replicatespu.h"
#include "cr_environment.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_string.h"
#include "cr_packfunctions.h"
#include "cr_url.h"

#include <arpa/inet.h>
#include <X11/Xmd.h>
#include <X11/extensions/vnc.h>



/**
 * This is called periodically to see if we have received any Xvnc
 * events from the X servers' VNC modules.
 */
void
replicatespuCheckVncEvents(void)
{
	if (replicate_spu.glx_display) {
		while (XPending(replicate_spu.glx_display)) {
#if 0
			int VncConn = replicate_spu.VncEventsBase + XVncConnected;
			int VncDisconn = replicate_spu.VncEventsBase + XVncDisconnected;
#endif
			int VncChromiumConn = replicate_spu.VncEventsBase + XVncChromiumConnected;
			XEvent event;
		
			XNextEvent (replicate_spu.glx_display, &event);

#if 0
			if (event.type == VncConn) {
				crWarning("Replicate SPU: Received VncConn");
			} 
			else
#endif
			if (event.type == VncChromiumConn) {
				XVncConnectedEvent *e = (XVncConnectedEvent*) &event;
				struct in_addr addr;

				addr.s_addr = e->ipaddress;
				crWarning("ReplicateSPU: someone just connected!\n");

				if (e->ipaddress) {
					replicatespuReplicate(e->ipaddress);
				}
				else {
					crWarning("Replicate SPU: Someone connected, but with no ipaddress?");
				}
			} 
#if 0
			else
			if (event.type == VncDisconn) {
				crWarning("Replicate SPU: Received VncDisconn");
			}
#endif
		}
	}
}



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


/**
 * Used by the following two crHashTableWalk callbacks.
 * They need to know which replicant/server we're talking to.
 */
static int ServerIndex = -1;


/**
 * Callback called by crHashTableWalk() below.
 * Used to create viewer-side windows for all the application windows.
 */
static void
replicatespuReplicateWindows(unsigned long key, void *data1, void *data2)
{
	ThreadInfo *thread = (ThreadInfo *) data2;
	WindowInfo *winInfo = (WindowInfo *) data1;
	GLint window = 0;
	GLint writeback = 1;
	Window root;
	int x, y;
	unsigned int width, height, bw, d;
	GLboolean unviewable = GL_FALSE;
	XWindowAttributes winAtt;

	CRASSERT(ServerIndex >= 0);

	/**
	 * Get application window's attributes
	 */
	old_xerror_handler = XSetErrorHandler( x11_error_handler );
	XSync(replicate_spu.glx_display, 0);
	XGetGeometry(replicate_spu.glx_display, (Window)winInfo->nativeWindow,
							 &root, &x, &y, &width, &height, &bw, &d );
	XGetWindowAttributes(replicate_spu.glx_display,
											 (Window) winInfo->nativeWindow, &winAtt);
	XSetErrorHandler( old_xerror_handler );
	if (!caught_x11_error) {
		 unviewable = (winAtt.map_state == IsUnviewable);
	}
	caught_x11_error = 0;

	/*
	 * Create the server-side window
	 */
	if (replicate_spu.swap)
		crPackWindowCreateSWAP( replicate_spu.dpyName, winInfo->visBits, &window, &writeback);
	else
		crPackWindowCreate( replicate_spu.dpyName, winInfo->visBits, &window, &writeback);

	replicatespuFlushOne(thread, ServerIndex);

	/* Get return value */
	while (writeback) {
		crNetRecv();
	}
	if (replicate_spu.swap)
		window = (GLint) SWAP32(window);

	/* save the server-side window ID */
	winInfo->id[ServerIndex] = window;

	crDebug("Replicate SPU: created server-side window %d", window);

	if (window < 0) {
		crWarning("Replicate SPU: failed to create server-side window");
		return;
	}

	XVncChromiumMonitor(replicate_spu.glx_display,
											window, winInfo->nativeWindow);

	/*
	 * If the app window is not visible, hide the server-side window too.
	 */
	if (unviewable)
	{
		if (replicate_spu.swap)
			crPackWindowShowSWAP( window, GL_FALSE );
		else
			crPackWindowShow( window, GL_FALSE );
		
		replicatespuFlushOne(thread, ServerIndex);
	}
}


/**
 * Callback called from crHashTableWalk() from in
 * replicatespuReplicate() to update window sizes.
 */
static void
replicatespuResizeWindows(unsigned long key, void *data1, void *data2)
{
	ThreadInfo *thread = (ThreadInfo *) data2;
	WindowInfo *winInfo = (WindowInfo *) data1;

	CRASSERT(ServerIndex >= 0);

	if (winInfo->width > 0) {
		if (replicate_spu.swap)
			crPackWindowSizeSWAP( winInfo->id[ServerIndex], winInfo->width, winInfo->height );
		else
			crPackWindowSize( winInfo->id[ServerIndex], winInfo->width, winInfo->height );

		replicatespuFlushOne(thread, ServerIndex);
	}

	/* XXX what's this for? */
#if 1
	if (winInfo->nativeWindow)
		replicatespuRePositionWindow(winInfo);
#endif
}


/**
 * Replicate our contexts on a new server (indicated by ServerIndex).
 */
static void
replicatespuReplicateContexts(unsigned long key, void *data1, void *data2)
{
	ThreadInfo *thread = (ThreadInfo *) data2;
	ContextInfo *context = (ContextInfo *) data1;
	CRContext *temp_c;
	GLint return_val = 0;
	int writeback;

	if (!context->State) /* XXX need this */
		return;

	/* Send CreateContext */
	if (replicate_spu.swap)
		crPackCreateContextSWAP( replicate_spu.dpyName, context->visBits, &return_val, &writeback);
	else
		crPackCreateContext( replicate_spu.dpyName, context->visBits, &return_val, &writeback);
			
	replicatespuFlushOne(thread, ServerIndex);

	/* Get return value */
	writeback = 1;
	while (writeback)
		crNetRecv();
	if (replicate_spu.swap)
		return_val = (GLint) SWAP32(return_val);

	if (return_val <= 0) {
		crWarning("Replicate SPU: CreateContext failed");
		return;
	}

	/* Fill in the new context info */
	temp_c = crStateCreateContext(NULL, context->visBits);

	context->rserverCtx[ServerIndex] = return_val;

	/* MakeCurrent */
	/* XXX Why are we making current here??? */
	{
		int serverWindow;
		if (context->currentWindow)
			serverWindow = context->currentWindow->id[ServerIndex];
		else
			serverWindow = 0;

		if (replicate_spu.swap)
			crPackMakeCurrentSWAP( serverWindow, 0, return_val );
		else
			crPackMakeCurrent( serverWindow, 0, return_val );
	}

	replicatespuFlushOne(thread, ServerIndex);

	/* Diff Context */
#if 0
	crDebug("BEFORE CONTEXT DIFF\n");
	replicatespuDebugOpcodes( &thread->packer->buffer );
#endif
#if 1
	/* Recreate all the textures */
	{
		unsigned int u;
		CRTextureState *to = &(context->State->texture);
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
	crStateDiffContext( temp_c, context->State );

#if 0
	crDebug("AFTER CONTEXT DIFF\n");
	replicatespuDebugOpcodes( &thread->packer->buffer );
#endif

	replicatespuFlushOne(thread, ServerIndex);

	/* Send over all the display lists for this context. The temporary
	 * context should have all the client information needed, so that
	 * we can restore correct client state after we're done.
	 */
	crDLMSetupClientState(&replicate_spu.diff_dispatch);
	crDLMSendAllDLMLists(replicate_spu.displayListManager, 
											 &replicate_spu.diff_dispatch);
	crDLMRestoreClientState(&temp_c->client, &replicate_spu.diff_dispatch);
			
	replicatespuFlushOne(thread, ServerIndex);

	/* DestroyContext (only the temporary one) */
	crStateDestroyContext( temp_c );
}


/**
 * This is the main routine responsible for replicating our GL state
 * for a new VNC viewer.  Called when we detect that a new VNC viewer
 * has been started.
 * \param ipaddress  the IP address where the new viewer is running.
 */
void
replicatespuReplicate(int ipaddress) 
{
	GET_THREAD(thread);
	struct in_addr addr;
	char *hosturl;
	char *ipstring;
	int r_slot;

	crDebug("Enter replicatespuReplicate(ipaddress=0x%x)", ipaddress);

	replicatespuFlush( (void *)thread );

#ifdef CHROMIUM_THREADSAFE_notyet
	crLockMutex(&_ReplicateMutex);
#endif

	/*
	 * find empty slot
	 */
	for (r_slot = 1; r_slot < CR_MAX_REPLICANTS; r_slot++) {
		if (!replicate_spu.rserver[r_slot].conn ||
				replicate_spu.rserver[r_slot].conn->type == CR_NO_CONNECTION)
			break;
	}
	if (r_slot == CR_MAX_REPLICANTS) {
		crWarning("Replicate SPU: no more replicant slots available");
		return;
	}

	/**
	 ** OK, now rserver[r_slot] is free for use.
	 **/

	if (replicate_spu.vncAvailable) {
		/* Find the mothership port that we're using and pass it to
		 * along to the VNC server.  The VNC server will, in turn, pass it
		 * on to the VNC viewer and chromium module.
		 */
		char protocol[100], hostname[1000];
		const char *mothershipURL;
		unsigned short mothershipPort;
		mothershipURL = crGetenv("CRMOTHERSHIP");
		crDebug("CRMOTHERSHIP env var = %s", mothershipURL);
		if (mothershipURL)
			crParseURL(mothershipURL, protocol, hostname, &mothershipPort,
								 DEFAULT_MOTHERSHIP_PORT);
		else
			mothershipPort = DEFAULT_MOTHERSHIP_PORT;
		XVncChromiumStart(replicate_spu.glx_display, ipaddress,
											CHROMIUM_START_PORT + r_slot, mothershipPort);
	}

	addr.s_addr = ipaddress;
	ipstring = inet_ntoa(addr);
	hosturl = crAlloc(crStrlen(ipstring) + 9);
	sprintf(hosturl, "tcpip://%s", ipstring);

	crDebug("Replicate SPU attaching to %s",hosturl);

	/* connect to the remote VNC server */
	replicate_spu.rserver[r_slot].name = crStrdup( replicate_spu.name );
	replicate_spu.rserver[r_slot].buffer_size = replicate_spu.buffer_size;
	replicate_spu.rserver[r_slot].conn
		= crNetConnectToServer( hosturl, CHROMIUM_START_PORT + r_slot,
														replicate_spu.rserver[r_slot].buffer_size, 1);

	/*
	 * Create server-side windows and contexts by walking tables of app windows
	 * and contexts.
	 */
	ServerIndex = r_slot;
	crHashtableWalk(replicate_spu.windowTable, replicatespuReplicateWindows, thread);
	crHashtableWalk(replicate_spu.contextTable, replicatespuReplicateContexts, thread);
	ServerIndex = -1;

	/* MakeCurrent, the current context */
	if (thread->currentContext) {
		int serverWindow = thread->currentContext->currentWindow->id[r_slot];
		int serverContext = thread->currentContext->rserverCtx[r_slot];
		if (replicate_spu.swap)
			crPackMakeCurrentSWAP(serverWindow, 0, serverContext);
	 	else
			crPackMakeCurrent(serverWindow, 0, serverContext);

		crStateMakeCurrent( thread->currentContext->State );
	}

	replicatespuFlushOne(thread, r_slot);

	/*
	 * Set window sizes
	 */
	ServerIndex = r_slot;
	crHashtableWalk(replicate_spu.windowTable, replicatespuResizeWindows, thread);
	ServerIndex = -1;

	crDebug("Replicate SPU: leaving replicatespuReplicate");

#ifdef CHROMIUM_THREADSAFE_notyet
	crUnlockMutex(&_ReplicateMutex);
#endif
}
