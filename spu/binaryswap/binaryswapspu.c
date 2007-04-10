/* Copyright (c) 2001, Stanford University
* All rights reserved
*
* See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "cr_mem.h"
#include "cr_error.h"
#include "cr_bbox.h"
#include "cr_url.h"
#include "binaryswapspu.h"


#define CLAMP(a, min, max) \
    ( ((a) < (min)) ? (min) : (((a) > (max)) ? (max) : (a)) )


/**
 * Build swap arrays
 */
static void
BuildSwapLimits(WindowInfo * window)
{
	int i = 0;
	int other_x = 0, other_y = 0;
	int other_width = window->width, other_height = window->height;

	/* clean up old data structures */
	if (window->read_x)
		crFree(window->read_x);
	if (window->read_y)
		crFree(window->read_y);
	if (window->read_width)
		crFree(window->read_width);
	if (window->read_height)
		crFree(window->read_height);

	window->read_x = crAlloc(binaryswap_spu.stages * sizeof(int));
	window->read_y = crAlloc(binaryswap_spu.stages * sizeof(int));
	window->read_width = crAlloc(binaryswap_spu.stages * sizeof(int));
	window->read_height = crAlloc(binaryswap_spu.stages * sizeof(int));

	/* figure out swap positions */
	/* BUG: Need to deal with odd window dimensions, 
	 * the current code can be off by one along the edge */
	for (i = 0; i < binaryswap_spu.stages; i++)
	{
		/* even stage => right/left */
		if (i % 2 == 0)
		{
			/* right */
			if (!binaryswap_spu.highlow[i])
			{
				crDebug("Binary Swap %d: right", i);
				if (i == 0)
				{
					window->read_x[i] = window->width / 2;
					window->read_y[i] = 0;
					window->read_width[i] = window->width / 2 + window->width % 2;
					window->read_height[i] = window->height;

					other_x = 0;
					other_y = 0;
					other_width = window->width / 2;
					other_height = window->height;
				}
				else
				{
					window->read_x[i] = other_x + other_width / 2;
					window->read_y[i] = other_y;
					window->read_width[i] = other_width / 2 + other_width % 2;
					window->read_height[i] = other_height;

					other_width = other_width / 2;
				}
			}
			/* left */
			else
			{
				crDebug("Binary Swap %d: left", i);
				if (i == 0)
				{
					window->read_y[i] = 0;
					window->read_x[i] = 0;
					window->read_width[i] = window->width / 2;
					window->read_height[i] = window->height;

					other_x = window->width / 2;
					other_y = 0;
					other_width = window->width / 2 + window->width % 2;
					other_height = window->height;
				}
				else
				{
					window->read_x[i] = other_x;
					window->read_y[i] = other_y;
					window->read_width[i] = other_width / 2;
					window->read_height[i] = other_height;

					other_x = other_x + other_width / 2;
					other_width = other_width / 2 + other_width % 2;
				}
			}
		}
		/* odd stage  => top/bottom */
		else
		{
			/* top */
			if (binaryswap_spu.highlow[i])
			{
				crDebug("Binary Swap %d: top", i);
				window->read_x[i] = other_x;
				window->read_y[i] = other_y + other_height / 2;
				window->read_width[i] = other_width;
				window->read_height[i] = other_height / 2 + other_height % 2;

				other_height = other_height / 2;
			}
			/* bottom */
			else
			{
				crDebug("Binary Swap %d: bottom", i);
				window->read_x[i] = other_x;
				window->read_y[i] = other_y;
				window->read_width[i] = other_width;
				window->read_height[i] = other_height / 2;

				other_y = other_y + other_height / 2;
				other_height = other_height / 2 + other_height % 2;
			}
		}
		crDebug("Binary Swap SPU: Width: %d, Height: %d, x: %d, y: %d",
						window->read_width[i], window->read_height[i],
						window->read_x[i], window->read_y[i]);
	}
}


/**
 * Allocate the color and depth buffers needed for the glDraw/ReadPixels
 * commands for the given window.
 */
static void
AllocBuffers(WindowInfo * window)
{
	CRASSERT(window);
	CRASSERT(window->width >= 0);
	CRASSERT(window->height >= 0);

	if (window->msgBuffer)
		crFree(window->msgBuffer);

	window->msgBuffer = (GLubyte *) crAlloc(sizeof(BinarySwapMsg) +
																					window->width * window->height
																					* ((window->bytesPerDepth +
																							window->bytesPerColor) *
																						 sizeof(GLubyte)));
	/* Setup message type to keep network layer happy */
	((BinarySwapMsg *) window->msgBuffer)->header.type = CR_MESSAGE_OOB;
}



/**
 * Called to resize a window.  This involves allocating new image buffers.
 */
static void
binaryswapspu_ResizeWindow(WindowInfo * window, int newWidth, int newHeight)
{
	CRASSERT(newWidth > 0);
	CRASSERT(newHeight > 0);

	window->width = newWidth;
	window->height = newHeight;

	if (binaryswap_spu.alpha_composite)
		window->bytesPerColor = 4 * sizeof(GLubyte);
	else
		window->bytesPerColor = 3 * sizeof(GLubyte);

	if (binaryswap_spu.depth_composite)
		window->bytesPerDepth = sizeof(GLuint);
	else
		window->bytesPerDepth = 0;

	if (binaryswap_spu.resizable)
	{
		/* update super/render SPU window size & viewport */
		binaryswap_spu.super.WindowSize(window->renderWindow,
																		newWidth, newHeight);
		binaryswap_spu.super.Viewport(0, 0, newWidth, newHeight);

		/* set child SPU's window size and viewport too */
		binaryswap_spu.child.WindowSize(window->renderWindow,
																		newWidth, newHeight);
		binaryswap_spu.child.Viewport(0, 0, newWidth, newHeight);

		/* clear the stencil buffer */
		binaryswap_spu.super.Clear(GL_STENCIL_BUFFER_BIT);
	}
	AllocBuffers(window);
	BuildSwapLimits(window);
}


