/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_PROTOCOL_H
#define CR_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	/* first message types is 'wGL\001', so we can immediately
		 recognize bad message types */
	CR_MESSAGE_OPCODES = 0x77474c01,
	CR_MESSAGE_WRITEBACK,
	CR_MESSAGE_READBACK,
	CR_MESSAGE_READ_PIXELS,
	CR_MESSAGE_MULTI_BODY,
	CR_MESSAGE_MULTI_TAIL,
	CR_MESSAGE_FLOW_CONTROL,
	CR_MESSAGE_OOB,
	CR_MESSAGE_NEWCLIENT,
	CR_MESSAGE_ERROR
} CRMessageType;

typedef union {
	/* pointers are usually 4 bytes, but on 64-bit machines (Alpha,
	 * SGI-n64, etc.) they are 8 bytes.  Pass network pointers around
	 * as an opaque array of bytes, since the interpretation & size of
	 * the pointer only matter to the machine which emitted the
	 * pointer (and will eventually receive the pointer back again) */
	unsigned int  ptrAlign[2];
	unsigned char ptrSize[8];
	/* hack to make this packet big */
	/* unsigned int  junk[512]; */
} CRNetworkPointer;

typedef struct {
	CRMessageType          type;
	unsigned int           conn_id;
} CRMessageHeader;

typedef struct CRMessageOpcodes {
	CRMessageHeader        header;
	unsigned int           numOpcodes;
} CRMessageOpcodes;

typedef struct CRMessageWriteback {
	CRMessageHeader        header;
	CRNetworkPointer       writeback_ptr;
} CRMessageWriteback;

typedef struct CRMessageReadback {
	CRMessageHeader        header;
	CRNetworkPointer       writeback_ptr;
	CRNetworkPointer       readback_ptr;
} CRMessageReadback;

typedef struct CRMessageMulti {
	CRMessageHeader        header;
} CRMessageMulti;

typedef struct CRMessageFlowControl {
	CRMessageHeader        header;
	unsigned int           credits;
} CRMessageFlowControl;

typedef struct CRMessageReadPixels {
	CRMessageHeader        header;
	unsigned int           bytes_per_row;
	unsigned int           stride;
	unsigned int           rows;
	int                    alignment;
    	int                    skipRows;
    	int                    skipPixels;
	int		       format;
	int		       type;
	CRNetworkPointer       pixels;
} CRMessageReadPixels;

typedef struct CRMessageNewClient {
	CRMessageHeader	       header;
} CRMessageNewClient;

typedef union {
	CRMessageHeader      header;
	CRMessageOpcodes     opcodes;
	CRMessageWriteback   writeback;
	CRMessageReadback   readback;
	CRMessageReadPixels  readPixels;
	CRMessageMulti       multi;
	CRMessageFlowControl flowControl;
	CRMessageNewClient   newclient;
} CRMessage;

void crNetworkPointerWrite( CRNetworkPointer *dst, void *src );

#ifdef __cplusplus
}
#endif

#endif /* CR_PROTOCOL_H */
