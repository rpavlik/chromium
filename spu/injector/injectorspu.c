/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include "cr_spu.h"
#include "cr_protocol.h"
#include "cr_opcodes.h"
#include "cr_unpack.h"
#include "injectorspu.h"

void INJECTORSPU_APIENTRY
injectorspuSwapBuffers( GLint window, GLint flags )
{
	CRMessage *msg;

	injector_spu.reading_frame = 1 ; /* SwapBuffers from the injected (OOB) stream will reset this and stop decoding */

	/* crDebug( "Injector: Entering loop to detect injected commands" ) ; */
	while ( injector_spu.reading_frame ) {
		CRMessageOpcodes *msg_opcodes;
		char *data_ptr;
		unsigned int len;

		/* Don't use GetMessage, because it pulls stuff off
		 * the network too quickly */
		len = crNetPeekMessage( injector_spu.oob_conn, &msg );
		if (len == 0)
			break;

		if (msg->header.type != CR_MESSAGE_OPCODES)
		{
			crError( "Injector %s sent me garbage (type=0x%x)", injector_spu.oob_url, msg->header.type );
		}

		msg_opcodes = (CRMessageOpcodes *) msg;
		data_ptr = (char *) msg_opcodes + sizeof( *msg_opcodes) + ((msg_opcodes->numOpcodes + 3 ) & ~0x03);
		crUnpack( data_ptr, data_ptr-1, msg_opcodes->numOpcodes, &(injector_spu.oob_dispatch) );
		crNetFree( injector_spu.oob_conn, msg );
	}
	/* crDebug( "Injector: Exiting loop to detect injected commands" ) ; */

	crNetSend( injector_spu.oob_conn, NULL, &injector_spu.info_msg, sizeof(injector_spu.info_msg) ) ;
	injector_spu.child.SwapBuffers( window, flags ) ;

	injector_spu.info_msg.frameNum++ ;
}

void INJECTORSPU_APIENTRY
injectorspuOOBSwapBuffers( GLint window, GLint flags )
{
	injector_spu.reading_frame = 0 ; /* Stop accepting stuff over the OOB channel */
}

void INJECTORSPU_APIENTRY
injectorspuChromiumParameteri( GLenum param, GLint i )
{
	/*crDebug( "injectorSPU: crParam %d = %d", (int)param, i ) ;*/
	if ( param == GL_SAVEFRAME_FRAMENUM_CR ) 
		injector_spu.info_msg.frameNum = i ;

	injector_spu.child.ChromiumParameteriCR( param, i ) ;
}

SPUNamedFunctionTable injector_table[] = {
	{ "ChromiumParameteriCR", (SPUGenericFunction) injectorspuChromiumParameteri },
	{ "SwapBuffers", (SPUGenericFunction) injectorspuSwapBuffers },
	{ NULL, NULL }
};
