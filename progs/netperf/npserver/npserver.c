/*
 * net perf server
 * Brian Paul
 * 12 March 2004
 */

#include <assert.h>
#include <stdlib.h>
#include "chromium.h"
#include "cr_net.h"
#include "cr_mothership.h"
#include "cr_error.h"
#include "cr_string.h"

static int MTU = 10 * 1000;
static double TotalBytesReceived = 0.0;


/*
 * Callback which is registered with the network layer to recieve and
 * process incoming messages.
 * conn - the network connection
 * buf - points to incoming buffer
 * len - length of buffer in bytes
 */
static int
ReceiveFunc( CRConnection *conn, void *buf, unsigned int len )
{
	CRMessage *msg = (CRMessage *) buf;

	/*
	printf("In %s len=%d msg.type=0x%x\n", __FUNCTION__, len, msg->header.type);
	*/

	switch( msg->header.type )
	{
		case CR_MESSAGE_OPCODES:
			{
				CRMessageOpcodes *opcodeMsg = (CRMessageOpcodes *) msg;
				const char *buffer = (const char *) opcodeMsg;
				const char *firstCode;
				/*
				printf("Got Opcodes message.  %d codes\n", opcodeMsg->numOpcodes);
				printf("First code = %d\n", buffer[sizeof(CRMessageOpcodes)]);
				*/
				TotalBytesReceived += (double) len;
				firstCode = buffer + sizeof(CRMessageOpcodes);
				if (*firstCode == 42) {
					printf("npserver: %g bytes received\n", TotalBytesReceived);
					printf("npserver:  Exiting!\n");
					exit(0);
				}				 
				assert(*firstCode == 99);
				crNetFree( conn, buf );
				/*printf("Received %d bytes\n", len);*/
			}
			break;
		default:
			return 0; /* NOT HANDLED */
	}

	return 1; /* HANDLED */
}


/*
 * Called when connection is closed.
 */
static void
CloseFunc( unsigned int sender_id )
{
	/*printf("In %s\n", __FUNCTION__);*/
}


static void
PrintHelp(void)
{
	printf("Usage:\n");
	printf("  npserver [options] server1 [server2 ...]\n");
	printf("Options:\n");
	printf("  -p P   set protocol to P (default is tcpip)\n");
	printf("  -m N   specifies MTU size (in bytes)\n");
	printf("  -h     print this help info\n");
}


int
main(int argc, char *argv[])
{
	const char *protocol = "tcpip";
	const unsigned short port = 10000;
	CRConnection *clientConn;
	int i;

	for (i = 1; i < argc; i++) {
		if (!crStrcmp(argv[i], "-p") && i + 1 < argc) {
			protocol = argv[i];
		}
		else if (!crStrcmp(argv[i], "-m") && i + 1 < argc) {
			MTU = crStrToInt(argv[i+1]);
			i++;
		}
		else {
			PrintHelp();
			return 0;
		}
	}

	crNetInit( ReceiveFunc, CloseFunc );

	printf("npserver:  Waiting for connection from npclient...\n");
	clientConn = crNetAcceptClient( protocol, NULL, port, MTU, 0 );
	printf("npserver:  Accepting connection from %s\n", clientConn->hostname);

	while (1) {
		/*printf("Calling crNetRecv()\n");*/
		crNetRecv();
		/*printf("Return from crNetRecv()\n");*/
	}

	crNetDisconnect(clientConn);

	return 0;
};