/**
 * Determine the size of the given binaryswap SPU window.
 * We may either have to query the super or child SPU window dims.
 * Reallocate the glReadPixels RGBA/depth buffers if the size changes.
 */
static void
CheckWindowSize(WindowInfo * window)
{
	GLint superSize[2], childSize[2], *newSize;

	superSize[0] = superSize[1] = 0;
	childSize[0] = childSize[1] = 0;

	/* query parent Render SPU's window size */
	binaryswap_spu.super.GetChromiumParametervCR(GL_WINDOW_SIZE_CR,
																							 window->renderWindow,
																							 GL_INT, 2, superSize);
	/* query child SPU's window size */
	binaryswap_spu.child.GetChromiumParametervCR(GL_WINDOW_SIZE_CR,
																							 window->childWindow,
																							 GL_INT, 2, childSize);

	/* determine which size to use */
	if (binaryswap_spu.resizable)
	{
		if (binaryswap_spu.renderToAppWindow)
		{
			newSize = superSize;
		}
		else
		{
			if (childSize[0] > 0 && childSize[1] > 0)
				newSize = childSize;
			else
				newSize = superSize;
		}
	}
	else
	{
		/* not resizable, use super/render's fixed size */
		newSize = superSize;
	}

	/*
	crDebug("%s %d new %d x %d", __FUNCTION__, __LINE__, newSize[0], newSize[1]);
	crDebug("%s %d win %d x %d", __FUNCTION__, __LINE__, window->width, window->height);
	*/

	if ((newSize[0] != window->width || newSize[1] != window->height)
			&& newSize[0] > 0 && newSize[1] > 0)
	{
		/* The window size has actually changed (or first-time init) */
		binaryswapspu_ResizeWindow(window, newSize[0], newSize[1]);
	}
}


/**
 * This is the guts of the binaryswap operation.  Here, we call glReadPixels
 * to read a region of the color and depth buffers from the parent (render
 * SPU) window.	 We then begin the process of composition using binary swap.
 * Each node does a glReadPixels and sends its swap region to its swap 
 * partner.  Once we receive our partner's frame region, we use draw pixels to
 * composite against our frame.  This repeats for all swap partners.  Once we
 * have all of the regions composited against the one we are responsible for,
 * we issue another read and send our region to final display by glDrawPixels 
 * (and some other GL functions) on the child/downstream SPU to send our region
 * to the final display node.
 * Input:  window - the window we're processing
 *	   startx, starty - glReadPixels start coordinates
 *	   endx,   endy   - glReadPixels ending coordinates
 */
