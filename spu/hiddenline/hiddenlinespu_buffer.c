#include "hiddenlinespu.h"
#include "cr_bufpool.h"
#include "cr_mem.h"

static CRBufferPool bufpool;

/*
 * Get an empty packing buffer from the buffer pool, or allocate a new one.
 * Then tell the packer to use it.
 */
void hiddenlineProvidePackBuffer(void)
{
	static int first_time = 1;
	void *buf;
	if (first_time)
	{
		first_time = 0;
		crBufferPoolInit( &bufpool, 16 );
	}

	buf = crBufferPoolPop( &bufpool );
	if (!buf)
	{
		buf = crAlloc( hiddenline_spu.buffer_size );
	}
	crPackInitBuffer( &(hiddenline_spu.pack_buffer), buf, hiddenline_spu.buffer_size, hiddenline_spu.buffer_size );
	crPackSetBuffer( hiddenline_spu.packer, &(hiddenline_spu.pack_buffer) );
}

/*
 * Put the given buffer list into the free buffer pool.  We do this when
 * we've finished rendering a frame and no longer need the buffer list.
 */
void hiddenlineReclaimPackBuffer( BufList *bl )
{
	if (bl->can_reclaim)
	{
		crBufferPoolPush( &bufpool, bl->buf );
	}
	else
	{
		crPackFree( bl->buf );
	}
}

/*
 * Allocate a BufList object and initialize it with the given parameters.
 * Add it to the tail of the linked list anchored to frame_head.
 * The idea is that we're building a linked list of OpenGL command
 * buffers which we'll replay later.
 * Called by the Flush/Huge functions below.
 */
static void hiddenlineRecord( void *buf, void *data, void *opcodes, unsigned int num_opcodes, int can_reclaim )
{
	BufList *bl;
	bl = (BufList *) crAlloc( sizeof( *bl ) );
	bl->buf = buf;
	bl->data = data;
	bl->opcodes = opcodes;
	bl->num_opcodes = num_opcodes;
	bl->can_reclaim = can_reclaim;
	bl->next = NULL;

	if (hiddenline_spu.frame_tail == NULL)
	{
		hiddenline_spu.frame_head = bl;
	}
	else
	{
		hiddenline_spu.frame_tail->next = bl;
	}
	hiddenline_spu.frame_tail = bl;
}

/*
 * This is called by the packer when the packer's buffer is full and
 * it needs to be emptied.
 * This function is registered as a callback with crPackFlushFunc().
 */
void hiddenlineFlush( void *arg )
{
	CRPackBuffer *buf = &(hiddenline_spu.pack_buffer);
	crPackGetBuffer( hiddenline_spu.packer, buf );

	hiddenlineRecord( buf->pack, buf->data_start, buf->opcode_start, buf->opcode_start - buf->opcode_current , 1 );

	hiddenlineProvidePackBuffer();
	(void) arg;
}

/*
 * This is a callback function called by the packer when it needs to
 * pack something big (like glDrawPixels).
 */
void hiddenlineHuge( CROpcode opcode, void *buf )
{
	/* write the opcode in just before the packet length */

	((unsigned char *) buf)[-5] = (unsigned char) opcode;

	/* Just record the actual buffer; it will be free'd properly by
	 * crPackFree later. */
	hiddenlineRecord( buf, (unsigned char *) buf - 4, (unsigned char *) buf - 5, 1, 0 );
}
