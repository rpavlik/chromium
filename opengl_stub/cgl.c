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

#define GET_CONTEXTINFO(c)  (ContextInfo *) crHashtableSearch( stub.contextTable, (unsigned long) (c) )


GLuint FindVisualInfo( CGLPixelFormatObj pix )
{
	GLuint b = 0;
	long val = 0;

	crDebug("FindVisualInfo");
	CRASSERT(pix);

	stub.wsInterface.CGLDescribePixelFormat( pix, 0, kCGLPFADepthSize, &val );
	if( val > 0 )
		b |= CR_DEPTH_BIT;

	stub.wsInterface.CGLDescribePixelFormat( pix, 0, kCGLPFAAccumSize, &val );
	if( val > 0 )
		b |= CR_ACCUM_BIT;

	stub.wsInterface.CGLDescribePixelFormat( pix, 0, kCGLPFAColorSize, &val );
	if( val > 8 )
		b |= CR_RGB_BIT;

	stub.wsInterface.CGLDescribePixelFormat( pix, 0, kCGLPFAStencilSize, &val );
	if( val > 0 )
		b |= CR_STENCIL_BIT;

	stub.wsInterface.CGLDescribePixelFormat( pix, 0, kCGLPFAAlphaSize, &val );
	if( val > 0 )
		b |= CR_ALPHA_BIT;

	stub.wsInterface.CGLDescribePixelFormat( pix, 0, kCGLPFADoubleBuffer, &val );
	if( val )
		b |= CR_DOUBLE_BIT;

	stub.wsInterface.CGLDescribePixelFormat( pix, 0, kCGLPFAStereo, &val );
	if( val )
		b |= CR_STEREO_BIT;

	return b;
}


CGLError CGLCreateContext( CGLPixelFormatObj pix, CGLContextObj share, CGLContextObj *ctx )
{
	crDebug("CGLCreateContext");
	stubInit();
	return stubCreateContext( pix, share, ctx );
}


CGLError CGLDestroyContext( CGLContextObj ctx )
{
	crDebug("CGLDestroyContext");
	stubDestroyContext( (unsigned long) ctx );
	return noErr;
}


CGLError CGLSetCurrentContext( CGLContextObj ctx )
{
//	crDebug("CGLSetCurrentContext");
	stubInit();
	return ( stubMakeCurrent(NULL, GET_CONTEXTINFO(ctx), GL_FALSE) ? noErr : kCGLBadContext );
}


CGLError CGLFlushDrawable( CGLContextObj ctx )
{
//	crDebug("CGLFlushDrawable");
	stubSwapContextBuffers( GET_CONTEXTINFO(ctx), 0);
	return noErr;
}