static void
CompositeNode(WindowInfo * window, int startx, int starty, int endx, int endy)
{
	int i = 0;
	int read_start_x = 0, read_start_y = 0;
	int read_width = 0, read_height = 0;
	CRMessage *incoming_msg = NULL;
	GLubyte *incoming_color = NULL;
	GLuint *incoming_depth = NULL;
	BinarySwapMsg *render_info = NULL;
	int draw_x = 0, draw_y = 0;
	int draw_width = 0, draw_height = 0;
	float other_depth = 0.0;

	int recalc_end_x = 0, recalc_end_y = 0;
	int recalc_start_x = 0, recalc_start_y = 0;
	int recalc_temp;

	CRASSERT(window->width > 0);
	CRASSERT(window->height > 0);
	/* figure out our portion for each stage */
	for (i = 0; i < binaryswap_spu.stages; i++)
	{
		BinarySwapMsg *msg = (BinarySwapMsg *) window->msgBuffer;

		/* set up message header */
		msg->start_x = window->read_x[i];
		msg->start_y = window->read_y[i];
		msg->width = window->read_width[i];
		msg->height = window->read_height[i];
		msg->depth = binaryswap_spu.depth;

		if (startx < window->read_x[i])
			msg->clipped_x = window->read_x[i];
		else
			msg->clipped_x = startx;

		if (starty < window->read_y[i])
			msg->clipped_y = window->read_y[i];
		else
			msg->clipped_y = starty;

		if (endx > (window->read_x[i] + window->read_width[i]))
			msg->clipped_width =
				(window->read_x[i] + window->read_width[i]) - msg->clipped_x;
		else
			msg->clipped_width = endx - msg->clipped_x;

		if (endy > (window->read_y[i] + window->read_height[i]))
			msg->clipped_height =
				(window->read_y[i] + window->read_height[i]) - msg->clipped_y;
		else
			msg->clipped_height = endy - msg->clipped_y;

		if (msg->clipped_width < 0)
			msg->clipped_width = 0;
		if (msg->clipped_height < 0)
			msg->clipped_height = 0;

		read_start_x = msg->clipped_x;
		read_start_y = msg->clipped_y;
		read_width = msg->clipped_width;
		read_height = msg->clipped_height;

		/* read our portion for this pass */
		/* figure out which mode to use, depth or alpha */
		if (binaryswap_spu.alpha_composite)
		{
			if (read_width > 0 && read_height > 0)
				binaryswap_spu.super.ReadPixels(read_start_x, read_start_y,
                                                read_width, read_height,
                                                GL_RGBA, GL_UNSIGNED_BYTE,
                                                (GLubyte *) window->msgBuffer +
                                                binaryswap_spu.offset);

			if (binaryswap_spu.highlow[i])
			{
				/* lower of pair => recv,send */
				crNetGetMessage(binaryswap_spu.peer_recv[i], &incoming_msg);
				crNetSend(binaryswap_spu.peer_send[i], NULL, window->msgBuffer,
                          (read_width * read_height * 4) + binaryswap_spu.offset);
			}
			else
			{
				/* higher of pair => send,recv */
				crNetSend(binaryswap_spu.peer_send[i], NULL, window->msgBuffer,
                          (read_width * read_height * 4) + binaryswap_spu.offset);
				crNetGetMessage(binaryswap_spu.peer_recv[i], &incoming_msg);
			}

			if (binaryswap_spu.mtu > binaryswap_spu.peer_send[i]->mtu)
				binaryswap_spu.mtu = binaryswap_spu.peer_send[i]->mtu;

			/* get render info from other node */
			render_info = (BinarySwapMsg *) incoming_msg;
			draw_x = render_info->clipped_x;
			draw_y = render_info->clipped_y;
			draw_width = render_info->clipped_width;
			draw_height = render_info->clipped_height;
			other_depth = render_info->depth;

			if (draw_width > 0 && draw_height > 0)
			{
				/* get incoming fb */
				incoming_color =
					(GLubyte *) ((GLubyte *) incoming_msg + binaryswap_spu.offset);
				/* figure out blend function based on z */
				binaryswap_spu.super.Enable(GL_BLEND);
				/* Other image is on top of ours! */
				if (binaryswap_spu.depth > other_depth)
				{
					/* over operator */
					binaryswap_spu.super.BlendFuncSeparateEXT(GL_SRC_ALPHA,
                                                              GL_ONE_MINUS_SRC_ALPHA,
                                                              GL_ONE, GL_ONE);
					binaryswap_spu.super.WindowPos2iARB(draw_x, draw_y);
					binaryswap_spu.super.DrawPixels(draw_width, draw_height,
                                                    GL_RGBA, GL_UNSIGNED_BYTE,
                                                    incoming_color);
				}
				/* other image is under ours */
				else if(binaryswap_spu.depth < other_depth)
				{
					/* under operator */
					binaryswap_spu.super.BlendFuncSeparateEXT(GL_ONE_MINUS_DST_ALPHA,
                                                              GL_DST_ALPHA,
                                                              GL_ONE,
                                                              GL_ONE);
					binaryswap_spu.super.WindowPos2iARB(draw_x, draw_y);
					binaryswap_spu.super.DrawPixels(draw_width, draw_height,
                                                    GL_RGBA, GL_UNSIGNED_BYTE,
                                                    incoming_color);
				}
				else
				{
					if(binaryswap_spu.highlow[i])
					{
						/* over operator */
						binaryswap_spu.super.BlendFuncSeparateEXT(GL_SRC_ALPHA,
																											GL_ONE_MINUS_SRC_ALPHA,
																											GL_ONE, GL_ONE);
						binaryswap_spu.super.WindowPos2iARB(draw_x, draw_y);
						binaryswap_spu.super.DrawPixels(draw_width, draw_height,
																						GL_RGBA, GL_UNSIGNED_BYTE,
																						incoming_color);
					}
					else
					{
						/* under operator */
						binaryswap_spu.super.BlendFuncSeparateEXT(GL_ONE_MINUS_DST_ALPHA,
																											GL_DST_ALPHA,
																											GL_ONE,
																											GL_ONE);
						binaryswap_spu.super.WindowPos2iARB(draw_x, draw_y);
						binaryswap_spu.super.DrawPixels(draw_width, draw_height,
																						GL_RGBA, GL_UNSIGNED_BYTE,
																						incoming_color);
					}
				}
				if (binaryswap_spu.depth > other_depth)
				{
					binaryswap_spu.depth = other_depth;
				}
			}
		}
		else
		{
			/* depth composite */
			if (read_width > 0 && read_height > 0)
			{
				GLubyte *colors =
					(GLubyte *) window->msgBuffer	+	binaryswap_spu.offset;
				GLuint *depths =
					(GLuint *) ((GLubyte *) window->msgBuffer +	/* base address */
											(read_width * read_height * 3) +	/* color information */
											binaryswap_spu.offset);	        /* message header */

				binaryswap_spu.super.ReadPixels(read_start_x, read_start_y,
																				read_width, read_height,
																				GL_RGB, GL_UNSIGNED_BYTE,
																				colors);
				binaryswap_spu.super.ReadPixels(read_start_x, read_start_y,
																				read_width, read_height,
																				GL_DEPTH_COMPONENT, GL_UNSIGNED_INT,
																				depths);
			}

			if (binaryswap_spu.highlow[i])
			{
				/* lower of pair => recv,send */
				crNetGetMessage(binaryswap_spu.peer_recv[i], &incoming_msg);
				crNetSend(binaryswap_spu.peer_send[i], NULL, window->msgBuffer,
									read_width * read_height * (3 + 4) + binaryswap_spu.offset);
			}
			else
			{
				/* higher of pair => send,recv */
				crNetSend(binaryswap_spu.peer_send[i], NULL, window->msgBuffer,
									read_width * read_height * (3 + 4) + binaryswap_spu.offset);
				crNetGetMessage(binaryswap_spu.peer_recv[i], &incoming_msg);
			}
			if (binaryswap_spu.mtu > binaryswap_spu.peer_send[i]->mtu)
				binaryswap_spu.mtu = binaryswap_spu.peer_send[i]->mtu;

			/* get render info from other node */
			render_info = (BinarySwapMsg *) incoming_msg;
			draw_x = render_info->clipped_x;
			draw_y = render_info->clipped_y;
			draw_width = render_info->clipped_width;
			draw_height = render_info->clipped_height;

			if (draw_width > 0 && draw_height > 0)
			{
				/* get incoming fb */
				incoming_color =
					(GLubyte *) ((GLubyte *) incoming_msg + binaryswap_spu.offset);
				incoming_depth =
					(GLuint *) (incoming_color + draw_width * draw_height * 3);

				binaryswap_spu.super.WindowPos2iARB(draw_x, draw_y);

				/* Draw the depth image into the depth buffer, setting the stencil
				 * to one wherever we pass the Z test, clearinging to zero where
				 * we fail.
				 */
				binaryswap_spu.super.ColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
				binaryswap_spu.super.StencilOp(GL_KEEP, GL_ZERO, GL_REPLACE);
				binaryswap_spu.super.StencilFunc(GL_ALWAYS, 1, ~0);
				binaryswap_spu.super.Enable(GL_STENCIL_TEST);
				binaryswap_spu.super.Enable(GL_DEPTH_TEST);
				binaryswap_spu.super.DepthFunc(GL_LEQUAL);
				binaryswap_spu.super.DrawPixels(draw_width, draw_height,
																				GL_DEPTH_COMPONENT,
																				GL_UNSIGNED_INT, incoming_depth);

				/* Now draw the RGBA image, only where the stencil is one, reset
				 * stencil to zero as we go
				 * (to avoid calling glClear(STENCIL_BUFFER_BIT)).
				 */
				binaryswap_spu.super.Disable(GL_DEPTH_TEST);
				binaryswap_spu.super.StencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
				binaryswap_spu.super.StencilFunc(GL_EQUAL, 1, ~0);
				binaryswap_spu.super.ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				binaryswap_spu.super.DrawPixels(draw_width, draw_height, GL_RGB,
																				GL_UNSIGNED_BYTE, incoming_color);
				binaryswap_spu.super.Disable(GL_STENCIL_TEST);
			}
		}

		/* make sure everything got drawn for next pass */
		binaryswap_spu.super.Flush();

		/* find optimal starting point for readback based on 
		   this node's region and the partner's region */
		recalc_start_x = render_info->start_x;
		if (startx < render_info->clipped_x)
		{
			recalc_temp = startx;
		}
		else
		{
			recalc_temp = render_info->clipped_x;
			if (render_info->clipped_width > 0 && render_info->clipped_height > 0)
				startx = recalc_temp;
		}
		if (recalc_temp > recalc_start_x)
		{
			recalc_start_x = recalc_temp;
		}

		recalc_start_y = render_info->start_y;
		if (starty < render_info->clipped_y)
		{
			recalc_temp = starty;
		}
		else
		{
			recalc_temp = render_info->clipped_y;
			if (render_info->clipped_width > 0 && render_info->clipped_height > 0)
				starty = recalc_temp;
		}
		if (recalc_temp > recalc_start_y)
		{
			recalc_start_y = recalc_temp;
		}

		/* find optimal ending point for readback based on 
		 * this node's region and the partner's region
		 */
		recalc_end_x = render_info->start_x + render_info->width;
		if (endx > (render_info->clipped_x + render_info->clipped_width))
		{
			recalc_temp = endx;
		}
		else
		{
			recalc_temp = (render_info->clipped_x + render_info->clipped_width);
			if (render_info->clipped_width > 0 && render_info->clipped_height > 0)
				endx = recalc_temp;
		}
		if (recalc_end_x > recalc_temp)
		{
			recalc_end_x = recalc_temp;
		}

		recalc_end_y = render_info->start_y + render_info->height;
		if (endy > (render_info->clipped_y + render_info->clipped_height))
		{
			recalc_temp = endy;
		}
		else
		{
			recalc_temp = (render_info->clipped_y + render_info->clipped_height);
			if (render_info->clipped_width > 0 && render_info->clipped_height > 0)
				endy = recalc_temp;
		}
		if (recalc_end_y > recalc_temp)
		{
			recalc_end_y = recalc_temp;
		}

		/* clean up the memory allocated for the recv */
		crNetFree(binaryswap_spu.peer_recv[i], incoming_msg);
	}
    
	draw_x = recalc_start_x;
	draw_y = recalc_start_y;
	draw_width = recalc_end_x - recalc_start_x;
	draw_height = recalc_end_y - recalc_start_y;

	if (draw_width > 0 && draw_height > 0)
	{
		/* send our final portion off to child */
		binaryswap_spu.super.ReadPixels(draw_x, draw_y, draw_width, draw_height,
																		GL_RGB, GL_UNSIGNED_BYTE,
																		window->msgBuffer +
																		binaryswap_spu.offset);

		/*
		 * Set the downstream viewport.  If we don't do this, and the
		 * downstream window is resized, the glRasterPos command doesn't
		 * seem to be reliable.  This is a problem both with Mesa and the
		 * NVIDIA drivers.  Technically, this may not be a driver bug at
		 * all since we're doing funny stuff.  Anyway, this fixes the problem.
		 * Note that the width and height are arbitrary since we only care
		 * about getting the origin right.  glDrawPixels, glClear, etc don't
		 * care what the viewport size is.  (BrianP)
		 */
		CRASSERT(window->width > 0);
		CRASSERT(window->height > 0);

		/* Begin critical region */
		binaryswap_spu.child.SemaphorePCR(MUTEX_SEMAPHORE);

		/* Update child window position, in case we're using the windowtracker
		 * and VNC SPUs.
		 */
		{
			GLint pos[2];
			binaryswap_spu.child.GetChromiumParametervCR(GL_WINDOW_POSITION_CR,
																									 window->childWindow,
																									 GL_INT, 2, pos);
			if (pos[0] != window->child_xpos ||
					pos[1] != window->child_ypos) {
				binaryswap_spu.child.WindowPosition(window->childWindow,
																						pos[0], pos[1]);
				window->child_xpos = pos[0];
				window->child_ypos = pos[1];
			}
		}

		binaryswap_spu.child.WindowPos2iARB(draw_x, draw_y);
		binaryswap_spu.child.DrawPixels(draw_width, draw_height,
																		GL_RGB, GL_UNSIGNED_BYTE,
																		window->msgBuffer +
																		binaryswap_spu.offset);

		/* end critical region */
		binaryswap_spu.child.SemaphoreVCR(MUTEX_SEMAPHORE);
	}
}


