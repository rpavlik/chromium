/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "hiddenlinespu.h"
#include "cr_unpack.h"
#include "cr_mem.h"

static void hiddenPlayback( SPUDispatchTable *table )
{
	BufList *temp;
	GET_CONTEXT(context);
	for (temp = context->frame_head ; temp ; temp = temp->next )
	{
		/* The Chromium unpacker isn't thread-safe!
		 * It's normally only used in the crserver (which only has
		 * one thread).  Just use a mutex here.
		 */
		crLockMutex(&(hiddenline_spu.mutex));
		crUnpack( temp->data, temp->opcodes, temp->num_opcodes, table );
		crUnlockMutex(&(hiddenline_spu.mutex));
	}
}

void HIDDENLINESPU_APIENTRY hlHandleEnable( GLenum cap )
{
	if( cap != GL_TEXTURE_2D && cap != GL_BLEND && cap != GL_LIGHTING )
	{
		hiddenline_spu.child.Enable( cap );
	}
}

void HIDDENLINESPU_APIENTRY hlHandleColor3bv (const GLbyte *v)    {}
void HIDDENLINESPU_APIENTRY hlHandleColor3dv (const GLdouble *v)  {}
void HIDDENLINESPU_APIENTRY hlHandleColor3fv (const GLfloat *v)   {}
void HIDDENLINESPU_APIENTRY hlHandleColor3iv (const GLint *v)     {}
void HIDDENLINESPU_APIENTRY hlHandleColor3sv (const GLshort *v)   {}
void HIDDENLINESPU_APIENTRY hlHandleColor3ubv (const GLubyte *v)  {}
void HIDDENLINESPU_APIENTRY hlHandleColor3uiv (const GLuint *v)   {}
void HIDDENLINESPU_APIENTRY hlHandleColor3usv (const GLushort *v) {}
void HIDDENLINESPU_APIENTRY hlHandleColor4bv (const GLbyte *v)    {}
void HIDDENLINESPU_APIENTRY hlHandleColor4dv (const GLdouble *v)  {}
void HIDDENLINESPU_APIENTRY hlHandleColor4fv (const GLfloat *v)   {}
void HIDDENLINESPU_APIENTRY hlHandleColor4iv (const GLint *v)     {}
void HIDDENLINESPU_APIENTRY hlHandleColor4sv (const GLshort *v)   {}
void HIDDENLINESPU_APIENTRY hlHandleColor4ubv (const GLubyte *v)  {}
void HIDDENLINESPU_APIENTRY hlHandleColor4uiv (const GLuint *v)   {}
void HIDDENLINESPU_APIENTRY hlHandleColor4usv (const GLushort *v) {}

