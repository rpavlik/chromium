/* File originally created as duplicate of wgl.c:
 * Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "chromium.h"
#include "cr_error.h"
#include "cr_spu.h"
#include "cr_environment.h"
#include "stub.h"

#define GET_CONTEXTINFO(c) \
	(ContextInfo *) crHashtableSearch( stub.contextTable, (unsigned long) (c) )

#define MakeCurrent(w,c)	\
	( stubMakeCurrent((w), (c)) ? noErr : kCGLBadContext )

#if 1
#define DEBUG_FUNCTION(s)	crDebug(#s)
#else
#define DEBUG_FUNCTION(s)	
#endif

static GLuint desiredVisual = CR_RGB_BIT;


/**
 * Compute a mask of CR_*_BIT flags which reflects the attributes of
 * the given pixel format.
 */
static GLuint ComputeVisBits( CGLPixelFormatObj pix )
{
	GLuint bits = 0;
	long val = 0;

	CRASSERT(pix);

#define DESCRIBE(name, v) \
	stub.wsInterface.CGLDescribePixelFormat( pix, 0, (name), &(v) )

	DESCRIBE(kCGLPFADepthSize, val);
	if( val > 0 )
		bits |= CR_DEPTH_BIT;

	DESCRIBE(kCGLPFAAccumSize, val);
	if( val > 0 )
		bits |= CR_ACCUM_BIT;

	DESCRIBE(kCGLPFAColorSize, val);
	if( val > 8 )
		bits |= CR_RGB_BIT;

	DESCRIBE(kCGLPFAStencilSize, val);
	if( val > 0 )
		bits |= CR_STENCIL_BIT;

	DESCRIBE(kCGLPFAAlphaSize, val);
	if( val > 0 )
		bits |= CR_ALPHA_BIT;

	DESCRIBE(kCGLPFADoubleBuffer, val);
	if( val )
		bits |= CR_DOUBLE_BIT;

	DESCRIBE(kCGLPFAStereo, val);
	if( val )
		bits |= CR_STEREO_BIT;

	return bits;
}

/*
 * Darwin swaps by context, not window
 */
void stubSwapContextBuffers( const ContextInfo *context, GLint flags )
{
	if( !context )
		return;

	if( context->type == NATIVE ) {
		stub.wsInterface.CGLUpdateContext( context->cglc );
	} else if( context->type == CHROMIUM ) {
		if( context->currentDrawable )
			stubSwapBuffers( context->currentDrawable, 0 );
		else
			crDebug("stubSwapContextBuffers: Not sure what to do with chromium context buffers.");
	} else {
		crDebug("Calling SwapContextBuffers on a window we haven't seen before (no-op).");
	}
}


CGLError
CGLCreateContext( CGLPixelFormatObj pix, CGLContextObj share, CGLContextObj *ctx )
{
	ContextInfo *context;

	DEBUG_FUNCTION(CGLCreateContext);
	stubInit();

	CRASSERT(stub.contextTable);

	if( stub.haveNativeOpenGL )
		desiredVisual |= ComputeVisBits( pix );

	context = stubNewContext("", desiredVisual, UNDECIDED);
	if (!context)
		return 0;

	context->share = (ContextInfo *) crHashtableSearch(stub.contextTable, (unsigned long) share);

	// Store extra info
	if( stub.haveNativeOpenGL )
		stub.wsInterface.CGLDescribePixelFormat( pix, 0, kCGLPFADisplayMask, &context->disp_mask );

	*ctx = (CGLContextObj) context->id;
	return noErr;
}


CGLError CGLDestroyContext( CGLContextObj ctx )
{
	stubDestroyContext( (unsigned long) ctx );
	return noErr;
}


CGLError CGLSetCurrentContext( CGLContextObj ctx )
{
	ContextInfo *context = GET_CONTEXTINFO(ctx);

//	DEBUG_FUNCTION(CGLSetCurrentContext);

	stubInit();

	if( context->currentDrawable )
		return MakeCurrent( context->currentDrawable, context );

	/* Don't do anything until we have a window (and the context with it) */
	return noErr;
}