/**
 * Do binaryswap/composite for a window.
 * This involves:
 *   - computing the image regions (tiles) to process
 *   - bounding box testing (only if running on the faker / whole window)
 *   - doing glClear
 *   - call CompositeNode() for each region
 */
static void
ProcessNode(WindowInfo * window)
{
	GLboolean bbox;
	int x1, y1, x2, y2;

	/* compute region to process */
	if (window->bboxUnion.x1 == 0 && window->bboxUnion.x2 == 0)
	{
		/* use whole window */
		x1 = 0;
		y1 = 0;
		x2 = window->width;
		y2 = window->height;
		bbox = GL_FALSE;
	}
	else
	{
		/* Clamp the screen bbox union to the window dims.
		 * The border is kind of a fudge factor to be sure we don't miss
		 * any pixels right along the edges of the bouding box.
		 */
		const int border = 1;
		x1 = CLAMP(window->bboxUnion.x1 - border, 0, window->width - 1);
		y1 = CLAMP(window->bboxUnion.y1 - border, 0, window->height - 1);
		x2 = CLAMP(window->bboxUnion.x2 + border, 0, window->width - 1);
		y2 = CLAMP(window->bboxUnion.y2 + border, 0, window->height - 1);
		bbox = GL_TRUE;
	}

	if (bbox)
	{
		/* Need to clear final destination window's color buffer since we
		 * probably won't paint the whole thing, just the bbox regions.
		 */
		binaryswap_spu.child.Clear(GL_COLOR_BUFFER_BIT);
		/* wait for everyone to finish clearing */
		binaryswap_spu.child.BarrierExecCR(CLEAR_BARRIER);
	}
	else {
		/* glDrawPixels will cover the whole window, no need to clear it */
	}

	CompositeNode(window, x1, y1, x2, y2);
}


