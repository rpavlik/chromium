/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "cr_error.h" 
#include "cr_mem.h" 
#include "stub.h"
#include "api_templates.h"

/*
 * Returns -1 on error
 */
int APIENTRY crCreateContext( const char *dpyName, GLint visBits )
{
	ContextInfo *context;
	stubInit();
	context = stubNewContext(dpyName, visBits, UNDECIDED);
	return context ? (int) context->id : -1;
}

void APIENTRY crDestroyContext( GLint context )
{
  	stubDestroyContext(context);
}

void APIENTRY crMakeCurrent( GLint window, GLint context )
{
	WindowInfo *winInfo = (WindowInfo *)
		crHashtableSearch(stub.windowTable, (unsigned int) window);
	ContextInfo *contextInfo = (ContextInfo *)
		crHashtableSearch(stub.contextTable, context);
	if (contextInfo && contextInfo->type == NATIVE) {
		crWarning("Can't call crMakeCurrent with native GL context");
		return;
	}
	stubMakeCurrent(winInfo, contextInfo);
}

GLint APIENTRY crGetCurrentContext( void )
{
	stubInit();
	if (stub.currentContext)
	  return (GLint) stub.currentContext->id;
	else
	  return 0;
}

GLint APIENTRY crGetCurrentWindow( void )
{
	stubInit();
	if (stub.currentContext && stub.currentContext->currentDrawable)
	  return stub.currentContext->currentDrawable->spuWindow;
	else
	  return -1;
}

void APIENTRY crSwapBuffers( GLint window, GLint flags )
{
	const WindowInfo *winInfo = (const WindowInfo *)
		crHashtableSearch(stub.windowTable, (unsigned int) window);
	if (winInfo)
		stubSwapBuffers(winInfo, flags);
}

/*
 * Returns -1 on error
 */
GLint APIENTRY crWindowCreate( const char *dpyName, GLint visBits )
{
	stubInit();
	return stubNewWindow( dpyName, visBits );
}

void APIENTRY crWindowDestroy( GLint window )
{
	WindowInfo *winInfo = (WindowInfo *)
		crHashtableSearch(stub.windowTable, (unsigned int) window);
	if (winInfo && winInfo->type == CHROMIUM) {
		stub.spu->dispatch_table.WindowDestroy( winInfo->spuWindow );
		crHashtableDelete(stub.windowTable, window, crFree);
	}
}

void APIENTRY crWindowSize( GLint window, GLint w, GLint h )
{
	const WindowInfo *winInfo = (const WindowInfo *)
		crHashtableSearch(stub.windowTable, (unsigned int) window);
	if (winInfo && winInfo->type == CHROMIUM)
		stub.spu->dispatch_table.WindowSize( window, w, h );
}

void APIENTRY crWindowPosition( GLint window, GLint x, GLint y )
{
	const WindowInfo *winInfo = (const WindowInfo *)
		crHashtableSearch(stub.windowTable, (unsigned int) window);
	if (winInfo && winInfo->type == CHROMIUM)
		stub.spu->dispatch_table.WindowPosition( window, x, y );
}

void APIENTRY crWindowShow( GLint window, GLint flag )
{
	const WindowInfo *winInfo = (const WindowInfo *)
		crHashtableSearch(stub.windowTable, (unsigned int) window);
	if (winInfo && winInfo->type == CHROMIUM)
		stub.spu->dispatch_table.WindowShow( window, flag );
}

void APIENTRY stub_GetChromiumParametervCR( GLenum target, GLuint index, GLenum type, GLsizei count, GLvoid *values )
{
	char **ret;
	switch( target )
	{
		case GL_HEAD_SPU_NAME_CR:
			ret = (char **) values;
			*ret = stub.spu->name;
			return;
		default:
			stub.spu->dispatch_table.GetChromiumParametervCR( target, index, type, count, values );
			break;
	}
}


PICAerror APIENTRY crPicaListCompositors(const PICAuint *config, 
					     PICAint *numResults, 
					     PICAcompItem *results)
{
     return stub.spu->dispatch_table.PicaListCompositors(config, 
						      numResults, 
						      results);
}
PICAerror APIENTRY crPicaGetCompositorParamiv(PICAcompID compositor,
						  PICAparam pname,
						  PICAint *params)
{
     return stub.spu->dispatch_table.PicaGetCompositorParamiv(compositor,
							   pname, 
							   params);
}
PICAerror APIENTRY crPicaGetCompositorParamfv(PICAcompID compositor,
						  PICAparam pname,
						  PICAfloat *params)
{
     return stub.spu->dispatch_table.PicaGetCompositorParamfv(compositor,
							   pname, 
							   params);
}
PICAerror APIENTRY crPicaGetCompositorParamcv(PICAcompID compositor,
						  PICAparam pname,
						  PICAchar **params)
{
     return stub.spu->dispatch_table.PicaGetCompositorParamcv(compositor,
							   pname, 
							   params);
}
PICAerror APIENTRY crPicaListNodes(PICAcompID compositor, 
				   PICAint *num,
				   PICAnodeItem *results)
{
     return stub.spu->dispatch_table.PicaListNodes(compositor, num, results);
}

PICAcontextID APIENTRY crPicaCreateContext(const PICAuint *config, 
					  const PICAnodeID *nodes, 
					  PICAuint numNodes)
{
     return stub.spu->dispatch_table.PicaCreateContext(config, nodes, numNodes);
}
PICAerror APIENTRY crPicaDestroyContext(PICAcontextID ctx)
{
     return stub.spu->dispatch_table.PicaDestroyContext(ctx);
}