CGLError CGLFlushDrawable( CGLContextObj ctx )
{
	ContextInfo *context = GET_CONTEXTINFO(ctx);
//	DEBUG_FUNCTION(CGLFlushDrawable);

	stubSwapContextBuffers( context, 0 );
	return noErr;
}

#define DEBUG_ENTRY(s)  \
	case (s):			\
		crDebug("CGLChoosePixelFormat: " #s);	\
		break;

CGLError CGLChoosePixelFormat( const CGLPixelFormatAttribute *attribList, CGLPixelFormatObj *pix, long *npix )
{
	CGLPixelFormatAttribute* attrib = (CGLPixelFormatAttribute*)attribList;
	CGLPixelFormatAttribute attribCopy[128];
	int copy=0;

	stubInit();

	/* 
	 * NOTE!!!
	 * Here we're telling the renderspu not to use the GDI
	 * equivalent's of ChoosePixelFormat/DescribePixelFormat etc
	 * There are subtle differences in the use of these calls.
	 */
//	crSetenv("CR_WGL_DO_NOT_USE_GDI", "yes");

	for( ; *attrib != 0; attrib++ ) {
		attribCopy[copy++] = *attrib;

		switch( *attrib ) {
		case kCGLPFADoubleBuffer:
			crDebug("CGLChoosePixelFormat: kCGLPFADoubleBuffer");
			desiredVisual |= CR_DOUBLE_BIT;
			break;

		case kCGLPFAStereo:
			crDebug("CGLChoosePixelFormat: kCGLPFAStereo");
			desiredVisual |= CR_STEREO_BIT;
			break;

		case kCGLPFAAuxBuffers:
			crDebug("CGLChoosePixelFormat: kCGLPFAAuxBuffers: %i", attrib[0]);
			if( attrib[0] != 0 )
				crWarning("CGLChoosePixelFormat: aux_buffers=%d unsupported", attrib[0]);
			attribCopy[copy++] = *(++attrib);
			break;

		case kCGLPFAColorSize:
			crDebug("CGLChoosePixelFormat: kCGLPFAColorSize: %i", attrib[0]);
			if( attrib[0] > 0 )
				desiredVisual |= CR_RGB_BIT;
			attribCopy[copy++] = *(++attrib);
			break;

		case kCGLPFAAlphaSize:
			crDebug("CGLChoosePixelFormat: kCGLPFAAlphaSize: %i", attrib[0]);
			if( attrib[0] > 0 )
				desiredVisual |= CR_ALPHA_BIT;
			attribCopy[copy++] = *(++attrib);
			break;

		case kCGLPFADepthSize:
			crDebug("CGLChoosePixelFormat: kCGLPFADepthSize: %i", attrib[0]);
			if( attrib[0] > 0 )
				desiredVisual |= CR_DEPTH_BIT;
			attribCopy[copy++] = *(++attrib);
			break;

		case kCGLPFAStencilSize:
			crDebug("CGLChoosePixelFormat: kCGLPFAStencilSize: %i", attrib[0]);
			if( attrib[0] > 0 )
				desiredVisual |= CR_STENCIL_BIT;
			attribCopy[copy++] = *(++attrib);
			break;

		case kCGLPFAAccumSize:
			crDebug("CGLChoosePixelFormat: kCGLPFAAccumSize: %i", attrib[0]);
			if( attrib[0] > 0 )
				desiredVisual |= CR_ACCUM_BIT;
			attribCopy[copy++] = *(++attrib);
			break;

		DEBUG_ENTRY(kCGLPFAAllRenderers)
		DEBUG_ENTRY(kCGLPFAMinimumPolicy)
		DEBUG_ENTRY(kCGLPFAMaximumPolicy)
		DEBUG_ENTRY(kCGLPFAOffScreen)
		DEBUG_ENTRY(kCGLPFAFullScreen)
		DEBUG_ENTRY(kCGLPFASingleRenderer)
		DEBUG_ENTRY(kCGLPFANoRecovery)
		DEBUG_ENTRY(kCGLPFAAccelerated)
		DEBUG_ENTRY(kCGLPFAClosestPolicy)
		DEBUG_ENTRY(kCGLPFARobust)
		DEBUG_ENTRY(kCGLPFABackingStore)
		DEBUG_ENTRY(kCGLPFAMPSafe)
		DEBUG_ENTRY(kCGLPFAMultiScreen)
		DEBUG_ENTRY(kCGLPFACompliant)
		DEBUG_ENTRY(kCGLPFAWindow)

		case kCGLPFARendererID:
			crDebug("CGLChoosePixelFormat: kCGLPFARendererID: %i", attrib[0]);
			attribCopy[copy++] = *(++attrib);
			break;

		case kCGLPFADisplayMask:
			crDebug("CGLChoosePixelFormat: kCGLPFADisplayMask: %i", attrib[0]);
			attribCopy[copy++] = *(++attrib);
			break;

		case kCGLPFAVirtualScreenCount:
			crDebug("CGLChoosePixelFormat: kCGLPFAVirtualScreenCount: %i", attrib[0]);
			attribCopy[copy++] = *(++attrib);
			break;

		default:
			crError("CGLChoosePixelFormat: doesn't support 0x%x", *attrib);
			break;
		}
	}

	attribCopy[copy++] = 0;

	if( stub.haveNativeOpenGL ) {
		stub.wsInterface.CGLChoosePixelFormat( attribList, pix, npix );
		if( *pix )
			desiredVisual = ComputeVisBits( *pix );
		else
			crDebug("Couldnt choose pixel format?");
	}
	else {
		crDebug("I dont know what to do without the native OpenGL libraries!");
	}

	return noErr;
}


CGLError CGLDescribePixelFormat( CGLPixelFormatObj pix, long pix_num, CGLPixelFormatAttribute attrib, long *value )
{
	int visBits = 0;

	stubInit();

	if( stub.haveNativeOpenGL )
		return stub.wsInterface.CGLDescribePixelFormat( pix, pix_num, attrib, value );

	// TODO -- this needs to be filled out properly
	switch( attrib ) {
		case kCGLPFAAllRenderers:
			*value = 0;
			break;

		case kCGLPFADoubleBuffer:
			*value = 1;
			break;

		case kCGLPFAStereo:
			*value = 1;
			break;

		case kCGLPFAAuxBuffers:
			*value = 0;
			break;

		case kCGLPFAColorSize:
			*value = 24;
			break;

		case kCGLPFAAlphaSize:
			*value = 8;
			break;

		case kCGLPFADepthSize:
			visBits |= CR_DEPTH_BIT;
			*value = 16;
			break;

		case kCGLPFAStencilSize:
			visBits |= CR_STENCIL_BIT;
			*value = 8;
			break;

		case kCGLPFAAccumSize:
			visBits |= CR_ACCUM_BIT;
			*value = 0;
			break;

		case kCGLPFAMinimumPolicy:
			*value = 0;
			break;

		case kCGLPFAMaximumPolicy:
			*value = 0;
			break;

		case kCGLPFAOffScreen:
			*value = 0;
			break;

		case kCGLPFAFullScreen:
			*value = 0;
			break;

		case kCGLPFASampleBuffers:
			*value = 0;
			break;

		case kCGLPFASamples:
			*value = 0;
			break;

		case kCGLPFAAuxDepthStencil:
			*value = 0;
			break;

		case kCGLPFAColorFloat:
			*value = 0;
			break;

		case kCGLPFARendererID:
			*value = 0;
			break;

		case kCGLPFASingleRenderer:
			*value = 0;
			break;

		case kCGLPFANoRecovery:
			*value = 0;
			break;

		case kCGLPFAAccelerated:
			*value = 0;
			break;

		case kCGLPFAClosestPolicy:
			*value = 0;
			break;

		case kCGLPFARobust:
			*value = 0;
			break;

		case kCGLPFABackingStore:
			*value = 0;
			break;

		case kCGLPFAMPSafe:
			*value = 0;
			break;

		case kCGLPFAWindow:
			*value = 0;
			break;

		case kCGLPFAMultiScreen:
			*value = 0;
			break;

		case kCGLPFACompliant:
			*value = 0;
			break;

		case kCGLPFADisplayMask:
			*value = 0;
			break;

		case kCGLPFAPBuffer:
			*value = 0;
			break;

		case kCGLPFAVirtualScreenCount:
			*value = 1;
			break;

		default:
			crError("CGLDescribePixelFormat: doesn't support 0x%x", attrib);
			break;
	}

	return noErr;
}


CGLContextObj CGLGetCurrentContext( void )
{
	return (CGLContextObj) ( stub.currentContext ? stub.currentContext->id : 0 );
}

///

CGLError CGLDestroyPixelFormat( CGLPixelFormatObj pix )
{
	crDebug("CGLDestroyPixelFormat");
	return stub.wsInterface.CGLDestroyPixelFormat( pix );
}


CGLError CGLCopyContext( CGLContextObj src, CGLContextObj dst, unsigned long mask )
{
	(void) src;
	(void) dst;
	(void) mask;
	crWarning( "Unsupported CGL Call: CGLCopyContext()" );
	return noErr;
}


CGLError CGLQueryRendererInfo( unsigned long display_mask, CGLRendererInfoObj *rend, long *nrend ) {
	DEBUG_FUNCTION(CGLQueryRendererInfo);
	stubInit();

	return stub.wsInterface.CGLQueryRendererInfo( display_mask, rend, nrend );;
}


CGLError CGLDestroyRendererInfo( CGLRendererInfoObj rend ) {
	DEBUG_FUNCTION(CGLDestroyRendererInfo);

	return stub.wsInterface.CGLDestroyRendererInfo( rend );
}


CGLError CGLDescribeRenderer( CGLRendererInfoObj rend, long rend_num, CGLRendererProperty prop, long *value ) {
	DEBUG_FUNCTION(CGLDescribeRenderer);

	return stub.wsInterface.CGLDescribeRenderer( rend, rend_num, prop, value );
}


CGLError CGLSetOffScreen( CGLContextObj ctx, long width, long height, long rowbytes, void *baseaddr ) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	DEBUG_FUNCTION(CGLSetOffScreen);

	return stub.wsInterface.CGLSetOffScreen( context->cglc, width, height, rowbytes, baseaddr );
}


