/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <assert.h>
#include "cr_packfunctions.h"
#include "packspu.h"

void PACKSPU_APIENTRY packspu_Finish( void )
{
	GET_THREAD(thread);
	int writeback = pack_spu.thread[0].server.conn->type == CR_DROP_PACKETS ? 0 : 1;
	if (pack_spu.swap)
	{
		crPackFinishSWAP(  );
		crPackWritebackSWAP( &writeback );
	}
	else
	{
		crPackFinish(  );
		crPackWriteback( &writeback );
	}
	packspuFlush( (void *) thread );
	while (writeback)
		crNetRecv();
}


GLint PACKSPU_APIENTRY packspu_crCreateWindow( const char *dpyName, GLint visBits )
{
	GET_THREAD(thread);
	int writeback = pack_spu.thread[0].server.conn->type == CR_DROP_PACKETS ? 0 : 1;
	GLint return_val = (GLint) 0;
	if (pack_spu.swap)
	{
		crPackcrCreateWindowSWAP( dpyName, visBits, &return_val, &writeback );
	}
	else
	{
		crPackcrCreateWindow( dpyName, visBits, &return_val, &writeback );
	}
	packspuFlush( (void *) thread );
	if (pack_spu.thread[0].server.conn->type == CR_FILE) {
		return 0;
	}
	else
	{
		while (writeback)
			crNetRecv();
		if (pack_spu.swap)
		{
			return_val = (GLint) SWAP32(return_val);
		}
		return return_val;
	}
}