#define DEBUG_ENTRY(s)  \
	case (s):			\
		crDebug(#s);	\
		break;

CGLError CGLChoosePixelFormat( const CGLPixelFormatAttribute *attribList, CGLPixelFormatObj *pix, long *npix )
{
	CGLPixelFormatAttribute *attrib = attribList;
	CGLPixelFormatAttribute attribCopy[128];
	int copy=0;

	crDebug("CGLChoosePixelFormat");
	stubInit();

	/* 
	 * NOTE!!!
	 * Here we're telling the renderspu not to use the GDI
	 * equivalent's of ChoosePixelFormat/DescribePixelFormat etc
	 * There are subtle differences in the use of these calls.
	 */
	crSetenv("CR_WGL_DO_NOT_USE_GDI", "yes");

	for( ; *attrib != NULL; attrib++ ) {
		attribCopy[copy++] = *attrib;

		switch( *attrib ) {
		case kCGLPFADoubleBuffer:
			crDebug("kCGLPFADoubleBuffer");
			stub.desiredVisual |= CR_DOUBLE_BIT;
			break;

		case kCGLPFAStereo:
			crDebug("kCGLPFAStereo");
			stub.desiredVisual |= CR_STEREO_BIT;
			break;

		case kCGLPFAAuxBuffers:
			crDebug("kCGLPFAAuxBuffers: %i", attrib[0]);
			if( attrib[0] != 0 )
				crWarning("CGLChoosePixelFormat: aux_buffers=%d unsupported", attrib[0]);
			attribCopy[copy++] = *(++attrib);
			break;

		case kCGLPFAColorSize:
			crDebug("kCGLPFAColorSize: %i", attrib[0]);
			if( attrib[0] > 0 )
				stub.desiredVisual |= CR_RGB_BIT;
			attribCopy[copy++] = *(++attrib);
			break;

		case kCGLPFAAlphaSize:
			crDebug("kCGLPFAAlphaSize: %i", attrib[0]);
			if( attrib[0] > 0 )
				stub.desiredVisual |= CR_ALPHA_BIT;
			attribCopy[copy++] = *(++attrib);
			break;

		case kCGLPFADepthSize:
			crDebug("kCGLPFADepthSize: %i", attrib[0]);
			if( attrib[0] > 0 )
				stub.desiredVisual |= CR_DEPTH_BIT;
			attribCopy[copy++] = *(++attrib);
			break;

		case kCGLPFAStencilSize:
			crDebug("kCGLPFAStencilSize: %i", attrib[0]);
			if( attrib[0] > 0 )
				stub.desiredVisual |= CR_STENCIL_BIT;
			attribCopy[copy++] = *(++attrib);
			break;

		case kCGLPFAAccumSize:
			crDebug("kCGLPFAAccumSize: %i", attrib[0]);
			if( attrib[0] > 0 )
				stub.desiredVisual |= CR_ACCUM_BIT;
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
			crDebug("kCGLPFARendererID: %i", attrib[0]);
			attribCopy[copy++] = *(++attrib);
			break;

		case kCGLPFADisplayMask:
			crDebug("kCGLPFADisplayMask: %i", attrib[0]);
			attribCopy[copy++] = *(++attrib);
			break;

		case kCGLPFAVirtualScreenCount:
			crDebug("kCGLPFAVirtualScreenCount: %i", attrib[0]);
			attribCopy[copy++] = *(++attrib);
			break;

		default:
			crError("CGLChoosePixelFormat: doesn't support 0x%x", *attrib);
		}
	}

	attribCopy[copy++] = NULL;
	crDebug("CGLChoosePixelFormat num=%i", copy);

	if( stub.haveNativeOpenGL ) {
		stub.wsInterface.CGLChoosePixelFormat( attribList, pix, npix );
		if( *pix )
			stub.desiredVisual = FindVisualInfo( *pix );
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
	crDebug("CGLDescribePixelFormat");
	stubInit();

	/* the max PFD index */
	return stub.wsInterface.CGLDescribePixelFormat( pix, pix_num, attrib, value );
}


CGLContextObj CGLGetCurrentContext( void )
{
	crDebug("CGLGetCurrentContext");
	return (CGLContextObj) ( stub.currentContext ? stub.currentContext->id : NULL );
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
	stubInit();
	crDebug("CGLQueryRendererInfo");
	return stub.wsInterface.CGLQueryRendererInfo( display_mask, rend, nrend );;
}


CGLError CGLDestroyRendererInfo( CGLRendererInfoObj rend ) {
	crDebug( "CGLDestroyRendererInfo" );
	return stub.wsInterface.CGLDestroyRendererInfo( rend );
}


CGLError CGLDescribeRenderer( CGLRendererInfoObj rend, long rend_num, CGLRendererProperty prop, long *value ) {
	crDebug( "CGLDescribeRenderer" );
	return stub.wsInterface.CGLDescribeRenderer( rend, rend_num, prop, value );
}


CGLError CGLSetOffScreen( CGLContextObj ctx, long width, long height, long rowbytes, void *baseaddr ) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	crDebug( "CGLSetOffScreen" );
	return stub.wsInterface.CGLSetOffScreen( context->cglc, width, height, rowbytes, baseaddr );
}


CGLError CGLGetOffScreen( CGLContextObj ctx, long *width, long *height, long *rowbytes, void **baseaddr ) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	crDebug( "CGLGetOffScreen" );
	return stub.wsInterface.CGLGetOffScreen( context->cglc, width, height, rowbytes, baseaddr );
}


CGLError CGLSetFullScreen( CGLContextObj ctx ) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	crDebug( "CGLSetFullScreen" );
	return stub.wsInterface.CGLSetFullScreen( context->cglc );
}


CGLError CGLClearDrawable( CGLContextObj ctx ) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	crDebug( "CGLClearDrawable" );
	return stub.wsInterface.CGLClearDrawable( context->cglc );
}


CGLError CGLEnable( CGLContextObj ctx, CGLContextEnable pname ) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	crDebug( "CGLEnable" );
	return stub.wsInterface.CGLEnable( context->cglc, pname );
}


CGLError CGLDisable( CGLContextObj ctx, CGLContextEnable pname ) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	crDebug( "CGLDisable" );
	return stub.wsInterface.CGLDisable( context->cglc, pname );
}