CGLError CGLGetOffScreen( CGLContextObj ctx, long *width, long *height, long *rowbytes, void **baseaddr ) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	DEBUG_FUNCTION(CGLGetOffScreen);

	return stub.wsInterface.CGLGetOffScreen( context->cglc, width, height, rowbytes, baseaddr );
}


CGLError CGLSetFullScreen( CGLContextObj ctx ) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	DEBUG_FUNCTION(CGLSetFullScreen);

	return stub.wsInterface.CGLSetFullScreen( context->cglc );
}


CGLError CGLClearDrawable( CGLContextObj ctx ) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	DEBUG_FUNCTION(CGLClearDrawable);

	if( context->currentDrawable )
		return MakeCurrent( NULL, context );

	return stub.wsInterface.CGLClearDrawable( context->cglc );
}

#define CGL_OPT_SWAP_RECT	0x01
#define CGL_OPT_SWAP_LIMIT	0x02
#define CGL_OPT_RASTERIZE	0x04
#define CGL_OPT_STATE_VAL	0x10
#define CGL_OPT_DRAW_LINE	0x20

CGLError CGLEnable( CGLContextObj ctx, CGLContextEnable pname ) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	DEBUG_FUNCTION(CGLEnable);

	if( stub.haveNativeOpenGL )
		return stub.wsInterface.CGLEnable( context->cglc, pname );

	switch( pname ) {
	case kCGLCESwapRectangle:
		context->options |= CGL_OPT_SWAP_RECT;
		break;

	case kCGLCESwapLimit:
		context->options |= CGL_OPT_SWAP_LIMIT;
		break;

	case kCGLCERasterization:
		context->options |= CGL_OPT_RASTERIZE;
		break;

	case kCGLCEStateValidation:
		context->options |= CGL_OPT_STATE_VAL;
		break;

/*	case kCGLCEDrawSyncBlueLine:
		context->options |= CGL_OPT_DRAW_LINE;
		break;
*/
	default:
		crError("CGLEnable: doesn't support 0x%x", pname);
		break;
	}

	return noErr;
}