PICAerror APIENTRY crPicaSetContextParami(PICAcontextID ctx,
					      PICAparam pname,
					      PICAint param)
{
     return stub.spu->dispatch_table.PicaSetContextParami(ctx, pname, param);
}
PICAerror APIENTRY crPicaSetContextParamiv(PICAcontextID ctx,
					       PICAparam pname,
					       const PICAint *param)
{
     return stub.spu->dispatch_table.PicaSetContextParamiv(ctx, pname, param);
}
PICAerror APIENTRY crPicaSetContextParamf(PICAcontextID ctx,
					      PICAparam pname,
					      PICAfloat param)
{
     return stub.spu->dispatch_table.PicaSetContextParamf(ctx, pname, param);
}
PICAerror APIENTRY crPicaSetContextParamfv(PICAcontextID ctx,
					       PICAparam pname,
					       const PICAfloat *param)
{
     return stub.spu->dispatch_table.PicaSetContextParamfv(ctx, pname, param);
}
PICAerror APIENTRY crPicaSetContextParamv(PICAcontextID ctx,
					      PICAparam pname,
					      const PICAvoid *param)
{
     return stub.spu->dispatch_table.PicaSetContextParamv(ctx, pname, param);
}

PICAerror APIENTRY crPicaGetContextParamiv(PICAcontextID ctx,
					       PICAparam pname,
					       PICAint *param)
{
     return stub.spu->dispatch_table.PicaGetContextParamiv(ctx, pname, param);
} 
PICAerror APIENTRY crPicaGetContextParamfv(PICAcontextID ctx,
					       PICAparam pname,
					       PICAfloat *param)
{
     return stub.spu->dispatch_table.PicaGetContextParamfv(ctx, pname, param);
}  
PICAerror APIENTRY crPicaGetContextParamcv(PICAcontextID ctx,
					       PICAparam pname,
					       PICAchar **param) 
{
     return stub.spu->dispatch_table.PicaGetContextParamcv(ctx, pname, param);
} 
PICAerror APIENTRY crPicaGetContextParamv(PICAcontextID ctx,
					      PICAparam pname,
					      PICAvoid *param)
{
     return stub.spu->dispatch_table.PicaGetContextParamv(ctx, pname, param);
}

PICAcontextID APIENTRY crPicaBindLocalContext(PICAcontextID globalCtx, 
						  PICAnodeID node)
{
     return stub.spu->dispatch_table.PicaBindLocalContext(globalCtx, node);
}
PICAerror APIENTRY crPicaDestroyLocalContext(PICAcontextID lctx)
{
     return stub.spu->dispatch_table.PicaDestroyLocalContext(lctx);
}

PICAerror APIENTRY crPicaStartFrame(PICAcontextID lctx,
					const PICAframeID *frameID,
					PICAuint numLocalFramelets,
					PICAuint numOrders,
					const PICAuint *orderList,
					const PICArect *srcRectList,
					const PICApoint *dstList)
{
     return stub.spu->dispatch_table.PicaStartFrame(lctx, 
						    frameID, 
						    numLocalFramelets, 
						    numOrders, 
						    orderList, 
						    srcRectList, 
						    dstList);
}
PICAerror APIENTRY crPicaEndFrame(PICAcontextID lctx)
{
     return stub.spu->dispatch_table.PicaEndFrame(lctx);
}
PICAerror APIENTRY crPicaCancelFrame(PICAcontextID ctx, 
					 PICAframeID frameID)
{
     return stub.spu->dispatch_table.PicaCancelFrame(ctx, frameID);
} 
PICAstatus APIENTRY crPicaQueryFrame(PICAcontextID ctx,
					 PICAframeID frameID,
					 PICAnodeID nodeID,
					 PICAfloat timeout)
{
     return stub.spu->dispatch_table.PicaQueryFrame(ctx, frameID, nodeID, timeout);
}
PICAerror APIENTRY crPicaAddGfxFramelet(PICAcontextID lctx,
					    const PICArect *srcRect,
					    const PICApoint *dstPos,
					    PICAuint order,
					    PICAint iVolatile)
{
     return stub.spu->dispatch_table.PicaAddGfxFramelet(lctx, 
							srcRect,
							dstPos, 
							order, 
							iVolatile);
}
PICAerror APIENTRY crPicaAddMemFramelet(PICAcontextID lctx,
					    const PICAvoid *colorBuffer,
					    const PICAvoid *depthBuffer,
					    PICAuint span_x,
					    const PICArect *srcRect,
					    const PICApoint *dstPos,
					    PICAuint order,
					    PICAint iVolatile)
{
     return stub.spu->dispatch_table.PicaAddMemFramelet(lctx, 
							colorBuffer, 
							depthBuffer, 
							span_x, 
							srcRect, 
							dstPos, 
							order, 
							iVolatile);
}
PICAerror APIENTRY crPicaReadFrame(PICAcontextID lctx,
				   PICAframeID frameID,
				   PICAuint format,
				   PICAvoid *colorBuffer,
				   PICAvoid *depthBuffer,
				   const PICArect *rect)
{
     return stub.spu->dispatch_table.PicaReadFrame(lctx,
					    frameID,
					    format,
					    colorBuffer,
					    depthBuffer,
					    rect);
}