void HIDDENLINESPU_APIENTRY hiddenlinespu_SwapBuffers( GLint window, GLint flags )
{
	static SPUDispatchTable hacked_child_dispatch;
	BufList *temp, *next;
	static int frame_counter = 1;
	static int do_hiddenline = 0; /* So textures can load (in Windows) */
	GET_CONTEXT(context);

#ifdef WINDOWS
#define PRESSED( key ) (GetAsyncKeyState( key ) & (1<<15))
	if (PRESSED( VK_SCROLL )) do_hiddenline = !do_hiddenline;
	if (PRESSED( VK_NEXT ) && hiddenline_spu.line_width > 1) hiddenline_spu.line_width--;
	if (PRESSED( VK_PRIOR )) hiddenline_spu.line_width++;
#else
	do_hiddenline = 1;
#endif

	if (frame_counter == 1)
	{
		/* one-time init */
		crSPUInitDispatchTable( &(hacked_child_dispatch) );
		crSPUCopyDispatchTable( &(hacked_child_dispatch), &(hiddenline_spu.child) );

		/* Note -- the unpacker always calls the vector versions of the color functions */

		hacked_child_dispatch.Enable = hlHandleEnable;
		hacked_child_dispatch.Color3bv = hlHandleColor3bv;
		hacked_child_dispatch.Color3dv = hlHandleColor3dv;
		hacked_child_dispatch.Color3fv = hlHandleColor3fv;
		hacked_child_dispatch.Color3iv = hlHandleColor3iv;
		hacked_child_dispatch.Color3sv = hlHandleColor3sv;
		hacked_child_dispatch.Color3ubv = hlHandleColor3ubv;
		hacked_child_dispatch.Color3uiv = hlHandleColor3uiv;
		hacked_child_dispatch.Color3usv = hlHandleColor3usv;
		hacked_child_dispatch.Color4bv = hlHandleColor4bv;
		hacked_child_dispatch.Color4dv = hlHandleColor4dv;
		hacked_child_dispatch.Color4fv = hlHandleColor4fv;
		hacked_child_dispatch.Color4iv = hlHandleColor4iv;
		hacked_child_dispatch.Color4sv = hlHandleColor4sv;
		hacked_child_dispatch.Color4ubv = hlHandleColor4ubv;
		hacked_child_dispatch.Color4uiv = hlHandleColor4uiv;
		hacked_child_dispatch.Color4usv = hlHandleColor4usv;
	} 
	
	/* Flush the pack buffer so the remainder of the current
	 * frame makes it onto the list! */

	hiddenlineFlush( NULL );

	if (hiddenline_spu.single_clear)
		hiddenline_spu.super.Clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	if (do_hiddenline)
	{
		hiddenline_spu.super.PushAttrib( GL_ENABLE_BIT | GL_CURRENT_BIT | GL_POLYGON_BIT | GL_LINE_BIT | GL_FOG_BIT );

		hiddenline_spu.super.PolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		hiddenline_spu.super.Disable( GL_TEXTURE_2D );
		hiddenline_spu.super.Disable( GL_LIGHTING );
		hiddenline_spu.super.Disable( GL_BLEND );
		hiddenline_spu.super.Enable( GL_DEPTH_TEST );
		if (hiddenline_spu.silhouette_mode)
				hiddenline_spu.super.Enable( GL_FOG );
		hiddenline_spu.super.Enable( GL_POLYGON_OFFSET_FILL );

		/* Play back the frame just to the depth buffer, with polygon
		 * offsets.  Note that this means we need to ignore calls to glColor,
		 * disable texturing, disable lighting, things like that.
		 */
		if (hiddenline_spu.silhouette_mode)
				hiddenline_spu.super.PolygonOffset( -2.5f, 0.000001f );
		else
				hiddenline_spu.super.PolygonOffset( +1.5f, 0.000001f );
		hiddenline_spu.super.Fogi( GL_FOG_MODE, GL_LINEAR );
		hiddenline_spu.super.Fogf( GL_FOG_START, 1.0 );
		hiddenline_spu.super.Fogf( GL_FOG_END, 1800.0 );
		hiddenline_spu.super.Fogfv( GL_FOG_COLOR, hiddenline_spu.fog_color );
		hiddenline_spu.super.Color3f( hiddenline_spu.poly_r, hiddenline_spu.poly_g, hiddenline_spu.poly_b );

		hiddenPlayback( &(hacked_child_dispatch) );

		/* Play back the frame with polygons set to draw in wireframe 
		 * mode.  This time there's nothing special to do, just let the thing
		 * run its course.
		 */
		hiddenline_spu.super.PolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		hiddenline_spu.super.LineWidth( hiddenline_spu.line_width );
		hiddenline_spu.super.Color3f( hiddenline_spu.line_r, hiddenline_spu.line_g, hiddenline_spu.line_b );
		hiddenPlayback( &(hacked_child_dispatch) );

		hiddenline_spu.super.PopAttrib();
	}
	else
	{
		hiddenPlayback( &(hiddenline_spu.child) );
	}

	hiddenline_spu.super.SwapBuffers( window, flags );
	
	/* Release the resources needed to record the past frame so we
	 * can record the next one */

	for (temp = context->frame_head ; temp ; temp = next )
	{
		hiddenlineReclaimPackBuffer( temp );
		next = temp->next;
		crFree( temp );
	}
	context->frame_head = context->frame_tail = NULL;

	frame_counter++;
}
