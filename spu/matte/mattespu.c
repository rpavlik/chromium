/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_spu.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "mattespu.h"

static void MATTESPU_APIENTRY matteViewport( GLint x, GLint y, GLsizei width, GLsizei height)
{
    /* No clamping is necessary - just offset the viewport to match the matte region. */
    matte_spu.super.Viewport(
	x + matte_spu.matteRegion.x,
	y + matte_spu.matteRegion.y,
	width, 
	height
    );
}

static void MATTESPU_APIENTRY matteScissor( GLint x, GLint y, GLsizei width, GLsizei height)
{
    /* Clamp the desired scissor region to our virtual matte region (0,0)-(w,h) */
    if (x < 0) x = 0;
    else if (x > matte_spu.matteRegion.width) width = 0;
    else if (x + width > matte_spu.matteRegion.width) width = matte_spu.matteRegion.width - x;

    if (y < 0) y = 0;
    else if (y > matte_spu.matteRegion.height) height = 0;
    else if (y + height > matte_spu.matteRegion.height) height = matte_spu.matteRegion.height - y;

    /* Now scissor the clamped region, with the matte offset */
    matte_spu.super.Scissor(
	x + matte_spu.matteRegion.x,
	y + matte_spu.matteRegion.y,
	width, 
	height
    );
}

static void MATTESPU_APIENTRY matteClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    GET_CONTEXT_INFO(contextInfo);
    /* Save the application's intended clear color; we use this if we end up
     * changing the clear color ourselves.
     */
    contextInfo->actualClearColor.red = red;
    contextInfo->actualClearColor.green = green;
    contextInfo->actualClearColor.blue = blue;
    contextInfo->actualClearColor.alpha = alpha;

    matte_spu.super.ClearColor(red, green, blue, alpha);
}

static void MATTESPU_APIENTRY matteClear(GLbitfield mask)
{
    GET_CONTEXT_INFO(contextInfo);

    /* We need to intercept the very first call (to initialize
     * the scissor test, even if we're not rendering); after
     * that, we only need to process if we're told to matte
     * on every glClear call.
     */
    if (matte_spu.matteEveryClear || contextInfo->clearCount == 0) {
	/* Turn off the scissor test, and draw the matte. */
	matte_spu.super.Disable(GL_SCISSOR_TEST);

	/* If we're using a matte color, clear to it with glClear. */
	if (matte_spu.useMatteColor) {
	    matte_spu.super.ClearColor(
		matte_spu.matteColor.red,
		matte_spu.matteColor.green,
		matte_spu.matteColor.blue,
		matte_spu.matteColor.alpha
	    );
	    matte_spu.super.Clear(GL_COLOR_BUFFER_BIT);
	    matte_spu.super.ClearColor(
		contextInfo->actualClearColor.red,
		contextInfo->actualClearColor.green,
		contextInfo->actualClearColor.blue,
		contextInfo->actualClearColor.alpha
	    );
	}

	/* If this is our first time, set our scissor region to the
	 * matted region
	 */
	if (contextInfo->clearCount == 0) {
	    matte_spu.super.Scissor(
		matte_spu.matteRegion.x,
		matte_spu.matteRegion.y,
		matte_spu.matteRegion.width,
		matte_spu.matteRegion.height
	    );
	}

	/* Re-enable the scissor test.  Note that we always leave the
	 * scissor test on to guarantee that we don't draw into the matte.
	 */
	matte_spu.super.Enable(GL_SCISSOR_TEST);
    }
    contextInfo->clearCount++;

    /* Pass the original clear call on to the renderer. */
    matte_spu.super.Clear(mask);
}

static void MATTESPU_APIENTRY matteDisable(GLenum cap)
{
    if (cap == GL_SCISSOR_TEST) {
	/* Don't disable scissors; just restore our original scissor region */
	matte_spu.super.Scissor(
	    matte_spu.matteRegion.x,
	    matte_spu.matteRegion.y,
	    matte_spu.matteRegion.width,
	    matte_spu.matteRegion.height
	);
    }
    else {
	matte_spu.super.Disable(cap);
    }
}

static void MATTESPU_APIENTRY matteMakeCurrent(GLint crWindow, GLint nativeWindow, GLint ctx)
{
    ContextInfo *contextInfo;

    matte_spu.super.MakeCurrent(crWindow, nativeWindow, ctx);

    /* If we've already got a context record for this, use it.
     * Otherwise, make a new one.
     */
    contextInfo = crHashtableSearch(matte_spu.contextTable, ctx);
    if (contextInfo == NULL) {
	/* Make our own. */
	contextInfo = crCalloc(sizeof(ContextInfo));
	if (contextInfo == NULL) {
	    crError("matte: could not allocate context info record for context %d", ctx);
	    return;
	}

	crHashtableAdd(matte_spu.contextTable, ctx, contextInfo);
    }

    /* Set this a s the current context */
    SET_CONTEXT_INFO(contextInfo);
}

void matteFreeContextInfo(void *data)
{
    ContextInfo *contextInfo = (ContextInfo *)data;
    crFree(contextInfo);
}

static void MATTESPU_APIENTRY matteDestroyContext(GLint ctx)
{
    ContextInfo *contextInfo;
    GET_CONTEXT_INFO(currentContextInfo);

    /* If we're destroying the current context, the current
     * context goes to NULL.
     */
    contextInfo = crHashtableSearch(matte_spu.contextTable, ctx);
    if (contextInfo == currentContextInfo) {
	SET_CONTEXT_INFO(NULL);
    }

    /* Destroy the context info record */
    if (contextInfo != NULL) {
	crHashtableDelete(matte_spu.contextTable, ctx, matteFreeContextInfo);
    }

    /* Pass it along */
    matte_spu.super.DestroyContext(ctx);
}


/**
 * SPU function table
 */
SPUNamedFunctionTable _cr_matte_table[] = {
    {"Viewport", (SPUGenericFunction) matteViewport},
    {"Scissor", (SPUGenericFunction) matteScissor},
    {"ClearColor", (SPUGenericFunction) matteClearColor},
    {"Clear", (SPUGenericFunction) matteClear},
    {"Disable", (SPUGenericFunction) matteDisable},
    {"MakeCurrent", (SPUGenericFunction) matteMakeCurrent},
    {"DestroyContext", (SPUGenericFunction) matteDestroyContext},
};
