/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "chromium.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_net.h"
#include "cr_pixeldata.h"
#include "cr_unpack.h"
#include "server_dispatch.h"
#include "server.h"


void SERVER_DISPATCH_APIENTRY crServerDispatchReadPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
{
	CRMessageReadPixels *rp;
	int msg_len = sizeof(*rp);
	GLint stride = READ_DATA( 24, GLint );
	GLint alignment = READ_DATA( 28, GLint );
	GLint skipRows = READ_DATA( 32, GLint );
	GLint skipPixels = READ_DATA( 36, GLint );
	GLint bytes_per_row = READ_DATA( 40, GLint );

	CRASSERT(bytes_per_row > 0);

	msg_len += bytes_per_row * height;

	rp = (CRMessageReadPixels *) crAlloc( msg_len );
	cr_server.head_spu->dispatch_table.ReadPixels( x, y, width, height, format, type, rp + 1);

	rp->header.type = CR_MESSAGE_READ_PIXELS;
	rp->bytes_per_row = bytes_per_row;
	rp->stride = stride;
	rp->format = format;
	rp->type = type;
	rp->rows = height;
	rp->alignment = alignment;
	rp->skipRows = skipRows;
	rp->skipPixels = skipPixels;

	/* <pixels> points to the 8-byte network pointer */
	crMemcpy( &rp->pixels, pixels, sizeof(rp->pixels) );
	
	crNetSend( cr_server.curClient->conn, NULL, rp, msg_len );
	crFree( rp );
}