CGLError CGLDisable( CGLContextObj ctx, CGLContextEnable pname ) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	DEBUG_FUNCTION(CGLDisable);

	if( stub.haveNativeOpenGL )
		return stub.wsInterface.CGLDisable( context->cglc, pname );

	switch( pname ) {
	case kCGLCESwapRectangle:
		context->options &= ~CGL_OPT_SWAP_RECT;
		break;

	case kCGLCESwapLimit:
		context->options &= ~CGL_OPT_SWAP_LIMIT;
		break;

	case kCGLCERasterization:
		context->options &= ~CGL_OPT_RASTERIZE;
		break;

	case kCGLCEStateValidation:
		context->options &= ~CGL_OPT_STATE_VAL;
		break;

/*	case kCGLCEDrawSyncBlueLine:
		context->options &= ~CGL_OPT_DRAW_LINE;
		break;
*/
	default:
		crError("CGLDisable: doesn't support 0x%x", pname);
		break;
	}

	return noErr;
}


CGLError CGLIsEnabled( CGLContextObj ctx, CGLContextEnable pname, long *enable ) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	DEBUG_FUNCTION(CGLIsEnabled);

	if( stub.haveNativeOpenGL )
		return stub.wsInterface.CGLIsEnabled( context->cglc, pname, enable );

	switch( pname ) {
	case kCGLCESwapRectangle:
		*enable = ( context->options & CGL_OPT_SWAP_RECT ) != 0;
		break;

	case kCGLCESwapLimit:
		*enable = ( context->options & CGL_OPT_SWAP_LIMIT ) != 0;
		break;

	case kCGLCERasterization:
		*enable = ( context->options & CGL_OPT_RASTERIZE ) != 0;
		break;

	case kCGLCEStateValidation:
		*enable = ( context->options & CGL_OPT_STATE_VAL ) != 0;
		break;

/*	case kCGLCEDrawSyncBlueLine:
		*enable = ( context->options & CGL_OPT_DRAW_LINE ) != 0;
		break;
*/
	default:
		crError("CGLIsEnabled: doesn't support 0x%x", pname);
		break;
	}

	return noErr;
}


