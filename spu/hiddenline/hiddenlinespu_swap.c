#include "hiddenlinespu.h"
#include "cr_glwrapper.h"
#include "cr_unpack.h"
#include "cr_mem.h"

static void hiddenPlayback( SPUDispatchTable *table )
{
	BufList *temp;
	for (temp = hiddenline_spu.frame_head ; temp ; temp = temp->next )
	{
		crUnpack( temp->data, temp->opcodes, temp->num_opcodes, table );
	}
}

void HIDDENLINESPU_APIENTRY hiddenlinespu_SwapBuffers( void )
{
	BufList *temp, *next;
	static int frame_counter = 1;

	/* Step 0: Flush the pack buffer so the remainder of the current
	 * frame makes it onto the list! */

	hiddenlineFlush( NULL );

	/* Step 1: Play back the frame just to the depth buffer, with polygon
	 * offsets.  Note that this means we need to ignore calls to glColor,
	 * disable texturing, disable lighting, things like that. */

	hiddenline_spu.super.Clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	hiddenline_spu.super.Enable( GL_DEPTH_TEST );
	hiddenline_spu.super.ColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
	hiddenline_spu.super.Enable( GL_POLYGON_OFFSET_FILL );
	hiddenline_spu.super.PolygonOffset( 1.5f, 0.000001f );
	hiddenline_spu.super.PolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	/* Now, Play it back just to the depth buffer */
	hiddenPlayback( &(hiddenline_spu.child) );

	/* Step 2: Play back the frame with polygons set to draw in wireframe 
	 * mode.  This time there's nothing special to do, just let the thing
	 * run its course. */

	hiddenline_spu.super.PolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	hiddenline_spu.super.ColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	hiddenPlayback( &(hiddenline_spu.child) );

	/* Step 3: Actually swap the stupid buffers, duh. */

	hiddenline_spu.super.SwapBuffers();
	
	/* Step 4: Release the resources needed to record the past frame so we
	 * can record the next one */

	for (temp = hiddenline_spu.frame_head ; temp ; temp = next )
	{
		hiddenlineReclaimPackBuffer( temp );
		next = temp->next;
		crFree( temp );
	}
	hiddenline_spu.frame_head = hiddenline_spu.frame_tail = NULL;
	frame_counter++;
}