CGLError CGLIsEnabled( CGLContextObj ctx, CGLContextEnable pname, long *enable ) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	crDebug( "CGLIsEnabled" );

	if( !context )
		return kCGLBadContext;

	return stub.wsInterface.CGLIsEnabled( context->cglc, pname, enable );
}


/*
 * If the context is undecided, that means it hasnt been created yet; therefore,
 *  we'll put off setting these values until the context has been created.
 */
CGLError CGLSetParameter( CGLContextObj ctx, CGLContextParameter pname, const long *params ) {
	CGLError retval = noErr;
	ContextInfo *ci = GET_CONTEXTINFO( ctx );

	if( ci->type == UNDECIDED ) {
		if( pname == kCGLCPSwapRectangle ) {
			crDebug( "CGLSetParameter: SwapRec: {%i %i %i %i}", params[0], params[1], params[2], params[3] );
			ci->parambits |= VISBIT_SWAP_RECT;
			ci->swap_rect[0] = params[0];
			ci->swap_rect[1] = params[1];
			ci->swap_rect[2] = params[2];
			ci->swap_rect[3] = params[3];
		} else

		if( pname == kCGLCPSwapInterval ) {
			crDebug( "CGLSetParameter: SwapInterval: %i", *params );
			ci->parambits |= VISBIT_SWAP_INTERVAL;
			ci->swap_interval = *params;
		} else

		if( pname == kCGLCPClientStorage ) {
			crDebug( "CGLSetParameter: ClientStorage: %i", *params );
			ci->parambits |= VISBIT_CLIENT_STORAGE;
			ci->client_storage = (unsigned long) *params;
		}
	} else {
		crDebug( "CGLSetParameter (Native) %i %i %i", pname, ctx, params[0] );
		retval = stub.wsInterface.CGLSetParameter( ci->cglc, pname, params );
	}

	return retval;
}


CGLError CGLGetParameter(CGLContextObj ctx, CGLContextParameter pname, long *params) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	crDebug( "CGLGetParameter" );
	return stub.wsInterface.CGLGetParameter( context->cglc, pname, params );
}


CGLError CGLSetVirtualScreen(CGLContextObj ctx, long screen) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	crDebug( "CGLSetVirtualScreen" );
	return stub.wsInterface.CGLSetVirtualScreen( context->cglc, screen );
}


CGLError CGLGetVirtualScreen(CGLContextObj ctx, long *screen) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	crDebug( "CGLGetVirtualScreen" );
	return stub.wsInterface.CGLGetVirtualScreen( context->cglc, screen );
}


CGLError CGLSetOption(CGLGlobalOption pname, long param) {
	crDebug( "CGLSetOption" );
	if( !stub.wsInterface.CGLSetOption )
		stubInit();
	return stub.wsInterface.CGLSetOption( pname, param );
}


CGLError CGLGetOption(CGLGlobalOption pname, long *param) {
	crDebug( "CGLGetOption" );
	return stub.wsInterface.CGLGetOption( pname, param );
}


void CGLGetVersion(long *majorvers, long *minorvers) {
	crDebug( "CGLGetVersion" );
//	stub.wsInterface.CGLGetVersion( majorvers, minorvers );
	*majorvers = 1;
	*minorvers = 4;
}


const char *CGLErrorString( CGLError err ) {
	crDebug( "CGLErrorString( %i )", err );
	return stub.wsInterface.CGLErrorString( err );
}


GLboolean gluCheckExtension( const GLubyte *extName, const GLubyte *extString ) {
	return false;
}


/*
 * I don't know if the parameters for these last functions are right at all
 */
CGLError CGLSetSurface( CGLContextObj ctx, unsigned long b, unsigned long c, unsigned long d ) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	crDebug( "CGLSetSurface: %i %i %i %i", ctx, b, c, d );
	if( context->type == UNDECIDED )
		crDebug("it doesnt know what it wants to be!");
	return stub.wsInterface.CGLSetSurface( context->cglc, b, c, d );
}


CGLError CGLGetSurface( CGLContextObj ctx, unsigned long b, unsigned long c, unsigned long d ) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	crDebug( "CGLGetSurface: %i %i %i %i", ctx, b, c, d );
	return stub.wsInterface.CGLGetSurface( context->cglc, b, c, d );
}

CGLError CGLUpdateContext( CGLContextObj ctx ) {
	ContextInfo *context = GET_CONTEXTINFO( ctx );
	crDebug( "CGLUpdateContext" );
	return stub.wsInterface.CGLUpdateContext( context->cglc );
}