/**
 ************************************************************
 * Deals with putting back settings we muck with and checking
 * for possible window resize.
 ************************************************************/
static void
DoBinaryswap(WindowInfo * window)
{
	static int first_time = 1;

	/* values used to restore state we change */
	GLint super_packAlignment, super_unpackAlignment;
	GLint child_unpackAlignment;
	GLboolean super_blend = GL_FALSE;
	GLint super_blend_dst = 0, super_blend_src = 0;
	GLboolean super_color_writemask[4];
	GLint super_depth_func = 0, super_stencil_func = 0, super_stencil_ref = 0;
	GLint super_stencil_value_mask = 0, super_stencil_fail = 0;
	GLint super_stencil_pass_depth_fail = 0, super_stencil_pass_depth_pass = 0;
	GLboolean super_stencil_test = GL_FALSE, super_depth_test = GL_FALSE;

	if (first_time || window->width < 1 || window->height < 1)
	{
		CheckWindowSize(window);
	}

	if (first_time)
	{
		/* one-time initializations */
		binaryswap_spu.child.BarrierCreateCR(CLEAR_BARRIER, 0);
		binaryswap_spu.child.BarrierCreateCR(SWAP_BARRIER, 0);
		binaryswap_spu.child.BarrierCreateCR(POST_SWAP_BARRIER, 0);
		binaryswap_spu.child.SemaphoreCreateCR(MUTEX_SEMAPHORE, 1);
		((BinarySwapMsg *) window->msgBuffer)->header.type = CR_MESSAGE_OOB;
		binaryswap_spu.offset = sizeof(BinarySwapMsg);
		first_time = 0;
	}
	else if (binaryswap_spu.resizable)
	{
		/* check if window size changed, reallocate buffers if needed */
		CheckWindowSize(window);
	}

	/*
	 * Save pack/unpack alignments, and set to one.
	 */
	binaryswap_spu.super.GetIntegerv(GL_PACK_ALIGNMENT, &super_packAlignment);
	binaryswap_spu.super.GetIntegerv(GL_UNPACK_ALIGNMENT,
																	 &super_unpackAlignment);
	binaryswap_spu.child.GetIntegerv(GL_UNPACK_ALIGNMENT,
																	 &child_unpackAlignment);
	binaryswap_spu.super.PixelStorei(GL_PACK_ALIGNMENT, 1);
	binaryswap_spu.super.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
	binaryswap_spu.child.PixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/* Things alpha compositing mucks with */
	if (binaryswap_spu.alpha_composite)
	{
		super_blend = binaryswap_spu.super.IsEnabled(GL_BLEND);
		binaryswap_spu.super.GetIntegerv(GL_BLEND_DST, &super_blend_dst);
		binaryswap_spu.super.GetIntegerv(GL_BLEND_SRC, &super_blend_src);
	}

	/* Things depth compositing mucks with */
	if (binaryswap_spu.depth_composite)
	{
		binaryswap_spu.super.GetBooleanv(GL_COLOR_WRITEMASK,
																		 super_color_writemask);
		binaryswap_spu.super.GetIntegerv(GL_DEPTH_FUNC, &super_depth_func);
		binaryswap_spu.super.GetIntegerv(GL_STENCIL_FUNC, &super_stencil_func);
		binaryswap_spu.super.GetIntegerv(GL_STENCIL_REF, &super_stencil_ref);
		binaryswap_spu.super.GetIntegerv(GL_STENCIL_VALUE_MASK,
																		 &super_stencil_value_mask);
		binaryswap_spu.super.GetIntegerv(GL_STENCIL_FAIL, &super_stencil_fail);
		binaryswap_spu.super.GetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL,
																		 &super_stencil_pass_depth_fail);
		binaryswap_spu.super.GetIntegerv(GL_STENCIL_PASS_DEPTH_PASS,
																		 &super_stencil_pass_depth_pass);
		super_stencil_test = binaryswap_spu.super.IsEnabled(GL_STENCIL_TEST);
		super_depth_test = binaryswap_spu.super.IsEnabled(GL_DEPTH_TEST);
	}

	ProcessNode(window);

	/*
	 * Restore pack/unpack alignments
	 */
	binaryswap_spu.super.PixelStorei(GL_PACK_ALIGNMENT, super_packAlignment);
	binaryswap_spu.super.PixelStorei(GL_UNPACK_ALIGNMENT,
																	 super_unpackAlignment);
	binaryswap_spu.child.PixelStorei(GL_UNPACK_ALIGNMENT,
																	 child_unpackAlignment);

	/* Restore GL state we may have changed */
	if (binaryswap_spu.alpha_composite)
	{
		if (super_blend)
			binaryswap_spu.super.Enable(GL_BLEND);
		else
			binaryswap_spu.super.Disable(GL_BLEND);

		binaryswap_spu.super.BlendFunc(super_blend_src, super_blend_dst);
	}
	if (binaryswap_spu.depth_composite)
	{
		if (super_depth_test)
			binaryswap_spu.super.Enable(GL_DEPTH_TEST);
		else
			binaryswap_spu.super.Disable(GL_DEPTH_TEST);

		if (super_stencil_test)
			binaryswap_spu.super.Enable(GL_STENCIL_TEST);
		else
			binaryswap_spu.super.Disable(GL_STENCIL_TEST);

		binaryswap_spu.super.ColorMask(super_color_writemask[0],
																	 super_color_writemask[1],
																	 super_color_writemask[2],
																	 super_color_writemask[3]);
		binaryswap_spu.super.DepthFunc(super_depth_func);
		binaryswap_spu.super.StencilFunc(super_stencil_func,
																		 super_stencil_ref,
																		 super_stencil_value_mask);
		binaryswap_spu.super.StencilOp(super_stencil_fail,
																	 super_stencil_pass_depth_fail,
																	 super_stencil_pass_depth_pass);
	}
}


