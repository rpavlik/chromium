#include "hiddenlinespu.h"
#include "cr_mem.h"

#define RECLAIM_PACK_FREE 0
#define RECLAIM_POOL 1
#define RECLAIM_HUGE 2


/*
 * Get an empty packing buffer from the buffer pool, or allocate a new one.
 * Then tell the packer to use it.
 */
void hiddenlineProvidePackBuffer(void)
{
	void *buf;
	GET_CONTEXT(context);

	CRASSERT(context);

	buf = crBufferPoolPop( context->bufpool, hiddenline_spu.buffer_size );
	if (!buf)
	{
		buf = crAlloc( hiddenline_spu.buffer_size );
	}
	crPackInitBuffer( &(context->pack_buffer), buf, hiddenline_spu.buffer_size, hiddenline_spu.buffer_size );
	crPackSetBuffer( context->packer, &(context->pack_buffer) );
}

/*
 * Put the given buffer list into the free buffer pool.  We do this when
 * we've finished rendering a frame and no longer need the buffer list.
 */
void hiddenlineReclaimPackBuffer( BufList *bl )
{
	if (bl->can_reclaim == RECLAIM_POOL)
	{
		GET_CONTEXT(context);
		crBufferPoolPush( context->bufpool, bl->buf, hiddenline_spu.buffer_size );
	}
	else if (bl->can_reclaim == RECLAIM_PACK_FREE)
	{
		crPackFree( bl->buf );
	}
	else if (bl->can_reclaim == RECLAIM_HUGE)
	{
		crFree(bl->buf);
	}
	else {
		CRASSERT(0);
	}
}

/*
 * Allocate a BufList object and initialize it with the given parameters.
 * Add it to the tail of the linked list anchored to frame_head.
 * The idea is that we're building a linked list of OpenGL command
 * buffers which we'll replay later.
 * Called by the Flush/Huge functions below.
 */
static void
hiddenlineRecord( void *buf, void *data, void *opcodes,
									unsigned int num_opcodes, int reclaim )
{
	GET_CONTEXT(context);
	BufList *bl;
	bl = (BufList *) crAlloc( sizeof( *bl ) );
	bl->buf = buf;
	bl->data = data;
	bl->opcodes = opcodes;
	bl->num_opcodes = num_opcodes;
	bl->can_reclaim = reclaim;
	bl->next = NULL;

	CRASSERT(context);
	if (context->frame_tail == NULL)
	{
		context->frame_head = bl;
	}
	else
	{
		context->frame_tail->next = bl;
	}
	context->frame_tail = bl;
}

/*
 * This is called by the packer when the packer's buffer is full and
 * it needs to be emptied.
 * This function is registered as a callback with crPackFlushFunc().
 */
void hiddenlineFlush( void *arg )
{
	GET_CONTEXT(context);
	CRPackBuffer *buf;

	CRASSERT(context);
	buf = &(context->pack_buffer);

	/* release previous buffer if any */
	crPackReleaseBuffer( context->packer );

	if (buf->opcode_start - buf->opcode_current > 0)
		hiddenlineRecord( buf->pack, buf->data_start, buf->opcode_start,
											buf->opcode_start - buf->opcode_current , RECLAIM_POOL );

	hiddenlineProvidePackBuffer();
	(void) arg;
}

/*
 * This is a callback function called by the packer when it needs to
 * pack something big (like glDrawPixels).
 */
void hiddenlineHuge( CROpcode opcode, void *buf )
{
	const unsigned int len = ((unsigned int *) buf)[-1];
	char *newData;

	/* need to make copy of the huge buffer, as the packing function will free
	 * the data when we return.
	 */
	newData = (char *) crAlloc(len + 2);
	newData[0] = opcode;
	crMemcpy(newData + 1, (unsigned char *) buf - 4, len);

	hiddenlineRecord( newData, newData + 1, newData, 1, RECLAIM_HUGE );
}
