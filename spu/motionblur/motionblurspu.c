/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_spu.h"
#include "motionblurspu.h"



static void MOTIONBLURSPU_APIENTRY motionblurspuSwapBuffers( GLint window, GLint flags )
{
	if (motionblur_spu.beginBlurFlag)
	{
		motionblur_spu.super.Accum( GL_LOAD, 1.0 );
		motionblur_spu.beginBlurFlag = GL_FALSE;
	}
	else {	
		motionblur_spu.super.Accum( GL_MULT, motionblur_spu.accumCoef );
		motionblur_spu.super.Accum( GL_ACCUM, 1.0f - motionblur_spu.accumCoef );
	}
	motionblur_spu.super.Accum( GL_RETURN, 1.0 );
	motionblur_spu.super.SwapBuffers( window, flags );
}


static void MOTIONBLURSPU_APIENTRY motionblurspuWindowSize( GLint window, GLint w, GLint h )
{
	motionblur_spu.child.WindowSize( window, w, h );
	/* reset accum buffer */
	motionblur_spu.beginBlurFlag = GL_TRUE;
}


static GLint MOTIONBLURSPU_APIENTRY motionblurspuWindowCreate( const char *dpyName, GLint visBits )
{
	/* request window with accumulation buffer */
	return motionblur_spu.super.WindowCreate(dpyName, visBits | CR_ACCUM_BIT);
}


static GLint MOTIONBLURSPU_APIENTRY motionblurspuCreateContext( const char *dpyName, GLint visBits)
{
	/* request context with accum buffer capability */
	return motionblur_spu.super.CreateContext(dpyName, visBits | CR_ACCUM_BIT);
}


SPUNamedFunctionTable _cr_motionblur_table[] = {
	{ "SwapBuffers", (SPUGenericFunction) motionblurspuSwapBuffers },
	{ "WindowSize", (SPUGenericFunction) motionblurspuWindowSize },
	{ "WindowCreate", (SPUGenericFunction) motionblurspuWindowCreate },
	{ "CreateContext", (SPUGenericFunction) motionblurspuCreateContext },
	{ NULL, NULL }
};
