/* Copyright (c) 2001, Stanford University
	All rights reserved.

	See the file LICENSE.txt for information on redistributing this software. */
	
#ifndef CR_API_TEMPLATES_H
#define CR_API_TEMPLATES_H

#include "cr_spu.h" 
extern SPUDispatchTable glim;

extern GLint APIENTRY crCreateContext(const char *dpyName, GLint visBits);
extern void APIENTRY crDestroyContext(GLint context);
extern void APIENTRY crMakeCurrent(GLint drawable, GLint context);
extern GLint APIENTRY crGetCurrentContext( void );
extern GLint APIENTRY crGetCurrentWindow( void );
extern void APIENTRY crSwapBuffers(GLint drawable, GLint flags);

extern GLint APIENTRY crWindowCreate(const char *dpyName, GLint visBits);
extern void APIENTRY crWindowDestroy(GLint window);
extern void APIENTRY crWindowSize(GLint window, GLint w, GLint h);
extern void APIENTRY crWindowPosition(GLint window, GLint x, GLint y);
extern void APIENTRY crWindowShow(GLint window, GLint flag);

/* PICA stuff */
extern PICAerror APIENTRY crPicaListCompositors(const PICAuint *config, 
						PICAint *numResults, 
						PICAcompItem *results);
extern PICAerror APIENTRY crPicaGetCompositorParamiv(PICAcompID compositor,
						     PICAparam pname,
						     PICAint *params);
extern PICAerror APIENTRY crPicaGetCompositorParamfv(PICAcompID compositor,
						     PICAparam pname,
						     PICAfloat *params);
extern PICAerror APIENTRY crPicaGetCompositorParamcv(PICAcompID compositor,
						     PICAparam pname,
						     PICAchar **params);
extern PICAerror APIENTRY crPicaListNodes(PICAcompID compositor, 
					  PICAint *num,
					  PICAnodeItem *results);

extern PICAcontextID APIENTRY crPicaCreateContext(const PICAuint *config, 
						  const PICAnodeID *nodes, 
						  PICAuint numNodes);
extern PICAerror APIENTRY crPicaDestroyContext(PICAcontextID ctx);

extern PICAerror APIENTRY crPicaSetContextParami(PICAcontextID ctx,
						 PICAparam pname,
						 PICAint param);
extern PICAerror APIENTRY crPicaSetContextParamiv(PICAcontextID ctx,
						  PICAparam pname,
						  const PICAint *param);
extern PICAerror APIENTRY crPicaSetContextParamf(PICAcontextID ctx,
						 PICAparam pname,
						 PICAfloat param);
extern PICAerror APIENTRY crPicaSetContextParamfv(PICAcontextID ctx,
						  PICAparam pname,
						  const PICAfloat *param);
extern PICAerror APIENTRY crPicaSetContextParamv(PICAcontextID ctx,
						 PICAparam pname,
						 const PICAvoid *param);

extern PICAerror APIENTRY crPicaGetContextParamiv(PICAcontextID ctx,
						  PICAparam pname,
						  PICAint *param);  
extern PICAerror APIENTRY crPicaGetContextParamfv(PICAcontextID ctx,
						  PICAparam pname,
						  PICAfloat *param);  
extern PICAerror APIENTRY crPicaGetContextParamcv(PICAcontextID ctx,
						  PICAparam pname,
						  PICAchar **param);  
extern PICAerror APIENTRY crPicaGetContextParamv(PICAcontextID ctx,
						 PICAparam pname,
						 PICAvoid *param);  

extern PICAcontextID APIENTRY crPicaBindLocalContext(PICAcontextID globalCtx, 
						     PICAnodeID node);
extern PICAerror APIENTRY crPicaDestroyLocalContext(PICAcontextID lctx);

extern PICAerror APIENTRY crPicaStartFrame(PICAcontextID lctx,
					   const PICAframeID *frameID,
					   PICAuint numLocalFramelets,
					   PICAuint numOrders,
					   const PICAuint *orderList,
					   const PICArect *srcRectList,
					   const PICApoint *dstList);
extern PICAerror APIENTRY crPicaEndFrame(PICAcontextID lctx);
extern PICAerror APIENTRY crPicaCancelFrame(PICAcontextID ctx, 
					    PICAframeID frameID);
extern PICAstatus APIENTRY crPicaQueryFrame(PICAcontextID ctx,
					    PICAframeID frameID,
					    PICAnodeID nodeID,
					    PICAfloat timeout);
extern PICAerror APIENTRY crPicaAddGfxFramelet(PICAcontextID lctx,
					       const PICArect *srcRect,
					       const PICApoint *dstpos,
					       PICAuint order,
					       PICAint iVolatile);
extern PICAerror APIENTRY crPicaAddMemFramelet(PICAcontextID lctx,
					       const PICAvoid *colorBuffer,
					       const PICAvoid *depthBuffer,
					       PICAuint span_x,
					       const PICArect *srcRect,
					       const PICApoint *dstpos,
					       PICAuint order,
					       PICAint iVolatile);
extern PICAerror APIENTRY crPicaReadFrame(PICAcontextID lctx,
					  PICAframeID frameID,
					  PICAuint format,
					  PICAvoid *colorbuffer,
					  PICAvoid *depthbuffer,
					  const PICArect *rect);
#endif /* CR_API_TEMPLATES_H */