static void
ResetAccumulatedBBox(void)
{
	GET_CONTEXT(context);
	WindowInfo *window = context->currentWindow;
	window->bboxUnion.x1 = 0;
	window->bboxUnion.x2 = 0;
	window->bboxUnion.y1 = 0;
	window->bboxUnion.y2 = 0;
}


static void
AccumulateFullWindow(void)
{
	GET_CONTEXT(context);
	WindowInfo *window = context->currentWindow;
	window->bboxUnion.x1 = 0;
	window->bboxUnion.y1 = 0;
	window->bboxUnion.x2 = 100 * 1000;
	window->bboxUnion.y2 = 100 * 1000;
}


/**
 * Transform the given object-space bounds to window coordinates and
 * update the window's bounding box union.
 */
static void
AccumulateObjectBBox(const GLfloat * bbox)
{
	GLfloat proj[16], modl[16], viewport[4];
	GLfloat x1, y1, z1, x2, y2, z2;
	CRrecti winBox;
	GET_CONTEXT(context);
	WindowInfo *window = context->currentWindow;

	x1 = bbox[0];
	y1 = bbox[1];
	z1 = bbox[2];
	x2 = bbox[3];
	y2 = bbox[4];
	z2 = bbox[5];

	/* transform by modelview and projection */
	binaryswap_spu.super.GetFloatv(GL_PROJECTION_MATRIX, proj);
	binaryswap_spu.super.GetFloatv(GL_MODELVIEW_MATRIX, modl);
    
	crProjectBBox(modl, proj, &x1, &y1, &z1, &x2, &y2, &z2);

	/* Sanity check... */
	if (x2 < x1 || y2 < y1 || z2 < z1)
	{
		crWarning("Damnit!!!!, we screwed up the clipping somehow...");
		return;
	}
    
	/* adjust depth for alpha composite */
	binaryswap_spu.depth = z1;

	/* map to window coords */
	binaryswap_spu.super.GetFloatv(GL_VIEWPORT, viewport);
	winBox.x1 = (int) ((x1 + 1.0f) * (viewport[2] * 0.5F) + viewport[0]);
	winBox.y1 = (int) ((y1 + 1.0f) * (viewport[3] * 0.5F) + viewport[1]);
	winBox.x2 = (int) ((x2 + 1.0f) * (viewport[2] * 0.5F) + viewport[0]);
	winBox.y2 = (int) ((y2 + 1.0f) * (viewport[3] * 0.5F) + viewport[1]);

	if (window->bboxUnion.x1 == 0 && window->bboxUnion.x2 == 0)
	{
		/* this is the first box */
		window->bboxUnion = winBox;
	}
	else
	{
		/* compute union of current screen bbox and this one */
		crRectiUnion(&window->bboxUnion, &window->bboxUnion, &winBox);
	}
}


static void
AccumulateScreenBBox(const GLfloat * bbox)
{
	CRrecti winBox;
	GET_CONTEXT(context);
	WindowInfo *window = context->currentWindow;
	GLfloat z1 = bbox[2];

	winBox.x1 = (int) bbox[0];
	winBox.y1 = (int) bbox[1];
	winBox.x2 = (int) bbox[4];
	winBox.y2 = (int) bbox[5];

	/* adjust depth for alpha composite */
	binaryswap_spu.depth = z1;

	if (window->bboxUnion.x1 == 0 && window->bboxUnion.x2 == 0)
	{
		/* this is the first box */
		window->bboxUnion = winBox;
	}
	else
	{
		/* compute union of current screen bbox and this one */
		crRectiUnion(&window->bboxUnion, &window->bboxUnion, &winBox);
	}
}


static void BINARYSWAPSPU_APIENTRY
binaryswapspuFlush(void)
{
	GET_CONTEXT(context);
	WindowInfo *window = context->currentWindow;
	if (!window)
		return;

	if ((window->childVisBits & CR_DOUBLE_BIT) == 0) {
		/* if the child window isn't double-buffered, do readback now */
		DoBinaryswap(window);
		/*
		 * XXX \todo I'm not sure we need to sync on glFlush,
		 * but let's be safe for now.
		 */
		binaryswap_spu.child.BarrierExecCR(SWAP_BARRIER);
	}
	else {
		binaryswap_spu.super.Flush();
	}
}



static void BINARYSWAPSPU_APIENTRY
binaryswapspuSwapBuffers(GLint win, GLint flags)
{
	WindowInfo *window;

	window = (WindowInfo *) crHashtableSearch(binaryswap_spu.windowTable, win);
	CRASSERT(window);

	DoBinaryswap(window);

	/*
	 * Everyone syncs up here before calling SwapBuffers().
	 */
	binaryswap_spu.child.BarrierExecCR(SWAP_BARRIER);

	/* One will typically use serverNode.Conf('only_swap_once', 1) to
	 * prevent extraneous glClear and SwapBuffer calls on the server.
	 * Otherwise, N compositing nodes will cause N SwapBuffers!
	 */
	binaryswap_spu.child.SwapBuffers(window->childWindow, flags);

	binaryswap_spu.child.Finish();

	if (binaryswap_spu.local_visualization)
	{
		binaryswap_spu.super.SwapBuffers(window->renderWindow, 0);
	}

	/* Wait for all swaps are done before starting next frame.  Otherwise
	 * a glClear from the next frame could sneak in before we swap.
	 */
	binaryswap_spu.child.BarrierExecCR(POST_SWAP_BARRIER);

	ResetAccumulatedBBox();
}