/*
 * If the context is undecided, that means it hasnt been created yet; therefore,
 *  we'll put off setting these values until the context has been created.
 */
CGLError CGLSetParameter( CGLContextObj ctx, CGLContextParameter pname, const long *params ) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );

	if( stub.haveNativeOpenGL && context->type != UNDECIDED ) {
		crDebug( "CGLSetParameter (Native) %i %i %i", ctx, pname, params[0] );
		return stub.wsInterface.CGLSetParameter( context->cglc, pname, params );
	}

	switch( pname ) {
	case kCGLCPSwapRectangle:
		crDebug( "CGLSetParameter: SwapRec: {%i %i %i %i}", params[0], params[1], params[2], params[3] );
		context->swap_rect[0] = params[0];
		context->swap_rect[1] = params[1];
		context->swap_rect[2] = params[2];
		context->swap_rect[3] = params[3];
		break;

	case kCGLCPSwapInterval:
		crDebug( "CGLSetParameter: SwapInterval: %i", *params );
		context->swap_interval = *params;
		break;

	case kCGLCPClientStorage:
		crDebug( "CGLSetParameter: ClientStorage: %i", *params );
		context->client_storage = (unsigned long) *params;
		break;

	case kCGLCPSurfaceOrder:
		crDebug( "CGLSetParameter: SurfaceOrder: %i", *params );
		context->surf_order = *params;
		break;

	case kCGLCPSurfaceOpacity:
		crDebug( "CGLSetParameter: SurfaceOpacy: %i", *params );
		context->surf_opacy = *params;
		break;

	default:
		crError("CGLSetParameter: doesn't support 0x%x", pname);
		break;
	}

	return noErr;
}


