#ifndef CR_PROTOCOL_H
#define CR_PROTOCOL_H

typedef enum {
	/* first message types is 'wGL\001', so we can immediately
		 recognize bad message types */
	CR_MESSAGE_OPCODES = 0x77474c01,
	CR_MESSAGE_WRITEBACK,
	CR_MESSAGE_READ_PIXELS,
	CR_MESSAGE_MULTI_BODY,
	CR_MESSAGE_MULTI_TAIL,
	CR_MESSAGE_FLOW_CONTROL,
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

typedef struct CRMessageOpcodes {
	CRMessageType          type;
	unsigned int           senderId;
	unsigned int           numOpcodes;
} CRMessageOpcodes;

typedef struct CRMessageWriteback {
	CRMessageType          type;
} CRMessageWriteback;

typedef struct CRMessageMulti {
	CRMessageType          type;
} CRMessageMulti;

typedef struct CRMessageFlowControl {
	CRMessageType          type;
	unsigned int           credits;
} CRMessageFlowControl;

typedef struct CRMessageReadPixels {
	CRMessageType          type;
	unsigned int           bytes_per_row;
	unsigned int           stride;
	unsigned int           rows;
	CRNetworkPointer       pixels;
} CRMessageReadPixels;

typedef union {
	CRMessageType        type;
	CRMessageOpcodes     opcodes;
	CRMessageWriteback   writeback;
	CRMessageReadPixels  readPixels;
	CRMessageMulti       multi;
	CRMessageFlowControl flowControl;
} CRMessage;

void *crNetworkPointerRead( CRNetworkPointer *src );
void crNetworkPointerWrite( CRNetworkPointer *dst, void *src );

#endif /* CR_PROTOCOL_H */