/**
 * When we create a window or context, we need to twiddle with the
 * visual bitmask a bit.  Do that here.
 */
void
binaryswapspuTweakVisBits(GLint visBits,
													GLint *childVisBits, GLint *superVisBits)
{
	*superVisBits = visBits;

	/* If doing z-compositing, need stencil and depth buffers */
	if (binaryswap_spu.depth_composite)
		*superVisBits |= (CR_STENCIL_BIT | CR_DEPTH_BIT);
	else if (binaryswap_spu.alpha_composite)
		*superVisBits |= CR_ALPHA_BIT;

	/* we should probably be able to get away with a single-buffered pbuffer */
	if (visBits & CR_PBUFFER_BIT)
		*superVisBits &= ~CR_DOUBLE_BIT;

	/* final display window should probably be visible */
	*childVisBits = visBits & ~CR_PBUFFER_BIT;
}


static GLint BINARYSWAPSPU_APIENTRY
binaryswapspuCreateContext(const char *dpyName, GLint visBits, GLint shareCtx)
{
	static GLint freeID = 0;
	ContextInfo *context;
	GLint childVisBits, superVisBits;
	GLint childShareCtx = 0, superShareCtx = 0;

	CRASSERT(binaryswap_spu.child.BarrierCreateCR);

	if (shareCtx > 0) {
		/* get child/super context IDs */
		context =
			(ContextInfo *) crHashtableSearch(binaryswap_spu.contextTable, shareCtx);
		if (context) {
			childShareCtx = context->childContext;
			superShareCtx = context->renderContext;
		}
	}


	if (freeID != 0)
	{
		crError("Binaryswap cannot support multiple contexts");
		return 0;
	}

	context = (ContextInfo *) crCalloc(sizeof(ContextInfo));
	if (!context)
	{
		crWarning("binaryswap SPU: create context failed.");
		return -1;
	}

	binaryswapspuTweakVisBits(visBits, &childVisBits, &superVisBits);

	context->renderContext =
		binaryswap_spu.super.CreateContext(dpyName, superVisBits, superShareCtx);
	context->childContext =
		binaryswap_spu.child.CreateContext(dpyName, childVisBits, childShareCtx);
	context->childVisBits = childVisBits;
	context->superVisBits = superVisBits;

	if (context->renderContext < 0 || context->childContext < 0)
	{
		crFree(context);
		return -1;
	}

	/* put into hash table */
	crHashtableAdd(binaryswap_spu.contextTable, freeID, context);
	freeID++;
	return freeID - 1;
}


static void BINARYSWAPSPU_APIENTRY
binaryswapspuDestroyContext(GLint ctx)
{
	ContextInfo *context;
	context =
		(ContextInfo *) crHashtableSearch(binaryswap_spu.contextTable, ctx);
	CRASSERT(context);
	binaryswap_spu.super.DestroyContext(context->renderContext);
	crHashtableDelete(binaryswap_spu.contextTable, ctx, crFree);
}


static void BINARYSWAPSPU_APIENTRY
binaryswapspuMakeCurrent(GLint win, GLint nativeWindow, GLint ctx)
{
	ContextInfo *context;
	WindowInfo *window;

	context =
		(ContextInfo *) crHashtableSearch(binaryswap_spu.contextTable, ctx);
	window = (WindowInfo *) crHashtableSearch(binaryswap_spu.windowTable, win);

	if (context && window)
	{
		CRASSERT(context->superVisBits == window->superVisBits);
		CRASSERT(context->childVisBits == window->childVisBits);
		SET_CONTEXT(context);
		CRASSERT(window);
		context->currentWindow = window;
		binaryswap_spu.super.MakeCurrent(window->renderWindow,
																		 nativeWindow, context->renderContext);
		binaryswap_spu.child.MakeCurrent(window->childWindow,
																		 nativeWindow, context->childContext);
	}
	else
	{
		SET_CONTEXT(NULL);
	}
}


static GLint BINARYSWAPSPU_APIENTRY
binaryswapspuWindowCreate(const char *dpyName, GLint visBits)
{
	WindowInfo *window;
	static GLint freeID = 1;			/* skip default window 0 */
	GLint childVisBits, superVisBits;

	/* Error out on second window */
	if (freeID != 1)
	{
		crError("Binaryswap can't deal with multiple windows!");
		return 0;
	}

	/* allocate window */
	window = (WindowInfo *) crCalloc(sizeof(WindowInfo));
	if (!window)
	{
		crWarning("binaryswap SPU: unable to allocate window.");
		return -1;
	}

	binaryswapspuTweakVisBits(visBits, &childVisBits, &superVisBits);

	/* init window */
	window->index = freeID;
	window->renderWindow =
		binaryswap_spu.super.WindowCreate(dpyName, superVisBits);
	window->childWindow =
		binaryswap_spu.child.WindowCreate(dpyName, childVisBits);
	window->width = -1;						/* unknown */
	window->height = -1;					/* unknown */
	window->msgBuffer = NULL;
	window->childVisBits = childVisBits;
	window->superVisBits = superVisBits;

	if (window->renderWindow < 0 || window->childWindow < 0)
	{
		crFree(window);
		return -1;
	}

	/* put into hash table */
	crHashtableAdd(binaryswap_spu.windowTable, window->index, window);
	freeID++;

	return freeID - 1;
}


static void BINARYSWAPSPU_APIENTRY
binaryswapspuWindowDestroy(GLint win)
{
	WindowInfo *window;
	window = (WindowInfo *) crHashtableSearch(binaryswap_spu.windowTable, win);
	CRASSERT(window);
	binaryswap_spu.super.WindowDestroy(window->renderWindow);
	crHashtableDelete(binaryswap_spu.windowTable, win, crFree);
}


