/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include <stdlib.h>
#include "cr_applications.h"
#include "cr_string.h"
#include "cr_error.h"

float verts[4][6] = {
	{ 0, 0, 1, 0, 1, 1 },
	{ 0, 0, -1, 0, -1, 1 },
	{ 0, 0, -1, 0, -1, -1 },
	{ 0, 0, 1, 0, 1, -1 }
};
float colors[7][3] = {
	{0,0,1},
	{0,1,0},
	{0,1,1},
	{1,0,0},
	{1,0,1},
	{1,1,0},
	{1,1,1}
};

static const int MASTER_BARRIER = 1;

int main(int argc, char *argv[])
{
	int rank = -1, size=-1;
	int i;

	if (argc < 5)
	{
		crError( "Usage: %s -rank <ID NUMBER> -size <SIZE>", argv[0] );
	}

	for (i = 1 ; i < argc ; i++)
	{
		if (!crStrcmp( argv[i], "-rank" ))
		{
			if (i == argc - 1)
			{
				crError( "-rank requires an argument" );
			}
			rank = crStrToInt( argv[i+1] );
			i++;
		}
		if (!crStrcmp( argv[i], "-size" ))
		{
			if (i == argc - 1)
			{
				crError( "-size requires an argument" );
			}
			size = crStrToInt( argv[i+1] );
			i++;
		}
	} 
	if (rank == -1)
	{
		crError( "Rank not specified" );
	}
	if (size == -1)
	{
		crError( "Size not specified" );
	}
	if (rank >= size || rank < 0)
	{
		crError( "Bogus rank: %d (size = %d)", rank, size );
	}

	crCreateContext();
	crMakeCurrent();

	/* It's OK for everyone to create this, as long as all the "size"s match */
	glBarrierCreate( MASTER_BARRIER, size );

	for (;;)
	{
		if (rank == 0)
		{
			glClear( GL_COLOR_BUFFER_BIT );
		}

		glBarrierExec( MASTER_BARRIER );

		glRotatef(1,0,0,1);
		glBegin( GL_TRIANGLES );
		glColor3fv(colors[rank%7]);
		glVertex2f(verts[rank%4][0], verts[rank%4][1]);
		glVertex2f(verts[rank%4][2], verts[rank%4][3]);
		glVertex2f(verts[rank%4][4], verts[rank%4][5]);
		glEnd();

		glBarrierExec( MASTER_BARRIER );

		if (rank == 0)
		{
			crSwapBuffers();
		}

		/* ARGH -- need to trick out the compiler this sucks. */
		if (argv[0] == NULL)
		{
			return 0;
		}
	}
}
