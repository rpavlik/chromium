/*
 * net perf client
 * Brian Paul
 * 12 March 2004
 */

#include <stdio.h>
#include <stdlib.h>
#include "chromium.h"
#include "cr_mem.h"
#include "cr_net.h"
#include "cr_netserver.h"
#include "cr_mothership.h"
#include "cr_error.h"
#include "cr_string.h"
#include "cr_timer.h"


#define MAX_SERVERS 100

static int MTU = 10 * 1000;
static CRTimer *MyTimer;



static int
ReceiveFunc( CRConnection *conn, void *buf, unsigned int len )
{
	return 0;
}


static void
CloseFunc( unsigned int sender_id )
{
}


/*
 * Send a Chromium opcode buffer across the given net connection.
 * size - size of opcode buffer in bytes.
 * exitFlag - if true, send 'exit' code to server.
 */
static void
SendOpcodes(CRConnection *conn, int size, int exitFlag)
{
	char *buffer, *firstCode;
	CRMessageOpcodes *msg;

	if (size <= MTU) {
		buffer = crNetAlloc(conn);

		msg = (CRMessageOpcodes *) buffer;
		msg->header.type = CR_MESSAGE_OPCODES;
		msg->numOpcodes = size;
		firstCode = buffer + sizeof(CRMessageOpcodes);
		if (exitFlag)
			 *firstCode = 42;
		else
			 *firstCode = 99;
		crNetSend(conn, NULL, buffer, MTU);

		crNetFree(conn, buffer);
	}
	else {
		/* send "huge" buffer */
		const int totalSize = size + 8 + sizeof(CRMessageOpcodes);
		unsigned int *uiptr;
		buffer = crAlloc(totalSize);
		msg = (CRMessageOpcodes *) buffer;
		msg->header.type = CR_MESSAGE_OPCODES;
		msg->numOpcodes = 1;
		uiptr = (unsigned int *) (buffer + sizeof(CRMessageOpcodes) + 4);
		uiptr[0] = size;

		firstCode = buffer + sizeof(CRMessageOpcodes);
		if (exitFlag)
			 *firstCode = 42;
		else
			 *firstCode = 99;

		crNetSend(conn, NULL, buffer, size);
		crFree(buffer);
	}
}


static void
PrintHelp(void)
{
	printf("Usage:\n");
	printf("  npclient [options] server1 [server2 ...]\n");
	printf("Options:\n");
	printf("  -a     auto mode; send a variety of buffer sizes\n");
	printf("  -b N   specifies size of buffer to send (in bytes)\n"); 
	printf("  -n N   specifies number of buffers to send\n");
	printf("  -m N   specifies MTU size (in bytes)\n");
	printf("  -h     print this help info\n");
}


int
main(int argc, char *argv[])
{
	const int port = 10000;
	CRConnection *conn[MAX_SERVERS];
	const char *servers[MAX_SERVERS];
	int numServers = 0;
	int bufferSize = 10000;
	int numBuffers = 10;
	int autoMode = 0;
	int size;
	int i, j;

	if (argc < 2) {
		PrintHelp();
		return 0;
	}

	for (i = 1; i < argc; i++) {
		if (!crStrcmp(argv[i], "-a")) {
			autoMode = 1;
		}
		else if (!crStrcmp(argv[i], "-b") && (i + 1 < argc)) {
			bufferSize = crStrToInt(argv[i + 1]);
			i++;
		}
		else if (!crStrcmp(argv[i], "-h")) {
			PrintHelp();
			return 0;
		}
		else if (!crStrcmp(argv[i], "-m") && (i + 1 < argc)) {
			MTU = crStrToInt(argv[i + 1]);
			i++;
		}
		else if (!crStrcmp(argv[i], "-n") && (i + 1 < argc)) {
			numBuffers = crStrToInt(argv[i + 1]);
			i++;
		}
		else if ((argv[i][0] != '-') && (numServers < MAX_SERVERS)) {
			servers[numServers] = argv[i];
			numServers++;
		}
	}

	if (numServers == 0) {
		printf("npclient error: need to specify at least one server\n");
		return 1;
	}

	if (autoMode)
		printf("npclient:  MTU=%d  (automatic buffer sizing)\n", MTU);
	else
		printf("npclient:  MTU=%d  bufferSize=%d  numBuffers=%d\n",
					 MTU, bufferSize, numBuffers);

	MyTimer = crTimerNewTimer();
	crNetInit( ReceiveFunc, CloseFunc );

	printf("npclient:  Connecting to servers\n");
	for (i = 0; i < numServers; i++) {
		conn[i] = crNetConnectToServer(servers[i], (unsigned short) port, MTU, 0);
		if (conn[i]) {
			printf("npclient:  Connection to %s OK.\n", servers[i]);
		}
		else {
			printf("npclient:  Connection to %s failed!\n", servers[i]);
			exit(1);
		}
	}

	printf("npclient:  Testing...\n");

	if (autoMode) {
		bufferSize = 10000;
		for (size = 0; size < 5; size++) {
			double t0, t1, dt, rate;
			double bytes;
			int buffers;

			bytes = 0.0;
			buffers = 0;
			crStartTimer(MyTimer);
			t0 = crTimerTime(MyTimer);

			do {
				for (j = 0; j < numServers; j++) {
					SendOpcodes(conn[j], bufferSize, 0);
				}
				bytes += (double) bufferSize;
				buffers++;
				t1 = crTimerTime(MyTimer);
			} while (t1 - t0 < 5.0);

			crStopTimer(MyTimer);

			dt = t1 - t0;
			rate = (double) bytes / dt / 1000000.0;
			printf("npclient:  %8.3f MB/s (%d bytes / buffer, %d buffers, %d servers)\n",
						 rate, bufferSize, buffers, numServers);
			if (rate < 0.0) {
				char *t = 0;
				*t = 0;
				CRASSERT(rate >= 0.0);
			}
			bufferSize *= 10;
		}
	}
	else {
		double t0, t1, dt, rate;
		double bytes;

		crStartTimer(MyTimer);
		t0 = crTimerTime(MyTimer);
		bytes = 0.0;
		for (i = 0; i < numBuffers; i++) {
			for (j = 0; j < numServers; j++) {
				SendOpcodes(conn[j], bufferSize, 0);
			}
			bytes += (double) bufferSize;
		}
		t1 = crTimerTime(MyTimer);

		dt = t1 - t0;
		rate = (double) bytes / dt / 1000000.0;
		printf("npclient:  %8.3f MB/s (%d bytes / buffer, %d buffers, %d servers)\n",
					 rate, bufferSize, numBuffers, numServers);
	}


	/* Send exit message to servers */
	for (j = 0; j < numServers; j++) {
		SendOpcodes(conn[j], 100, 1);
	}

	for (j = 0; j < numServers; j++) {
		crNetDisconnect(conn[j]);
	}

	printf("npclient:  done!\n");

	return 0;
}