static void BINARYSWAPSPU_APIENTRY
binaryswapspuWindowSize(GLint win, GLint w, GLint h)
{
	WindowInfo *window;
	window = (WindowInfo *) crHashtableSearch(binaryswap_spu.windowTable, win);
	CRASSERT(window);
	CRASSERT(w > 0);
	CRASSERT(h > 0);
	if (window->width != w || window->height != h) {
		binaryswap_spu.super.WindowSize(window->renderWindow, w, h);
		binaryswap_spu.child.WindowSize(window->childWindow, w, h);
		binaryswapspu_ResizeWindow(window, w, h);
	}
}


/**
 * If you really don't want to allow windows to move, set the
 * 'ignore_window_moves' config option.  This function propogates
 * window moves downstream because that's sometimes useful.
 */
static void BINARYSWAPSPU_APIENTRY
binaryswapspuWindowPosition(GLint win, GLint x, GLint y)
{
	WindowInfo *window;
	window = (WindowInfo *) crHashtableSearch(binaryswap_spu.windowTable, win);
	CRASSERT(window);
	binaryswap_spu.super.WindowPosition(window->renderWindow, x, y);
	binaryswap_spu.child.WindowPosition(window->childWindow, x, y);
}


/* don't implement WindowPosition() */


/* I'M REALLY NOT HAPPY ABOUT DROPPING BARRIERS ON THE FLOOR, BUT WITHOUT THIS
 * SOME APPS LIKE PSUBMIT FAIL TO WORK.  BUT, WHAT IF WE ARE NOT AT THE END OF THE CHAIN?
 * FOR EXAMPLE, WHAT IF I WANT TO DO SOME OTHER TYPE OF COMPOSITE BEFORE BINARYSWAP THAT
 * NEEDS BARRIERS?
 * Likely answer: add an 'ignore_papi' config option, like other SPUs have.
 */
static void BINARYSWAPSPU_APIENTRY
binaryswapspuBarrierCreateCR(GLuint name, GLuint count)
{
	(void) name;
	/* no-op */
}


static void BINARYSWAPSPU_APIENTRY
binaryswapspuBarrierDestroyCR(GLuint name)
{
	(void) name;
	/* no-op */
}


static void BINARYSWAPSPU_APIENTRY
binaryswapspuBarrierExecCR(GLuint name)
{
	(void) name;
	/* no-op */
}


static void BINARYSWAPSPU_APIENTRY
binaryswapspuClearColor(GLclampf red,
												GLclampf green, GLclampf blue, GLclampf alpha)
{
	binaryswap_spu.super.ClearColor(red, green, blue, alpha);
	binaryswap_spu.child.ClearColor(red, green, blue, alpha);
}


static void BINARYSWAPSPU_APIENTRY
binaryswapspuViewport(GLint x, GLint y, GLint w, GLint h)
{
	binaryswap_spu.super.Viewport(x, y, w, h);
}


static void BINARYSWAPSPU_APIENTRY
binaryswapspuChromiumParametervCR(GLenum target, GLenum type,
																	GLsizei count, const GLvoid * values)
{
	switch (target)
	{
	case GL_OBJECT_BBOX_CR:
		CRASSERT(type == GL_FLOAT);
		CRASSERT(count == 6);
		AccumulateObjectBBox((GLfloat *) values);
		break;
	case GL_SCREEN_BBOX_CR:
		CRASSERT(type == GL_FLOAT);
		CRASSERT(count == 8);
		AccumulateScreenBBox((GLfloat *) values);
		break;
	case GL_DEFAULT_BBOX_CR:
		CRASSERT(count == 0);
		AccumulateFullWindow();
		break;
	default:
		binaryswap_spu.child.ChromiumParametervCR(target, type, count, values);
		break;
	}
}


static void BINARYSWAPSPU_APIENTRY
binaryswapspuDrawBuffer(GLenum buffer)
{
	GET_CONTEXT(context);
	WindowInfo *window;

	window = context->currentWindow;
	CRASSERT(window);

	if (window->superVisBits & CR_PBUFFER_BIT) {
		/* we only have a front color buffer */
		if (buffer != GL_FRONT) {
			crWarning("Binaryswap SPU: bad glDrawBuffer(0x%x)", buffer);
			buffer = GL_FRONT;
		}
		binaryswap_spu.super.DrawBuffer(buffer);
	}
}


SPUNamedFunctionTable _cr_binaryswap_table[] = {
	{"SwapBuffers", (SPUGenericFunction) binaryswapspuSwapBuffers},
	{"CreateContext", (SPUGenericFunction) binaryswapspuCreateContext},
	{"DestroyContext", (SPUGenericFunction) binaryswapspuDestroyContext},
	{"MakeCurrent", (SPUGenericFunction) binaryswapspuMakeCurrent},
	{"WindowCreate", (SPUGenericFunction) binaryswapspuWindowCreate},
	{"WindowDestroy", (SPUGenericFunction) binaryswapspuWindowDestroy},
	{"WindowSize", (SPUGenericFunction) binaryswapspuWindowSize},
	{"WindowPosition", (SPUGenericFunction) binaryswapspuWindowPosition},
	{"BarrierCreateCR", (SPUGenericFunction) binaryswapspuBarrierCreateCR},
	{"BarrierDestroyCR", (SPUGenericFunction) binaryswapspuBarrierDestroyCR},
	{"BarrierExecCR", (SPUGenericFunction) binaryswapspuBarrierExecCR},
	{"Viewport", (SPUGenericFunction) binaryswapspuViewport},
	{"Flush", (SPUGenericFunction) binaryswapspuFlush},
	{"ClearColor", (SPUGenericFunction) binaryswapspuClearColor},
	{"ChromiumParametervCR", (SPUGenericFunction) binaryswapspuChromiumParametervCR},
	{"DrawBuffer", (SPUGenericFunction) binaryswapspuDrawBuffer},
	{NULL, NULL}
};
