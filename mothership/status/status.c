/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdio.h>
#include <string.h>

#include "cr_mothership.h"
#include "cr_error.h"

#define RESP_SIZE 4096

int main( int argc, char *argv[] )
{
	CRConnection *conn;
	
	if (argc > 1 && strncmp(argv[1], "-help", 5) == 0)
	{
		printf(
			"usage: %s [op] [node num] [attribute]\n"
			"  op = 0 or not given: print a quick summary of the nodes\n"
			"  op = 1: print a detailed summary of the nodes\n"
			"  op = 2: print the number of nodes\n"
			"  op = 3: for node 'node num' print the attribute 'attribute'\n",
			argv[0]
		);
		
		return 0;
	}
	
	conn = crMothershipConnect();
	
	if (conn)
	{
		int i;
		char resp[RESP_SIZE];
		char *begin, *end;
		
		resp[0] = '\0';
		
		for (i = 1; i < argc; ++i)
			strncat(strcat(resp, " "), argv[i], RESP_SIZE - strlen(resp) - 1);
		
		crDebug("Sending 'getstatus%s' to mothership.", resp);
		crMothershipSendString(conn, resp, "getstatus%s", resp);
		
		begin = end = resp;
		
		while (end)
		{
			end = strstr(begin, "<br>");
			
			if (end)
				*end = '\0';
			
			printf("%s\n", begin);
			begin = end + 4;
		}
		
		crMothershipDisconnect(conn);
	}
	else
	{
		crWarning("\n\nNO MOTHERSHIP RUNNING?!\n\n");
	}
	
	return 0;
}