CGLError CGLGetParameter(CGLContextObj ctx, CGLContextParameter pname, long *params) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	DEBUG_FUNCTION(CGLGetParameter);

	if( stub.haveNativeOpenGL )
		return stub.wsInterface.CGLGetParameter( context->cglc, pname, params );

	switch( pname ) {
	case kCGLCPSwapRectangle:
		params[0] = context->swap_rect[0];
		params[1] = context->swap_rect[1];
		params[2] = context->swap_rect[2];
		params[3] = context->swap_rect[3];
		break;

	case kCGLCPSwapInterval:
		*params = context->swap_interval;
		break;

	case kCGLCPClientStorage:
		*params = context->client_storage;
		break;

	case kCGLCPSurfaceOrder:
		*params = context->surf_order;
		break;

	case kCGLCPSurfaceOpacity:
		*params = context->surf_opacy;
		break;

	default:
		crError("CGLGetParameter: doesn't support 0x%x", pname);
		break;
	}

	return noErr;
}


CGLError CGLSetVirtualScreen(CGLContextObj ctx, long screen) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	DEBUG_FUNCTION(CGLSetVirtualScreen);

	return stub.wsInterface.CGLSetVirtualScreen( context->cglc, screen );
}


CGLError CGLGetVirtualScreen(CGLContextObj ctx, long *screen) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	DEBUG_FUNCTION(CGLGetVirtualScreen);

	return stub.wsInterface.CGLGetVirtualScreen( context->cglc, screen );
}


CGLError CGLSetOption(CGLGlobalOption pname, long param) {
//	DEBUG_FUNCTION(CGLSetOption);
	crDebug("CGLSetOption( %i )", pname);

	stubInit();

	return stub.wsInterface.CGLSetOption( pname, param );
}


CGLError CGLGetOption(CGLGlobalOption pname, long *param) {
	DEBUG_FUNCTION(CGLGetOption);

	return stub.wsInterface.CGLGetOption( pname, param );
}


void CGLGetVersion(long *majorvers, long *minorvers) {
	DEBUG_FUNCTION(CGLGetVersion);

//	stub.wsInterface.CGLGetVersion( majorvers, minorvers );
	*majorvers = 1;
	*minorvers = 4;
}


const char *CGLErrorString( CGLError err ) {
	DEBUG_FUNCTION(CGLErrorString);

	return stub.wsInterface.CGLErrorString( err );
}


GLboolean gluCheckExtension( const GLubyte *extName, const GLubyte *extString ) {
	return false;
}


/*
 * Not too sure about these function parameters, but they're close enough that things work.
 */

CGLError CGLSetSurface( CGLContextObj ctx, CGSConnectionID connID, CGSWindowID winID, CGSSurfaceID surfaceID ) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	WindowInfo  *window  = stubGetWindowInfo( winID );
	DEBUG_FUNCTION(CGLSetSurface);

	window->connection = connID;
	window->surface = surfaceID;

	return MakeCurrent( window, context );
}

/*
 * Not too certain on these... but it should work. (although, nothing seems to use this..)
 */
CGLError CGLGetSurface( CGLContextObj ctx, CGSConnectionID connID, CGSWindowID winID, CGSSurfaceID *surfaceID ) {
	WindowInfo  *window  = stubGetWindowInfo( winID );
	DEBUG_FUNCTION(CGLGetSurface);

	if( connID != window->connection )
		return kCGLBadContext;

	*surfaceID = window->surface;

	return noErr;
}


CGLError CGLUpdateContext( CGLContextObj ctx ) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	DEBUG_FUNCTION(CGLUpdateContext);

	stubSwapContextBuffers( context, 0 );

	return noErr;
}

