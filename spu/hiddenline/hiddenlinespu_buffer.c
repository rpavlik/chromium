#include "hiddenlinespu.h"
#include "cr_bufpool.h"
#include "cr_mem.h"

CRBufferPool bufpool;

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
	crPackInitBuffer( &(hiddenline_spu.pack_buffer), buf, hiddenline_spu.buffer_size, 0 );
	crPackSetBuffer( &(hiddenline_spu.pack_buffer) );
}

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

void hiddenlineFlush( void *arg )
{
	CRPackBuffer *buf = &(hiddenline_spu.pack_buffer);
	crPackGetBuffer( buf );

	hiddenlineRecord( buf->pack, buf->data_start, buf->opcode_start, buf->opcode_start - buf->opcode_current , 1 );

	hiddenlineProvidePackBuffer();
	(void *) arg;
}

void hiddenlineHuge( CROpcode opcode, void *buf )
{
	/* write the opcode in just before the packet length */

	((unsigned char *) buf)[-5] = (unsigned char) opcode;

	/* Just record the actual buffer; it will be free'd properly by
	 * crPackFree later. */
	hiddenlineRecord( buf, (unsigned char *) buf - 4, (unsigned char *) buf - 5, 1, 0 );
}
